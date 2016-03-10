#ifndef TRACER_H
#define TRACER_H

#include <iostream>
#include <string>
#include <sys/user.h>

#include "Globals.hpp"
#include "Mnemonic.hpp"

using namespace std;

//Tracing modes:
enum TracingMode
{
    TRACING_MODE_NONE,
    TRACING_MODE_STDOUT,
    TRACING_MODE_FILE
};

class Tracer
{
    //Members:
private:

    //The current tracing mode:
    TracingMode mode;

    //Is tracing active?
    bool active;

    //The current output stream:
    ostream* output;

    //Methods:
public:

    //Get the tracing mode:
    inline TracingMode getTracingMode() const { return this->mode; }

    //Get/Set active flag:
    inline bool getTracingActive() const { return this->active; }
    inline void setTracingActive(bool flag) { this->active = flag; }

    //Constructor:
    Tracer();

    //Destructor:
    virtual ~Tracer();

    //Close the output:
    void closeOutput();

    //Select the output:
    void selectStdout();
    void selectFile(string filePath);

    //Trace a mnemonic with address and registers:
    void trace(Mnemonic& mnemonic, const user_regs_struct& registers);
};

#endif // TRACER_H
