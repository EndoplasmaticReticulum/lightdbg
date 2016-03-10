#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>

#include "DebugLoop.hpp"
#include "Globals.hpp"

class DebugLoop;

using namespace std;

class Command
{
    //Members:
private:

    //Methods:
public:

    //Return the command strings the command should be registered for:
    virtual vector<string> getCommandStrings() = 0;

    //Invoke the command:
    virtual void invoke(DebugLoop& loop, vector<string>& args) = 0;

    //Destructor:
    virtual ~Command();
};

#endif // COMMAND_H
