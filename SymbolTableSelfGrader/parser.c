#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"
#include <dirent.h>
#define true 1
#define false 0

// Prototypes of parser functions
ParserInfo ParseClass();
ParserInfo ParseClassVarDec();
ParserInfo ParseSubroutineDec();
ParserInfo ParseParameterList();
ParserInfo ParseStatement();
ParserInfo ParseLetStatement();
ParserInfo ParseVarStatement();
ParserInfo ParseIfStatement();
ParserInfo ParseWhileStatement();
ParserInfo ParseDoStatement();
ParserInfo ParseReturnStatement();
ParserInfo ParseExpression();
ParserInfo ParseFunctionCall();
ParserInfo ParseSubroutineCall();

// Initialize the lexer
int InitParser(char *file_name)
{
  return InitLexer(file_name);
}

ParserInfo Parse()
{
  return ParseClass();
}

ParserInfo ParseClass()
{
  ParserInfo pi;
  Token tk = GetNextToken();

  if (strcmp(tk.lx, "Error: new line in string constant") == 0)
  {
    pi.er = lexerErr;
    pi.tk = tk;
    return pi;
  }

  // Check for 'class' keyword
  if (tk.tp != RESWORD || strcmp(tk.lx, "class") != 0)
  {
    pi.er = classExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for identifier after 'class' keyword
  tk = GetNextToken();
  if (tk.tp != ID)
  {
    pi.er = idExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for opening brace
  tk = GetNextToken();
  if (tk.tp != SYMBOL || strcmp(tk.lx, "{") != 0)
  {
    pi.er = openBraceExpected;
    pi.tk = tk;
    return pi;
  }

  // Parse class variable declarations and subroutine declarations
  while (1)
  {
    tk = PeekNextToken();
    if (tk.tp == RESWORD && (strcmp(tk.lx, "static") == 0 || strcmp(tk.lx, "field") == 0))
    {
      pi = ParseClassVarDec();
    }
    else if (tk.tp == RESWORD && (strcmp(tk.lx, "constructor") == 0 || strcmp(tk.lx, "function") == 0 || strcmp(tk.lx, "method") == 0))
    {
      pi = ParseSubroutineDec();
    }
    else
    {
      break;
    }

    if (pi.er != none)
    {
      return pi;
    }
  }

  // Check for closing brace
  tk = GetNextToken();
  if (tk.tp != SYMBOL || strcmp(tk.lx, "}") != 0)
  {
    pi.er = closeBraceExpected;
    pi.tk = tk;
    return pi;
  }

  // If no errors are encountered, set er to none
  pi.er = none;
  pi.tk = tk;
  return pi;
}

int isOperator(const char *symbol)
{
  // List of valid operators
  const char *operators[] = {
      "+", "-", "*", "/", "&", "|", "<", ">", "=", "~"};
  int numOperators = sizeof(operators) / sizeof(operators[0]);

  for (int i = 0; i < numOperators; i++)
  {
    if (strcmp(symbol, operators[i]) == 0)
    {
      return 1; // Return true if the symbol is a valid operator
    }
  }

  return 0; // Return false if the symbol is not a valid operator
}

ParserInfo ParseExpression()
{
  ParserInfo pi;
  Token tk = GetNextToken();

  // If the token is a unary operator like '~'
  if (tk.tp == SYMBOL && isOperator(tk.lx))
  {
    // Parse the expression that the unary operator applies to
    pi = ParseExpression();
    if (pi.er != none)
    {
      return pi;
    }
  }
  else if (tk.tp == STRING)
  {
    pi.er = none;
    return pi;
  }
  else if (tk.tp == RESWORD)
  {
    pi.er = none;
    return pi;
  }
  else if (tk.tp == SYMBOL && tk.lx[0] == '(')
  {
    pi = ParseExpression();
    if (pi.er != none)
    {
      return pi;
    }

    tk = PeekNextToken();
    if (tk.tp == SYMBOL && tk.lx[0] == ';')
    {
      // End of statement
      pi.er = none;
      return pi;
    }
    else
    {
      tk = GetNextToken();
      if (tk.tp != SYMBOL || tk.lx[0] != ')')
      {
        pi.er = closeParenExpected;
        pi.tk = tk;
        return pi;
      }
    }

    tk = PeekNextToken();
    if (tk.tp == SYMBOL && isOperator(tk.lx))
    {
      GetNextToken();

      ParserInfo pi_rhs = ParseExpression();
      if (pi_rhs.er != none)
      {
        return pi_rhs;
      }
    }
  }
  else if (tk.tp == ID || tk.tp == STRING)
  {
    Token nextTk = PeekNextToken();
    if (nextTk.tp == SYMBOL && nextTk.lx[0] == '.')
    {
      GetNextToken();

      tk = GetNextToken();
      if (tk.tp != ID)
      {
        pi.er = idExpected;
        pi.tk = tk;
        return pi;
      }

      nextTk = PeekNextToken();
      if (nextTk.tp == SYMBOL && (nextTk.lx[0] == '(' || isOperator(nextTk.lx)))
      {
        if (isOperator(nextTk.lx))
        {
          GetNextToken();

          ParserInfo pi_rhs = ParseExpression();
          if (pi_rhs.er != none)
          {
            return pi_rhs;
          }
        }
        else
        {
          pi = ParseFunctionCall();
          if (pi.er != none)
          {
            return pi;
          }
        }
      }
      else
      {
        pi.er = none;
        return pi;
      }
    }
    else if (nextTk.tp == SYMBOL && nextTk.lx[0] == '[')
    {
      GetNextToken();

      ParserInfo pi_index = ParseExpression();
      if (pi_index.er != none)
      {
        return pi_index;
      }

      tk = GetNextToken();
      if (tk.tp != SYMBOL || tk.lx[0] != ']')
      {
        pi.er = closeBracketExpected;
        pi.tk = tk;
        return pi;
      }
    }
  }

  tk = PeekNextToken();
  if (tk.tp == SYMBOL && isOperator(tk.lx))
  {
    GetNextToken();

    ParserInfo pi_rhs = ParseExpression();
    if (pi_rhs.er != none)
    {
      return pi_rhs;
    }
  }

  pi.er = none;
  return pi;
}

ParserInfo ParseFunctionCall()
{
  ParserInfo pi;
  Token tk;

  // Check for '('
  tk = GetNextToken();
  if (tk.tp != SYMBOL || tk.lx[0] != '(')
  {
    pi.er = openParenExpected;
    pi.tk = tk;
    return pi;
  }

  // Parse arguments until we see a ')' or encounter an error
  while (true)
  {
    // Parse an expression for the argument
    ParserInfo pi_arg = ParseExpression();
    if (pi_arg.er != none)
    {
      return pi_arg;
    }

    // Check for ',' or ')'
    tk = PeekNextToken();
    if (tk.lx[0] == ';')
    {
      pi.er = none;
      return pi;
    }
    else
    {
      tk = GetNextToken();
      if (tk.tp != SYMBOL || (tk.lx[0] != ',' && tk.lx[0] != ')'))
      {
        pi.er = closeParenExpected;
        pi.tk = tk;
        return pi;
      }
    }

    // If we saw a ')', end the loop
    if (tk.lx[0] == ')')
    {
      break;
    }
  }

  pi.er = none;
  return pi;
}

ParserInfo ParseClassVarDec()
{
  ParserInfo pi;
  Token tk = GetNextToken();

  // Check for 'static' or 'field' keyword
  if (tk.tp != RESWORD || (strcmp(tk.lx, "static") != 0 && strcmp(tk.lx, "field") != 0))
  {
    pi.er = classVarErr; // Update here
    pi.tk = tk;
    return pi;
  }

  // Check for type
  tk = GetNextToken();
  if (tk.tp != RESWORD && ((strcmp(tk.lx, "int") != 0 && strcmp(tk.lx, "char") != 0 && strcmp(tk.lx, "boolean") != 0) && tk.tp != ID))
  {
    pi.er = illegalType; // Update here
    pi.tk = tk;
    return pi;
  }

  // Check for variable name
  tk = GetNextToken();
  if (tk.tp != ID)
  {
    pi.er = idExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for optional additional variable names
  while (1)
  {
    tk = GetNextToken();

    if (tk.tp != SYMBOL || strcmp(tk.lx, ",") != 0)
    {
      break;
    }

    tk = GetNextToken();
    if (tk.tp != ID)
    {
      pi.er = idExpected;
      pi.tk = tk;
      return pi;
    }
  }

  // Check for semicolon
  if (tk.tp != SYMBOL || strcmp(tk.lx, ";") != 0)
  {
    pi.er = semicolonExpected;
    pi.tk = tk;
    return pi;
  }

  pi.er = none;
  pi.tk = tk;
  return pi;
}

ParserInfo ParseSubroutineDec()
{
  ParserInfo pi;
  Token tk = GetNextToken();

  // Check for 'constructor', 'function' or 'method' keyword
  if (tk.tp != RESWORD || (strcmp(tk.lx, "constructor") != 0 && strcmp(tk.lx, "function") != 0 && strcmp(tk.lx, "method") != 0))
  {
    pi.er = subroutineDeclarErr;
    pi.tk = tk;
    return pi;
  }

  // Check for return type
  tk = GetNextToken();
  if (tk.tp != RESWORD && tk.tp != ID)
  {
    pi.er = illegalType;
    pi.tk = tk;
    return pi;
  }

  // Check for subroutine name (identifier)
  tk = GetNextToken();
  if (tk.tp != ID)
  {
    pi.er = idExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for open parenthesis
  tk = GetNextToken();
  if (tk.tp != SYMBOL || strcmp(tk.lx, "(") != 0)
  {
    pi.er = openParenExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for parameter list
  // This should be another function that parses the parameter list
  // This is a simplified version that just checks for a closing parenthesis

  // Parse parameter list (if any)
  tk = PeekNextToken();
  if (tk.tp != SYMBOL && strcmp(tk.lx, ")") != 0)
  {
    pi = ParseParameterList();
    if (pi.er != none)
    {
      return pi;
    }
  }

  tk = GetNextToken();
  if (tk.tp != SYMBOL || strcmp(tk.lx, ")") != 0)
  {
    pi.er = closeParenExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for subroutine body
  // This should be another function that parses the subroutine body
  // This is a simplified version that just checks for an opening and closing brace
  tk = GetNextToken();
  if (tk.tp != SYMBOL || strcmp(tk.lx, "{") != 0)
  {
    pi.er = openBraceExpected;
    pi.tk = tk;
    return pi;
  }

  // Parse statements (if any)
  tk = PeekNextToken();
  while (tk.tp != SYMBOL || strcmp(tk.lx, "}") != 0)
  {
    pi = ParseStatement();
    if (pi.er != none)
    {
      return pi;
    }
    tk = PeekNextToken();
  }

  // Check for closing brace
  tk = GetNextToken();
  if (tk.tp != SYMBOL || strcmp(tk.lx, "}") != 0)
  {
    pi.er = closeBraceExpected;
    pi.tk = tk;
    return pi;
  }

  pi.er = none;
  pi.tk = tk;
  return pi;
}

// ParseParameterList function
ParserInfo ParseParameterList()
{
  ParserInfo pi;
  Token tk;

  while (1)
  {
    tk = GetNextToken();

    // Check for type
    if (tk.tp != RESWORD && ((strcmp(tk.lx, "int") != 0 && strcmp(tk.lx, "char") != 0 && strcmp(tk.lx, "boolean") != 0) && tk.tp != ID))
    {
      pi.er = illegalType; // Update here
      pi.tk = tk;
      return pi;
    }

    // Check for variable name
    tk = GetNextToken();
    if (tk.tp != ID)
    {
      pi.er = idExpected;
      pi.tk = tk;
      return pi;
    }

    // Check for comma (optional)
    tk = PeekNextToken();
    if (tk.tp != SYMBOL || strcmp(tk.lx, ",") != 0)
    {
      break;
    }

    // Eat comma
    GetNextToken();
  }

  pi.er = none;
  pi.tk = tk;
  return pi;
}

// Additional parser functions

ParserInfo ParseVarDec()
{
  ParserInfo pi;
  Token tk = GetNextToken();

  // Check for 'var' keyword
  if (tk.tp != RESWORD || strcmp(tk.lx, "var") != 0)
  {
    pi.er = memberDeclarErr;
    pi.tk = tk;
    return pi;
  }

  // Check for type
  if (tk.tp != RESWORD)
  { // Adjust this as necessary for your language's valid types
    pi.er = illegalType;
    pi.tk = tk;
    return pi;
  }

  // Check for variable name (identifier)
  tk = GetNextToken();
  if (tk.tp != ID)
  {
    pi.er = idExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for ';'
  tk = GetNextToken();
  if (tk.tp != SYMBOL || tk.lx[0] != ';')
  {
    pi.er = semicolonExpected;
    pi.tk = tk;
    return pi;
  }

  pi.er = none;
  return pi;
}

ParserInfo ParseStatement()
{
  ParserInfo pi;

  Token tk = PeekNextToken();

  if (tk.tp == RESWORD)
  {
    if (strcmp(tk.lx, "let") == 0)
    {
      return ParseLetStatement();
    }
    else if (strcmp(tk.lx, "var") == 0)
    {
      return ParseVarStatement();
    }
    else if (strcmp(tk.lx, "if") == 0)
    {
      return ParseIfStatement();
    }
    else if (strcmp(tk.lx, "while") == 0)
    {
      return ParseWhileStatement();
    }
    else if (strcmp(tk.lx, "do") == 0)
    {
      return ParseDoStatement();
    }
    else if (strcmp(tk.lx, "return") == 0)
    {
      return ParseReturnStatement();
    }

    else
    {
      pi.er = syntaxError;
      pi.tk = tk;
      return pi;
    }
  }
  else
  {
    pi.er = syntaxError;
    pi.tk = tk;
    return pi;
  }
}

ParserInfo ParseLetStatement()
{
  ParserInfo pi;
  Token tk = GetNextToken();

  // Check for 'let' keyword
  if (tk.tp != RESWORD || strcmp(tk.lx, "let") != 0)
  {
    pi.er = syntaxError;
    pi.tk = tk;
    return pi;
  }

  // Check for identifier
  tk = GetNextToken();
  if (tk.tp != ID)
  {
    pi.er = idExpected;
    pi.tk = tk;
    return pi;
  }

  // Check if this is an array access
  Token nextTk = PeekNextToken();
  if (nextTk.tp == SYMBOL && nextTk.lx[0] == '[')
  {
    // Consume the '[' token
    GetNextToken();

    // Parse the array index
    ParserInfo pi_index = ParseExpression();
    if (pi_index.er != none)
    {
      return pi_index;
    }

    // Check for ']'
    tk = GetNextToken();
    if (tk.tp != SYMBOL || tk.lx[0] != ']')
    {
      pi.er = closeBracketExpected;
      pi.tk = tk;
      return pi;
    }
  }

  // Check for '='
  tk = GetNextToken();
  if (tk.tp != SYMBOL || tk.lx[0] != '=')
  {
    pi.er = equalExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for expression
  pi = ParseExpression();
  if (pi.er != none)
  {
    return pi;
  }

  // Check for ';'
  tk = PeekNextToken();
  if (tk.lx[0] == '}')
  {
    strcpy(tk.lx, ";");
    tk.ln--;
    pi.er = syntaxError;
    pi.tk = tk;
    return pi;
  }
  else
  {
    tk = GetNextToken();
  }
  if (tk.tp != SYMBOL || tk.lx[0] != ';')
  {
    pi.er = semicolonExpected;
    pi.tk = tk;
    return pi;
  }

  pi.er = none;
  return pi;
}

ParserInfo ParseVarStatement()
{
  ParserInfo pi;
  Token tk;

  // consume 'var'
  tk = GetNextToken();

  // check for variable type
  tk = GetNextToken();
  if (tk.tp != ID && tk.tp != RESWORD)
  {
    pi.er = syntaxError;
    pi.tk = tk;
    return pi;
  }

  while (1)
  {
    // check for variable name
    tk = GetNextToken();
    if (tk.tp != ID)
    {
      pi.er = syntaxError;
      pi.tk = tk;
      return pi;
    }

    // check for ',' or ';'
    tk = GetNextToken();
    if (tk.tp != SYMBOL || (tk.lx[0] != ',' && tk.lx[0] != ';'))
    {
      pi.er = syntaxError;
      pi.tk = tk;
      return pi;
    }

    // if ';', then end of var statement
    if (tk.lx[0] == ';')
    {
      break;
    }
  }

  pi.er = none;
  return pi;
}

ParserInfo ParseIfStatement()
{
  ParserInfo pi, pi_expr, pi_stmt;
  Token tk = GetNextToken();

  // Check for 'if' keyword
  if (tk.tp != RESWORD || strcmp(tk.lx, "if") != 0)
  {
    pi.er = syntaxError;
    pi.tk = tk;
    return pi;
  }

  // Check for '('
  tk = GetNextToken();
  if (tk.tp != SYMBOL || tk.lx[0] != '(')
  {
    pi.er = openParenExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for expression
  pi_expr = ParseExpression();
  if (pi_expr.er != none)
  {
    return pi_expr;
  }

  // Check for ')'
  tk = GetNextToken();
  if (tk.tp != SYMBOL || tk.lx[0] != ')')
  {
    pi.er = closeParenExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for '{'
  tk = GetNextToken();
  if (tk.tp != SYMBOL || tk.lx[0] != '{')
  {
    pi.er = openBraceExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for statements
  while (true)
  {
    tk = PeekNextToken();
    if (tk.tp == SYMBOL && tk.lx[0] == '}')
    {
      break;
    }
    pi_stmt = ParseStatement();
    if (pi_stmt.er != none)
    {
      return pi_stmt;
    }
  }

  // Check for '}'
  tk = GetNextToken();
  if (tk.tp != SYMBOL || tk.lx[0] != '}')
  {
    pi.er = closeBraceExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for optional 'else'
  tk = PeekNextToken();
  if (tk.tp == RESWORD && strcmp(tk.lx, "else") == 0)
  {
    GetNextToken(); // Consume the 'else'

    // Check for '{'
    tk = GetNextToken();
    if (tk.tp != SYMBOL || tk.lx[0] != '{')
    {
      pi.er = openBraceExpected;
      pi.tk = tk;
      return pi;
    }

    // Check for statements
    while (true)
    {
      tk = PeekNextToken();
      if (tk.tp == SYMBOL && tk.lx[0] == '}')
      {
        break;
      }
      pi_stmt = ParseStatement();
      if (pi_stmt.er != none)
      {
        return pi_stmt;
      }
    }

    // Check for '}'
    tk = GetNextToken();
    if (tk.tp != SYMBOL || tk.lx[0] != '}')
    {
      pi.er = closeBraceExpected;
      pi.tk = tk;
      return pi;
    }
  }

  pi.er = none;
  return pi;
}

ParserInfo ParseWhileStatement()
{
  ParserInfo pi, pi_expr, pi_stmt;
  Token tk = GetNextToken();

  // Check for 'while' keyword
  if (tk.tp != RESWORD || strcmp(tk.lx, "while") != 0)
  {
    pi.er = syntaxError;
    pi.tk = tk;
    return pi;
  }

  // Check for '('
  tk = GetNextToken();
  if (tk.tp != SYMBOL || tk.lx[0] != '(')
  {
    pi.er = openParenExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for expression
  pi_expr = ParseExpression();
  if (pi_expr.er != none)
  {
    return pi_expr;
  }

  // Check for ')'
  tk = GetNextToken();
  if (tk.tp != SYMBOL || tk.lx[0] != ')')
  {
    pi.er = closeParenExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for '{'
  tk = GetNextToken();
  if (tk.tp != SYMBOL || tk.lx[0] != '{')
  {
    pi.er = openBraceExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for statements
  while (true)
  {
    tk = PeekNextToken();
    if (tk.tp == SYMBOL && tk.lx[0] == '}')
    {
      break;
    }
    pi_stmt = ParseStatement();
    if (pi_stmt.er != none)
    {
      return pi_stmt;
    }
  }

  // Check for '}'
  tk = GetNextToken();
  if (tk.tp != SYMBOL || tk.lx[0] != '}')
  {
    pi.er = closeBraceExpected;
    pi.tk = tk;
    return pi;
  }

  pi.er = none;
  return pi;
}

ParserInfo ParseDoStatement()
{
  ParserInfo pi;
  Token tk = GetNextToken();

  // Check for 'do' keyword
  if (tk.tp != RESWORD || strcmp(tk.lx, "do") != 0)
  {
    pi.er = syntaxError;
    pi.tk = tk;
    return pi;
  }

  // Check for subroutineCall
  pi = ParseSubroutineCall();
  if (pi.er != none)
  {
    return pi;
  }

  // Check for ';'
  tk = GetNextToken();
  if (tk.tp != SYMBOL || tk.lx[0] != ';')
  {
    pi.er = semicolonExpected;
    pi.tk = tk;
    return pi;
  }

  pi.er = none;
  return pi;
}

ParserInfo ParseReturnStatement()
{
  ParserInfo pi;
  Token tk = GetNextToken();

  // Check for 'return' keyword
  if (tk.tp != RESWORD || strcmp(tk.lx, "return") != 0)
  {
    pi.er = syntaxError;
    pi.tk = tk;
    return pi;
  }

  // Peek at next token
  tk = PeekNextToken();
  if (tk.tp == SYMBOL && tk.lx[0] == ';')
  {
    // This is a return statement without an expression.
    // Consume the ';' token
    GetNextToken();
  }
  else
  {
    // Call ParseExpression() to handle possible function call or complex expression
    pi = ParseExpression();
    if (pi.er != none)
    {
      return pi;
    }

    // Check for ';'
    if (tk.lx[0] != '}')
    {
      tk = GetNextToken();
    }
    if (tk.tp != SYMBOL || tk.lx[0] != ';')
    {
      pi.er = semicolonExpected;
      pi.tk = tk;
      return pi;
    }
  }

  pi.er = none;
  return pi;
}

ParserInfo ParseSubroutineCall()
{
  ParserInfo pi;
  Token tk = GetNextToken();

  // Check for subroutine or method identifier
  if (tk.tp != ID)
  {
    pi.er = idExpected;
    pi.tk = tk;
    return pi;
  }

  // Check for '('
  Token nextTk = PeekNextToken();
  if (nextTk.tp == SYMBOL && nextTk.lx[0] == '(')
  {
    // This is a subroutine call
    // Consume the '(' token
    GetNextToken();

    // Parse arguments (if any)
    tk = PeekNextToken();
    while (tk.tp != SYMBOL || tk.lx[0] != ')')
    {
      pi = ParseExpression(); // Assuming arguments are expressions
      if (pi.er != none)
      {
        return pi;
      }

      // Check for comma or ')'
      tk = PeekNextToken();
      if (tk.lx[0] == ';')
      {
        pi.er = none;
        pi.tk = tk;
        return pi;
      }
      else if (tk.tp != SYMBOL || (tk.lx[0] != ',' && tk.lx[0] != ')'))
      {
        pi.er = syntaxError;
        pi.tk = tk;
        return pi;
      }

      if (tk.lx[0] == ',')
      {
        tk = GetNextToken();
        tk = PeekNextToken();
      }
    }

    // Check for closing parenthesis
    tk = GetNextToken();
    if (tk.tp != SYMBOL || tk.lx[0] != ')')
    {
      pi.er = closeParenExpected;
      pi.tk = tk;
      return pi;
    }
  }
  else if (nextTk.tp == SYMBOL && nextTk.lx[0] == '.')
  {

    // Consume the '.' token
    GetNextToken();

    pi = ParseSubroutineCall(); // Assuming arguments are expressions
    if (pi.er != none)
    {
      return pi;
    }

    // // This is a method call
    // // Consume the '.' token
    // GetNextToken();

    // // Expect another identifier (the method name)
    // tk = GetNextToken();
    // if (tk.tp != ID)
    // {
    //   pi.er = idExpected;
    //   pi.tk = tk;
    //   return pi;
    // }

    // if (nextTk.tp == SYMBOL && nextTk.lx[0] == '.')
    // {
    //   pi = ParseExpression(); // Assuming arguments are expressions
    //   if (pi.er != none)
    //   {
    //     return pi;
    //   }
    // }

    // // Check for '('
    // tk = GetNextToken();
    // if (tk.tp != SYMBOL || tk.lx[0] != '(')
    // {
    //   pi.er = openParenExpected;
    //   pi.tk = tk;
    //   return pi;
    // }

    // // Parse arguments (if any)
    // tk = PeekNextToken();
    // while (tk.tp != SYMBOL || tk.lx[0] != ')')
    // {
    //   pi = ParseExpression(); // Assuming arguments are expressions
    //   if (pi.er != none)
    //   {
    //     return pi;
    //   }

    //   printf("token: %s\n", tk.lx);

    //   // Check for comma or ')'
    //   tk = GetNextToken();
    //   if (tk.tp != SYMBOL || (tk.lx[0] != ',' && tk.lx[0] != ')'))
    //   {
    //     pi.er = syntaxError;
    //     pi.tk = tk;
    //     return pi;
    //   }

    //   if (tk.lx[0] == ',')
    //   {
    //     tk = PeekNextToken();
    //   }
    // }

    // // Check for closing parenthesis
    // // tk = GetNextToken();

    // if (tk.tp != SYMBOL || tk.lx[0] != ')')
    // {
    //   pi.er = closeParenExpected;
    //   pi.tk = tk;
    //   return pi;
    // }
  }
  else
  {
    pi.er = openParenExpected;
    pi.tk = nextTk;
    return pi;
  }

  pi.er = none;
  return pi;
}

int StopParser()
{
  return StopLexer();
}

#ifndef TEST_PARSER
int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Usage: %s <directory>\n", argv[0]);
    return 1;
  }

  char *dir_name = argv[1];
  DIR *dir = opendir(dir_name);
  if (!dir)
  {
    printf("Error opening directory: %s\n", dir_name);
    return 1;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL)
  {
    char *ext = strrchr(entry->d_name, '.');
    if (ext && strcmp(ext, ".jack") == 0)
    {
      char file_path[256];
      sprintf(file_path, "%s/%s", dir_name, entry->d_name);

      if (InitParser(file_path))
      {
        printf("Parsing file: %s\n", file_path);

        ParserInfo pi = Parse();
        if (pi.er != none)
        {
          printf("Parsing error: %d\n", pi.er);
          printf("Line Number: %d\n", pi.tk.ln);
          printf("token: %s\n", pi.tk.lx);
          printf("Token type: %d\n", pi.tk.tp);
          StopParser();
          return 1;
        }
        StopParser();
      }
      else
      {
        printf("Error opening file: %s\n", file_path);
      }
    }
  }

  closedir(dir);
  return 0;
}
#endif
