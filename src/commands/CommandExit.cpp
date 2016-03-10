#include "commands/CommandExit.hpp"

#include <iostream>

vector<string> CommandExit::getCommandStrings()
{
    return vector<string>({ "exit", "kill", "quit", "qt" });
}


void CommandExit::invoke(DebugLoop& loop, vector<string>& args)
{
    UNUSED(args);

    //Log:
    cout << "Killing the debugged process ..." << endl;

    //This uses kill:
    loop.getTracee().killProcess();

    loop.setShowPrompt(false);
    loop.setKeepLooping(false);
}
