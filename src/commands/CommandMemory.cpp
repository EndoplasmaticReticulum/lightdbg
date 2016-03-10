#include "CommandMemory.hpp"

#include <iostream>

vector<string> CommandMemory::getCommandStrings()
{
    return vector<string>({ "memory", "mem" });
}


void CommandMemory::invoke(DebugLoop& loop, vector<string>& args)
{
    UNUSED(args);

    //Always keep prompting:
    loop.setShowPrompt(true);


}
