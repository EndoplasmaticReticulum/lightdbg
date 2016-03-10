#ifndef MNEMONIC_H
#define MNEMONIC_H

#include "Globals.hpp"

#include <string>

using namespace std;

class Mnemonic
{
    //Members:
private:

    //The length of the opcode:
    int opcodeLength;

    //The opcode itself:
    byte opcode[MAX_INSTRUCTION_BYTES];

    //The string containing the corresponding assembly code:
    string assemblyString;

    //Methods:
public:

    //Access the members:
    inline int getOpcodeLength() const { return this->opcodeLength; }
    inline byte* getOpcode() { return this->opcode; }
    inline string getAssemblyString() const { return this->assemblyString; }

    //Constructor:
    Mnemonic(pword buffer, bool att);
};

#endif // MNEMONIC_H
