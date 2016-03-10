#include <iostream>
#include <signal.h>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "Tracee.hpp"
#include "DebugLoop.hpp"
#include "Globals.hpp"

using namespace std;

//We won't be killed by a SIGINT with that handler.
//This is needed because we share stdin with our children,
//but a SIGINT should only stop them, not us:
void signalHandler_SIGINT(int signal)
{
    UNUSED(signal);
}


int main(int argc, char** argv)
{
    //Install our signal handlers:
    signal(SIGINT, signalHandler_SIGINT);

    //TODO: Maybe general arguments, splash ...
    int traceeArgOffset = 1;

    //If there is no tracee arg, we just print a short info text:
    if (argc <= traceeArgOffset)
    {
        //TODO
        cout << "Usage here ..." << endl;
        return 0;
    }

    //Collect the arguments for the tracee:
    vector<string> traceeArgs = vector<string>(argv + traceeArgOffset, argv + argc);

    //Create the tracee:
    Tracee* tracee;

    try
    {
        tracee = new Tracee(traceeArgs);
    }
    catch (runtime_error rt)
    {
        cout << rt.what() << endl;
        return -1;
    }

    //Print some info about it:
    cout << *tracee << endl;

    //Create a debug loop for the tracee:
    DebugLoop* loop;

    try
    {
        loop = new DebugLoop(*tracee);
    }
    catch (runtime_error rt)
    {
        delete tracee;
        cout << rt.what() << endl;

        return -1;
    }

    //Start looping:
    try
    {
        loop->loop();
    }
    catch (runtime_error rt)
    {
        delete loop;
        delete tracee;

        cout << rt.what() << endl;

        return -1;
    }

    //Free the debug loop and the tracee:
    delete loop;
    delete tracee;

    return 0;
}
