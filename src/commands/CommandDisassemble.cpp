#include "CommandDisassemble.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "Mnemonic.hpp"

vector<string> CommandDisassemble::getCommandStrings()
{
    return vector<string>({ "disassemble", "d", "ds", "disas", "disasm" });
}


void CommandDisassemble::invoke(DebugLoop& loop, vector<string>& args)
{
    //Always keep prompting:
    loop.setShowPrompt(true);

    //Show possible args:
    if (args.size() == 0)
    {
        cout << "The first parameter must be a number specifying the number of instructions to disassemble (e.g. \"disassemble 10\")." << endl;
        cout << "Optional parameters may follow:" << endl;
        cout << "\t\"addr <address>\" to specify a starting address" << endl;
        cout << "\t\"att\" to use AT&T assembler flavor instead of Intel" << endl;
        cout << "\t\"obj\" to use pure object code instead of Intel" << endl;

        return;
    }

    //Get the number of instructions to disassemble:
    int instructionCount = 0;

    try
    {
        instructionCount = stoi(args[0]);
    }
    catch (...)
    {
        cout << "First parameter (instruction count) must be a number." << endl;
        return;
    }

    //Default optional parameters:
    word address = loop.getTracee().getRegisters().REG_IP;
    bool att = false;
    bool obj = false;

    //Only one address / format param:
    bool addressParamAppeared = false;
    bool formatParamAppeared = false;

    //Read the params:
    for (unsigned int i = 1; i < args.size(); i++)
    {
        //Address:
        if (args[i] == "addr")
        {
            //Only one address param:
            if (addressParamAppeared)
            {
                cout << "Only one address parameter is allowed." << endl;
                return;
            }

            //Value needed:
            if (i == (args.size() - 1))
            {
                cout << "Parameter \"addr\" needs an address value." << endl;
                return;
            }

            //Get value, omit it and cast it to a pword:
            istringstream(args[++i]) >> hex >> address;

            //Address param has appeared now:
            addressParamAppeared = true;
        }
        //Format AT&T flavor:
        else if (args[i] == "att")
        {
            //Only one format param:
            if (formatParamAppeared)
            {
                cout << "Only one format parameter is allowed." << endl;
                return;
            }

            att = true;
            formatParamAppeared = true;
        }
        //Format object code:
        else if (args[i] == "obj")
        {
            //Only one format param:
            if (formatParamAppeared)
            {
                cout << "Only one format parameter is allowed." << endl;
                return;
            }

            obj = true;
            formatParamAppeared = true;
        }
        else
        {
            cout << "Unknown parameter: \"" << args[i] << "\"." << endl;
            return;
        }
    }

    //Disassemble:
    try
    {
        int totalLength = 0;
        vector<Mnemonic> mnemonics = loop.getTracee().disassemble((pword)address, att, instructionCount, totalLength);

        for (vector<Mnemonic>::iterator it = mnemonics.begin(); it != mnemonics.end(); ++it)
        {
            //Get the mnemonic:
            Mnemonic mnemonic = *it;

            //Object code or assembly?
            if (!obj)
            {
                //Print the address and the disassembly:
                cout << "\t<0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << hex << address << dec << ">\t" << mnemonic.getAssemblyString() << endl;
            }
            else
            {
                //Print the object code:
                cout << "\t<0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << hex << address << dec << ">\t";

                for (int i = 0; i < mnemonic.getOpcodeLength(); i++)
                {
                    cout << setfill('0') << setw(2) << hex << (int)mnemonic.getOpcode()[i] << dec << " ";
                }

                cout << endl;
            }

            //Increment address (it's a word!):
            address += mnemonic.getOpcodeLength();
        }
    }
    catch (runtime_error err)
    {
        cout << err.what() << endl;
        return;
    }
}
