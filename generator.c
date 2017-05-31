/**
 * @file generator.c
 * @author Ramon Bisswanger
 * @version 1.1 (April 2015)
 * @brief This contains all function implementations for intermediate code
 *        generation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "generator.h"
#include "symboltable.h"
#include "compiler.h"

/**
 * Pointer to the first entry of the code list.<BR>
 * This variable is automatically initialized when adding the first entry.
 */
codeEntry* codeList = 0;

/**
 * Pointer to the last entry of the code list.<BR>
 * This is used to prevent walking through the whole list whenever adding a new
 * entry and to consider the nesting for if/while structures.
 */
codeEntry* currentCodeEntry = 0;

/**
 * Pointer to the current nesting context (for if/while).<BR>
 * This is set to <code>null</code> if the current code is not part of any
 * nested structure.
 */
codeEntry* currentContext = 0;

/**
 * This variable holds the current line number for the intermediate code.<BR>
 * Line numbers are required within intermediate code to determine target
 * locations for GOTO statements which are generated for if/while structures.
 */
int codeLineNumber;

/**
 * Pointer to the first entry of the code list for printing.<BR>
 * This variable is automatically initialized when adding the first entry.<BR>
 * All intermediate code entries are first added to this list and only written
 * to the target file after complete parsing. This is to evaluate all required
 * back-patching for GOTO statements.
 */
codePrintEntry* printCodeList = 0;

/**
 * Pointer to the last entry of the code list for printing.<BR>
 * This is used to prevent walking through the whole list whenever adding a new
 * entry.
 */
codePrintEntry* currentPrintCodeList = 0;

/**
 * Line number for the previous WHILE line marker.<BR>
 * Markers are required for WHILE loops as the condition part might have to be
 * recalculated for every loop in case any variable contained has been changed
 * within the loop body.
 * @see createMarkerWhile
 */
int lastWhileMarkerCodeLine = 0;

/**
 * This appends a code entry to the program flow.
 * @param newCodeEntry The code entry to be added.
 */
void appendCodeEntry(codeEntry* newCodeEntry)
{
    // Assign as new start for 1st entry
    if (!codeList)
    {
        codeList = newCodeEntry;
        currentCodeEntry = newCodeEntry;
    }
    // Add at the end otherwise
    else
    {
        currentCodeEntry->next = newCodeEntry;
        currentCodeEntry = newCodeEntry;
    }
}

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
                           int boolean)
{
    // Allocate required memory
    codeEntry* newCodeEntry = (codeEntry*) malloc(sizeof(codeEntry));

    // Set fields
    newCodeEntry->sourceLine = sourceLine;
    newCodeEntry->op = op;
    newCodeEntry->target = target;
    newCodeEntry->operand1 = operand1;
    newCodeEntry->operand2 = operand2;
    newCodeEntry->integer = integer;
    newCodeEntry->real = real;
    newCodeEntry->parent = currentContext;
    newCodeEntry->sub_1 = 0;
    newCodeEntry->sub_2 = 0;
    newCodeEntry->next = 0;

    return newCodeEntry;
}

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
                                int sourceLine)
{
    if ((op != OP_EQUAL) && (op != OP_NOT_EQUAL) && (op != OP_LESS_OR_EQUAL) &&
        (op != OP_GREATER_OR_EQUAL) && (op != OP_GREATER) && (op != OP_LESS))
    {
        fprintf(stderr, "Unexpected numeric comparison: %d. Expected OP_EQUAL, "
                        "OP_NOT_EQUAL, OP_LESS_OR_EQUAL, OP_GREATER_OR_EQUAL, "
                        "OP_GREATER or OP_LESS. (Line: %d)\n", op, sourceLine);
        return 0;
    }
    if (!target)
    {
        fprintf(stderr, "No target has been given for numeric comparison. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if (target->type != BOOLEAN)
    {
        fprintf(stderr, "Expected data type BOOLEAN for numeric comparison. "
                        "Got: %s. (Line: %d)\n", getTypeName(target->type),
                        sourceLine);
        return 0;
    }    
    if (!operand1)
    {
        fprintf(stderr, "Operand 1 missing for numeric comparison. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if ((operand1->type != INTEGER) && (operand1->type != REAL))
    {
        fprintf(stderr, "Operand 1 requires numeric data type for numeric "
                        "comparison. Got: %s. (Line: %d)\n",
                        getTypeName(operand1->type), sourceLine);
        return 0;
    }
    if (!operand2)
    {
        fprintf(stderr, "Operand 2 missing for numeric comparison. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if ((operand2->type != INTEGER) && (operand2->type != REAL))
    {
        fprintf(stderr, "Operand 2 requires numeric data type for numeric "
                        "comparison. Got: %s. (Line: %d)\n",
                        getTypeName(operand2->type), sourceLine);
        return 0;
    }
    
    codeEntry* entry = createCodeEntry(sourceLine, op, target, operand1,
                                       operand2, 0, 0, 0);
    appendCodeEntry(entry);
    return 1;
}

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
 *                   The entry needs to be of type BOOLEAN.<BR>
 *                   If operation is set to OP_NOT, this is expected to be
 *                   <code>null</code>.
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
                                 int sourceLine)
{
    if ((op != OP_AND) && (op != OP_OR) && (op != OP_NOT))
    {
        fprintf(stderr, "Unexpected logical combination: %d. Expected OP_AND, "
                        "OP_OR or OP_NOT. (Line: %d)\n", op, sourceLine);
        return 0;
    }
    if (!target)
    {
        fprintf(stderr, "No target has been given for logical combination. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if (target->type != BOOLEAN)
    {
        fprintf(stderr, "Expected data type BOOLEAN for logical combination. "
                        "Got: %s. (Line: %d)\n", getTypeName(target->type),
                        sourceLine);
        return 0;
    }
    if (!operand1)
    {
        fprintf(stderr, "Operand 1 missing for logical combination. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if (operand1->type != BOOLEAN)
    {
        fprintf(stderr, "Operand 1 requires data type BOOLEAN for logical "
                        "combination. Got: %s. (Line: %d)\n",
                        getTypeName(operand1->type), sourceLine);
        return 0;
    }
    if ((op != OP_NOT) && !operand2)
    {
        fprintf(stderr, "Operand 2 missing for logical combination. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if ((op == OP_NOT) && operand2)
    {
        fprintf(stderr, "Operand 2 expected null for operation OP_NOT. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if ((op != OP_NOT) && (operand2->type != BOOLEAN))
    {
        fprintf(stderr, "Operand 2 requires data type BOOLEAN for logical "
                        "combination. Got: %s. (Line: %d)\n",
                        getTypeName(operand2->type), sourceLine);
        return 0;
    }
    
    codeEntry* entry = createCodeEntry(sourceLine, op, target, operand1,
                                       operand2, 0, 0, 0);
    appendCodeEntry(entry);
    return 1;
}

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
int createCodeIf(symbolTableEntry* condition, int sourceLine)
{
    if (!condition)
    {
        fprintf(stderr, "No condition has been given for WHILE loop. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if (condition->type != BOOLEAN)
    {
        fprintf(stderr, "Expected data type BOOLEAN for loop condition. "
                        "Got: %s. (Line: %d)\n", getTypeName(condition->type),
                        sourceLine);
        return 0;
    }
    
    // Create an entry for the IF
    codeEntry* entry = createCodeEntry(sourceLine, OP_IF, 0, condition,
                                       0, 0, 0, 0);
    appendCodeEntry(entry);
    currentContext = entry;
    
    // Create a nested context for all IF statements
    codeEntry* ifEntry = createCodeEntry(sourceLine, OP_NOP, 0, 0, 0, 0, 0, 0);
    ifEntry->parent = entry;
    entry->sub_1 = ifEntry;
    currentCodeEntry = ifEntry;
    
    return 1;
}

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
int createCodeElse(int sourceLine)
{
    if (!currentContext)
    {
        fprintf(stderr, "Failed to create intermediate code for ELSE statement."
                        " No nested structure is open. (Line: %d)\n",
                        sourceLine);
        return 0;
    }
    if (currentContext->op != OP_IF)
    {
        fprintf(stderr, "Failed to create intermediate code for ELSE statement."
                        " Not contained in an IF statement. (Line: %d)\n",
                        sourceLine);
        return 0;
    }
    if (currentContext->sub_2)
    {
        fprintf(stderr, "Failed to create intermediate code for ELSE statement."
                        " IF statement already contains an ELSE flow. "
                        "(Line: %d)\n",
                        sourceLine);
        return 0;
    }
    
    // Create a new entry for the ELSE flow
    codeEntry* elseEntry = createCodeEntry(sourceLine, OP_NOP, 0, 0, 0, 0, 0,
                                           0);
    elseEntry->parent = currentContext;
    currentContext->sub_2 = elseEntry;
    currentCodeEntry = elseEntry;
    
    return 1;
}

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
int createCodeWhile(symbolTableEntry* condition, int sourceLine)
{
    if (!condition)
    {
        fprintf(stderr, "No condition has been given for WHILE loop. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if (condition->type != BOOLEAN)
    {
        fprintf(stderr, "Expected data type BOOLEAN for loop condition. "
                        "Got: %s. (Line: %d)\n", getTypeName(condition->type),
                        sourceLine);
        return 0;
    }
    if (lastWhileMarkerCodeLine == 0)
    {
        fprintf(stderr, "Failed to create WHILE loop. No marker has been "
                        "defined. (Line: %d)\n", sourceLine);
        return 0;
    }
    
    // Create an entry for the WHILE loop
    codeEntry* entry = createCodeEntry(sourceLine, OP_WHILE, 0, condition,
                                       0, 0, 0, 0);
    appendCodeEntry(entry);
    currentContext = entry;
    
    // Create a nested context for all loop statements
    codeEntry* whileEntry = createCodeEntry(sourceLine, OP_NOP, 0, 0, 0, 0, 0,
                                            0);
    whileEntry->parent = entry;
    entry->sub_1 = whileEntry;
    currentCodeEntry = whileEntry;
    
    // Reset code marker
    lastWhileMarkerCodeLine = 0;
    
    return 1;
}

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
int createMarkerWhile(int sourceLine)
{
    if (lastWhileMarkerCodeLine > 0)
    {
        fprintf(stderr, "Failed to create WHILE marker. There is already an "
                        "open marker from definition in line %d. (Line: %d)\n",
                        lastWhileMarkerCodeLine, sourceLine);
        return 0;
    }
    
    lastWhileMarkerCodeLine = sourceLine;
    codeEntry* entry = createCodeEntry(sourceLine, OP_MARKER_WHILE,
                                       0, 0, 0, 0, 0, 0);
    appendCodeEntry(entry);
    return 1;
}

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
int createCodeEnd(int sourceLine)
{
    if (!currentContext)
    {
        fprintf(stderr, "Failed to create intermediate code for END statement. "
                        "No nested structure is open. (Line: %d)\n",
                        sourceLine);
        return 0;
    }
    
    currentCodeEntry = currentContext;
    currentContext = currentContext->parent;
    return 1;
}

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
int createCodeExit(symbolTableEntry* result, int sourceLine)
{
    if (!result)
    {
        fprintf(stderr, "No result has been given for program exit. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if (currentContext)
    {
        fprintf(stderr, "Failed to create program exit. There is a nested "
                        "structure which has not been closed yet. (Line: %d)\n",
                        sourceLine);
        return 0;
    }
    if (lastWhileMarkerCodeLine > 0)
    {
        fprintf(stderr, "Failed to create program exit. There is an open WHILE "
                        "marker from definition in line %d. (Line: %d)\n",
                        lastWhileMarkerCodeLine, sourceLine);
        return 0;
    }
    
    codeEntry* entry = createCodeEntry(sourceLine, OP_EXIT, 0, result, 0, 0, 0,
                                       0);
    appendCodeEntry(entry);
    return 1;
}

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
                                    int sourceLine)
{
    if ((op != OP_PLUS) && (op != OP_MINUS) && (op != OP_MULTIPLY) &&
        (op != OP_DIVIDE) && (op != OP_MODULO))
    {
        fprintf(stderr, "Unexpected mathematical operation: %d. Expected "
                        "OP_PLUS, OP_MINUS, OP_MULTIPLY, OP_DIVIDE or OP_MODULO"
                        ". (Line: %d)\n", op, sourceLine);
        return 0;
    }
    if (!target)
    {
        fprintf(stderr, "No target has been given for mathematical operation. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if (!operand1)
    {
        fprintf(stderr, "Operand 1 missing for mathematical operation. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if ((operand1->type != INTEGER) && (operand1->type != REAL))
    {
        fprintf(stderr, "Operand 1 requires numeric data type for mathematical "
                        "operation. Got: %s. (Line: %d)\n",
                        getTypeName(operand1->type), sourceLine);
        return 0;
    }
    if ((op == OP_MODULO) && (operand1->type != INTEGER))
    {
        fprintf(stderr, "Operand 1 requires INTEGER data type for modulo "
                        "operation. Got: %s. (Line: %d)\n",
                        getTypeName(operand1->type), sourceLine);
        return 0;
    }
    if (!operand2)
    {
        fprintf(stderr, "Operand 2 missing for mathematical operation. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if ((operand2->type != INTEGER) && (operand2->type != REAL))
    {
        fprintf(stderr, "Operand 2 requires numeric data type for mathematical "
                        "operation. Got: %s. (Line: %d)\n",
                        getTypeName(operand2->type), sourceLine);
        return 0;
    }
    if ((op == OP_MODULO) && (operand2->type != INTEGER))
    {
        fprintf(stderr, "Operand 2 requires INTEGER data type for modulo "
                        "operation. Got: %s. (Line: %d)\n",
                        getTypeName(operand2->type), sourceLine);
        return 0;
    }
    
    dataType resultType = REAL;
    if ((operand1->type == INTEGER) && (operand2->type == INTEGER))
    {
        resultType = INTEGER;
    }
    
    if ((target->type != resultType) &&
        (target->type != REAL || resultType != INTEGER))
    {
        fprintf(stderr, "Result variable has incompatible data type. Got: "
                        "%s + %s. (Line: %d)\n", getTypeName(target->type), 
                        getTypeName(resultType), sourceLine);
        return 0;
    }
    
    codeEntry* entry = createCodeEntry(sourceLine, op, target, operand1,
                                       operand2, 0, 0, 0);
    appendCodeEntry(entry);
    return 1;
}

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
int createCodeIncrement(symbolTableEntry* target, operation op, int sourceLine)
{
    if ((op != OP_INCREMENT) && (op != OP_DECREMENT))
    {
        fprintf(stderr, "Unexpected increment operation: %d. Expected "
                        "OP_INCREMENT or OP_DECREMENT. (Line: %d)\n", op,
                        sourceLine);
        return 0;
    }
    if (!target)
    {
        fprintf(stderr, "No target has been given for increment. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if (target->type != INTEGER)
    {
        fprintf(stderr, "Expected data type INTEGER for increment. Got: %s. "
                        "(Line: %d)\n", getTypeName(target->type), sourceLine);
        return 0;
    }
    
    codeEntry* entry = createCodeEntry(sourceLine, op, target, 0, 0, 0, 0, 0);
    appendCodeEntry(entry);
    return 1;
}

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
                         int sourceLine)
{
    if (!target)
    {
        fprintf(stderr, "No target has been given for assignment."
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if (!source)
    {
        fprintf(stderr, "No source has been given for assignment."
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if ((target->type != source->type) &&
        (target->type != REAL || source->type != INTEGER))
    {
        fprintf(stderr, "Expected compatible data types for assignment. Got: "
                        "%s + %s. (Line: %d)\n", getTypeName(target->type),
                        getTypeName(source->type), sourceLine);
        return 0;
    }
    
    codeEntry* entry = createCodeEntry(sourceLine, OP_ASSIGN, target, source,
                                       0, 0, 0, 0);
    appendCodeEntry(entry);
    return 1;
}

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
int createCodeIntConst(symbolTableEntry* target, int value, int sourceLine)
{
    if (!target)
    {
        fprintf(stderr, "No target has been given for int constant. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if (target->type != INTEGER)
    {
        fprintf(stderr, "Expected data type INTEGER for int constant. Got: %s. "
                        "(Line: %d)\n", getTypeName(target->type), sourceLine);
        return 0;
    }
   
    codeEntry* entry = createCodeEntry(sourceLine, OP_INT_CONSTANT, target,
                                       0, 0, value, 0, 0);
    appendCodeEntry(entry);
    return 1;
}

/**
 * This creates the intermediate code for a float constant.
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
int createCodeFloatConst(symbolTableEntry* target, float value, int sourceLine)
{
    if (!target)
    {
        fprintf(stderr, "No target has been given for float constant. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if (target->type != REAL)
    {
        fprintf(stderr, "Expected data type REAL for float constant. Got: %s. "
                        "(Line: %d).", getTypeName(target->type), sourceLine);
        return 0;
    }
    
    codeEntry* entry = createCodeEntry(sourceLine, OP_FLOAT_CONSTANT, target,
                                       0, 0, 0, value, 0);
    appendCodeEntry(entry);
    return 1;
}

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
int createCodeBoolConst(symbolTableEntry* target, int value, int sourceLine)
{
    if (!target)
    {
        fprintf(stderr, "No target has been given for boolean constant. "
                        "(Line: %d)\n", sourceLine);
        return 0;
    }
    if (target->type != BOOLEAN)
    {
        fprintf(stderr, "Expected data type BOOLEAN for boolean constant. "
                        "Got: %s. (Line: %d).", getTypeName(target->type),
                        sourceLine);
        return 0;
    }
    
    codeEntry* entry = createCodeEntry(sourceLine, OP_BOOL_CONSTANT, target,
                                       0, 0, 0, 0, value);
    appendCodeEntry(entry);
    return 1;
}

/**
 * This writes the current intermediate code into a text file called 
 * <code>2_intermediate</code>.
 **/
void printCode()
{
    // Create the intermediate code (including backtracking)
    codeEntry* iterator = codeList;
    codeLineNumber = 0;
    lastWhileMarkerCodeLine = 0;
    
    while (iterator != 0)
    {
        printCodeEntry(iterator);
        iterator = iterator->next;
    }
    
    // Print the code to file
    FILE *f = fopen("2_intermediate", "w");
    fprintf(f, "== INTERMEDIATE CODE ==\n");
    codePrintEntry* iterator2 = printCodeList;
    while (iterator2 != 0)
    {
        if (strlen(iterator2->code) < 8)
        {
            fprintf(f, "L%d:\t%s\t\t\t\t[From: %d]\n", iterator2->lineNumber,
                    iterator2->code, iterator2->sourceLine);
        }
        else if (strlen(iterator2->code) < 16)
        {
            fprintf(f, "L%d:\t%s\t\t\t[From: %d]\n", iterator2->lineNumber,
                    iterator2->code, iterator2->sourceLine);
        }
        else
        {
            fprintf(f, "L%d:\t%s\t\t[From: %d]\n", iterator2->lineNumber,
                    iterator2->code, iterator2->sourceLine);
        }
        iterator2 = iterator2->next;
    }
    fprintf(f, "== INTERMEDIATE CODE ==\n");
    fclose(f);
}

/**
 * This appends a new code entry to the intermediate code print output.
 * @param code       The code print output to be added.
 * @param sourceLine The line number within the source file which relates to the
 *                   created intermediate code.<BR>
 *                   Note: This is used for output and debug purposes only. It
 *                   has no effect on the intermediate code or its execution.
 * @return The created intermediate code entry.
 */
codePrintEntry* appendPrintCodeEntry(char* code, int sourceLine)
{
    codePrintEntry* codeEntry =
        (codePrintEntry*) malloc(sizeof(codePrintEntry));
    codeEntry->code = code;
    codeEntry->lineNumber = ++codeLineNumber;
    codeEntry->sourceLine = sourceLine;
    codeEntry->next = 0;
    
    // Assign as new start for 1st entry
    if (!printCodeList)
    {
        printCodeList = codeEntry;
        currentPrintCodeList = codeEntry;
    }
    // Add at the end otherwise
    else
    {
        currentPrintCodeList->next = codeEntry;
        currentPrintCodeList = codeEntry;
    }
    
    return codeEntry;
}

/**
 * This creates the print output representation of an intermediate code
 * entry.<BR>
 * This includes GOTO statements and required backpatching.
 * @param iterator The code entry which shall be added to the print output.
 */
void printCodeEntry(codeEntry* iterator)
{
    char* codeSnippet = (char*)malloc(sizeof(char) * 100);
    int startLineNumber = codeLineNumber + 1;
    codeEntry* iterator2 = 0;
    
    // Copy the while marker to support nested while calls
    int lastWhileMarkerCodeLineLocal = lastWhileMarkerCodeLine;
    
    switch (iterator->op)
    {
        /* Numeric Comparison Operators */
        case OP_EQUAL:
            sprintf(codeSnippet, "%s := %s == %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;
        
        case OP_NOT_EQUAL:
            sprintf(codeSnippet, "%s := %s != %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;
        
        case OP_LESS_OR_EQUAL:
            sprintf(codeSnippet, "%s := %s <= %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;
        
        case OP_GREATER_OR_EQUAL:
            sprintf(codeSnippet, "%s := %s >= %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;
        
        case OP_GREATER:
            sprintf(codeSnippet, "%s := %s > %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;
        
        case OP_LESS:
            sprintf(codeSnippet, "%s := %s < %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;
        
        /* Logical Comparison Operators */
        case OP_AND:
            sprintf(codeSnippet, "%s := %s AND %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;
        
        case OP_OR:
            sprintf(codeSnippet, "%s := %s OR %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;
        
        case OP_NOT:
            sprintf(codeSnippet, "%s := NOT %s", iterator->target->name,
                    iterator->operand1->name);
            break;
        
        /* Control Flow */
        case OP_IF:
            // begin:  if true goto start
            //         goto end
            // start:  code body
            // end:
            if (iterator->sub_2 == 0)
            {
                sprintf(codeSnippet, "IF %s GOTO %d", iterator->operand1->name,
                        startLineNumber + 2);
                appendPrintCodeEntry(codeSnippet, iterator->sourceLine);
                
                codeSnippet = (char*)malloc(sizeof(char) * 100);
                sprintf(codeSnippet, "GOTO [LINE AFTER IF BODY]");
                codePrintEntry* entryFalse =
                    appendPrintCodeEntry(codeSnippet, iterator->sourceLine);
                
                // Print all sub code
                iterator2 = iterator->sub_1;
                
                while (iterator2 != 0)
                {
                    printCodeEntry(iterator2);
                    iterator2 = iterator2->next;
                }
                
                // Backpatch the GOTO target for the false part
                // to the line after the if body
                sprintf(entryFalse->code, "GOTO %d", codeLineNumber + 1);
            }
            // begin:  if true goto start
            //         goto else
            // start:  code body
            //         goto end
            // else:   code body for else
            // end:
            else
            {
                sprintf(codeSnippet, "IF %s GOTO %d", iterator->operand1->name,
                        startLineNumber + 2);
                appendPrintCodeEntry(codeSnippet, iterator->sourceLine);
                
                codeSnippet = (char*)malloc(sizeof(char) * 100);
                sprintf(codeSnippet, "GOTO [LINE AFTER IF BODY]");
                codePrintEntry* entryFalse =
                    appendPrintCodeEntry(codeSnippet, iterator->sourceLine);
                
                // Print all sub code for if
                iterator2 = iterator->sub_1;
                
                while (iterator2 != 0)
                {
                    printCodeEntry(iterator2);
                    iterator2 = iterator2->next;
                }
                
                codeSnippet = (char*)malloc(sizeof(char) * 100);
                sprintf(codeSnippet, "GOTO [LINE AFTER ELSE BODY]");
                codePrintEntry* entryEnd =
                    appendPrintCodeEntry(codeSnippet, iterator->sourceLine);
                
                // Backpatch the GOTO target for the false part
                // to the line after the if body
                sprintf(entryFalse->code, "GOTO %d", codeLineNumber + 1);
                
                // Print all sub code for else
                iterator2 = iterator->sub_2;
                
                while (iterator2 != 0)
                {
                    printCodeEntry(iterator2);
                    iterator2 = iterator2->next;
                }
                
                // Backpatch the GOTO target for the if part
                // to the line after the else body
                sprintf(entryEnd->code, "GOTO %d", codeLineNumber + 1);
            }
            
            // Ignore the general handling below
            return;
        
        case OP_WHILE:
            // begin:  if true goto start
            //         goto end
            // start:  code body
            //         goto begin
            // end:
            sprintf(codeSnippet, "IF %s GOTO %d", iterator->operand1->name,
                    startLineNumber + 2);
            appendPrintCodeEntry(codeSnippet, iterator->sourceLine);

            codeSnippet = (char*)malloc(sizeof(char) * 100);
            sprintf(codeSnippet, "GOTO [LINE AFTER WHILE BODY]");
            codePrintEntry* entryFalse =
                appendPrintCodeEntry(codeSnippet, iterator->sourceLine);

            // Print all sub code
            iterator2 = iterator->sub_1;

            while (iterator2 != 0)
            {
                printCodeEntry(iterator2);
                iterator2 = iterator2->next;
            }

            codeSnippet = (char*)malloc(sizeof(char) * 100);
            sprintf(codeSnippet, "GOTO %d", lastWhileMarkerCodeLineLocal);
            appendPrintCodeEntry(codeSnippet, iterator->sourceLine);

            // Backpatch the GOTO target for the false part
            // to the line after the while body
            sprintf(entryFalse->code, "GOTO %d", codeLineNumber + 1);

            // Ignore the general handling below
            return;

        case OP_MARKER_WHILE:
            // Remember current position
            lastWhileMarkerCodeLine = codeLineNumber + 1;

            // Ignore the general handling below
            return;

        case OP_EXIT:
            sprintf(codeSnippet, "RETURN %s", iterator->operand1->name);
            break;

        /* Mathematical Operators */
        case OP_PLUS:
            sprintf(codeSnippet, "%s := %s + %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;

        case OP_MINUS:
            sprintf(codeSnippet, "%s := %s - %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;

        case OP_MULTIPLY:
            sprintf(codeSnippet, "%s := %s * %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;

        case OP_DIVIDE:
            sprintf(codeSnippet, "%s := %s / %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;

        case OP_MODULO:
            sprintf(codeSnippet, "%s := %s %% %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            break;

        case OP_INCREMENT:
            sprintf(codeSnippet, "%s := %s + 1", iterator->target->name,
                    iterator->target->name);
            break;

        case OP_DECREMENT:
            sprintf(codeSnippet, "%s := %s - 1", iterator->target->name,
                    iterator->target->name);
            break;

        /* Assignment */
        case OP_ASSIGN:
            sprintf(codeSnippet, "%s := %s", iterator->target->name,
                    iterator->operand1->name);
            break;

        /* Numeric constants */
        case OP_INT_CONSTANT:
            sprintf(codeSnippet, "%s := %d", iterator->target->name,
                    iterator->integer);
            break;

        case OP_FLOAT_CONSTANT:
            sprintf(codeSnippet, "%s := %.2f", iterator->target->name,
                    iterator->real);
            break;

        case OP_BOOL_CONSTANT:
            sprintf(codeSnippet, "%s := %s", iterator->target->name,
                    getBooleanValue(iterator->boolean));
            break;

        /* Place holder for if/else/while */
        case OP_NOP:
            return;

        default:
            sprintf(codeSnippet, "ERROR: Unexpected operation: %u",iterator->op);
    }

    appendPrintCodeEntry(codeSnippet, iterator->sourceLine);
}

/**
 * Determines the display value of a boolean variable.
 * @param value boolean value
 * @return String representation of the boolean value.<BR>
 *         <code>false</code> for value 0.<BR>
 *         <code>true</code> for all other values.
 */
char* getBooleanValue(int value)
{
    if (!value)
    {
        return "false";
    }
    
    return "true";
}
