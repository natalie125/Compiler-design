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

int InitCompiler()
{
  printf("Compiler initialized.\n");
  return 0; // You may return a different status based on your implementation
}

ParserInfo compile(char *dir_name)
{
  printf("Compiling %s.\n", dir_name);

  // As we're compiling a single file per ParserInfo, the dir_name should be the dir_name
  int initResult = InitParser(dir_name);
  if (initResult != 0)
  {
    printf("Failed to initialize parser.\n");
    ParserInfo errInfo;
    errInfo.er = syntaxError;
    return errInfo;
  }

  ParserInfo info = Parse(); // Parse the input file

  // Check for errors in parsing
  if (info.er != none)
  {
    printf("Compilation failed due to parser error: %s\n", ErrorString(info.er));
    PrintError(info);
  }
  else
  {
    printf("Compilation successful.\n");
  }

  StopCompiler();

  return info;
}

int StopCompiler()
{
  printf("Compiler stopped.\n");
  return StopParser(); // Clean up the parser
}

#ifdef TEST_COMPILER
int main()
{
  InitCompiler();
  char *test_file = "./test.jack"; // Replace with your test file
  compile(test_file);
  StopCompiler();
  return 0;
}
#endif
