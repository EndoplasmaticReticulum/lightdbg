#include "commands/CommandStep.hpp"

#include <iostream>
#include <string.h>

vector<string> CommandStep::getCommandStrings()
{
    return vector<string>({ "next", "nextstep", "step", "st", "ns", "s" });
}


void CommandStep::invoke(DebugLoop& loop, vector<string>& args)
{
    UNUSED(args);

    //Log:
    cout << "Performing a single step ..." << endl;

    //This uses PTRACE_SINGLESTEP:
    loop.getTracee().performStep();

    loop.setShowPrompt(false);
    loop.setKeepLooping(true);
}
