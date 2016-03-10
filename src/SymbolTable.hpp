#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <map>
#include <string>

#include "Globals.hpp"
#include "Symbol.hpp"

using namespace std;

typedef map<string, Symbol*> SymbolTableMap;

class SymbolTable
{
    //Members:
private:

    //The table itself:
    SymbolTableMap table;

    //Methods:
public:

    //Get the table:
    inline SymbolTableMap const& getMap() const { return table; }

    //Constructor:
    SymbolTable(string path);

    //Destructor:
    virtual ~SymbolTable();
};

#endif // SYMBOLTABLE_H
