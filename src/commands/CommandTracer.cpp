#include "CommandTracer.hpp"

#include <iostream>
#include <sys/user.h>

vector<string> CommandTracer::getCommandStrings()
{
    return vector<string>({ "tracer", "trace", "tr" });
}


void CommandTracer::invoke(DebugLoop& loop, vector<string>& args)
{
    //Show another prompt after this (nearly all cases except running):
    loop.setShowPrompt(true);

    //No arguments:
    //Show current tracing mode.
    if (args.size() == 0)
    {
        cout << "Command syntax: \"trace mode\" to check/specify the mode or \"trace run\" to start tracing." << endl;
        return;
    }

    //Setting the mode:
    try
    {
        if (args[0] == "mode")
        {
            //Show possible modes:
            if (args.size() < 2)
            {
                cout << "Current trace mode: ";

                switch (loop.getTracer().getTracingMode())
                {
                case TRACING_MODE_NONE: cout << "none"; break;
                case TRACING_MODE_STDOUT: cout << "stdout"; break;
                case TRACING_MODE_FILE: cout << "file"; break;

                default: cout << "-";
                }

                cout << "." << endl << "Possible modes: none, file, stdout." << endl;
                return;
            }

            //None:
            if (args[1] == "none")
            {
                loop.getTracer().closeOutput();
                cout << "Trace mode has been set to none." << endl;

                return;
            }
            //Stdout:
            else if (args[1] == "stdout")
            {
                loop.getTracer().selectStdout();
                cout << "Trace mode has been set to stdout." << endl;

                return;
            }
            //File:
            else if (args[1] == "file")
            {
                if (args.size() < 3)
                {
                    cout << "Please provide a trace file path as parameter." << endl;
                    return;
                }

                //Fix args together again (yes, dirty ...):
                string filePath = args[2];

                for (vector<string>::iterator it = args.begin() + 3; it != args.end(); ++it)
                {
                    filePath += " " + *it;
                }

                loop.getTracer().selectFile(filePath);
                cout << "Trace mode has been set to file." << endl;

                return;
            }
            else
            {
                cout << "Unknown trace mode. \"tracer mode\" will display all valid choices." << endl;
                return;
            }
        }
    }
    catch (runtime_error rt)
    {
        cout << "Failed to set trace mode: " << rt.what() << endl;
        return;
    }

    //Starting the trace:
    if (args[0] == "run")
    {
        if (loop.getTracer().getTracingMode() == TRACING_MODE_NONE)
        {
            cout << "Please set a trace mode first." << endl;
            return;
        }

        //Enable the tracing:
        loop.getTracer().setTracingActive(true);

        //Disable the prompt, enable the loop:
        loop.setShowPrompt(false);
        loop.setKeepLooping(true);

        //Initiate by performing a single step:
        loop.getTracee().performStep();

        return;
    }

    //Unknown:
    cout << "Unknown parameter. Please use \"trace\" to display possible parameters." << endl;
}
