/**
 * @file symboltable.c
 * @author Ramon Bisswanger
 * @version 1.1 (April 2015)
 * @brief This contains all function implementations for symbol table handling
 *        (add/get/print).
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symboltable.h"

/**
 * Variable to enable/disable debug mode.<BR>
 * [defined in file compiler.c]
 */
extern int debug;

/**
 * Variable to track current line number.<BR>
 * [defined in file compiler.l]
 */
extern int inputLineNumber;


/**
 * Pointer to the first entry of the symbol table.<BR>
 * This variable is automatically initialized when adding the first entry.
 */
symbolTableEntry* symbolTable = 0;

/**
 * Determines the display name of a data type.
 * @param type input type
 * @return String representation of the data type.<BR>
 *         This is the name of the corresponding enum value.<BR>
 *         If an invalid value is given, the text <code>-UNKNOWN-</code> will be
 *         returned.
 */
char* getTypeName(dataType type)
{
    switch (type)
    {
        case INTEGER:
            return "INTEGER";
        case REAL:
            return "REAL";
        case BOOLEAN:
            return "BOOLEAN";
        default:
            return "-UNKNOWN-";
    }
}

/**
 * This adds a new entry to the symbol table.
 * @param name Name of the variable to be added.<BR>
 *             Note: This function does not check whether the entry is already
 *             existing within the symbol table.<BR>
 *             Prevention of duplicates need to be taken care of before calling
 *             this function.
 * @param type Desired variable type.
 * @param line Line number of the input file where the variable has been
 *             defined.<BR>
 *             Note: This is used for output and debug purposes only. It has no
 *                   effect on the symbol table itself.
 * @return The new entry which has been added to the symbol table.<BR>
 *         <code>null</code> if invalid input data has been given.
 */
symbolTableEntry* addEntryToSymbolTable(char* name, dataType type, int line)
{
    // Validate parameter values
    if ((name == 0) || (strlen(name) == 0))
    {
        fprintf(stderr, "Call to addEntryToSymbolTable failed: "
                        "No variable name given.");
        return 0;
    }
    if ((type != INTEGER) && (type != REAL) && (type != BOOLEAN))
    {
        fprintf(stderr, "Call to addEntryToSymbolTable failed: "
                        "Invalid type given.");
        return 0;
    }

    // Allocate required memory
    symbolTableEntry* newSymtabEntry = (symbolTableEntry*)
                                       malloc(sizeof(symbolTableEntry));
    newSymtabEntry->name = (char*) malloc(strlen(name) +1);
    strcpy(newSymtabEntry->name,name);
    newSymtabEntry->type = type;
    newSymtabEntry->line = line; 
    // Set fields
    // TODO - IMPLEMENTATION NEEDED

    if (debug > 0)
    {
        printf("Adding new entry to symbol table: %s %s %d\n",
               name, getTypeName(type), line);
    }

    // Assign as new symbol table for 1st entry
    if (!symbolTable)
    {
        symbolTable = newSymtabEntry;
    }
    // Add at the end otherwise
    else
    {
        symbolTableEntry* iterator = symbolTable;
        while (iterator->next)
        {
            iterator = iterator->next;
        }
        iterator->next = newSymtabEntry;
    }

    return newSymtabEntry;
}

/**
 * This searches for an entry in the symbol table by name.
 * @param name The desired symbol table entry.
 * @return The symbol table entry for the given variable name.<BR>
 *         `null` is returned if no corresponding entry has been found in the
 *         symbol table.<BR>
 *         Note: if multiple entries with the same name have been added to the
 *               symbol table, only the one which was added first is returned.
 **/
symbolTableEntry* getEntryFromSymbolTable(char* name)
{
    // Loop until all entries in the symbol table and check the name
    symbolTableEntry* iterator = symbolTable;

    while ((iterator != 0) && (strcmp(iterator->name, name) != 0))
    {
        iterator = iterator->next;
    }

    return iterator;
}

/**
 * This writes the current symbol table into a text file called
 * <code>1_symboltable</code>.
 **/
void printSymbolTable()
{
    // TODO - IMPLEMENTATION NEEDED
    symbolTableEntry* iterator2 = symbolTable;
    for (;iterator2->next;iterator2=iterator2->next)
    {
        printf("Eintrag: %s = %s in line %d\n",
                iterator2->name,
                getTypeName(iterator2->type),
                iterator2->line);
    }
}

char* getName()
{
  char* number= malloc(sizeof(char)*10);
  sprintf(number, "_H%d", inputLineNumber);
  return number;
}
