/*  Abstract syntax tree code

    This code is used to define an AST node, 
    routine for printing out the AST
    defining an enumerated type so we can figure out what we need to
    do with this.  The ENUM is basically going to be every non-terminal
    and terminal in our language.

    Shaun Cooper February 2020

*/

/*   
     Levi Lopez
     December 1, 2022

     Changelog: 
     - Changed format of A_ASSIGN to better handshake with GAS output
*/


#include<stdio.h>
#include<malloc.h>
#include "ast.h" 


/* uses malloc to create an ASTnode and passes back the heap address of the newley created node */
ASTnode *ASTCreateNode(enum AST_Tree_Element_Type mytype)
{
    ASTnode *p;
    if (debugsw) fprintf(stderr,"Creating AST Node \n");
    p=(ASTnode *)malloc(sizeof(ASTnode)); // get head data
    p->type=mytype; // set up the Element type
    p->S1=NULL;
    p->S2=NULL;  // det default values
    p->value=0;
    return(p);
}

/* void PT(int ) */
/* PRE: integer (number of tabs) */
/* POST: nothing */
void PT(int howmany)
{
	 for(int i = 0; i < howmany-1; i++) {
	 	printf(" ");
         }
}

/* AST_Print_Type (enum ) */
/* PRE: enum TYPE */
/* POST: string representing type */
char* AST_Print_Type( enum AST_Decaf_Types t)
{
	switch (t) {

		case  A_Decaf_INT : return("INT");
	                              break;
	        case  A_Decaf_VOID : return("VOID");
	                              break;
	        case  A_Decaf_BOOL : return("BOOLEAN");
	                              break;
	        case  A_Decaf_STRING : return("STRING");
	                              break;
          default:  return("Unknown AST DECAF TYPE !!!\n");
	} // of switch

}// of AST_Print_Type


/*  Print out the abstract syntax tree */
void ASTprint(int level,ASTnode *p)
{
   int i;
   if (p == NULL ) return;
   else
     { 
       if (p->type != A_ELSE) PT(level); /*indent */
       switch (p->type) {
        case A_PROGRAM :
                     ASTprint(level+1,p->S1);  
                     ASTprint(level+1,p->S2);                                                          
                     break;

        case A_EXTERN : printf("EXTERN FUNC %s\n", p->name);
		     ASTprint(level+1,p->S1);     
		     PT(level);                                                     
        	     printf("END EXTERN with Type: %s\n\n", AST_Print_Type(p->declared_type));
        	     break;

	case A_EXTERN_TYPE : printf("EXTERN Type: %s\n", AST_Print_Type(p->declared_type));
         	     break;

        case A_PACKAGE : printf("Package : %s\n\n",p->name);
                     ASTprint(level+1,p->S1);  
		     ASTprint(level+1,p->S2);                                                          
                     break;

        case A_VARDEC : printf("Variable ");
                     printf("%s",p->name);
		     if (p->S1!=NULL)
			printf(" [%d]",p->value);
                     printf(" with type %s", AST_Print_Type(p->declared_type));
		     if (p->S2!=NULL && p->S2->value > 0 && p->declared_type == A_Decaf_INT)
                        printf(" = %d",p->S2->value);
		     else if (p->S2!=NULL && p->S2->value > 0 && p->declared_type == A_Decaf_BOOL) {
			if (p->S2->value == 1)
				printf(" = TRUE");
			if (p->S2->value == 2)
				printf(" = FALSE");
		     }
                     printf("\n");
                     break;

        case A_METHODDEC : printf("METHOD FUNCTION %s ",p->name);
		     printf("%s\n", AST_Print_Type(p->declared_type));
                     /* print out the parameter list */
                     if (p->S1 == NULL ) 
		      { PT(level+2); 
		        printf (" (VOID) "); }
                     else
                       { 
                         PT(level+2);
                         printf( "( \n");
                          ASTprint(level+2, p->S1);
                         PT(level+2);
                         printf( ") ");
                       }
                     printf("\n");
                     ASTprint(level+2, p->S2); // print out the block
                     break;

        case A_METHODID : printf("PARAMETER");
                     printf(" %s ", AST_Print_Type(p->declared_type));
		     printf("%s",p->name);
                     printf("\n");
                     break;

        case A_EXPR : printf("EXPR ");
                     switch(p->operator) {
   			case A_PLUS : printf(" + ");
                           break;
   			case A_MINUS: printf(" - ");
                           break;
   			case A_UMINUS: printf(" - ");
                           break;
   			case A_TIMES: printf(" * ");
                           break;
   			case A_DIVIDE: printf(" / ");
                           break;
   			case A_MOD: printf(" % ");
                           break;
   			case A_AND: printf(" && ");
                           break;
   			case A_OR: printf(" || ");
                           break;
   			case A_LEFTSHIFT: printf(" << ");
                           break;
   			case A_RIGHTSHIFT: printf(" >> ");
                           break;
   			case A_NOT: printf(" ! ");
                           break;
   			case A_LEQ: printf(" <= ");
                           break;
   			case A_LT: printf(" < ");
                           break;
   			case A_GT: printf(" > ");
                           break;
   			case A_GEQ: printf(" >= ");
                           break;
   			case A_EQ: printf("==");
                           break;
   			case A_NEQ: printf("!=");
                           break;
			default: printf(" unknown EXPR operator");
                       }
                     printf("\n");
                     ASTprint(level+1, p->S1);
		     if (p->operator != A_NOT) 
                        ASTprint(level+1, p->S2);
                     break;

        case A_BLOCK : printf("BLOCK STATEMENT\n",p->name);
                     ASTprint(level+1, p->S1);
                     ASTprint(level+1, p->S2);
                     break;

	case A_BREAK : printf("BREAK STATEMENT \n");
		     break;

	case A_RETURN : printf("RETURN STATEMENT \n");
                     ASTprint(level+1, p->S1);
		     break;

	case A_CONTINUE : printf("CONTINUE STATEMENT \n");
		     break;

	case A_WHILE : printf("WHILE STATEMENT \n");
		     ASTprint(level+1, p->S1);
                     ASTprint(level+2, p->S2);
		     break;

	case A_CONSTANT_INT : printf("CONSTANT INTEGER %d\n", p->value);
                     ASTprint(level+1, p->S1);
		     break;

	case A_CONSTANT_STRING : 
		     printf("CONSTANT STRING %s\n", p->name);
                     ASTprint(level+1, p->S1);
		     break;

	case A_CONSTANT_BOOL : printf("CONSTANT BOOLEAN ");
		     if (p->value == 1)
				printf("TRUE\n");
		     if (p->value == 2)
				printf("FALSE\n");
		     ASTprint(level+1, p->S1);
		     break;

	case A_METHOD : printf("METHOD CALL name: %s\n", p -> name);
                     PT(level+2);
                     printf( "( \n");
                     ASTprint(level+2, p->S1);
                     PT(level+2);
                     printf( ") "); 
                     printf("\n");
                     break;

        case A_METHODARG : printf("METHOD ARG\n");
		     ASTprint(level, p->S1);
                     break;

        case A_IF : printf("IF STATEMENT\n");
		     ASTprint(level+1, p->S1);
		     ASTprint(level+1, p->S2);
                     break;
	case A_ELSE: 
		    ASTprint(level+2, p->S1);
		    if (p->S1 != NULL ) {
         	          PT(level);
		          printf("ELSE\n");
		          ASTprint(level+1, p->S2);
                    }
		    break;



	case A_ASSIGN : printf("ASSIGNMENT STATEMENT\n");
		    ASTprint(level, p->S1);
		    ASTprint(level+1, p->S2);
                    break;

	case A_VAR_RVALUE : printf("Variable %s\n", p->name);
		    if (p->S1 != NULL ) {
                         PT(level);
                         printf( "[ \n");
                         ASTprint(level+2, p->S1);
                         PT(level);
                         printf( "] \n");
		    }
		    ASTprint(level+1, p->S2);
                    break;

	case A_VAR_LVALUE :   printf("Variable %s\n", p->name);
		    break;

        default: printf("unknown type in ASTprint\n");

       }
       ASTprint(level, p->next);
     }

}

/* int check_parameters (ASTnode , ASTnode) */
/* PRE: pointers to both sets of parameters */
/* POST: 1 or 0 indicating if parameters match or not */
int check_parameters(ASTnode *formal, ASTnode *actual) {
	ASTnode *p = formal;
	ASTnode *p2 = actual;

	while (p!=NULL) {
		if (p2==NULL) 
			return 0;
		if (p->declared_type!=p2->declared_type)
			return 0;
		p=p->next;
		p2=p2->next;
	}

	return 1;
}
