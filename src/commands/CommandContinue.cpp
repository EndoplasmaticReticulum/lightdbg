#include "commands/CommandContinue.hpp"

#include <iostream>
#include <string.h>

vector<string> CommandContinue::getCommandStrings()
{
    return vector<string>({ "continue", "ct" });
}


void CommandContinue::invoke(DebugLoop& loop, vector<string>& args)
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
        cout << "Continuing the execution ..." << endl;
    }
    else
    {
        cout << "Continuing the execution with " << strsignal(signal) << " ..." << endl;
    }

    //Install the breakpoints:
    loop.setBreakpointsInstalled(true);

    //This uses PTRACE_SYSCALL:
    loop.getTracee().continueProcess(signal);

    loop.setShowPrompt(false);
    loop.setKeepLooping(true);
}
