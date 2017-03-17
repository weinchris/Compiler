/**
 * @file generator.h
 * @author Ramon Bisswanger
 * @version 1.1 (April 2015)
 * @brief This defines all data structures and functions for intermediate code
 *        generation.
 */

#include "symboltable.h"
#include "compiler.h"
#include <stdio.h>

#ifndef GENERATOR_H_
#define GENERATOR_H_

/**
 * Type definition to simplify usage of the enumeration.
 */
typedef enum e_operation operation;

/**
 * This enumeration contains all operations which are supported in the
 * intermediate code.
 */
enum e_operation
{
    /**
     * Numeric comparison: TARGET := OP1 == OP2
     */
    OP_EQUAL,
    
    /**
     * Numeric comparison: TARGET := OP1 != OP2
     */
    OP_NOT_EQUAL,
     
    /**
     * Numeric comparison: TARGET := OP1 <= OP2
     */
    OP_LESS_OR_EQUAL,
    
    /**
     * Numeric comparison: TARGET := OP1 >= OP2
     */
    OP_GREATER_OR_EQUAL,
    
    /**
     * Numeric comparison: TARGET := OP1 > OP2
     */
    OP_GREATER,
    
    /**
     * Numeric comparison: TARGET := OP1 < OP2
     */
    OP_LESS,
    
    /**
     * Logical combination: TARGET := OP1 AND OP2
     */
    OP_AND,
    
    /**
     * Logical combination: TARGET := OP1 OR OP2
     */
    OP_OR,
    
    /**
     * Logical combination: TARGET := NOT OP1
     */
    OP_NOT,
    
    /**
     * Control flow: IF
     */
    OP_IF,
    
    /**
     * Control flow: WHILE
     */
    OP_WHILE,
    
    /**
     * Markers are required for WHILE loops as the condition part might have to
     * be recalculated for every loop in case any variable contained has been
     * changed within the loop body.
     */
    OP_MARKER_WHILE,
     
    /**
     * Control flow: RETURN TARGET
     */
    OP_EXIT,
    
    /**
     * Mathematical operation: TARGET := OP1 + OP2
     */
    OP_PLUS,
    
    /**
     * Mathematical operation: TARGET := OP1 - OP2
     */
    OP_MINUS,
    
    /**
     * Mathematical operation: TARGET := OP1 * OP2
     */
    OP_MULTIPLY,
    
    /**
     * Mathematical operation: TARGET := OP1 / OP2
     */
    OP_DIVIDE,
    
    /**
     * Mathematical operation: TARGET := OP1 % OP2
     */
    OP_MODULO,
    
    /**
     * Mathematical operation: TARGET := TARGET + 1
     */
    OP_INCREMENT,
    
    /**
     * Mathematical operation: TARGET := TARGET - 1
     */
    OP_DECREMENT,
    
    /**
     * Assignment: TARGET := OP1
     */
    OP_ASSIGN,
    
    /**
     * Constant: TARGET := INT
     */
    OP_INT_CONSTANT,
    
    /**
     * Constant: TARGET := FLOAT
     */
    OP_FLOAT_CONSTANT,
    
    /**
     * Constant: TARGET := BOOLEAN
     */
    OP_BOOL_CONSTANT,
    
    /**
     * Place holder for sub-structures in if/else/while statements.<BR>
     * This does not create any intermediate code.
     */
    OP_NOP
};

/**
 * Type definition to simplify usage of the structure.
 */
typedef struct a_codeEntry codeEntry;

/**
 * This structure defines an intermediate code entry.
 */
struct a_codeEntry
{
    /**
     * The line number within the source file which relates to the
     * created intermediate code.<BR>
     * Note: This is used for output and debug purposes only. It has no effect
     *       on the intermediate code or its execution.
     */
    int sourceLine;
    
    /**
     * Desired operation.
     */
    operation op;
    
    /**
     * Target of operation (if applicable).
     */
    symbolTableEntry* target;
    
    /**
     * 1st operand of operation (if applicable).
     */
    symbolTableEntry* operand1;
    
    /**
     * 2nd operand of operation (if applicable).
     */
    symbolTableEntry* operand2;
    
    /**
     * Constant integer value of operation (if applicable).
     */
    int integer;
    
    /**
     * Constant real of operation (if applicable).
     */
    float real;
    
    /**
     * Constant boolean of operation (if applicable).
     */
    int boolean;
    
    /**
     * Pointer to a nested list of sub-code.<BR>
     * This is set for if/while statements.
     */
    codeEntry* sub_1;
    
    /**
     * Pointer to a nested list of sub-code.<BR>
     * This is set for else statements.
     */
    codeEntry* sub_2;
    
    /**
     * Pointer to the parent intermediate code entry.<BR>
     * This is set for nested structures (if/else/while).
     */
    codeEntry* parent;
    
    /**
     * Pointer to the following intermediate code entry.<BR>
     * This is supposed to be set to <code>null</code> for the last code entry.
     */
    codeEntry* next;
};

/**
 * Type definition to simplify usage of the structure.
 */
typedef struct a_codePrintEntry codePrintEntry;

/**
 * This structure defines an line of intermediate code for print output.
 */
struct a_codePrintEntry
{
    /**
     * The created intermediate code.<BR>
     * Note: this might be updated after initial creation in case of required
     *       backpatching steps.
     */
    char* code;
    
    /**
     * The line number within the intermediate code.<BR>
     * This is relevant to determine the target location for GOTO statements.
     */
    int lineNumber;
    
    /**
     * The line number within the source file which relates to the
     * created intermediate code.<BR>
     * Note: This is used for output and debug purposes only. It has no effect
     *       on the intermediate code or its execution.
     */
    int sourceLine;
    
    /**
     * Pointer to the following print code entry.<BR>
     * This is supposed to be set to <code>null</code> for the last code entry.
     */
    codePrintEntry* next;
};

/**
 * This appends a code entry to the program flow.
 * @param newCodeEntry The code entry to be added.
 */
void appendCodeEntry(codeEntry* newCodeEntry);

/**
 * This creates a new code entry.<BR>
 * Note: The code entry is only created in memory but not added to the program
 *       flow. The function appendCodeEntry needs to be called after the
 *       creation.<BR>
 * Note: No validation will take place within this function. Please use the
 *       specific code creation functions below as they include full validation.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @param op         Desired operation.
 * @param target     Target of operation (if applicable).
 * @param operand1   1st operand of operation (if applicable).
 * @param operand2   2nd operand of operation (if applicable).
 * @param integer    Constant integer value of operation (if applicable).
 * @param real       Constant real of operation (if applicable).
 * @param boolean    Constant boolean of operation (if applicable).
 * @return The new code entry.
 */
codeEntry* createCodeEntry(int sourceLine, operation op,
                           symbolTableEntry* target, symbolTableEntry* operand1,
                           symbolTableEntry* operand2, int integer, float real,
                           int boolean);

/**
 * This creates the intermediate code for numeric comparisons.
 * @param target     The symbol table entry into which the result shall be
 *                   stored.<BR>
 *                   The entry needs to be of type BOOLEAN.
 * @param op         The operation to be performed.<BR>
 *                   This needs to be one of the value: OP_EQUAL, OP_NOT_EQUAL,
 *                   OP_LESS_OR_EQUAL, OP_GREATER_OR_EQUAL, OP_GREATER, OP_LESS.
 * @param operand1   1st operand for the calculation.<BR>
 *                   The entry needs to be a numeric type.
 * @param operand2   2nd operand for the calculation.<BR>
 *                   The entry needs to be a numeric type.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeNumericComparison(symbolTableEntry* target, operation op,
                                symbolTableEntry* operand1,
                                symbolTableEntry* operand2,
                                int sourceLine);

/**
 * This creates the intermediate code for logical combinations.
 * @param target     The symbol table entry into which the result shall be
 *                   stored.<BR>
 *                   The entry needs to be of type BOOLEAN.
 * @param op         The operation to be performed.<BR>
 *                   This needs to be one of the value: OP_AND, OP_OR, OP_NOT.
 * @param operand1   1st operand for the calculation.<BR>
 *                   The entry needs to be of type BOOLEAN.
 * @param operand2   2nd operand for the calculation.<BR>
 *                   The entry needs to be of type BOOLEAN.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeLogicalCombination(symbolTableEntry* target, operation op,
                                 symbolTableEntry* operand1,
                                 symbolTableEntry* operand2,
                                 int sourceLine);

/**
 * This creates the intermediate code for an IF statement.
 * @param condition  The symbol table entry containing the condition for the IF.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeIf(symbolTableEntry* condition, int sourceLine);

/**
 * This creates the intermediate code for an ELSE flow.<BR>
 * Note: This may only be called of there is an open IF flow.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeElse(int sourceLine);

/**
 * This creates the intermediate code for a WHILE statement.<BR>
 * Note: Before calling this function, a valid marker needs to be set.
 * @param condition  The symbol table entry containing the condition for the
 *                   WHILE loop.<BR>
 *                   The entry needs to be of type BOOLEAN.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeWhile(symbolTableEntry* condition, int sourceLine);

/**
 * This creates a marker required for WHILE statements.<BR>
 * The marker needs to be set before creating the calculations which are done
 * for determining the conditions of the WHILE loop. The marker will be used to
 * create the GOTO statement in such way, that the calculations are updated
 * during every loop.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createMarkerWhile(int sourceLine);

/**
 * This creates the intermediate code for the end of a nested structure
 * (if/while).
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeEnd(int sourceLine);

/**
 * This creates the intermediate code for the program exit.
 * @param result     The symbol table entry which stores the program result.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeExit(symbolTableEntry* result, int sourceLine);

/**
 * This creates the intermediate code for mathematical operations.
 * @param target     The symbol table entry into which the result shall be
 *                   stored.<BR>
 *                   The entry needs to be a numeric type compatible to the
 *                   operand types.
 * @param op         The operation to be performed.<BR>
 *                   This needs to be one of the value: OP_PLUS, OP_MINUS,
 *                   OP_MULTIPLY, OP_DIVIDE, OP_MODULO.
 * @param operand1   1st operand for the calculation.<BR>
 *                   The entry needs to be a numeric type.<BR>
 *                   The entry needs to be of type INTEGER for operation
 *                   OP_MODULO.
 * @param operand2   2nd operand for the calculation.<BR>
 *                   The entry needs to be a numeric type.<BR>
 *                   The entry needs to be of type INTEGER for operation
 *                   OP_MODULO.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeMathematicalOperation(symbolTableEntry* target, operation op,
                                    symbolTableEntry* operand1,
                                    symbolTableEntry* operand2,
                                    int sourceLine);

/**
 * This creates the intermediate code for increments/decrements.
 * @param target     The symbol table entry for which the value shall be
 *                   incremented/decremented.<BR>
 *                   The entry needs to be of type INTEGER.
 * @param op         The operation to be performed.<BR>
 *                   This needs to be one of the value: OP_INCREMENT,
 *                   OP_DECREMENT.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeIncrement(symbolTableEntry* target, operation op, int sourceLine);

/**
 * This creates the intermediate code for assignments.
 * @param target     The symbol table entry for which the value shall be
 *                   assigned.
 * @param source     The value which shall be assigned to the target.<BR>
 *                   The type of the parameters target and value must be
 *                   compatible with each other (Either same type of target=REAL
 *                   and source=INTEGER)
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeAssignment(symbolTableEntry* target, symbolTableEntry* source,
                         int sourceLine);

/**
 * This creates the intermediate code for an integer constant.
 * @param target     The symbol table entry for which the constant float value
 *                   shall be assigned.<BR>
 *                   The entry needs to be of type INTEGER.
 * @param value      The float value which shall be assigned to the target.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeIntConst(symbolTableEntry* target, int value, int sourceLine);

/**
 * This creates the intermediate code for a float constant.<BR>
 * @param target     The symbol table entry for which the constant float value
 *                   shall be assigned.<BR>
 *                   The entry needs to be of type REAL.
 * @param value      The float value which shall be assigned to the target.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It 
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeFloatConst(symbolTableEntry* target, float value, int sourceLine);

/**
 * This creates the intermediate code for a boolean constant.
 * @param target     The symbol table entry for which the constant boolean value
 *                   shall be assigned.<BR>
 *                   The entry needs to be of type BOOLEAN.
 * @param value      The boolean value which shall be assigned to the target.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return <code>1</code> if the code has been successfully added.<BR>
 *         <code>0</code> if the supplied parameters are invalid.
 */
int createCodeBoolConst(symbolTableEntry* target, int value, int sourceLine);

/**
 * This writes the current intermediate code into a text file called 
 * <code>2_intermediate</code>.
 **/
void printCode();

/**
 * This appends a new code entry to the intermediate code print output.
 * @param code       The code print output to be added.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only.
 *                   has no effect on the intermediate code or its execution.
 * @return The created intermediate code entry.
 */
codePrintEntry* appendPrintCodeEntry(char* code, int sourceLine);

/**
 * This creates the print output representation of an intermediate code
 * entry.<BR>
 * This includes GOTO statements and required backpatching.
 * @param iterator The code entry which shall be added to the print output.
 */
void printCodeEntry(codeEntry* iterator);

/**
 * Determines the display value of a boolean variable.
 * @param value boolean value
 * @return String representation of the boolean value.<BR>
 *         <code>false</code> for value 0.<BR>
 *         <code>true</code> for all other values.
 */
char* getBooleanValue(int value);

#endif /*GENERATOR_H_*/
