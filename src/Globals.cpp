#include "Globals.hpp"

//The breakpoint instruction.
//Padded to word size with 0x00s, the "dead bytes" ...
#ifdef __i386__
word breakpointInstruction[MAX_BREAKPOINT_INSTRUCTION_WORDS] = { 0xCC };
#elif __amd64__
word breakpointInstruction[MAX_BREAKPOINT_INSTRUCTION_WORDS] = { 0xCC };
#endif
