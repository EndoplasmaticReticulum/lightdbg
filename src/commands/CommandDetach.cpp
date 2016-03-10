#include "commands/CommandDetach.hpp"

#include <iostream>
#include <string.h>

vector<string> CommandDetach::getCommandStrings()
{
    return vector<string>({ "detach", "dt" });
}


void CommandDetach::invoke(DebugLoop& loop, vector<string>& args)
{
    //Maybe get a signal to deliver to the debugged process:
    int signal = 0;

    if (args.size())
    {
        try
        {
            signal = stoi(args[0]);
        }
        catch (...)
        {
            cout << "First parameter (signal) must be a number." << endl;
            loop.setShowPrompt(true);

            return;
        }
    }

    //Log:
    if (signal == 0)
    {
        cout << "Detaching from the debugged process ..." << endl;
    }
    else
    {
        cout << "Detaching from the debugged process with " << strsignal(signal) << " ..." << endl;
    }

    //This uses PTRACE_DETACH:
    loop.getTracee().detachFromProcess(signal);

    loop.setShowPrompt(false);
    loop.setKeepLooping(false);
}
