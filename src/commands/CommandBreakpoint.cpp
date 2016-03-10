#include "CommandBreakpoint.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "Breakpoint.hpp"
#include "SymbolTable.hpp"

vector<string> CommandBreakpoint::getCommandStrings()
{
    return vector<string>({ "breakpoint", "break", "bp", "b" });
}


void CommandBreakpoint::invoke(DebugLoop& loop, vector<string>& args)
{
    //Always wait for another command:
    loop.setShowPrompt(true);

    if (args.size() == 0)
    {
        cout << "Command syntax is: \"breakpoint <list|addr|sym|del> [address resp. sym] [base <hex base>]\"." << endl;
        return;
    }

    //Breakpoints can be listed (0), adr-added (1), sym-added (2) or del (3):
    int action = 0;

    if (args[0] == "addr")
    {
        action = 1;
    }
    else if (args[0] == "sym")
    {
        action = 2;
    }
    else if (args[0] == "del")
    {
        action = 3;
    }
    else if (args[0] != "list")
    {
        cout << "Unknown parameter: \"" << args[0] << "\"." << endl;
        return;
    }

    //Execute action (0):
    if (action == 0)
    {
        cout << "Current breakpoints:";

        if (loop.getBreakpoints().size() == 0)
        {
            cout << " none" << endl;
        }
        else
        {
            cout << endl;

            for (map<pword, Breakpoint*>::const_iterator it = loop.getBreakpoints().begin(); it != loop.getBreakpoints().end(); ++it)
            {
                cout << "\t0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << hex << (word)it->first << dec << endl;
            }
        }

        return;
    }

    //Get address:
    word address = loop.getTracee().getRegisters().REG_IP;

    //Sym address:
    if (action == 2)
    {
        //Symbol parameter needed at that point:
        if (args.size() < 2)
        {
            cout << "Symbol parameter needed." << endl;
            return;
        }

        //Check if the symbol exists:
        const SymbolTableMap& syms = loop.getTracee().getSymbolTable()->getMap();

        if (syms.find(args[1]) == syms.end())
        {
            cout << "Symbol \"" << args[1] << "\" not found." << endl;
            return;
        }

        //Get its address:
        address = (word)(syms.at(args[1])->getAddress());
    }
    //Param address:
    else
    {
        //Address parameter needed at that point:
        if (args.size() < 2)
        {
            cout << "Address parameter needed." << endl;
            return;
        }

        //Read address parameter:
        istringstream(args[1]) >> hex >> address;
    }

    //Maybe get base:
    if (args.size() >= 3)
    {
        if (args[2] == "base")
        {
            //There must be a base:
            if (args.size() < 4)
            {
                cout << "No base address provided." << endl;
                return;
            }

            //Read the base:
            word base = 0;
            istringstream(args[3]) >> hex >> base;

            //Add it:
            address += base;
        }
        else
        {
            cout << "Unknown parameter: \"" << args[2] << "\"." << endl;
            return;
        }
    }

    //Execute action (1, 2, 3):
    try
    {
        if ((action == 1) || (action == 2))
        {
            loop.addBreakpoint((pword)address);
            cout << "Breakpoint " << "(" << loop.getBreakpoints().size() << ") at 0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << hex << address << dec << " added." << endl;
        }
        else
        {
            loop.removeBreakpoint((pword)address);
            cout << "Breakpoint removed (" << loop.getBreakpoints().size() << " remaining)." << endl;
        }
    }
    catch (runtime_error err)
    {
        cout << err.what() << endl;
        return;
    }
}
