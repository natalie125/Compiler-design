/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
Lexer Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Natalie Leung
Student ID: 201562361
Email: sc21nl@leeds.ac.uk
Date Work Commenced: 13/05/2023
*************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"


// YOU CAN ADD YOUR OWN FUNCTIONS, DECLARATIONS AND VARIABLES HERE
#include <dirent.h>
FILE *source_file;
static int line_number = 1;
int isKeyword(char *buffer) {
  const char *keywords[] = {"class", "constructor", "function", "method", "field", "static", "var", "int", "char", "boolean", "void", "true", "false", "null", "this", "let", "do", "if", "else", "while", "return"};
  for (int i = 0; i < 21; i++) {
    if (strcmp(buffer, keywords[i]) == 0) {
      return 1;
    }
  }
  return 0;
}



// IMPLEMENT THE FOLLOWING functions
//***********************************

// Initialise the lexer to read from source file
// file_name is the name of the source file
// This requires opening the file and making any necessary initialisations of the lexer
// If an error occurs, the function should return 0
// if everything goes well the function should return 1
int InitLexer(char *file_name) {
  source_file = fopen(file_name, "r");
  if (!source_file) {
    return 0;
  }

  line_number = 1;
  return 1;
}


// Get the next token from the source file
Token GetNextToken() {
  Token t;
  char current_char;
  char buffer[256]; // assuming max token length is 256
  int buffer_index = 0;
  int is_comment = 0;

  while ((current_char = fgetc(source_file)) != EOF) {
    // Track line numbers
    if (current_char == '\n') {
      line_number++;
      continue;
    }

    // Handle whitespace
    if (isspace(current_char)) {
      continue;
    }

    // Handle identifiers, keywords
    if (isalpha(current_char) || current_char == '_') {
      do {
        buffer[buffer_index++] = current_char;
        current_char = fgetc(source_file);
      } while (isalnum(current_char) || current_char == '_');
      buffer[buffer_index] = '\0'; 

      if (isKeyword(buffer)) {
        t.tp = RESWORD;
      } else {
        t.tp = ID;
      }
      strcpy(t.lx, buffer);
      t.ln = line_number;
      ungetc(current_char, source_file); 
    }
    // Handle integers
    else if (isdigit(current_char)) {
      do {
        buffer[buffer_index++] = current_char;
        current_char = fgetc(source_file);
      } while (isdigit(current_char));
      buffer[buffer_index] = '\0';

      t.tp = INT;
      strcpy(t.lx, buffer);
      t.ln = line_number;
      ungetc(current_char, source_file);
    }
    // Handle string constants
    else if (current_char == '\"') {
      current_char = fgetc(source_file); // consume the opening quote
      while (current_char != '\"' && current_char != EOF) {
        if (current_char == '\n') {
          // Error: new line in string constant
          t.tp = ERR;
          strcpy(t.lx, "Error: new line in string constant");
          t.ln = line_number;
          return t;
        }
        buffer[buffer_index++] = current_char;
        current_char = fgetc(source_file);
      }
      buffer[buffer_index] = '\0';

      if (current_char == EOF) {
        // Error: unexpected eof in string constant
        t.tp = ERR;
        strcpy(t.lx, "Error: unexpected eof in string constant");
        t.ln = line_number;
        return t;
      } else {
        t.tp = STRING;
        strcpy(t.lx, buffer);
        t.ln = line_number;
      }
    }
    // Handle symbols and comments
    else if (strchr("{}()[].,;+-*/&|<>=~", current_char) || current_char == '/') {
      if (current_char == '/') {
        char next_char = fgetc(source_file);
        if (next_char == '/') { // single-line comment
          while ((current_char = fgetc(source_file)) != '\n' && current_char != EOF);
          if (current_char == '\n') {
            line_number++;
          }
          continue;
        }
        else if (next_char == '*') { // multi-line comment
          do {
            if (current_char == '\n') {
              line_number++;
            }
            while ((current_char = fgetc(source_file)) != '*' && current_char != EOF) {
              if (current_char == '\n') {
                line_number++;
              }
            }
            if (current_char == EOF) {
              // Error: unexpected eof in comment
              t.tp = ERR;
              strcpy(t.lx, "Error: unexpected eof in comment");
              t.ln = line_number;
              return t;
            }
            current_char = fgetc(source_file);
          } while (current_char != '/' && current_char != EOF);
          if (current_char == EOF) {
            // Error: unexpected eof in comment
            t.tp = ERR;
            strcpy(t.lx, "Error: unexpected eof in comment");
            t.ln = line_number;
            return t;
          }
          continue;
        }
        else { // '/' symbol
          ungetc(next_char, source_file); 
        }
      }
      // if not comment, then it's a symbol
      buffer[0] = current_char;
      buffer[1] = '\0';
      t.tp = SYMBOL;
      strcpy(t.lx, buffer);
      t.ln = line_number;
    }
    // Handle illegal symbols
    else {
      // Error: illegal symbol in source file
      t.tp = ERR;
      strcpy(t.lx, "Error: illegal symbol in source file");
      t.ln = line_number;
      return t;
    }

    return t;
  }

  // if we get here, we have reached EOF
  t.tp = EOFile;
  strcpy(t.lx, "End of File");
  t.ln = line_number;
  return t;
}



// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken() {
    long position = ftell(source_file);
    int saved_line_number = line_number; // save the current line number
    Token t = GetNextToken();
    fseek(source_file, position, SEEK_SET);
    line_number = saved_line_number; // restore the line number
    return t;
}


// clean out at end, e.g. close files, free memory, ... etc
int StopLexer() {
  if (source_file) {
    fclose(source_file);
    source_file = NULL;
    line_number = 1;
  }
  return 0;
}

// do not remove the next line
#ifndef TEST

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    char *dir_name = argv[1];
    DIR *dir = opendir(dir_name);
    if (!dir) {
        printf("Error opening directory: %s\n", dir_name);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char *ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".jack") == 0) {
            char file_path[256];
            sprintf(file_path, "%s/%s", dir_name, entry->d_name);

            Token token;
            if (InitLexer(file_path)) {
                printf("Reading tokens from file: %s\n", file_path);

                do {
                    token = GetNextToken();
                    if (token.tp != EOFile && token.tp != ERR) {
                        printf("Token type: %d, Lexeme: %s, Line number: %d\n", token.tp, token.lx, token.ln);
                    } else if (token.tp == ERR) {
                        printf("Error, line %d, close to \"%s\", error code: %d\n", token.ln, token.lx, token.ec);
                        break;
                    }
                } while (token.tp != EOFile);

                StopLexer();
            } else {
                printf("Error opening file: %s\n", file_path);
            }
        }
    }

    closedir(dir);
    return 0;
}
// do not remove the next line
#endif
