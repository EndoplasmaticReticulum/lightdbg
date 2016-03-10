#ifndef BREAKPOINT_H
#define BREAKPOINT_H

#include "Tracee.hpp"
#include "Globals.hpp"

using namespace std;

class Breakpoint
{
    //Members:
private:

    //A reference to the tracee.
    Tracee& tracee;

    //The address where the breakpoint is placed:
    pword address;

    //The original data:
    word data[MAX_BREAKPOINT_INSTRUCTION_WORDS];

    //Is the breakpoint installed?
    //That shows if it is present in memory!
    bool installed;

    //Methods:
public:

    //Get the address:
    inline pword getAddress() const { return this->address; }

    //Constructor:
    Breakpoint(Tracee& tracee, pword address);

    //Get/Set the installed state:
    inline bool isInstalled() const { return this->installed; }
    void setInstalled(bool flag);
};

#endif // BREAKPOINT_H
