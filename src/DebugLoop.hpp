#ifndef DEBUGLOOP_H
#define DEBUGLOOP_H

#include <map>
#include <string>
#include <vector>

#include "Tracee.hpp"
#include "Breakpoint.hpp"
#include "Tracer.hpp"

#include "commands/Command.hpp"

#define DEFAULT_PROMPT "(ldb)"

using namespace std;

class Command;

//The result of the debug loop waiting for a signal:
enum SignalResult
{
    SIGNAL_RESULT_EXITED,
    SIGNAL_RESULT_STOPPED,
    SIGNAL_RESULT_CONTINUED
};

class DebugLoop
{
    //Members:
private:

    //A reference to the tracee.
    //The caller must ensure the tracee lives at least as long as the debug loop!
    Tracee& tracee;

    //Did we already initialize the tracee?
    bool initialized;

    //Syscall handling:
    bool syscallActive;
    word syscallNumber;
    vector<word> syscallArgs;

    //Should we keep looping?
    bool keepLooping;

    //Should we show a prompt or continue the loop?
    bool showPrompt;

    //The last signal the debugged process was stopped with:
    int stopSignal;

    //Obfuscation handling:
    bool obfuscateTraceMe = false;
    bool obfuscateTraceMeTriggered = false;

    //The breakpoints:
    bool breakpointsInstalled;
    map<pword, Breakpoint*> breakpoints;

    //The runtime tracer:
    Tracer tracer;

    //Our commands:
    map<string, Command*> commands;

    //Methods:
private:

    //Wait for the child process to come back with a signal.
    //The 'result' parameter varies depending on the return value:
    //
    //SIGNAL_RESULT_EXITED: Exit code of the debugged process
    //SIGNAL_RESULT_STOPPED: Signal that made the debugged process stop
    //SIGNAL_RESULT_CONTINUED: Unused
    SignalResult waitForSignal(int& result);

    //Initialize:
    void performInitialization();

    //Trace the current mnemonic:
    void performTrace();

    //Handle SIGTRAP | 0x80:
    void performSyscall();

    //Check if we are at a breakpoint.
    //Restore the instruction if necessary:
    bool performBreakpoint(pword address);

    //Handle the results of waitForSignal():
    void handleExit(int exitCode);
    void handleStop(int signal);
    void handleContinue();

    //Handle a syscall:
    void handleSyscall(word result);

    //Show a prompt to the user to question a command:
    void prompt();

public:

    //Get the tracee:
    inline Tracee& getTracee() { return this->tracee; }

    //Get the breakpoints:
    inline const map<pword, Breakpoint*>& getBreakpoints() const { return this->breakpoints; }

    //Get the tracer:
    inline Tracer& getTracer() { return this->tracer; }

    //Get the last stop signal:
    inline int getStopSignal() const { return this->stopSignal; }

    //Set our intern runtime flags:
    inline void setKeepLooping(bool flag) { this->keepLooping = flag; }
    inline void setShowPrompt(bool flag) { this->showPrompt = flag; }

    //Get/Set the obfuscation flags:
    inline bool getObfuscateTraceMe() const { return this->obfuscateTraceMe; }
    inline void setObfuscateTraceMe(bool flag) { this->obfuscateTraceMe = flag; }

    //Constructor:
    DebugLoop(Tracee& tracee);

    //Desctructor:
    virtual ~DebugLoop();

    //Loop until the process is killed / we detach / etc.:
    void loop();

    //Add/Remove breakpoints:
    void addBreakpoint(pword address);
    void removeBreakpoint(pword address);

    //Install/Deinstall breakpoints:
    void setBreakpointsInstalled(bool flag);
};

#endif // DEBUGLOOP_H
