/**
 * @file interpreter.c
 * @author Ramon Bisswanger
 * @version 1.1 (April 2015)
 * @brief This contains all function implementations for code execution.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "interpreter.h"
#include "generator.h"
#include "symboltable.h"
#include "compiler.h"

/**
 * Pointer to the first entry of the code list.<BR>
 * [defined in generator.c]
 */
extern codeEntry* codeList;

/**
 * Pointer to the first entry of the variable table.<BR>
 * This variable is automatically initialized when adding the first entry.
 */
variableTableEntry* variableTable = 0;

/**
 * Reference to the previous WHILE line marker.<BR>
 * Markers are required for WHILE loops as the condition part might have to be
 * recalculated for every loop in case any variable contained has been changed
 * within the loop body.
 */
codeEntry* lastWhileMarker = 0;

/**
 * This variable holds the value of the RETURN statement after program execution
 * has been completed.<BR>
 * The value is converted to a string based on its data type.
 */
char programResult[200];

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
variableTableEntry* addEntryToVariableTable(symbolTableEntry* variable)
{
    if (!variable)
    {
        fprintf(stderr, "Call to addEntryToVariableTable failed: "
                        "No variable reference given.");
        return 0;
    }
    
    // Allocate required memory
    variableTableEntry* newVartabEntry =
        (variableTableEntry*) malloc(sizeof(variableTableEntry));
    newVartabEntry->variable = variable;
    newVartabEntry->next = 0;
    
    // Assign as new symbol table for 1st entry
    if (!variableTable)
    {
        variableTable = newVartabEntry;
    }
    // Add at the end otherwise
    else
    {
        variableTableEntry* iterator = variableTable;
        while (iterator->next)
        {
            iterator = iterator->next;
        }
        iterator->next = newVartabEntry;
    }
    
    return newVartabEntry;
}

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
variableTableEntry* getEntryFromVariableTable(symbolTableEntry* variable)
{
    // Loop until all entries in the symbol table and check the name
    variableTableEntry* iterator = variableTable;
    
    while ((iterator != 0) && (iterator->variable != variable))
    {
        iterator = iterator->next;
    }
    
    return iterator;
}

/**
 * This executes the intermediate code and writes all steps into a text file
 * called <code>3_execution</code>.<BR>
 * The variable table at program exit is written to file
 * <code>4_variabletable</code>.<BR>
 * The return value of the program is stored in variable programResult and
 * printed on screen.
 **/
void runCode()
{
    codeEntry* iterator = codeList;
    
    FILE *f = fopen("3_execution", "w");
    fprintf(f, "== CODE EXECUTION ==\n");
    
    while (iterator != 0)
    {
        runCodeEntry(iterator, f, "");
        iterator = iterator->next;
    }
    
    fprintf(f, "== CODE EXECUTION ==\n");
    fclose(f);
    
    // Write variable state at end of program
    f = fopen("4_variabletable", "w");
    fprintf(f, "== VARIABLE TABLE ==\n");
    fprintf(f, " Name\tType\tValue\n");
    
    variableTableEntry* iterator2 = variableTable;
    
    while (iterator2 != 0)
    {
        switch (iterator2->variable->type)
        {
            case INTEGER:
                fprintf(f, " %s\tINTEGER\t%d\n", iterator2->variable->name,
                        iterator2->value.intValue);
                break;
            case REAL:
                fprintf(f, " %s\tREAL\t%.2f\n", iterator2->variable->name,
                        iterator2->value.floatValue);
                break;
            case BOOLEAN:
                fprintf(f, " %s\tBOOLEAN\t%s\n", iterator2->variable->name,
                        getBooleanValue(iterator2->value.boolValue));
                break;
            default:
                fprintf(f, "-UNKNOWN-");
        }
        iterator2 = iterator2->next;
    }
    
    fprintf(f, "== VARIABLE TABLE ==\n");
    fclose(f);
    
    printf("\nPROGRAM RESULT = %s\n", programResult);
}

/**
 * This executes a single code statement (including nested sub-code).<BR>
 * @param iterator The code entry which shall be executed.
 * @param f        Reference to the file for storing the execution output.
 * @param indent   Current indentation to visualize nesting within the execution
 *                 output.
 */
void runCodeEntry(codeEntry* iterator, FILE *f, char* indent)
{
    codeEntry* iterator2 = 0;
    
    // Copy the while marker to support nested while calls
    codeEntry* lastWhileMarkerLocal = lastWhileMarker;
    
    char* sub_indent = (char*)malloc(sizeof(char)*(strlen(indent)+2));
    sprintf(sub_indent, "%s  ", indent);
    
    // Read entries from variable table (if existing)
    variableTableEntry* val_target = 0;
    variableTableEntry* val_op1 = 0;
    variableTableEntry* val_op2 = 0;
    if (iterator->target != 0)
    {
        val_target = getEntryFromVariableTable(iterator->target);
    }
    if (iterator->operand1 != 0)
    {
        val_op1 = getEntryFromVariableTable(iterator->operand1);
    }
    if (iterator->operand2 != 0)
    {
        val_op2 = getEntryFromVariableTable(iterator->operand2);
    }
    
    if (iterator->op != OP_MARKER_WHILE && iterator->op != OP_NOP)
    {
        fprintf(f, "%s", indent);
    }
    
    switch (iterator->op)
    {
        /* Numeric Comparison Operators */
        case OP_EQUAL:
            fprintf(f, "%s := %s == %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            if ((iterator->operand1->type == INTEGER)
                && (iterator->operand2->type == INTEGER))
            {
                val_target->value.boolValue = (val_op1->value.intValue
                                               == val_op2->value.intValue);
                fprintf(f, " := %d == %d", val_op1->value.intValue,
                        val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == INTEGER)
                     && (iterator->operand2->type == REAL))
            {
                val_target->value.boolValue = (val_op1->value.intValue
                                               == val_op2->value.floatValue);
                fprintf(f, " := %d == %.2f", val_op1->value.intValue,
                        val_op2->value.floatValue);
            }
            else if ((iterator->operand1->type == REAL)
                     && (iterator->operand2->type == INTEGER))
            {
                val_target->value.boolValue = (val_op1->value.floatValue
                                               == val_op2->value.intValue);
                fprintf(f, " := %.2f == %d", val_op1->value.floatValue,
                        val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == REAL)
                     && (iterator->operand2->type == REAL))
            {
                val_target->value.boolValue = (val_op1->value.floatValue
                                               == val_op2->value.floatValue);
                fprintf(f, " := %.2f == %.2f", val_op1->value.floatValue,
                        val_op2->value.floatValue);
            }
            fprintf(f, " := %s", getBooleanValue(val_target->value.boolValue));
            fprintf(f, "\n");
            break;
        
        case OP_NOT_EQUAL:
            fprintf(f, "%s := %s != %s", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name);
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.boolValue = val_op1->value.intValue != val_op2->value.intValue;
                fprintf(f, " := %d != %d", val_op1->value.intValue, val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == REAL))
            {
                val_target->value.boolValue = val_op1->value.intValue != val_op2->value.floatValue;
                fprintf(f, " := %d != %.2f", val_op1->value.intValue, val_op2->value.floatValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.boolValue = val_op1->value.floatValue != val_op2->value.intValue;
                fprintf(f, " := %.2f != %d", val_op1->value.floatValue, val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == REAL))
            {
                val_target->value.boolValue = val_op1->value.floatValue != val_op2->value.floatValue;
                fprintf(f, " := %.2f != %.2f", val_op1->value.floatValue, val_op2->value.floatValue);
            }
            fprintf(f, " := %s", getBooleanValue(val_target->value.boolValue));
            fprintf(f, "\n");
            break;
        
        case OP_LESS_OR_EQUAL:
            fprintf(f, "%s := %s <= %s", iterator->target->name, iterator->operand1->name, iterator->operand2->name);
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.boolValue = val_op1->value.intValue <= val_op2->value.intValue;
                fprintf(f, " := %d <= %d", val_op1->value.intValue, val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == REAL))
            {
                val_target->value.boolValue = val_op1->value.intValue <= val_op2->value.floatValue;
                fprintf(f, " := %d <= %.2f", val_op1->value.intValue, val_op2->value.floatValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.boolValue = val_op1->value.floatValue <= val_op2->value.intValue;
                fprintf(f, " := %.2f <= %d", val_op1->value.floatValue, val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == REAL))
            {
                val_target->value.boolValue = val_op1->value.floatValue <= val_op2->value.floatValue;
                fprintf(f, " := %.2f <= %.2f", val_op1->value.floatValue, val_op2->value.floatValue);
            }
            fprintf(f, " := %s", getBooleanValue(val_target->value.boolValue));
            fprintf(f, "\n");
            break;
        
        case OP_GREATER_OR_EQUAL:
            fprintf(f, "%s := %s >= %s", iterator->target->name, iterator->operand1->name, iterator->operand2->name);
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.boolValue = val_op1->value.intValue >= val_op2->value.intValue;
                fprintf(f, " := %d >= %d", val_op1->value.intValue, val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == REAL))
            {
                val_target->value.boolValue = val_op1->value.intValue >= val_op2->value.floatValue;
                fprintf(f, " := %d >= %.2f", val_op1->value.intValue, val_op2->value.floatValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.boolValue = val_op1->value.floatValue >= val_op2->value.intValue;
                fprintf(f, " := %.2f >= %d", val_op1->value.floatValue, val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == REAL))
            {
                val_target->value.boolValue = val_op1->value.floatValue >= val_op2->value.floatValue;
                fprintf(f, " := %.2f >= %.2f", val_op1->value.floatValue, val_op2->value.floatValue);
            }
            fprintf(f, " := %s", getBooleanValue(val_target->value.boolValue));
            fprintf(f, "\n");
            break;
        
        case OP_GREATER:
            fprintf(f, "%s := %s > %s", iterator->target->name, iterator->operand1->name, iterator->operand2->name);
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.boolValue = val_op1->value.intValue > val_op2->value.intValue;
                fprintf(f, " := %d > %d", val_op1->value.intValue, val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == REAL))
            {
                val_target->value.boolValue = val_op1->value.intValue > val_op2->value.floatValue;
                fprintf(f, " := %d > %.2f", val_op1->value.intValue, val_op2->value.floatValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.boolValue = val_op1->value.floatValue > val_op2->value.intValue;
                fprintf(f, " := %.2f > %d", val_op1->value.floatValue, val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == REAL))
            {
                val_target->value.boolValue = val_op1->value.floatValue > val_op2->value.floatValue;
                fprintf(f, " := %.2f > %.2f", val_op1->value.floatValue, val_op2->value.floatValue);
            }
            fprintf(f, " := %s", getBooleanValue(val_target->value.boolValue));
            fprintf(f, "\n");
            break;
        
        case OP_LESS:
            fprintf(f, "%s := %s < %s", iterator->target->name, iterator->operand1->name, iterator->operand2->name);
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.boolValue = val_op1->value.intValue < val_op2->value.intValue;
                fprintf(f, " := %d < %d", val_op1->value.intValue, val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == REAL))
            {
                val_target->value.boolValue = val_op1->value.intValue < val_op2->value.floatValue;
                fprintf(f, " := %d < %.2f", val_op1->value.intValue, val_op2->value.floatValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.boolValue = val_op1->value.floatValue < val_op2->value.intValue;
                fprintf(f, " := %.2f < %d", val_op1->value.floatValue, val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == REAL))
            {
                val_target->value.boolValue = val_op1->value.floatValue < val_op2->value.floatValue;
                fprintf(f, " := %.2f < %.2f", val_op1->value.floatValue, val_op2->value.floatValue);
            }
            fprintf(f, " := %s", getBooleanValue(val_target->value.boolValue));
            fprintf(f, "\n");
            break;
        
        /* Logical Comparison Operators */
        case OP_AND:
            fprintf(f, "%s := %s AND %s := %s AND %s", iterator->target->name, iterator->operand1->name, iterator->operand2->name, getBooleanValue(val_op1->value.boolValue), getBooleanValue(val_op2->value.boolValue));
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            val_target->value.boolValue = val_op1->value.boolValue && val_op2->value.boolValue;
            fprintf(f, " := %s", getBooleanValue(val_target->value.boolValue));
            fprintf(f, "\n");
            break;
        
        case OP_OR:
            fprintf(f, "%s := %s OR %s := %s OR %s", iterator->target->name, iterator->operand1->name, iterator->operand2->name, getBooleanValue(val_op1->value.boolValue), getBooleanValue(val_op2->value.boolValue));
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            val_target->value.boolValue = val_op1->value.boolValue || val_op2->value.boolValue;
            fprintf(f, " := %s", getBooleanValue(val_target->value.boolValue));
            fprintf(f, "\n");
            break;
        
        case OP_NOT:
            fprintf(f, "%s := NOT %s := NOT %s", iterator->target->name, iterator->operand1->name, getBooleanValue(val_op1->value.boolValue));
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            val_target->value.boolValue = !val_op1->value.boolValue;
            fprintf(f, " := %s", getBooleanValue(val_target->value.boolValue));
            fprintf(f, "\n");
            break;
        
        /* Control Flow */
        case OP_IF:
            fprintf(f, "IF %s := %s\n", iterator->operand1->name, getBooleanValue(val_op1->value.boolValue));
            if (val_op1->value.boolValue)
            {
                // Execute all sub code
                iterator2 = iterator->sub_1;
                
                while (iterator2 != 0)
                {
                    runCodeEntry(iterator2, f, sub_indent);
                    iterator2 = iterator2->next;
                }
            }
            else if (iterator->sub_2 != 0)
            {
                // Execute all sub code
                iterator2 = iterator->sub_2;
                
                while (iterator2 != 0)
                {
                    runCodeEntry(iterator2, f, sub_indent);
                    iterator2 = iterator2->next;
                }
            }
            break;
        
        case OP_WHILE:
            fprintf(f, "WHILE %s := %s\n", iterator->operand1->name, getBooleanValue(val_op1->value.boolValue));
            
            while (val_op1->value.boolValue)
            {
                // Execute all sub code
                iterator2 = iterator->sub_1;
                
                while (iterator2 != 0)
                {
                    runCodeEntry(iterator2, f, sub_indent);
                    iterator2 = iterator2->next;
                }
                
                // Execute everything which is part of the condition again
                iterator2 = lastWhileMarkerLocal;
                while (iterator2 != iterator)
                {
                    runCodeEntry(iterator2, f, sub_indent);
                    iterator2 = iterator2->next;
                }
                
                fprintf(f, "%sWHILE %s := %s\n", indent, iterator->operand1->name, getBooleanValue(val_op1->value.boolValue));
            }
            break;
        
        case OP_MARKER_WHILE:
            // Remember current position
            lastWhileMarker = iterator;
            break;
        
        case OP_EXIT:
            fprintf(f, "RETURN %s", iterator->operand1->name);
            switch (iterator->operand1->type)
            {
                case INTEGER:
                    fprintf(f, " := %d\n", val_op1->value.intValue);
                    sprintf(programResult, "%d", val_op1->value.intValue);
                    break;
                case REAL:
                    fprintf(f, " := %.2f\n", val_op1->value.floatValue);
                    sprintf(programResult, "%.2f", val_op1->value.floatValue);
                    break;
                case BOOLEAN:
                    fprintf(f, " := %s\n", getBooleanValue(val_op1->value.boolValue));
                    sprintf(programResult, "%s", getBooleanValue(val_op1->value.boolValue));
                    break;
            }
            break;
        
        /* Mathematical Operators */
        case OP_PLUS:
            fprintf(f, "%s := %s + %s", iterator->target->name, iterator->operand1->name, iterator->operand2->name);
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.intValue = val_op1->value.intValue + val_op2->value.intValue;
                fprintf(f, " := %d + %d := %d", val_op1->value.intValue, val_op2->value.intValue, val_op1->value.intValue + val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == REAL))
            {
                val_target->value.floatValue = val_op1->value.intValue + val_op2->value.floatValue;
                fprintf(f, " := %d + %.2f := %.2f", val_op1->value.intValue, val_op2->value.floatValue, val_op1->value.intValue + val_op2->value.floatValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.floatValue = val_op1->value.floatValue + val_op2->value.intValue;
                fprintf(f, " := %.2f + %d := %.2f", val_op1->value.floatValue, val_op2->value.intValue, val_op1->value.floatValue + val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == REAL))
            {
                val_target->value.floatValue = val_op1->value.floatValue + val_op2->value.floatValue;
                fprintf(f, " := %.2f + %.2f := %.2f", val_op1->value.floatValue, val_op2->value.floatValue, val_op1->value.floatValue + val_op2->value.floatValue);
            }
            fprintf(f, "\n");
            break;
        
        case OP_MINUS:
            fprintf(f, "%s := %s - %s", iterator->target->name, iterator->operand1->name, iterator->operand2->name);
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.intValue = val_op1->value.intValue - val_op2->value.intValue;
                fprintf(f, " := %d - %d := %d", val_op1->value.intValue, val_op2->value.intValue, val_op1->value.intValue - val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == REAL))
            {
                val_target->value.floatValue = val_op1->value.intValue - val_op2->value.floatValue;
                fprintf(f, " := %d - %.2f := %.2f", val_op1->value.intValue, val_op2->value.floatValue, val_op1->value.intValue - val_op2->value.floatValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.floatValue = val_op1->value.floatValue - val_op2->value.intValue;
                fprintf(f, " := %.2f - %d := %.2f", val_op1->value.floatValue, val_op2->value.intValue, val_op1->value.floatValue - val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == REAL))
            {
                val_target->value.floatValue = val_op1->value.floatValue - val_op2->value.floatValue;
                fprintf(f, " := %.2f - %.2f := %.2f", val_op1->value.floatValue, val_op2->value.floatValue, val_op1->value.floatValue - val_op2->value.floatValue);
            }
            fprintf(f, "\n");
            break;
        
        case OP_MULTIPLY:
            fprintf(f, "%s := %s * %s", iterator->target->name, iterator->operand1->name, iterator->operand2->name);
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.intValue = val_op1->value.intValue * val_op2->value.intValue;
                fprintf(f, " := %d * %d := %d", val_op1->value.intValue, val_op2->value.intValue, val_op1->value.intValue * val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == REAL))
            {
                val_target->value.floatValue = val_op1->value.intValue * val_op2->value.floatValue;
                fprintf(f, " := %d * %.2f := %.2f", val_op1->value.intValue, val_op2->value.floatValue, val_op1->value.intValue * val_op2->value.floatValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.floatValue = val_op1->value.floatValue * val_op2->value.intValue;
                fprintf(f, " := %.2f * %d := %.2f", val_op1->value.floatValue, val_op2->value.intValue, val_op1->value.floatValue * val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == REAL))
            {
                val_target->value.floatValue = val_op1->value.floatValue * val_op2->value.floatValue;
                fprintf(f, " := %.2f * %.2f := %.2f", val_op1->value.floatValue, val_op2->value.floatValue, val_op1->value.floatValue * val_op2->value.floatValue);
            }
            fprintf(f, "\n");
            break;
        
        case OP_DIVIDE:
            fprintf(f, "%s := %s / %s", iterator->target->name, iterator->operand1->name, iterator->operand2->name);
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.intValue = val_op1->value.intValue / val_op2->value.intValue;
                fprintf(f, " := %d / %d := %d", val_op1->value.intValue, val_op2->value.intValue, val_op1->value.intValue / val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == INTEGER) && (iterator->operand2->type == REAL))
            {
                val_target->value.floatValue = val_op1->value.intValue / val_op2->value.floatValue;
                fprintf(f, " := %d / %.2f := %.2f", val_op1->value.intValue, val_op2->value.floatValue, val_op1->value.intValue / val_op2->value.floatValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == INTEGER))
            {
                val_target->value.floatValue = val_op1->value.floatValue / val_op2->value.intValue;
                fprintf(f, " := %.2f / %d := %.2f", val_op1->value.floatValue, val_op2->value.intValue, val_op1->value.floatValue / val_op2->value.intValue);
            }
            else if ((iterator->operand1->type == REAL) && (iterator->operand2->type == REAL))
            {
                val_target->value.floatValue = val_op1->value.floatValue / val_op2->value.floatValue;
                fprintf(f, " := %.2f / %.2f := %.2f", val_op1->value.floatValue, val_op2->value.floatValue, val_op1->value.floatValue / val_op2->value.floatValue);
            }
            fprintf(f, "\n");
            break;
        
        case OP_MODULO:
            fprintf(f, "%s := %s %% %s := %d %% %d", iterator->target->name,
                    iterator->operand1->name, iterator->operand2->name,
                    val_op1->value.intValue, val_op2->value.intValue);
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            val_target->value.intValue = (val_op1->value.intValue
                                          % val_op2->value.intValue);
            fprintf(f, " := %d", val_target->value.intValue);
            fprintf(f, "\n");
            break;
        
        case OP_INCREMENT:
            fprintf(f, "%s := %s + 1 := %d + 1 := %d", iterator->target->name,
                    iterator->target->name, val_target->value.intValue,
                    val_target->value.intValue + 1);
            fprintf(f, "\n");
            val_target->value.intValue++;
            break;
        
        case OP_DECREMENT:
            fprintf(f, "%s := %s - 1 := %d - 1 := %d", iterator->target->name,
                    iterator->target->name, val_target->value.intValue,
                    val_target->value.intValue - 1);
            fprintf(f, "\n");
            val_target->value.intValue--;
            break;
        
        /* Assignment */
        case OP_ASSIGN:
            fprintf(f, "%s := %s", iterator->target->name,
                    iterator->operand1->name);
            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }
            switch (iterator->operand1->type)
            {
                case INTEGER:
                    fprintf(f, " := %d\n", val_op1->value.intValue);
                    
                    if (iterator->target->type == REAL)
                    {
                        val_target->value.floatValue = val_op1->value.intValue;
                    }
                    else
                    {
                        val_target->value.intValue = val_op1->value.intValue;
                    }
                    break;
                case REAL:
                    fprintf(f, " := %.2f\n", val_op1->value.floatValue);
                    val_target->value.floatValue = val_op1->value.floatValue;
                    break;
                case BOOLEAN:
                    fprintf(f, " := %s\n",
                            getBooleanValue(val_op1->value.boolValue));
                    val_target->value.boolValue = val_op1->value.boolValue;
                    break;
            }
            break;

        /* Constants */
        case OP_INT_CONSTANT:
            fprintf(f, "%s := %d", iterator->target->name, iterator->integer);
            fprintf(f, "\n");

            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }

            val_target->value.intValue = iterator->integer;
            break;

        case OP_FLOAT_CONSTANT:
            fprintf(f, "%s := %.2f", iterator->target->name, iterator->real);
            fprintf(f, "\n");

            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }

            val_target->value.floatValue = iterator->real;
            break;

        case OP_BOOL_CONSTANT:
            fprintf(f, "%s := %s", iterator->target->name,
                    getBooleanValue(iterator->boolean));
            fprintf(f, "\n");

            if (val_target == 0)
            {
                val_target = addEntryToVariableTable(iterator->target);
            }

            val_target->value.boolValue = iterator->boolean;
            break;

        /* Place holder for if/else/while */
        case OP_NOP:
            return;

        default:
            fprintf(f, "ERROR: Unexpected operation: %u",iterator->op);
    }
}
