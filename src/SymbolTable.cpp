#include "SymbolTable.hpp"

#define PACKAGE 1
#define PACKAGE_VERSION 1
#include <bfd.h>
#include <stdlib.h>

#include <stdexcept>

SymbolTable::~SymbolTable()
{
    //Free the symbols:
    for (SymbolTableMap::iterator it = this->table.begin(); it != this->table.end(); ++it)
    {
        delete it->second;
    }

    this->table.clear();
}


SymbolTable::SymbolTable(string path)
{
    //Open the file as binary file descriptor:
    bfd* descr = bfd_openr(path.c_str(), NULL);

    if (!descr)
    {
        throw runtime_error("Failed to open binary file descriptor.");
    }

    //Check the format of the file:
    if (!bfd_check_format(descr, bfd_object))
    {
        bfd_close(descr);
        throw runtime_error("Failed to verify binary format.");
    }

    //Get the size of the symbol table:
    long symbolTableSize = bfd_get_symtab_upper_bound(descr);

    if (symbolTableSize <= 0)
    {
        bfd_close(descr);

        if (symbolTableSize < 0)
        {
            throw runtime_error("Failed to read the upper bound of the symbol table.");
        }
        else
        {
            return;
        }
    }

    //Alloc space for the table:
    asymbol** symbolTable = (asymbol**)malloc(symbolTableSize);

    if (!symbolTable)
    {
        bfd_close(descr);
        throw runtime_error("Failed to allocate space for the symbol table.");
    }

    //Get the number of symbols by canonicalizing the table:
    long symbolTableCount = bfd_canonicalize_symtab(descr, symbolTable);

    if (symbolTableCount < 0)
    {
        bfd_close(descr);
        free(symbolTable);

        throw runtime_error("Failed to read the number of symbols in the symbol table.");
    }

    //Iterate:
    for (int i = 0; i < symbolTableCount; i++)
    {
        //Is there a name?
        if (!symbolTable[i]->name)
        {
            continue;
        }

        //Get the symbol data:
        Symbol* newSymbol = new Symbol(string(bfd_asymbol_name(symbolTable[i])), (pword)bfd_asymbol_value(symbolTable[i]));

        //Check:
        if (!newSymbol)
        {
            throw runtime_error("Failed to allocate space for a symbol.");
        }

        this->table[newSymbol->getName()] = newSymbol;
    }

    //Close the file descriptor and free the table:
    bfd_close(descr);
    free(symbolTable);
}
