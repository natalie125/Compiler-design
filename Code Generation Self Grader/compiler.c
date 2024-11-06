/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Compiler Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Natalie Leung
Student ID: 201562361
Email: sc21nl@leeds.ac.uk
Date Work Commenced:
*************************************************************************/

#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>

// Global variables, adjust as per your requirements
SymbolTable *symbolTable;
ParserInfo parserInfo;

// Function to initialize the compiler
int InitCompiler()
{
  // Initialization code here
  // Initialize symbol table, open files etc.
  // The details will depend on your existing implementation
  symbolTable = createSymbolTable();
  return 1;
}

// Function to compile the code in the given directory
ParserInfo compile(char *dir_name)
{
  // Compiler logic here
  // This will depend heavily on your existing implementation
  // This function should return a ParserInfo struct that encapsulates the result of the compilation

  ParserInfo parserInfo;
  parserInfo.er = none; // this assumes that 'none' is an enum value indicating no error
  // You should adjust the initial values of parserInfo according to your implementation

  // TODO: add logic to actually compile the code

  return parserInfo;
}

// Function to stop the compiler and cleanup
int StopCompiler()
{
  // Cleanup code here
  // Close files, free memory, etc.
  // The details will depend on your existing implementation
  return 1;
}

// #ifdef TEST_COMPILER
// int main(int argc, char *argv[])
// {
//   // You can add code here to test the functions of the compiler
//   // For example:

//   char *testDirectory = "testDirectory"; // replace with the directory you want to compile
//   InitCompiler();
//   ParserInfo result = compile(testDirectory);
//   // Then you can add code to inspect the result and print some output or errors
//   StopCompiler();

//   return 0;
// }
// #endif
