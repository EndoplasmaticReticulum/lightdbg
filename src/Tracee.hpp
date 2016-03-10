#ifndef TRACEE_H
#define TRACEE_H

#include <iostream>
#include <string>
#include <sys/user.h>
#include <unistd.h>
#include <vector>

#include "Globals.hpp"
#include "Mnemonic.hpp"
#include "SymbolTable.hpp"

#define offset_of(tp, member) (((char*)&((tp*)0)->member) - (char*)0)

using namespace std;

class Tracee
{
    //Members:
private:

    //The PID of the debugged process:
    pid_t pid;

    //The creation mode:
    string creationMode;

    //The name of the binary:
    string name;

    //The path to the binary (including the file name at the end):
    string path;

    //The symbol table of the binary:
    SymbolTable* symbolTable;

    //The current registers:
    struct user_regs_struct registers;

    //Methods:
private:

    //Assign binary name and absolute path from a given (maybe relative) path.
    //This throws an exception if the path is invalid.
    void assignNameAndPath(string binaryPath);

    //Initialization methods called by the constructor:
    void run(vector<string>& args);
    void attach(vector<string>& args);

public:

    //Get the PID:
    inline pid_t getPID() const { return this->pid; }

    //Get the symbol table:
    inline SymbolTable* getSymbolTable() const { return this->symbolTable; }

    //Get the registers:
    inline const user_regs_struct& getRegisters() const { return this->registers; }

    //Constructor:
    Tracee(vector<string>& args);

    //Destructor:
    virtual ~Tracee();

    //Printing this with ostreams:
    friend ostream& operator<<(ostream& os, Tracee& dt);

    //Initialize (must be called in first wait state):
    void initialize();

    //Update all the registers:
    void updateRegisters();

    //Change all the registers:
    void setRegisters(struct user_regs_struct& registers);

    //Continue the execution via PTRACE_CONT and deliver the signal (if signal != 0):
    void continueProcess(int signal);

    //Detach from the debugged process via PTRACE_DETACH and deliver the signal (if signal != 0):
    void detachFromProcess(int signal);

    //Exit the debugged process:
    void killProcess();

    //Do a single step (this will fire a SIGTRAP):
    void performStep();

    //Peek a word:
    word peekWord(pword address);

    //Peek multiple bytes and write them to a ptr (count is in bytes):
    void peekToPtr(pword address, int count, byte* ptr);

    //Poke a word:
    void pokeWord(pword address, word content);

    //Disassemble a single instruction at a given address:
    Mnemonic disassemble(pword address, bool att);

    //Disassemble some instructions starting at a given address.
    vector<Mnemonic> disassemble(pword address, bool att, int count, int& bytesCount);

    //Disassemble at the current (ip) address:
    Mnemonic disassemble(bool att);
    vector<Mnemonic> disassemble(bool att, int count, int& bytesCount);
};


#endif // TRACEE_H
