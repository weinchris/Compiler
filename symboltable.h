/**
 * @file symboltable.h
 * @author Ramon Bisswanger
 * @version 1.1 (April 2015)
 * @brief This defines all data structures and functions for symbol table
 *        handling.
 */

#ifndef SYMBOLTABLE_H_
#define SYMBOLTABLE_H_

/**
 * Type definition to simplify usage of the enumeration.
 */
typedef enum e_dataType dataType;

/**
 * This enumeration contains all supported data types which can be stored within
 * the symbol table.
 **/
enum e_dataType
{
    /**
     * Integer numbers.
     */
    INTEGER,

    /**
     * Floating point numbers.
     */
    REAL,

    /**
     * Boolean values (<code>true</code> / <code>false</code>).
     */
    BOOLEAN
};

/**
 * Type definition to simplify usage of the structure.
 */
typedef struct s_symbolTableEntry symbolTableEntry;

/**
 * This structure defines an entry within the symbol table.
 */
struct s_symbolTableEntry
{
    /**
     * The name of the variable.
     */
    char* name;

    /**
     * The type of the variable.
     */
    dataType type;

    /**
     * Line number of the input file where the variable has been defined.
     * Note: This is used for output and debug purposes only. It has no effect
     *       on the symbol table itself.
     */
    int line;

    /**
     * Pointer to the following entry of the symbol table.<BR>
     * This is supposed to be set to <code>null</code> for the last entry.
     */
    symbolTableEntry* next;
};

/**
 * Determines the display name of a data type.
 * @param type input type
 * @return String representation of the data type.<BR>
 *         This is the name of the corresponding enum value.<BR>
 *         If an invalid value is given, the text <code>-UNKNOWN-</code> will be
 *         returned.
 */
char* getTypeName(dataType type);

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
symbolTableEntry* addEntryToSymbolTable(char* name, dataType type, int line);

/**
 * This searches for an entry in the symbol table by name.
 * @param name The desired symbol table entry.
 * @return The symbol table entry for the given variable name.<BR>
 *         `null` is returned if no corresponding entry has been found in the
 *         symbol table.<BR>
 *         Note: if multiple entries with the same name have been added to the
 *               symbol table, only the one which was added first is returned.
 **/
symbolTableEntry* getEntryFromSymbolTable(char* name);

/**
 * This writes the current symbol table into a text file called
 * <code>1_symboltable</code>.
 **/
void printSymbolTable();

char* getName();

#endif /*SYMBOLTABLE_H_*/
