#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "lexer.h"
#include "parser.h"

typedef enum {CLASS, SUBROUTINE, VAR} IdentifierKind;

typedef struct {
    char* identifier;
    IdentifierKind kind;
    char* type;
    int index;
} Symbol;

typedef struct {
    Symbol** symbols;
    int capacity;
    int size;
} SymbolTable;

SymbolTable* createSymbolTable();
void addSymbol(SymbolTable* table, char* identifier, IdentifierKind kind, char* type);
Symbol* lookupSymbol(SymbolTable* table, char* identifier);
void freeSymbolTable(SymbolTable* table);

#endif
