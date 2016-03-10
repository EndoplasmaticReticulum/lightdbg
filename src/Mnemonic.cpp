#include "Mnemonic.hpp"

#define PACKAGE 1
#define PACKAGE_VERSION 1
#include <bfd.h>

#include <dis-asm.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Mnemonic::Mnemonic(pword buffer, bool att)
    : opcodeLength(0), assemblyString("")
{
    //NULL the opcode:
    memset(this->opcode, 0, MAX_INSTRUCTION_BYTES);

    //Open a memory stream:
    char stringBuffer[256];
    FILE* memStream = fmemopen(stringBuffer, 256, "w");

    if (!memStream)
    {
        throw runtime_error("Failed to open memory stream.");
    }

    //Prepare disassembly info (bfd):
    struct disassemble_info info;
    init_disassemble_info(&info, memStream, (fprintf_ftype)fprintf);

    //This is the same for all architectures:
    info.buffer = (bfd_byte*)buffer;
    info.buffer_length = MAX_INSTRUCTION_BYTES;

    //Discriminate between architectures:
#ifdef __i386__
    info.arch = bfd_arch_i386;
    info.mach = att ? bfd_mach_i386_i386 : bfd_mach_i386_i386_intel_syntax;
    info.endian = BFD_ENDIAN_LITTLE;
    this->opcodeLength = att ? print_insn_i386(0, &info) : print_insn_i386_intel(0, &info);
#elif __amd64__
    info.arch = bfd_arch_i386;
    info.mach = att ? bfd_mach_x86_64 : bfd_mach_x86_64_intel_syntax;
    info.endian = BFD_ENDIAN_LITTLE;
    this->opcodeLength = att ? print_insn_i386(0, &info) : print_insn_i386_intel(0, &info);
#endif

    //Close the stream:
    fclose(memStream);

    //Assign:
    this->assemblyString = stringBuffer;

    //Build the raw opcode buffer from the length:
    memcpy(this->opcode, buffer, this->opcodeLength);
}
