#include "DebugLoop.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <vector>

#include "Mnemonic.hpp"

#include "commands/CommandBreakpoint.hpp"
#include "commands/CommandContinue.hpp"
#include "commands/CommandDetach.hpp"
#include "commands/CommandDisassemble.hpp"
#include "commands/CommandExit.hpp"
#include "commands/CommandObfuscate.hpp"
#include "commands/CommandRegisters.hpp"
#include "commands/CommandStack.hpp"
#include "commands/CommandStep.hpp"
#include "commands/CommandTracer.hpp"

SignalResult DebugLoop::waitForSignal(int& result)
{
    int status;

    while (true)
    {
        //Wait for the next signal in the child process:
        if (wait(&status) == -1)
        {
            throw runtime_error(string("Failed to wait for signal (wait error code: ") + strerror(errno) + ").");
        }

        //The child has exited:
        if (WIFEXITED(status))
        {
            result = WEXITSTATUS(status);
            return SIGNAL_RESULT_EXITED;
        }

        //The child has been terminated by a signal:
        if (WIFSIGNALED(status))
        {
            result = WTERMSIG(status);
            return SIGNAL_RESULT_EXITED;
        }

        //The child has been stopped:
        if (WIFSTOPPED(status))
        {
            result = WSTOPSIG(status);
            return SIGNAL_RESULT_STOPPED;
        }

        //The child has been continued.
        //We probably don't receive this because every signal sent to the child process is delegated to us.
        //That means only we are able to continue it at all - and that won't generate another notification to us.
        if (WIFCONTINUED(status))
        {
            result = -1;
            return SIGNAL_RESULT_CONTINUED;
        }
    }
}


void DebugLoop::performInitialization()
{
    //Set debug options:
    cout << "Initializing ..." << endl;

    this->tracee.initialize();
    this->initialized = true;
}


void DebugLoop::performTrace()
{
    //Trace the mnemonic:
    Mnemonic mnemonic = this->tracee.disassemble(false);
    this->tracer.trace(mnemonic, this->tracee.getRegisters());
}


void DebugLoop::performSyscall()
{
    //Is this the first call?
    if (!this->syscallActive)
    {
        //Syscall is active now. Save the number and arguments:
        this->syscallActive = true;
        this->syscallNumber = this->tracee.getRegisters().REG_ORIG_AX;

#ifdef __i386__
        this->syscallArgs.push_back(this->tracee.getRegisters().REG_BX);
        this->syscallArgs.push_back(this->tracee.getRegisters().REG_CX);
        this->syscallArgs.push_back(this->tracee.getRegisters().REG_DX);
        this->syscallArgs.push_back(this->tracee.getRegisters().REG_SI);
        this->syscallArgs.push_back(this->tracee.getRegisters().REG_DI);
        this->syscallArgs.push_back(this->tracee.getRegisters().REG_BP);
#elif __amd64__
        this->syscallArgs.push_back(this->tracee.getRegisters().REG_DI);
        this->syscallArgs.push_back(this->tracee.getRegisters().REG_SI);
        this->syscallArgs.push_back(this->tracee.getRegisters().REG_DX);

        //These are AMD64-specific:
        this->syscallArgs.push_back(this->tracee.getRegisters().r10);
        this->syscallArgs.push_back(this->tracee.getRegisters().r8);
        this->syscallArgs.push_back(this->tracee.getRegisters().r9);
#endif
    }
    //Is this already the second call?
    else
    {
        //Handle:
        handleSyscall(this->tracee.getRegisters().REG_AX);

        //Reset syscall:
        this->syscallActive = false;
        this->syscallNumber = 0;
        this->syscallArgs.clear();
    }
}


bool DebugLoop::performBreakpoint(pword address)
{
    try
    {
        //Remove the breakpoint (for now, we use one-time-breaks):
        removeBreakpoint(address);

        //Reset IP:
        struct user_regs_struct registers = this->tracee.getRegisters();
        registers.REG_IP = (word)address;
        this->tracee.setRegisters(registers);

        //Update:
        this->tracee.updateRegisters();
    }
    catch (...)
    {
        //Ignore for now ...
    }

    //Print info:
    cout << "This is a breakpoint." << endl;
    return true;
}


void DebugLoop::handleExit(int exitCode)
{
    cout << "Debugged process exited with code " << exitCode << "." << endl;

    //Stop the looping:
    setKeepLooping(false);
}


void DebugLoop::handleStop(int signal)
{
    //Initialize (only once):
    if (!this->initialized)
    {
        performInitialization();
    }

    //Save the signal:
    this->stopSignal = signal;

    //Read the current registers:
    this->tracee.updateRegisters();

    //Deinstall the breakpoints:
    bool breakpointsWereInstalled = this->breakpointsInstalled;
    setBreakpointsInstalled(false);

    //Are we tracing and is this a SIGTRAP?
    if ((this->tracer.getTracingActive()) && (this->tracer.getTracingMode() != TRACING_MODE_NONE) && (this->stopSignal == SIGTRAP))
    {
        //Perform the tracing itself:
        performTrace();

        //Do the next single step:
        this->tracee.performStep();

        return;
    }

    //Is this a syscall stop while continuing?
    if (this->stopSignal == (SIGTRAP | 0x80))
    {
        //Perform the syscall itself:
        performSyscall();

        //Re-Install the breakpoints:
        setBreakpointsInstalled(true);

        //Continue:
        this->tracee.continueProcess(0);

        return;
    }

    //Disable tracing when a signal appears:
    this->tracer.setTracingActive(false);

    //Show the signal that stopped us:
    cout << "Debugged process has received signal: " << strsignal(signal) << "." << endl;

    //Check if this is directly behind a breakpoint address, but only if the breakpoints were installed (-> we came to this point by continue) and this is SIGTRAP:
    pword breakpointAddress = (pword)(this->tracee.getRegisters().REG_IP - BREAKPOINT_INSTRUCTION_BYTES);

    if (breakpointsWereInstalled && (this->breakpoints.find(breakpointAddress) != this->breakpoints.end()) && (this->stopSignal == SIGTRAP))
    {
        performBreakpoint(breakpointAddress);
    }

    //Disassemble one instruction:
    cout << "\t<0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << hex << this->tracee.getRegisters().REG_IP << dec << ">\t" << this->tracee.disassemble(false).getAssemblyString() << endl;

    //Show a prompt on stop:
    setShowPrompt(true);

    do
    {
        //Prompt the user:
        prompt();
    } while (this->showPrompt);
}


void DebugLoop::handleContinue()
{

}


void DebugLoop::handleSyscall(word result)
{
    UNUSED(result);

    //Hook ptrace:
    if (this->syscallNumber == SYSCALL_PTRACE)
    {
        //Obfuscate a PTRACE_TRACEME:
        if (this->obfuscateTraceMe && (this->syscallArgs[0] == PTRACE_TRACEME) && !this->obfuscateTraceMeTriggered)
        {
            //Set the result to 0 (success) for the first call:
            struct user_regs_struct registers = this->tracee.getRegisters();
            registers.REG_AX = 0;
            this->tracee.setRegisters(registers);

            //Triggered:
            this->obfuscateTraceMeTriggered = true;
            cout << "<Obfuscation> traceme" << endl;
        }
    }
    else if (this->obfuscateTime && (this->syscallNumber == SYSCALL_TIME))
    {
        //Set the result to 01/01/2000:
        word newVal = 946684800;

        struct user_regs_struct registers = this->tracee.getRegisters();
        registers.REG_AX = newVal;
        this->tracee.setRegisters(registers);

        //Maybe also fix pointer:
        pword timePtr = (pword)(syscallArgs[0]);

        if (timePtr != NULL)
        {
            this->tracee.pokeWord(timePtr, newVal);
        }

        cout << "<Obfuscation> time" << endl;
    }
}


void DebugLoop::prompt()
{
    //Show the prompt:
    cout << DEFAULT_PROMPT << " ";

    //Read a line from the user:
    string commandStr;

    if (!getline(cin, commandStr))
    {
        //Handle this as detatching:
        setShowPrompt(false);
        setKeepLooping(false);

        this->tracee.detachFromProcess(0);

        return;
    }

    //Split into args:
    istringstream iss(commandStr);
    vector<string> args{ istream_iterator<string>{ iss }, istream_iterator<string>{} };

    if (!args.size())
    {
        //Don't do anything.
        return;
    }

    //Separate command and args:
    commandStr = args[0];
    transform(commandStr.begin(), commandStr.end(), commandStr.begin(), ::tolower);
    args.erase(args.begin() + 0);

    //Handle:
    if (this->commands.find(commandStr) != this->commands.end())
    {
        this->commands[commandStr]->invoke(*this, args);
    }
    else
    {
        cout << "Invalid command." << endl;
        setShowPrompt(true);
    }
}


DebugLoop::DebugLoop(Tracee& tracee)
    : tracee(tracee), initialized(false), syscallActive(false), syscallNumber(0), keepLooping(false), showPrompt(false), stopSignal(0), breakpointsInstalled(false)
{
    //Load all our commands:
    vector<Command*> commands = vector<Command*>({ new CommandBreakpoint(), new CommandContinue(), new CommandDetach(), new CommandDisassemble(), new CommandExit(), new CommandObfuscate(), new CommandRegisters(), new CommandStack(), new CommandStep(), new CommandTracer() });

    for (vector<Command*>::iterator it = commands.begin(); it != commands.end(); ++it)
    {
        //Get the command strings:
        vector<string> commandStrings = (*it)->getCommandStrings();

        //Add them:
        for (vector<string>::iterator it2 = commandStrings.begin(); it2 != commandStrings.end(); ++it2)
        {
            this->commands[*it2] = *it;
        }
    }
}


DebugLoop::~DebugLoop()
{
    //Free the breakpoints:
    for (map<pword, Breakpoint*>::iterator it = this->breakpoints.begin(); it != this->breakpoints.end(); ++it)
    {
        delete it->second;
    }

    this->breakpoints.clear();

    //Free (only the distinct) commands:
    set<Command*> ptrs;

    for (map<string, Command*>::iterator it = this->commands.begin(); it != this->commands.end(); ++it)
    {
        if (ptrs.find(it->second) == ptrs.end())
        {
            ptrs.insert(it->second);
            delete it->second;
        }
    }

    this->commands.clear();
}


void DebugLoop::loop()
{
    //Start with looping:
    setKeepLooping(true);

    do
    {
        //Wait for stop or exit:
        int result = 0;

        switch (waitForSignal(result))
        {
        case SIGNAL_RESULT_EXITED: handleExit(result); break;
        case SIGNAL_RESULT_STOPPED: handleStop(result); break;
        case SIGNAL_RESULT_CONTINUED: handleContinue(); break;
        }
    } while (this->keepLooping);

    cout << "The debug loop has been left." << endl;
}


void DebugLoop::addBreakpoint(pword address)
{
    //Go through all breakpoints and check if there is one colliding with the new one:
    for (map<pword, Breakpoint*>::iterator it = this->breakpoints.begin(); it != this->breakpoints.end(); ++it)
    {
        //Check the distance of the addresses:
        if (((word)address - (word)it->first) < BREAKPOINT_INSTRUCTION_BYTES)
        {
            throw runtime_error("The new breakpoint would collide with another one.");
        }
    }

    //Allocate the new breakpoint:
    Breakpoint* breakpoint = new Breakpoint(this->tracee, address);

    if (!breakpoint)
    {
        throw runtime_error("Failed to allocate breakpoint.");
    }

    //Add it to the map:
    this->breakpoints[address] = breakpoint;
}


void DebugLoop::removeBreakpoint(pword address)
{
    //Check if it exists:
    if (this->breakpoints.find(address) == this->breakpoints.end())
    {
        throw runtime_error("There is no breakpoint at that address.");
    }

    //Free and remove it:
    delete this->breakpoints[address];
    this->breakpoints.erase(address);
}


void DebugLoop::setBreakpointsInstalled(bool flag)
{
    //Iterate:
    for (map<pword, Breakpoint*>::iterator it = this->breakpoints.begin(); it != this->breakpoints.end(); ++it)
    {
        try
        {
            it->second->setInstalled(flag);
        }
        catch (...)
        {
            //Let's ignore that for now ...
        }
    }

    //Mark:
    this->breakpointsInstalled = flag;
}
