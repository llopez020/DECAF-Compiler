%{

/*
     Levi Lopez
     December 1, 2022

     Changelog: 
     - Properly gives extern type to extern methods
     - Added file output, debug setting, file output name setting and passes AST to emit.c
*/


/* begin specs */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "emit.h"
#include "symtable.h"

int line(); /* resolves warning about undefined line */
int LEVEL = 0; /* scoping level */
int OFFSET = 0; /* how much space we are using for the environment */
int GOFFSET = 0; /* how much space we are using for the environment (methods) */
int MAXOFFSET; /* max space we are using */

int mydebug = 0; /* debug switch */
ASTnode * Program; /* global variable for the AST Tree */

int yylex(); /* resolves warning about undefined yylex */

void yyerror (s)  /* Called by yyparse on error */
     char *s;
{
  int linecount = line(); /* get linecount */
  printf ("%s\n", s);
  fprintf(stderr,"at line %d\n",linecount); /* print linecount */
}

%}

/*  defines the start symbol, what tokens come back from LEX, and union values */
%start Program

%union
{
	int value;
	char* string;
	struct ASTnodetype *node;
	enum AST_Decaf_Types declared_type;
	enum AST_Operators operator;
}

%token T_AND
%token T_ASSIGN
%token T_BOOLTYPE
%token T_BREAK
%token T_CONTINUE
%token T_ELSE
%token T_EQ
%token T_EXTERN
%token T_FALSE
%token T_FUNC
%token T_GEQ
%token T_GT
%token T_LT
%token <string> T_ID
%token T_IF
%token <value> T_INTCONSTANT
%token T_INTTYPE
%token T_LEFTSHIFT
%token T_LEQ
%token T_NEQ
%token T_OR
%token T_PACKAGE
%token T_RETURN
%token T_RIGHTSHIFT
%token <string> T_STRINGCONSTANT
%token T_STRINGTYPE
%token T_TRUE
%token T_VAR
%token T_VOID
%token T_WHILE    

%type <node> Externs ExternDefn ExternTypeList1 Statement Lvalue MethodList1 Assign MethodArg Expr Term MethodCall Additiveexpression Simpleexpression BoolConstant Constant ArrayType Factor ExternTypeList FieldDecls TypeList TypeList1 MethodDecl Block MethodDecls VarDecl VarDecls FieldDecl Statements MethodList StatementList1 StatementList2 StatementList3
%type <declared_type> MethodType Type ExternType 
%type <operator> Multop Addop Relop

%%	/* end specs, begin rules */

Program                 : Externs T_PACKAGE T_ID '{' FieldDecls MethodDecls '}'
				{ 
				  if (Search($3, LEVEL, 0)!=NULL) {
				   	// it was in symbol table before
				   	printf("package already defined\n");
				   	yyerror($3);
				   	exit(1);
				  } 				  
				  
				  Program = ASTCreateNode(A_PROGRAM);
				  Program -> S1 = $1;
				  Insert($3, A_Decaf_INT, ID_Sub_Type_Package, LEVEL, 0, 0, NULL);
				  Program -> S2 = ASTCreateNode(A_PACKAGE);
				  Program -> S2 -> name = $3;
				  Program -> S2 -> S1 = $5;
				  Program -> S2 -> S2 = $6;
				}
	                ;
	
Externs                 : /*empty*/ 
				{ $$ = NULL; }
                        | ExternDefn Externs
                       		{ $$ = $1;
                        	  $$ -> next = $2;
                        	}
                        ;

ExternDefn              : T_EXTERN T_FUNC T_ID '(' ExternTypeList ')' MethodType ';'
				{ if (Search($3, LEVEL, 0)!=NULL) {
				   	// it was in symbol table before
				   	printf("extern already defined\n");
				   	yyerror($3);
				   	exit(1);
				  } 				  
				  
				  
				  $$ = ASTCreateNode(A_EXTERN);
				  
				  $$->symbol = Insert($3, $7, ID_Sub_Type_Extern, LEVEL, 0, 0, $5);

				  $$ -> name = $3;
				  $$ -> declared_type = $7;
				  $$ -> S1 = $5;
				}
                        ;

FieldDecls              : /*empty*/ 
				{ $$ = NULL; }
	                | FieldDecl FieldDecls
				{ $$ = $1;
                        	  $$ -> next = $2;
                        	}
	                ;
	   
ExternTypeList          : /*empty*/ 
				{ $$ = NULL;}
	                | ExternTypeList1 
				{ $$ = $1; }
	                ;
	 
ExternTypeList1         : ExternType 
				{ $$ = ASTCreateNode(A_EXTERN_TYPE);
				  $$ -> declared_type = $1; 
				}
		        | ExternTypeList1 ',' ExternType
		        	{ $$ = ASTCreateNode(A_EXTERN_TYPE);
			 	  $$ -> declared_type = $3;
				  $$ -> next = $1;
				}
		        ;
		

ExternType              : T_STRINGTYPE 
				{ $$ = A_Decaf_STRING; }
	                | Type 
				{ $$ = $1; }
	                ;
	   
Type                    : T_INTTYPE 
				{ $$ = A_Decaf_INT; }
                        | T_BOOLTYPE 
				{ $$ = A_Decaf_BOOL; }
                        ;
     
MethodType              : T_VOID 
				{ $$ = A_Decaf_VOID; }
                        | Type 
				{ $$ = $1; }
                        ; 

FieldDecl               : T_VAR T_ID Type ';'
				{ if (Search($2, LEVEL, 0)!=NULL) {
				   	// it was in symbol table before
				   	printf("variable already defined\n");
				   	yyerror($2);
				   	exit(1);
				  } 				  
				  
				  
				  $$ = ASTCreateNode(A_VARDEC);
				  
				  $$->symbol = Insert($2, $3, ID_Sub_Type_Scalar, LEVEL, 1, OFFSET, NULL);
				  OFFSET++;
				  
			  	  $$ -> name = $2;
				  $$ -> declared_type = $3;
				}
			| T_VAR T_ID  ArrayType ';'
				{ 
				  if (Search($2, LEVEL, 0)!=NULL) {
				   	// it was in symbol table before
				   	printf("variable already defined\n");
				   	yyerror($2);
				   	exit(1);
				  } 				  
				  
				  
				  $$ = ASTCreateNode(A_VARDEC);
				  
				  $$->symbol = Insert($2, $3 -> declared_type, ID_Sub_Type_Array, LEVEL, $3 -> value, OFFSET, NULL);
				  OFFSET+= $3 -> value;
				  
				  $$ -> name = $2;
	   			  $$ -> S1 = $3;
				  $$ -> value = $3 -> value;
				  $$ -> declared_type = $3 -> declared_type;
				}
			| T_VAR T_ID Type T_ASSIGN Constant ';'
				{ if (Search($2, LEVEL, 0)!=NULL) {
				   	// it was in symbol table before
				   	printf("variable already defined\n");
				   	yyerror($2);
				   	exit(1);
				  } 				  
				  
				  
				  $$ = ASTCreateNode(A_VARDEC);
				  
				  $$->symbol = Insert($2, $3, ID_Sub_Type_Scalar, LEVEL, 1, OFFSET, NULL);
				  OFFSET++;

				  $$ -> name = $2;
				  $$ -> declared_type = $3;
				  $$ -> S2 = $5;
				}
                        ;
                    
MethodDecls             : /*empty*/ 
				{ $$ = NULL; }
	                | MethodDecl MethodDecls
				{ $$ = $1;
                	          $$ -> next = $2;
                       		}
	                ;
	     
MethodDecl              : T_FUNC T_ID {GOFFSET=OFFSET; OFFSET=0; MAXOFFSET=OFFSET;}'(' TypeList ')' MethodType
				{ if (Search($2, LEVEL, 0)!=NULL) {
				   	// it was in symbol table before
				   	printf("method already defined\n");
				   	yyerror($2);
				   	exit(1);
				  }
				  Insert($2, $7, ID_Sub_Type_Method, LEVEL, 0, 0, $5); 		
				}		  
				 Block 
				{  
				  $$ = ASTCreateNode(A_METHODDEC);
				  
				  $$->symbol = Search($2, LEVEL, 0);
				
				  $$ -> name = $2;
				  $$ -> declared_type = $7;
				  $$ -> S1 = $5;
				  $$ -> S2 = $9;
				  $$ -> value = MAXOFFSET;
				  $$ -> symbol -> mysize = MAXOFFSET;
				  OFFSET = GOFFSET;
				}
	                ;
	   
TypeList                : /*empty*/ 
				{ $$ = NULL; }
			| TypeList1 ',' TypeList
			{ $$ = $1;
                	  $$ -> next = $3;
                       	}
			| TypeList1 { $$ = $1; }
			;

	 
TypeList1               : T_ID Type 
				{ if (Search($1, LEVEL+1, 0)!=NULL) {
				   	// it was in symbol table before
				   	printf("method arg already defined\n");
				   	yyerror($2);
				   	exit(1);
				  } 				  
				 
				  $$ = ASTCreateNode(A_METHODID);

				  $$->symbol = Insert($1, $2, ID_Sub_Type_Scalar, LEVEL+1, 1, OFFSET, NULL);
				  OFFSET++;

				  $$ -> name= $1;
				  $$ -> declared_type = $2;
			}
	           	;
	   	   
Block                   : '{' { LEVEL++; } VarDecls Statements '}'
				{ $$ = ASTCreateNode(A_BLOCK);
		 		  $$ -> S1 = $3;
		 		  $$ -> S2 = $4;
				  if (mydebug) Display();
				  if (OFFSET > MAXOFFSET) 
					MAXOFFSET = OFFSET;
		 		  OFFSET = OFFSET - Delete(LEVEL);
		 		  LEVEL--;
				}
                        ;
	 
VarDecls                : /*empty*/ 
				{ $$ = NULL; }
                        | VarDecl VarDecls
				{ $$ = $1;
                        	  $$ -> next = $2;
                        	}
                        ;
        
VarDecl                 : T_VAR T_ID Type ';'
				{ if (Search($2, LEVEL, 0)!=NULL) {
				   	// it was in symbol table before
				   	printf("variable already defined\n");
				   	yyerror($2);
				   	exit(1);
				  } 				  
				  
				  
				  $$ = ASTCreateNode(A_VARDEC);
				  
				  $$->symbol = Insert($2, $3, ID_Sub_Type_Scalar, LEVEL, 1, OFFSET, NULL);
				  OFFSET++;
				  
			  	  $$ -> name = $2;
				  $$ -> declared_type = $3;
				}
			| T_VAR T_ID ArrayType ';'
				{ 
				  if (Search($2, LEVEL, 0)!=NULL) {
				   	// it was in symbol table before
				   	printf("variable already defined\n");
				   	yyerror($2);
				   	exit(1);
				  } 				  
				  
				  
				  $$ = ASTCreateNode(A_VARDEC);
				  
				  $$->symbol = Insert($2, $3 -> declared_type, ID_Sub_Type_Array, LEVEL, $3 -> value, OFFSET, NULL);
				  OFFSET+= $3 -> value;
				  
				  $$ -> name = $2;
	   			  $$ -> S1 = $3;
				  $$ -> value = $3 -> value;
				  $$ -> declared_type = $3 -> declared_type;
				}
                        ;

Statements              : /*empty*/ 
				{ $$ = NULL; }
                        | Statement Statements
				{ $$ = $1;
                        	  $$ -> next = $2;
                        	}
                        ;
           
Statement               : Block
				{ $$ = $1; }
			| Assign ';'
				{ $$ = $1; }
			| MethodCall ';'
				{ $$ = $1; }
			| T_IF '(' Expr ')' Block StatementList1 
				{ $$ = ASTCreateNode(A_IF);
				  $$ -> S1 = $3;
 				  $$ -> S2 = ASTCreateNode(A_ELSE);
				  $$ -> S2 -> S1 = $5;
				  $$ -> S2 -> S2 = $6;
			}
			| T_WHILE '(' Expr ')' Block
				{ $$ = ASTCreateNode(A_WHILE);
				  $$ -> S1 = $3;
				  $$ -> S2 = $5;
				}
			| T_RETURN StatementList2 ';'
				{ $$ = ASTCreateNode(A_RETURN); 
				  $$ -> S1 = $2;
				}
			| T_BREAK ';'
				{ $$ = ASTCreateNode(A_BREAK); }
			| T_CONTINUE ';'
				{ $$ = ASTCreateNode(A_CONTINUE); }
	                ;
	            
Assign                  : Lvalue T_ASSIGN Expr 
				{ 
				  if ($1->declared_type != $3->declared_type) {
				  	yyerror("type mismatch on assignment");
					exit(1);
				  } 

				  $$ = ASTCreateNode(A_ASSIGN);
				  $$ -> S1 = $1;
				  $$ -> S2 = $3;
				  $$ -> name = TEMP_CREATE();
				  $$ -> symbol = Insert($$->name, $1->declared_type, ID_Sub_Type_Scalar, LEVEL, 1, OFFSET, NULL);
				  OFFSET++;
				}
                        ;
       
Lvalue                  : T_ID 			
				{ 
				  struct SymbTab *p;
				  p = Search($1,LEVEL,1);

				  if (p==NULL) {
					printf("symbol not defined\n");
					yyerror($1);
					exit(1);
                                  }
				  if(p->SubType!=ID_Sub_Type_Scalar) {
				  	printf("needs to be of type scalar, wrong subtype\n");
					yyerror($1);
					exit(1);
				  }

				  $$ = ASTCreateNode(A_VAR_LVALUE); 
				  $$ -> name = $1;
				  $$ -> symbol = p;
				  $$ -> declared_type = p->Type;
				}
                        | T_ID '[' Expr ']'
				{ 
				  struct SymbTab *p;
				  p = Search($1,LEVEL,1);

				  if (p==NULL) {
					printf("symbol not defined\n");
					yyerror($1);
					exit(1);
                                  }
				  if(p->SubType!=ID_Sub_Type_Array) {
				  	printf("needs to be of type array, wrong subtype\n");
					yyerror($1);
					exit(1);
				  }


	                          $$ = ASTCreateNode(A_VAR_LVALUE); 
				  $$ -> name = $1;
				  $$ -> S1 = $3;
				  $$ -> symbol = p;
				  $$ -> declared_type = p->Type;
				}
                        ;
                        
MethodCall              : T_ID '(' MethodList ')'
				{
				  struct SymbTab *p;
				  p = Search($1,LEVEL-1,1);

				  if (p==NULL) {
					printf("symbol not defined\n");
					yyerror($1);
					exit(1);
                                  }

				  if(p->SubType!=ID_Sub_Type_Method &&  p->SubType!=ID_Sub_Type_Extern) {
				  	printf("needs to be of type method, wrong subtype\n");
					yyerror($1);
					exit(1);
				  }

				  if(check_parameters(p->fparms, $3) == 0) {
				  	printf("formal and actual parameters do not match\n");
					yyerror($1);
					exit(1);
				  }

				  $$ = ASTCreateNode(A_METHOD);
				  $$ -> name = $1;	
				  $$ -> S1 = $3;
				  $$ -> symbol = p;
				  $$ -> declared_type = p->Type;
				}
	                ;
	                
MethodList              : /*empty*/ 
				{ $$ = NULL; }
	                | MethodList1 
				{ $$ = $1; }
	                ;
	 
MethodList1             : MethodArg  
				{ $$ = $1; }
		        | MethodArg ',' MethodList1 
				{ $$ = $1;
				  $$ -> next = $3;
				}
		        ;

MethodArg               : Expr 
				{ $$ = ASTCreateNode(A_METHODARG); 
			          $$->S1 = $1; 
				  $$ -> declared_type = $1 -> declared_type;
				  $$ -> name = TEMP_CREATE();
				  $$ -> value = $1 -> value;
				  $$ -> symbol = Insert($$->name, $$ -> declared_type, ID_Sub_Type_Scalar, LEVEL, 1, OFFSET, NULL);
				  OFFSET++;
				}
			| T_STRINGCONSTANT
				{ 
	 			  $$ = ASTCreateNode(A_METHODARG); 
				  $$ -> declared_type = A_Decaf_STRING;
				  $$ -> name = TEMP_CREATE();
				  $$ -> symbol = Insert($$->name, $$ -> declared_type, ID_Sub_Type_Scalar, LEVEL, 1, OFFSET, NULL);
				  OFFSET++;
				  $$ -> S1 = ASTCreateNode(A_CONSTANT_STRING);
				  $$ -> S1 -> name = $1;
				  $$ -> S1 -> declared_type = A_Decaf_STRING;
				}
	                ;
	  	  
StatementList1          : /*empty*/ 
				{ $$ = NULL; }
	                | T_ELSE Block 
				{ $$ = $2; }
	                ;
	                
StatementList2          : /*empty*/ 
				{ $$ = NULL; }
	                | '(' StatementList3 ')' 
				{ $$ = $2; }
	                ;
          
StatementList3          : /*empty*/ 
				{ $$ = NULL; }
	                | Expr 
				{ $$ = $1; }
	                ;
	                          
Expr                    : Simpleexpression 
				{ $$ = $1; }
                        ;
     
Simpleexpression        : Additiveexpression 
				{ $$ = $1; }
                        | Simpleexpression Relop Additiveexpression
				{
				  if ($1->declared_type != $3->declared_type) {
				  	yyerror("type mismatch on simple expression");
					exit(1);
				  } 

				  $$ = ASTCreateNode(A_EXPR);
				  $$ -> S1 = $1;
				  $$ -> operator = $2;
				  $$ -> S2 = $3;
				  $$ -> declared_type = A_Decaf_BOOL;
				  $$ -> name = TEMP_CREATE();
				  $$ -> symbol = Insert($$->name, $$ -> declared_type, ID_Sub_Type_Scalar, LEVEL, 1, OFFSET, NULL);
				  OFFSET++;

				}
                        ;
                  
Relop                   : T_LEQ 
				{ $$ = A_LEQ; }
			| T_LT 
				{ $$ = A_LT; }
			| T_GT 
				{ $$ = A_GT; }
			| T_GEQ 
				{ $$ = A_GEQ; }
			| T_EQ 
				{ $$ = A_EQ; }
			| T_NEQ 
				{ $$ = A_NEQ; }
                        ;
      
Additiveexpression      : Term 
				{ $$ = $1; }
                        | Additiveexpression Addop Term
				{ 
				  if (($1->declared_type != $3->declared_type) || ($1->declared_type != A_Decaf_INT)) {
				  	yyerror("type mismatch on add/sub");
					exit(1);
				  } 

				  $$ = ASTCreateNode(A_EXPR);
				  $$ -> S1 = $1;
				  $$ -> operator = $2;
				  $$ -> S2 = $3;
				  $$ -> declared_type = $1 -> declared_type;
				  $$ -> name = TEMP_CREATE();
				  $$ -> symbol = Insert($$->name, $$ -> declared_type, ID_Sub_Type_Scalar, LEVEL, 1, OFFSET, NULL);
				  OFFSET++;

				}
                        ;
                    
Addop                   : '+' 
				{ $$ = A_PLUS; }
			| '-' 
				{ $$ = A_MINUS; }
                        ;
      
Term                    : Factor 
				{ $$ = $1; }
                        | Term Multop Factor
				{
				  if ($1->declared_type != $3->declared_type) {
				  	yyerror("type mismatch");
					exit(1);
				  } 

				  if ($1->declared_type==A_Decaf_BOOL && (($2==A_TIMES) || ($2==A_DIVIDE) || ($2==A_MOD))) {
				  	yyerror("cannot use boolean in arithmetic operation");
					exit(1);
				  }

				  if ($1->declared_type==A_Decaf_INT && (($2==A_AND) || ($2==A_OR) || ($2==A_LEFTSHIFT) || ($2==A_RIGHTSHIFT))) {
				  	yyerror("cannot use integer in boolean operation");
					exit(1);
				  }


				  $$ = ASTCreateNode(A_EXPR);
				  $$ -> S1 = $1;
				  $$ -> operator = $2;
				  $$ -> declared_type = $1 -> declared_type;
				  $$ -> S2 = $3;
				  $$ -> name = TEMP_CREATE();
				  $$ -> symbol = Insert($$->name, $$ -> declared_type, ID_Sub_Type_Scalar, LEVEL, 1, OFFSET, NULL);
				  OFFSET++;

				}
                        ;
     
Multop                  : '*' 
				{ $$ = A_TIMES; }
			| '/' 
				{ $$ = A_DIVIDE; }
			| '%' 
				{ $$ = A_MOD; }
			| T_AND 
				{ $$ = A_AND; }
			| T_OR 
				{ $$ = A_OR; }
			| T_LEFTSHIFT 
				{ $$ = A_LEFTSHIFT; }
			| T_RIGHTSHIFT 
				{ $$ = A_RIGHTSHIFT; }
                        ;
       
Factor                  : T_ID
				{ 
				  struct SymbTab *p;
				  p = Search($1,LEVEL,1);

				  if (p==NULL) {
					printf("symbol not defined\n");
					yyerror($1);
					exit(1);
                                  }

				  if(p->SubType!=ID_Sub_Type_Scalar) {
				  	printf("needs to be of type scalar, wrong subtype\n");
					yyerror($1);
					exit(1);
				  }

				  $$ = ASTCreateNode(A_VAR_RVALUE); 
				  $$ -> name = $1;
				  $$ -> symbol = p;
				  $$ -> declared_type = p->Type;
				}
                        | MethodCall 
				{ $$ = $1; }
                        | T_ID '[' Expr ']'
				{
				  struct SymbTab *p;
				  p = Search($1,LEVEL,1);

				  if (p==NULL) {
					printf("symbol not defined\n");
					yyerror($1);
					exit(1);
                                  }
				  if(p->SubType!=ID_Sub_Type_Array) {
				  	printf("needs to be of type array, wrong subtype\n");
					yyerror($1);
					exit(1);
				  }

				  $$ = ASTCreateNode(A_VAR_RVALUE); 
				  $$ -> name = $1;
				  $$ -> S1 = $3;
				  $$ -> symbol = p;
				  $$ -> declared_type = p->Type;
				}
                        | Constant 
				{ $$ = $1; }
                        | '(' Expr ')' 
				{ $$ = $2; }
                        | '!' Factor 
				{ if($2 -> declared_type != A_Decaf_BOOL) {
				  	yyerror("type mismatch, expecting a boolean");
					exit(1);
				  }
				  
				  $$ = ASTCreateNode(A_EXPR);
				  $$ -> operator = A_NOT;
				  $$ -> S1 = $2;
				  $$ -> declared_type = A_Decaf_BOOL;
				  $$ -> name = TEMP_CREATE();
				  $$ -> symbol = Insert($$->name, A_Decaf_BOOL, ID_Sub_Type_Scalar, LEVEL, 1, OFFSET, NULL);
				  OFFSET++;
				}
                        | '-' Factor 
				{ if($2 -> declared_type != A_Decaf_INT) {
				  	yyerror("type mismatch, expecting an integer");
					exit(1);
				  }
				  
				  $$ = ASTCreateNode(A_EXPR);
				  $$ -> operator = A_UMINUS;
				  $$ -> S1 = $2;
				  $$ -> declared_type = A_Decaf_INT;
				  $$ -> name = TEMP_CREATE();
				  $$ -> symbol = Insert($$->name, A_Decaf_INT, ID_Sub_Type_Scalar, LEVEL, 1, OFFSET, NULL);
				  OFFSET++;
				}
                        ;

BoolConstant            : T_TRUE 
				{ $$ = ASTCreateNode(A_CONSTANT_BOOL);
				  $$ -> value = 1;
				  $$ -> declared_type = A_Decaf_BOOL;
			 	}
	                | T_FALSE 
				{ $$ = ASTCreateNode(A_CONSTANT_BOOL);
				  $$ -> value = 2;
				  $$ -> declared_type = A_Decaf_BOOL;
				}
	                ;
	     
ArrayType               : '[' T_INTCONSTANT ']' Type
				{ $$ = ASTCreateNode(A_ARRAY_TYPE);
				  $$ -> value = $2;
				  $$ -> declared_type = $4;
				}
                        ;
          
Constant                : T_INTCONSTANT 
				{ $$ = ASTCreateNode(A_CONSTANT_INT);
				  $$ -> value = $1; 
				  $$ -> declared_type = A_Decaf_INT;
				}
                        | BoolConstant 
				{ $$ = $1; }
                        ;
                        
%%	/* end of rules, start of program */

int main(int argc, char *argv[])
{ 
	int i=0;
	FILE* fp;
	char * name;
	name = strdup("output");
	while (i<argc) {
		if (strcmp("-d", argv[i])==0)
			mydebug=1;
	
		if (strcmp("-o", argv[i])==0) {
			name=strdup(argv[i+1]);
		}
	i++;
	}
	strcat(name, ".s");
	fp = fopen(name, "w");
	
	if (mydebug) printf("Opening %s\n",name);
	yyparse();
	if (mydebug) Display();
	if (mydebug) printf("\nParsing Completed\n\n");
	if (mydebug) ASTprint(0,Program);
	if (mydebug) printf("\nFinished printing AST\n");
	EMITAST(Program, fp);

}


