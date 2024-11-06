/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Symbol Tables Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Natalie Leung
Student ID: 201562361
Email:sc21nl@leeds.ac.uk
Date Work Commenced:20/05/2023
*************************************************************************/

#include "symbols.h"
#include <stdlib.h>
#include <string.h>

SymbolTable *createSymbolTable()
{
  SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
  table->symbols = NULL;
  table->capacity = 0;
  table->size = 0;
  return table;
}

void addSymbol(SymbolTable *table, char *identifier, IdentifierKind kind, char *type)
{
  if (table->size == table->capacity)
  {
    table->capacity = (table->capacity == 0) ? 1 : table->capacity * 2;
    table->symbols = (Symbol **)realloc(table->symbols, table->capacity * sizeof(Symbol *));
  }

  Symbol *symbol = (Symbol *)malloc(sizeof(Symbol));
  symbol->identifier = strdup(identifier);
  symbol->kind = kind;
  symbol->type = strdup(type);
  symbol->index = table->size;

  table->symbols[table->size++] = symbol;
}

Symbol *lookupSymbol(SymbolTable *table, char *identifier)
{
  for (int i = 0; i < table->size; ++i)
  {
    if (strcmp(table->symbols[i]->identifier, identifier) == 0)
    {
      return table->symbols[i];
    }
  }
  return NULL;
}

void freeSymbolTable(SymbolTable *table)
{
  for (int i = 0; i < table->size; ++i)
  {
    free(table->symbols[i]->identifier);
    free(table->symbols[i]->type);
    free(table->symbols[i]);
  }
  free(table->symbols);
  free(table);
}