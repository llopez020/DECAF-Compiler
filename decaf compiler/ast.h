/*   Abstract syntax tree code

     Header file   
     Shaun Cooper January 2020

*/

/*   
 * 
     Levi Lopez
     December 1, 2022

     Changelog: 
     - Added Label into ASTnodetype, so that tree can store the given label when passing through emit.c
*/

#include<stdio.h>
#include<malloc.h>

#ifndef AST_H
#define AST_H
int debugsw; /* debug switch */

/* define the enumerated types for the AST.  This is used to tell us what 
sort of production rule we came across */

enum AST_Tree_Element_Type {
   A_PROGRAM,
   A_PACKAGE,
   A_EXTERN,
   A_EXTERN_TYPE,
   A_METHODDEC,
   A_METHODID,
   A_NUMBER,
   A_BLOCK,
   A_EXPR,
   A_VARDEC,
   A_CONSTANT_INT,
   A_CONSTANT_BOOL,
   A_CONSTANT_STRING,
   A_BREAK,
   A_RETURN,
   A_VAR_RVALUE,
   A_VAR_LVALUE,
   A_WHILE,
   A_CONTINUE,
   A_IF,
   A_METHOD,
   A_METHODARG,
   A_ASSIGN,
   A_ELSE,
   A_IFBLOCK,
   A_ARRAY_TYPE
};


enum AST_Operators {
   A_PLUS,
   A_MINUS,
   A_UMINUS,
   A_TIMES,
   A_DIVIDE,
   A_MOD,
   A_AND,
   A_OR,
   A_LEFTSHIFT,
   A_RIGHTSHIFT,
   A_NOT,
   A_LEQ,
   A_LT,
   A_GT,
   A_GEQ,
   A_EQ,
   A_NEQ
};

enum AST_Decaf_Types {
   A_Decaf_INT,
   A_Decaf_BOOL,
   A_Decaf_VOID,
   A_Decaf_STRING
};

/* define a type AST node which will hold pointers to AST structs that will
   allow us to represent the parsed code 
*/
typedef struct ASTnodetype
{
     enum AST_Tree_Element_Type type;
     enum AST_Operators operator;
     char * name;
     char * label;
     int value;
     struct SymbTab *symbol;
     enum AST_Decaf_Types declared_type;
     struct ASTnodetype *S1,*S2, *next ; /* used for holding IF and WHILE components -- not very descriptive */
} ASTnode;

#include "symtable.h"

/* uses malloc to create an ASTnode and passes back the heap address of the newley created node */
ASTnode *ASTCreateNode(enum AST_Tree_Element_Type mytype);

void PT(int howmany); // prints howmany spaces

ASTnode *program;  // Global Pointer for connection between YACC and backend

/*  Print out the abstract syntax tree */
void ASTprint(int level,ASTnode *p); // prints tree with level horizontal spaceing

/* returns whether or not parameters match */
int check_parameters(ASTnode *formal, ASTnode *actual);

#endif // of AST_H
