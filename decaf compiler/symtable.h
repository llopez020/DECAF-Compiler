/*  Symbol Table --linked list  headers
    Used for Compilers class

    Modified Spring 2015 to allow for name to pointed to by symtable, instead of copied, since the name is copied
    into the heap

    Modified to have levels.  A level 0 means global variable, other levels means in range of the function.  We
    start out our offsets at 0 (from stack pointer) for level 1,,,when we enter a functional declaration.
    We increment offset each time we insert a new variable.  A variable is considered to be valid if it is found in
    the symbol table at our level or lesser level.  If at 0, then it is global.  

    We return a pointer to the symbol table when a variable matches our creteria.

    We add a routine to remove variables at our level and above.
*/


/*   Levi Lopez
     October 30, 2022

     Changelog: 
     - Added function to create a temporary symbol name 
     - Added more Subtypes to ID_Sub_Type
*/


#include "ast.h"

#ifndef _SYMTAB 
#define _SYMTAB

void Display();

int Delete(int level);

// creates a temporary symbol name
char * TEMP_CREATE();

int FetchAddr (char *lab);

// The types an ID can have
enum ID_Sub_Type {
	ID_Sub_Type_Scalar,
	ID_Sub_Type_Array,
	ID_Sub_Type_Method,
	ID_Sub_Type_Extern,
	ID_Sub_Type_Package
};

struct SymbTab
{
     char *name;
     int offset; /* from activation record boundary */
     int mysize;  /* number of words this item is 1 or more */
     int level;  /* the level where we found the variable */
     enum AST_Decaf_Types Type;  /* the type of the symbol */
     enum ID_Sub_Type  SubType;  /* the element is a function */
     ASTnode * fparms; /* pointer to parameters of the function in the AST */

     struct SymbTab *next;
};


struct SymbTab * Insert(char *name, enum AST_Decaf_Types Type, enum ID_Sub_Type subtype, int  level, int mysize, int offset, ASTnode * fparms );

struct SymbTab * Search(char name[], int level, int recur);

#endif
