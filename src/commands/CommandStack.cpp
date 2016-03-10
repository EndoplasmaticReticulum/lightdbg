#include "CommandStack.hpp"

#include <iomanip>
#include <iostream>

vector<string> CommandStack::getCommandStrings()
{
    return vector<string>({ "stack", "sta", "stk" });
}


void CommandStack::invoke(DebugLoop& loop, vector<string>& args)
{
    //Always keep prompting:
    loop.setShowPrompt(true);

    //Show possible args:
    if (args.size() == 0)
    {
        cout << "Command syntax: \"stack <number of words>\"." << endl;
        return;
    }

    //Get the number of words to show:
    int wordCount = 0;

    try
    {
        wordCount = stoi(args[0]);
    }
    catch (...)
    {
        cout << "First parameter (word count) must be a number." << endl;
        return;
    }

    //Display:
    try
    {
        for (int i = 0; i < wordCount; i++)
        {
            pword address = ((pword)loop.getTracee().getRegisters().REG_SP) + i;
            cout << "\t<0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << hex << (word)address << ">\t0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << loop.getTracee().peekWord(address) << dec;

            //Additional info:
            if (i == 0)
            {
                cout << "\t<-- stack pointer";
            }

            if (address == (pword)(loop.getTracee().getRegisters().REG_BP))
            {
                cout << "\t<-- base pointer";
            }

            cout << endl;
        }
    }
    catch (runtime_error err)
    {
        cout << err.what();
        return;
    }
}
