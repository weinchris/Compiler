/**
 * @file interpreter.h
 * @author Ramon Bisswanger
 * @version 1.1 (April 2015)
 * @brief This defines all data structures and functions for code execution.
 */

#include "generator.h"
#include "symboltable.h"
#include "compiler.h"
#include <stdio.h>

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

/**
 * Type definition to simplify usage of the enumeration.
 */
typedef struct s_variableTableEntry variableTableEntry;

/**
 * This structure defines an entry within the variable table.
 */
struct s_variableTableEntry
{
    /**
     * Reference to the variable entry in the symbol table.
     */
    symbolTableEntry* variable;
    
    /**
     * Union which keeps the variable value.
     */
    union
    {
        /**
         * Integer value.
         */
        int intValue;
        
        /**
         * Float value.
         */
        double floatValue;
        
        /**
         * Boolean value.
         */
        int boolValue;
    } value;
    
    /**
     * Pointer to the following entry of the variable table.<BR>
     * This is supposed to be set to <code>null</code> for the last entry.
     */
    variableTableEntry* next;
};

/**
 * This adds a new entry to the variable table.
 * @param variable Reference to the variable entry within the symbol table.<BR>
 *                 Note: This function does not check whether the entry is
 *                 already existing within the variable table.<BR>
 *                 Prevention of duplicates need to be taken care of before
 *                 calling this function.
 * @return The new entry which has been added to the variable table.<BR>
 *         <code>null</code> if invalid input data has been given.
 */
variableTableEntry* addEntryToVariableTable(symbolTableEntry* variable);

/**
 * This searches for an entry in the variable table.
 * @param variable The desired variable table entry.
 * @return The variable table entry for the given variable name.<BR>
 *         `null` is returned if no corresponding entry has been found in the
 *         variable table.<BR>
 *         Note: if multiple entries for the same variable have been added to
 *               the variable  table, only the one which was added first is
 *               returned.
 **/
variableTableEntry* getEntryFromVariableTable(symbolTableEntry* variable);

/**
 * This executes the intermediate code and writes all steps into a text file
 * called <code>3_execution</code>.<BR>
 * The variable table at program exit is written to file
 * <code>4_variabletable</code>.<BR>
 * The return value of the program is stored in variable programResult and
 * printed on screen.
 **/
void runCode();

/**
 * This executes a single code statement (including nested sub-code).<BR>
 * @param iterator The code entry which shall be executed.
 * @param f        Reference to the file for storing the execution output.
 * @param indent   Current indentation to visualize nesting within the execution
 *                 output.
 */
void runCodeEntry(codeEntry* iterator, FILE *f, char* indent);

#endif /*INTERPRETER_H_*/
