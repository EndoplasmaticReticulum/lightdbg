#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>

#include "Globals.hpp"

using namespace std;

class Symbol
{
    //The name of the symbol:
    string name;

    //The address of the symbol:
    pword address;

public:

    //Getters:
    inline string getName() const { return this->name; }
    inline pword getAddress() const { return this->address; }

    //Constructor:
    Symbol(string name, pword address);
};

#endif // SYMBOL_H
