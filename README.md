Lexical Analysis (Lexer): Implement a lexer to process JACK source files (.jack) by extracting tokens while ignoring whitespace and comments. It should be robust against errors like unexpected file endings.

Syntax Analysis (Parser): Create a recursive descent parser based on the JACK language grammar. The parser will validate syntax and halt on encountering an error, providing specific error messages.

Symbol Table: Implement a symbol table to manage identifiers like variable and method names, storing their properties for later use.

Code Generation: Generate virtual machine (VM) code compatible with the JACK VM emulator, with code generation embedded within the parser functions.
