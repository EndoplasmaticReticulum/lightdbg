#include "Tracer.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <stdlib.h>

Tracer::Tracer()
    : mode(TRACING_MODE_NONE), output(NULL)
{

}


Tracer::~Tracer()
{
    //Close output stream:
    closeOutput();
}


void Tracer::closeOutput()
{
    if (this->output)
    {
        this->output->flush();
        delete this->output;
        this->output = NULL;
    }

    this->mode = TRACING_MODE_NONE;
}


void Tracer::selectStdout()
{
    //Create a new stream:
    ostream* stream = new ostream(cout.rdbuf());

    //Check it:
    if (!stream)
    {
        throw runtime_error("Failed to allocate ostream on cout.");
    }

    //Close old one:
    closeOutput();

    //Assign:
    this->output = stream;
    this->mode = TRACING_MODE_STDOUT;
}


void Tracer::selectFile(string filePath)
{
    //Create a new stream:
    ofstream* stream = new ofstream(filePath, ofstream::out);

    //Check it:
    if (!stream)
    {
        throw runtime_error("Failed to allocate new ofstream.");
    }

    if (stream->fail())
    {
        throw runtime_error("Opening the file for writing has failed.");
    }

    //Close old one:
    closeOutput();

    //Assign:
    this->output = stream;
    this->mode = TRACING_MODE_FILE;
}


void Tracer::trace(Mnemonic& mnemonic, const struct user_regs_struct& registers)
{
    UNUSED(registers);

    if ((this->mode == TRACING_MODE_NONE) || !this->output)
    {
        return;
    }

    //Try to write to the ofstream:
    try
    {
        *(this->output) << "\t0x" << setfill('0') << setw(2 * WORD_SIZE_BYTES) << hex << registers.REG_IP << dec << "\t" << mnemonic.getAssemblyString() << endl;
    }
    catch (...)
    {
        //Ignoring errors here ...
    }
}
