#ifndef TYPES_HPP
#define TYPES_HPP

#include <stdint.h>

//An unused var:
#define UNUSED(expr) do { (void)(expr); } while (0)

//A byte:
typedef uint8_t byte, *pbyte;

//A machine word (holds addresses, values of registers etc.):
typedef unsigned long word, *pword;

//The bit resp. byte size of the arch (4 / 8 bytes resp. 32 / 64 bits):
#define WORD_SIZE_BYTES ((int)(sizeof(word)))
#define WORD_SIZE_BITS ((int)(WORD_SIZE_BYTES * 8))

//The max number of bytes per instruction:
#ifdef __i386__
#define MAX_INSTRUCTION_BYTES ((int)15)
#elif __amd64__
#define MAX_INSTRUCTION_BYTES ((int)15)
#endif
#define MAX_INSTRUCTION_BITS ((int)(MAX_INSTRUCTION_BYTES * 8))

//How many words are needed to cover one instruction?
#define MAX_INSTRUCTION_WORDS ((int)(((MAX_INSTRUCTION_BYTES - 1) / WORD_SIZE_BYTES) + 1))

//The breakpoint instruction:
#ifdef __i386__
#define BREAKPOINT_INSTRUCTION_BYTES 1
#elif __amd64__
#define BREAKPOINT_INSTRUCTION_BYTES 1
#endif
#define MAX_BREAKPOINT_INSTRUCTION_WORDS ((int)(((BREAKPOINT_INSTRUCTION_BYTES - 1) / WORD_SIZE_BYTES) + 1))
#define BREAKPOINT_INSTRUCTION_DEAD_BYTES ((MAX_BREAKPOINT_INSTRUCTION_WORDS * WORD_SIZE_BYTES) - BREAKPOINT_INSTRUCTION_BYTES)
extern word breakpointInstruction[MAX_BREAKPOINT_INSTRUCTION_WORDS];

//Special registers:
#ifdef __i386__
#define REG_IP eip
#define REG_SP esp
#define REG_BP ebp
#define REG_AX eax
#define REG_ORIG_AX orig_eax
#define REG_BX ebx
#define REG_CX ecx
#define REG_DX edx
#define REG_SI esi
#define REG_DI edi
#define REG_FG eflags
#elif __amd64__
#define REG_IP rip
#define REG_SP rsp
#define REG_BP rbp
#define REG_AX rax
#define REG_ORIG_AX orig_rax
#define REG_BX rbx
#define REG_CX rcx
#define REG_DX rdx
#define REG_SI rsi
#define REG_DI rdi
#define REG_FG eflags
#endif

//Some system calls:
#ifdef __i386__
#define SYSCALL_PTRACE 26
#define SYSCALL_TIME 13
#elif __amd64__
#define SYSCALL_PTRACE 101
#define SYSCALL_TIME 201
#endif

#endif // TYPES_HPP
