#include "commands/CommandRegisters.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/user.h>

#include "Globals.hpp"

vector<string> CommandRegisters::getCommandStrings()
{
    return vector<string>({ "registers", "regs", "reg", "r", "rg" });
}


void CommandRegisters::invoke(DebugLoop& loop, vector<string>& args)
{
    //Show another prompt:
    loop.setShowPrompt(true);

    if (args.size() == 0)
    {
        cout << "Command syntax: \"registers get\" to display the current content, \"registers set <reg name> <value>\" to set a register." << endl;
        return;
    }

    //Get the registers:
    struct user_regs_struct regs = loop.getTracee().getRegisters();

    //Get:
    if (args[0] == "get")
    {
        //Log:
        cout << hex;
        cout << "Current register states:" << endl;

        //Discriminate between architectures:
#ifdef __i386__
        cout << "\tEIP: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_IP << "\t\tFLG: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_FG << endl;
        cout << "\tESP: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_SP << "\t\tEBP: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_BP << endl;
        cout << "\tEAX: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_AX << "\t\tEBX: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_BX << endl;
        cout << "\tECX: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_CX << "\t\tEDX: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_DX << endl;
        cout << "\tESI: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_SI << "\t\tEDI: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_DI << endl;
#elif __amd64__
        cout << "\tRIP: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_IP << "\t\tFLG: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_FG << endl;
        cout << "\tRSP: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_SP << "\t\tRBP: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_BP << endl;
        cout << "\tRAX: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_AX << "\t\tRBX: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_BX << endl;
        cout << "\tRCX: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_CX << "\t\tRDX: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_DX << endl;
        cout << "\tRSI: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_SI << "\t\tRDI: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.REG_DI << endl;

        cout << "\tR8 : 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.r8  << "\t\tR9 : 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.r9 << endl;
        cout << "\tR10: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.r10 << "\t\tR11: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.r11 << endl;
        cout << "\tR12: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.r12 << "\t\tR13: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.r13 << endl;
        cout << "\tR14: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.r14 << "\t\tR15: 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << regs.r15 << endl;
#endif

        //Back to decimal:
        cout << dec;

        return;
    }
    //Set:
    else if (args[0] == "set")
    {
        //Get register and value:
        if (args.size() < 3)
        {
            cout << "Register and/or value parameter missing." << endl;
            return;
        }

        //Value:
        word value = 0;
        istringstream(args[2]) >> hex >> value;

        //Get the register:
        string reg = args[1];
        transform(reg.begin(), reg.end(), reg.begin(), ::tolower);

        //Change regs struct:
#ifdef __i386__
        if (reg == "eip")
        {
            regs.eip = value;
        }
        else if (reg == "flg")
        {
            regs.eflags = value;
        }
        else if (reg == "esp")
        {
            regs.esp = value;
        }
        else if (reg == "ebp")
        {
            regs.ebp = value;
        }
        else if (reg == "eax")
        {
            regs.eax = value;
        }
        else if (reg == "ebx")
        {
            regs.ebx = value;
        }
        else if (reg == "ecx")
        {
            regs.ecx = value;
        }
        else if (reg == "edx")
        {
            regs.edx = value;
        }
        else if (reg == "esi")
        {
            regs.esi = value;
        }
        else if (reg == "edi")
        {
            regs.edi = value;
        }
#elif __amd64__
        if (reg == "rip")
        {
            regs.rip = value;
        }
        else if (reg == "flg")
        {
            regs.eflags = value;
        }
        else if (reg == "rsp")
        {
            regs.rsp = value;
        }
        else if (reg == "rbp")
        {
            regs.rbp = value;
        }
        else if (reg == "rax")
        {
            regs.rax = value;
        }
        else if (reg == "rbx")
        {
            regs.rbx = value;
        }
        else if (reg == "rcx")
        {
            regs.rcx = value;
        }
        else if (reg == "rdx")
        {
            regs.rdx = value;
        }
        else if (reg == "rsi")
        {
            regs.rsi = value;
        }
        else if (reg == "rdi")
        {
            regs.rdi = value;
        }
        else if (reg == "r8")
        {
            regs.r8 = value;
        }
        else if (reg == "r9")
        {
            regs.r9 = value;
        }
        else if (reg == "r10")
        {
            regs.r10 = value;
        }
        else if (reg == "r11")
        {
            regs.r11 = value;
        }
        else if (reg == "r12")
        {
            regs.r12 = value;
        }
        else if (reg == "r13")
        {
            regs.r13 = value;
        }
        else if (reg == "r14")
        {
            regs.r14 = value;
        }
        else if (reg== "r15")
        {
            regs.r15 = value;
        }
#endif
        else
        {
            cout << "Unknown register: \"" << reg << "\"." << endl;
            return;
        }

        //Write it back:
        try
        {
            loop.getTracee().setRegisters(regs);
        }
        catch (runtime_error err)
        {
            cout << err.what() << endl;
            return;
        }

        //Log:
        cout << "Wrote value \0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << value << " to " << args[1] << "." << endl;

        //Update the regs:
        try
        {
            loop.getTracee().updateRegisters();
        }
        catch (runtime_error err)
        {
            cout << err.what() << endl;
            return;
        }

        return;
    }
    else
    {
        cout << "Unknown parameter: \"" << args[0] << "\"." << endl;
        return;
    }
}
