#include "Breakpoint.hpp"

#include <string.h>

Breakpoint::Breakpoint(Tracee& tracee, pword address)
    : tracee(tracee), address(address), installed(false)
{
    //Null the data:
    memset(this->data, 0, MAX_BREAKPOINT_INSTRUCTION_WORDS * WORD_SIZE_BYTES);

    //Read it:
    for (int i = 0; i < MAX_BREAKPOINT_INSTRUCTION_WORDS; i++)
    {
        this->data[i] = this->tracee.peekWord(this->address + i);
    }
}


void Breakpoint::setInstalled(bool flag)
{
    //Which are the bytes used as replacement?
    pword overwritingBuffer = (flag ? breakpointInstruction : this->data);

    //Read the last word if the breakpoint instruction is not aligned (there are "dead bytes"):
    if (BREAKPOINT_INSTRUCTION_DEAD_BYTES)
    {
        word lastWord = this->tracee.peekWord(this->address + MAX_BREAKPOINT_INSTRUCTION_WORDS - 1);

        //Insert the end of the last word into the end of the overwriting buffer:
        memcpy(((pbyte)&overwritingBuffer[MAX_BREAKPOINT_INSTRUCTION_WORDS]) - BREAKPOINT_INSTRUCTION_DEAD_BYTES, ((pbyte)(&lastWord + 1)) - BREAKPOINT_INSTRUCTION_DEAD_BYTES, BREAKPOINT_INSTRUCTION_DEAD_BYTES);
    }

    //Overwrite with overwriting buffer:
    for (int i = 0; i < MAX_BREAKPOINT_INSTRUCTION_WORDS; i++)
    {
        this->tracee.pokeWord(this->address + i, overwritingBuffer[i]);
    }
}
