#include "Tracee.hpp"

#include <algorithm>
#include <elf.h>
#include <libgen.h>
#include <link.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <unistd.h>

void Tracee::assignNameAndPath(string binaryPath)
{
    //Get the absolute binary path:
    char* absolutePath;

    if (!(absolutePath = realpath(binaryPath.c_str(), NULL)))
    {
        throw runtime_error("Failed to obtain absolute file path.");
    }

    //Assign the path:
    this->path = absolutePath;

    //Get the file name from the path:
    this->name = basename(absolutePath);

    //Free the memory:
    delete absolutePath;
}


void Tracee::run(vector<string>& args)
{
    //Read the arguments:
    if (args.size() < 1)
    {
        throw runtime_error("No path for running provided.");
    }

    //Try to assign name and path:
    assignNameAndPath(args[0]);
    cout << "Running \"" << this->path << "\" ..." << endl;

    //Fork the process:
    this->pid = fork();

    //Error:
    if (this->pid == -1)
    {
        throw runtime_error("Failed to fork debugger process.");
    }
    //Child:
    else if (this->pid == 0)
    {
        //Mark this process as being traced:
        if (ptrace(PTRACE_TRACEME, 0, NULL, 0))
        {
            //ptrace failed. We kill the child process.
            cerr << "Failed to mark the child process as traceable." << endl;
            exit(-1);
        }

        //Replace the child process image with the destination process.
        //Pass the binary name as first parameter and the remaining args as other parameters.
        args[0] = this->name;
        vector<char*> cArgs;

        for (vector<string>::iterator it = args.begin(); it != args.end(); ++it)
        {
            //This memory will be released by the system if exec*() succeeds:
            char* newArg = new char[it->size() + 1];

            if (!newArg)
            {
                cerr << "Failed to allocate argument string." << endl;
                exit(-1);
            }

            strcpy(newArg, it->c_str());
            cArgs.push_back(newArg);
        }

        //exec*() wants a NULL at the end:
        cArgs.push_back(NULL);
        execvp(this->path.c_str(), &cArgs[0]);

        //exec*() only returns on error:
        switch (errno)
        {
        case EPERM:
        case EACCES: cerr << "No permission to execute the selected binary." << endl; break;
        case EIO: cerr << "I/O error while executing the selected binary." << endl; break;
        case ENOENT: cerr << "The selected binary was not found." << endl; break;
        case ENOEXEC: cerr << "The selected binary is malformed." << endl; break;

        default: cerr << "Failed to execute the selected binary (execvp error code: " << strerror(errno) << ")." << endl;
        }

        //Release the allocated arguments (without the trailing NULL):
        for (vector<char*>::iterator it = cArgs.begin(); it != cArgs.end() - 1; ++it)
        {
            delete *it;
        }

        //Kill the child process:
        exit(-1);
    }
}

void Tracee::attach(vector<string>& args)
{
    //Read the arguments:
    if (args.size() < 1)
    {
        throw runtime_error("No PID for attaching provided.");
    }

    //PID:
    try
    {
        this->pid = stoi(args[0]);
    }
    catch (...)
    {
        throw runtime_error("PID must be a number.");
    }

    cout << "Attaching to " << this->pid << " ..." << endl;

    //Retrieve the binary path from the PID.
    //We are on linux, so we can follow /proc/<pid>/exe:
    char path[PATH_MAX];
    memset(path, 0, PATH_MAX);

    if ((readlink(("/proc/" + to_string(this->pid) + "/exe").c_str(), path, PATH_MAX)) < 0)
    {
        switch (errno)
        {
        case EPERM:
        case EACCES: throw runtime_error("No permission to retrieve the binary path."); break;
        case EIO: throw runtime_error("I/O error while retrieving the binary path."); break;
        case ENOENT: throw runtime_error("The given PID does not correspond to a process."); break;

        default: throw runtime_error(string("Failed to retrieve the binary path (readlink error code: ") + strerror(errno) + ").");
        }
    }

    //Try to assign name and path:
    assignNameAndPath(path);

    //Attach to the given PID:
    if (ptrace(PTRACE_ATTACH, this->pid, NULL, 0))
    {
        switch (errno)
        {
        case EPERM:
        case EACCES: throw runtime_error("No permission to attach to the given PID."); break;
        case EIO: throw runtime_error("I/O error while attaching to the given PID."); break;
        case ENOENT: throw runtime_error("The given PID does not correspond to a process."); break;

        default: throw runtime_error(string("Failed to attach to the given process (ptrace error code: ") + strerror(errno) + ").");
        }
    }
}


Tracee::Tracee(vector<string>& args)
    : pid(-1), creationMode(""), name(""), path(""), symbolTable(NULL)
{
    //Null the structs:
    memset(&this->registers, 0, sizeof(this->registers));

    //There must be at least one argument to specify the method:
    if (args.size() < 1)
    {
        throw runtime_error("No method argument for tracee creation specified.");
    }

    this->creationMode = args[0];
    transform(this->creationMode.begin(), this->creationMode.end(), this->creationMode.begin(), ::tolower);
    args.erase(args.begin() + 0);

    //Run:
    if (this->creationMode == "run")
    {
        run(args);
    }
    //Attach:
    else if (this->creationMode == "attach")
    {
        attach(args);
    }
    else
    {
        throw runtime_error("Unknown tracee creation method: " + this->creationMode + ".");
    }

    //Load the symbol table:
    cout << "Loading symbol table ..." << endl;
    this->symbolTable = new SymbolTable(this->path);
}


Tracee::~Tracee()
{
    //Free the symbol table:
    if (this->symbolTable)
    {
        delete this->symbolTable;
        this->symbolTable = NULL;
    }
}


ostream& operator<<(ostream& os, Tracee& tracee)
{
    return os << "File path: \"" << tracee.path << "\", PID: " << tracee.pid << ", Creation mode: " << tracee.creationMode << ", Symbol table: " << tracee.symbolTable->getMap().size() << " symbols loaded.";
}


void Tracee::initialize()
{
    //Set options:
    if (ptrace(PTRACE_SETOPTIONS, this->pid, NULL, PTRACE_O_TRACESYSGOOD))
    {
        switch (errno)
        {
        case EPERM:
        case EACCES: throw runtime_error("No permission to set debug options."); break;
        case EIO: throw runtime_error("I/O error while setting debug options."); break;
        case ENOENT: throw runtime_error("The given PID does not correspond to a process."); break;

        default: throw runtime_error(string("Failed to set debug options (ptrace error code: ") + strerror(errno) + ").");
        }
    }
}


void Tracee::updateRegisters()
{
    //Read all the registers:
    if (ptrace(PTRACE_GETREGS, this->pid, NULL, &this->registers))
    {
        throw runtime_error(string("Failed to execute PTRACE_GETREGS (ptrace error code: ") + strerror(errno) + ").");
    }
}


void Tracee::setRegisters(struct user_regs_struct& registers)
{
    //Write all the registers:
    if (ptrace(PTRACE_SETREGS, this->pid, NULL, &registers))
    {
        throw runtime_error(string("Failed to execute PTRACE_SETREGS (ptrace error code: ") + strerror(errno) + ").");
    }
}


void Tracee::continueProcess(int signal)
{
    if (ptrace(PTRACE_SYSCALL, this->pid, NULL, signal))
    {
        throw runtime_error(string("Failed to execute PTRACE_SYSCALL (ptrace error code: ") + strerror(errno) + ").");
    }
}


void Tracee::detachFromProcess(int signal)
{
    if (ptrace(PTRACE_DETACH, this->pid, NULL, signal))
    {
        throw runtime_error(string("Failed to execute PTRACE_DETACH (ptrace error code: ") + strerror(errno) + ").");
    }
}


void Tracee::killProcess()
{
    if (kill(this->pid, SIGKILL))
    {
        throw runtime_error(string("Failed to kill debugged process (kill error code: ") + strerror(errno) + ").");
    }
}


void Tracee::performStep()
{
    if (ptrace(PTRACE_SINGLESTEP, this->pid, NULL, 0))
    {
        throw runtime_error(string("Failed to execute PTRACE_SINGLESTEP (ptrace error code: ") + strerror(errno) + ").");
    }
}


word Tracee::peekWord(pword address)
{
    //Execute the ptrace:
    word result = ptrace(PTRACE_PEEKDATA, this->pid, address, 0);

    if (errno)
    {
        throw runtime_error(string("Failed to execute PTRACE_PEEKDATA (ptrace error code: ") + strerror(errno) + ").");
    }

    return result;
}


void Tracee::peekToPtr(pword address, int count, byte* ptr)
{
    //Peek the aligned part:
    for (int i = 0; i < (count / WORD_SIZE_BYTES); i++)
    {
        //Peek the word and increment the pointer and address:
        *((pword)ptr) = peekWord(address++);
        ptr += WORD_SIZE_BYTES;
    }

    //Peek the dead bytes:
    int deadBytes = count % WORD_SIZE_BYTES;

    if (deadBytes > 0)
    {
        //Peek the word:
        word lastWord = peekWord(address);

        //Write the important part:
        for (int i = 0; i < deadBytes; i++)
        {
            ptr[i] = lastWord & ((word)0xFF << (8 * i));
        }
    }
}


void Tracee::pokeWord(pword address, word content)
{
    //Execute the ptrace:
    if (ptrace(PTRACE_POKEDATA, this->pid, address, content))
    {
        throw runtime_error(string("Failed to execute PTRACE_POKEDATA (ptrace error code: ") + strerror(errno) + ").");
    }
}


Mnemonic Tracee::disassemble(pword address, bool att)
{
    //Prepare a buffer:
    word buffer[MAX_INSTRUCTION_WORDS];

    //Read the words:
    for (int i = 0; i < MAX_INSTRUCTION_WORDS; i++)
    {
        buffer[i] = peekWord(address + i);
    }

    //Create and return mnemonic:
    return Mnemonic(buffer, att);
}


vector<Mnemonic> Tracee::disassemble(pword address, bool att, int count, int& bytesCount)
{
    //Collect mnemonics:
    vector<Mnemonic> mnemonics;
    bytesCount = 0;

    for (int i = 0; i < count; i++)
    {
        //Disassemble one instruction:
        Mnemonic mnemonic = disassemble(address, att);
        mnemonics.push_back(mnemonic);

        //Increment address and total count:
        address = (pword)(((pbyte)address) + mnemonic.getOpcodeLength());
        bytesCount += mnemonic.getOpcodeLength();
    }

    //Return the mnemonics:
    return mnemonics;
}


Mnemonic Tracee::disassemble(bool att)
{
    return disassemble((pword)this->registers.rip, att);
}


vector<Mnemonic> Tracee::disassemble(bool att, int count, int& bytesCount)
{
    return disassemble((pword)this->registers.rip, att, count, bytesCount);
}
