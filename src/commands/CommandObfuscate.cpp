#include "CommandObfuscate.hpp"

#include <iostream>

vector<string> CommandObfuscate::getCommandStrings()
{
    return vector<string>({ "obfuscate", "of" });
}


void CommandObfuscate::invoke(DebugLoop& loop, vector<string>& args)
{
    //Always keep prompting:
    loop.setShowPrompt(true);

    //Check args:
    if (args.size() == 0)
    {
        cout << "Command syntax is: \"obfuscate <type> [<on|off>]\"." << endl;
        cout << "Possible types:" << endl;
        cout << "\t\"traceme\": Makes ptrace(PTRACE_TRACEME, ...) calls from the debugged process succeed." << endl;
        cout << "\t\"time\": Plays around with the time." << endl;

        return;
    }

    //Second arg is on/off.
    //0: show, 1: on, 2: off.
    int action = 0;

    if (args.size() >= 2)
    {
        if (args[1] == "on")
        {
            action = 1;
        }
        else if (args[1] == "off")
        {
            action = 2;
        }
        else
        {
            cout << "Unknown action: \"" << args[1] << "\"." << endl;
            cout << "Type \"obfuscate\" to show possible actions." << endl;

            return;
        }
    }

    //First arg is obfuscation type:
    if (args[0] == "traceme")
    {
        if (action != 0)
        {
            loop.setObfuscateTraceMe(action == 1);
        }

        cout << "Obfuscation \"traceme\": " << (loop.getObfuscateTraceMe() ? "on" : "off") << "." << endl;
        return;
    }
    else if (args[0] == "time")
    {
        if (action != 0)
        {
            loop.setObfuscateTime(action == 1);
        }

        cout << "Obfuscation \"time\": " << (loop.getObfuscateTime() ? "on" : "off") << "." << endl;
        return;
    }
    //Unknown:
    else
    {
        cout << "Unknown obfuscation type: \"" << args[0] << "\"." << endl;
        cout << "Type \"obfuscate\" to show possible types." << endl;

        return;
    }
}
