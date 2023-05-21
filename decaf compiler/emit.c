/*
 * 
     Levi Lopez
     December 1, 2022

*/

/*  Subprograms which output GAS code.
 
    Shaun Cooper Spring 2015
    Updated Spring 2017 to utilize the data segement portion of MIPS instead of advancing the Stack and Global pointers
    to accomodate global variables.
    Shaun Cooper December 2020 updated for DECAF format
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#include "emit.h"


char  * CURRENT_FUNC; // need to know Current Function for return on main


// A data structure linked list which allows to keep track of current
// labels for break and continue
struct LabelList {
	char * Label;
	struct LabelList *next;
};


// global variables to maintain break and continue targets
struct LabelList *GOBACK=NULL;
struct LabelList *GETOUT=NULL;


// Push label on to stack
// PRE : Label to push and target list
// POST: New label list with given label pushed on to stack
struct LabelList * PushLabel (char * L, struct LabelList * target) 
{
	struct LabelList * p;
	p = (struct LabelList *) (malloc (sizeof (struct LabelList)));
	p->Label = L;
	p->next = target;
	return p;
}


static int GLABEL=0;  // global label counter


// Creates one_up labels of the form _L%d
// PRE: none
// POST: new copy of strring of the form _L% and GLABEL is incremented
char * genlabel()
{  
	char s[100];
	char * S1;
	sprintf(s, "_L%d",GLABEL);
	GLABEL++;
	S1 = strdup(s);
	return S1;
}


// Emit the HEADER
// PRE: PTR to AST, file pointer *fp
// Post: required header for GAS in FILE fp
void Emit_Global_headers(FILE *fp, ASTnode *program)
{
	char * s;
	emit(fp,"","","");
	emit(fp,"","","");
	s = strdup("\t\tPackage ");
	emit(fp,"","", strcat(s, program->name));
	emit(fp,"","","");
	emit(fp, "", ".section .rodata","start of start of the RODATA section, strings first\n");
	emit(fp, "percentD:", ".string \"%d\"", "ALWAYS needed for print int");
	emit(fp, "","","NEXT lines are all the strings in the program");
	Emit_Write_Global_Strings(fp, program); // emits global strings
	emit(fp, "", "", "END of print all the strings in the program\n");
    emit(fp,"",".data","start of the DATA section, strings first\n\n");
	emit(fp,"",".comm _SCANFHOLD,8,8","MANDITORY space for SCANF reads");
    emit(fp,"","","");
    Emit_Global(fp, program->S1); // emits global variables
	emit(fp, "", "", "END of DATA section for variables\n\n");
	emit(fp, "", ".text", "start of CODE segment\n");

	// finished with header
	
}// of Emit_Global_headers


// This function outputs through global "fp" all of the global variables
// PRE: PTR to globals, file pointer *fp
// POST: GAS code for every GLOBAL variable at level 0
void Emit_Global(FILE *fp, ASTnode *p)
{
	char * s;
	char * s2;
	char s3[100];
	
	if (p==NULL) return;
 	if (p->value!=0) { // if no initial value and an array
 		s = strdup(".comm ");
 		strcat(s,p->name);
 		sprintf(s3, ",%d,%d",WSIZE*p->value,WSIZE*p->value); // multiplies WSIZE by the size of the array
 		s2 = strdup(s3);
 		strcat(s,s2);
 		emit(fp,"",s,"define a global variable");
 		emit(fp,"",".align 8","");
 	} else if (p->S2!=NULL){ // if initial value and not an array
 		s = strdup(p->name);
 		strcat(s,":");
 		sprintf(s3, ".long %d", p->S2->value);
 		s2 = strdup(s3);
 		emit(fp,s,s2,"define a global variable with an initial value");
 		emit(fp,"",".align 8","");
 	} else { // if no initial value and not an array
 		s = strdup(".comm ");
 		strcat(s,p->name);
 		sprintf(s3, ",%d,%d",WSIZE,WSIZE);
 		s2 = strdup(s3);
 		strcat(s,s2);
 		emit(fp,"",s,"define a global variable");
 		emit(fp,"",".align 8","");
 	}
	
 	Emit_Global(fp, p->next); // recursively call function to ensure all global variables are written

}


// Program to write out the label, string pair
// PRE: PTR to global strings, file pointer *fp
// POST: GAS code for all strings printed with a label
void Emit_Write_Global_Strings(FILE *fp, ASTnode *program)
{
	char * s;
	char s2[100];
	char * s3;
	char * s4;
    if (program == NULL) return; // if pointer does not exist, leave
	if (program->declared_type == A_Decaf_STRING && program->type==A_CONSTANT_STRING) { // if a string, generate a label and print
 		sprintf(s2, ".string %s", program->name);
 		s = strdup(s2);
		s3 = genlabel();
		s4 = strdup(s3);
		program->label=s3;
		emit(fp,strcat(s4,":"),s,"global string");
	}

	// recursively call function so that all global strings are written
	Emit_Write_Global_Strings(fp, program->S1);
	Emit_Write_Global_Strings(fp, program->S2);
	Emit_Write_Global_Strings(fp, program->next);

} // of Emit_Write_Global_Strings


// Routine to set %rax to a variable ident
// PRE : PTR to ident, file pointer *fp
// POST: %rax has the mem location of ident
void emit_ident(ASTnode *p, FILE *fp)
{
    char s[100];

	if (p->symbol->level == 0) { // global variables
		
		if (p->S1 == NULL) // if variables
		{ // we have a straight identifier   
		 sprintf(s,"mov $%s, %rax", p->name);
	     emit(fp,"",s, "ident global");
		}
		else // if arrays
		{ // find offset and put into %rax 
		    sprintf(s,"mov $%d, %rax", (p->symbol->offset+p->S1->value) * WSIZE);
			emit(fp,"",s, "array ident");
			emit(fp,"","add %rsp, %rax","ident");
		}
		
	} else { // not global variables, have to use offset instead
		
		if (p->S1 == NULL) // if variables
		{ // find offset and put into %rax
			sprintf(s,"mov $%d, %rax", p->symbol->offset * WSIZE);
			emit(fp,"",s, "ident");
			emit(fp,"","add %rsp, %rax","ident");
		}
		else // if arrays
		{ // find offset and put into %rax
       		sprintf(s,"mov $%d, %rax", (p->symbol->offset+p->S1->value) * WSIZE);
			emit(fp,"",s, "array ident");
			emit(fp,"","add %rsp, %rax","ident");
		}
	}
	
} // of function emit_ident


// Given a PTR to a function call, handle it
// PRE : PTR to function call, file pointer *fp
// POST: GAS code for parameters put into registers, and function is called
void handle_function_call(ASTnode *p, FILE *fp)
{
	char s[100];
	ASTnode *fparm, *arglist, *args;

	if (p->symbol->SubType ==  ID_Sub_Type_Extern) { // if pointer is of type extern, redirect to extern call function
		handle_extern_function_call(p, fp);
		return;
	}

	fparm=p->symbol->fparms;
	arglist=p->S1;
  
    emit(fp,"","","about to call a function, set up each parameter in the new activation record"); // begin setting up function parameters
  
  	args=arglist;
	int count=0;
	while (args!=NULL)
	{
		count++;
		args=args->next;
	} // get amount of parameters
	
  	args=arglist;
	int start=0;
	int max=start;
	int i=0;
	while (args!=NULL && count!=0) { // cycles through parameters, giving each an activation record and loading them into registers
		emit_expr(args->S1, fp);
		sprintf(s,"mov  %rax,  %d(%rsp)", args->symbol->offset*WSIZE);
		emit(fp,"",s, "store arg value in our runtime stack");
		sprintf(s,"mov %d(%rsp), %rax", args->symbol->offset*WSIZE);
		emit(fp,"",s, "load calc argument into RAX");
		sprintf(s,"mov %rax, %r%d", i+8);
		emit(fp,"",s, "store parameter value into appropriate registers\n");
		args=args->next;
		i=i+1;
		start=start+8;
	}
	
	// call function
	sprintf(s,"CALL %s",p->name);
	emit(fp,"",s,"jump and link to function\n");

}


// Given a PTR to an extern function call, handle it
// PRE : PTR to extern function call, file pointer *fp
// POST: GAS code for when extern function call is called, or error is thrown is extern function is not known
void handle_extern_function_call(ASTnode *p, FILE *fp)
{
	char s[100];
	ASTnode *fparm, *arglist, *args;

	fparm=p->symbol->fparms;
	arglist=p->S1;

	if (strcmp(p->name,"print_string") == 0) // if function is print_string, load up string label, and print
	{
		sprintf(s,"mov $%s, %rdi", p->S1->S1->label);
		emit(fp,"",s, "RDI is the label address");
		emit(fp,"","mov $0, %rax", "RAX needs to be zero");
		emit(fp,"","call printf", "print a string");
		emit(fp,"","","");
		return;
	} 
   
	if (strcmp(p->name,"print_int") == 0) // if function is print_int, calculate expression and print %rax output
	{
		emit(fp,"","", "");
		emit_expr(arglist->S1, fp); // calculate expression, expression is put into %rax
		emit(fp,"","","print an INT");
		emit(fp,"","mov %rax, %rsi","RSI needs the value");
		emit(fp,"","mov $percentD, %rdi","RDI needs to be the int format");
		emit(fp,"","mov $0, %rax","RAX needs to be 0");
		emit(fp,"","call printf","print a number from expression\n");
		return;
	}

	if (strcmp(p->name,"read_int") == 0) // if function is read_int, set up scanf and load value into %rax
	{
		emit(fp,""," ","Ask user for input");
		emit(fp,"","mov $_SCANFHOLD, %rsi","read in a number");
		emit(fp,"","mov $percentD , %rdi","rdi has integer format for scanf");
		emit(fp,"","mov $0 , %rax","No other parameters for scanf");
		emit(fp,"","call  __isoc99_scanf","call read");
		emit(fp,"","mov _SCANFHOLD, %rax","bring value on STACK into RAX register for default value ");
		emit(fp,"","","");
		return;
	}
   
   fprintf(stderr, "Extern Method %s is unknown\n", p->name); // function is not any of the three
   exit(1); // exit program from error
   
}


// Given a PTR to a function, create function header and load arguments
// PRE : PTR to function, file pointer *fp
// POST: GAS code for function header and function parameter set up 
void emit_func_start(ASTnode *p, FILE *fp)
{
	
	char s[100];
	char * s2;
	ASTnode *args;
	sprintf(s,".globl %s",p->name);
	s2 = strdup(s);
	emit(fp,"",s2,"");	
	sprintf(s,".type %s, @function",p->name);
	s2 = strdup(s);
	emit(fp,"",s2,"");	
	sprintf(s,"%s:",p->name);
	s2 = strdup(s);
	emit(fp,s2,"","Start of Function\n\n");

	// standard function header
	emit(fp,"",".cfi_startproc","STANDARD FUNCTION HEADER FOR GAS");
	emit(fp,"","pushq %rbp","STANDARD FUNCTION HEADER FOR GAS");
	emit(fp,"",".cfi_def_cfa_offset 16","STANDARD FUNCTION HEADER FOR GAS");
	emit(fp,"",".cfi_offset 6, -16","STANDARD FUNCTION HEADER FOR GAS");
	emit(fp,"","movq %rsp, %rbp","STANDARD FUNCTION HEADER FOR GAS");
	emit(fp,"",".cfi_def_cfa_register 6","STANDARD FUNCTION HEADER FOR GAS\n\n");
	CURRENT_FUNC=p->name;  // so we know what it is in return statement
  
	sprintf(s,"subq $%d, %rsp", p->symbol->mysize*WSIZE); // carve out activation record
	emit(fp,"",s,"carve out activation record for function");

	args=p->S1;
	int i=0;
	while (args!=NULL) { // read in all arguments
		sprintf(s,"mov %r%d, %d(%rsp)", i+8,args->symbol->offset*WSIZE);
		emit(fp,"",s, "copy actual to appropriate slot");
		args=args->next;
		i++;
	}
  
	emit(fp,"","","");

}


// Given a PTR to a return, create the return statement
// PRE : PTR to function, file pointer *fp
// POST: GAS code for return value into %rax
void emit_func_return(ASTnode *p, FILE *fp)
{
    char s[100];
   
    // if main or has no return value, return 0 into rax
    if (p->type!=A_RETURN && (p->declared_type==A_Decaf_VOID || strcmp(p->name,"main")==0))
    {
		emit(fp,"","mov $0, %rax","return  NULL zero (0)");
		emit(fp,"","leave","leave the function");
		emit(fp,"",".cfi_def_cfa 7, 8","STANDARD end function for GAS");
		emit(fp,"","ret\n","");
		return;
    }
    else if (p->type!=A_RETURN && strcmp(p->name,"main")) // if no return value
    {
		emit(fp,"","mov $0, %rax","return  NULL zero (0)");
		emit(fp,"","leave","leave the function");
		emit(fp,"",".cfi_def_cfa 7, 8","STANDARD end function for GAS");
		emit(fp,"","ret\n","");
		return;
    } 
	else { // if function does have return value, evaluate expression and put the output into %rax
		emit_expr(p->S1, fp);
		emit(fp,"","leave","leave the function");
		emit(fp,"",".cfi_def_cfa 7, 8","STANDARD end function for GAS");
		emit(fp,"","ret\n","");
	}

}


// Given a PTR to an expr, calculate recursively
// PRE : PTR to expr, file pointer *fp
// POST: GAS code for output in %rax
void emit_expr(ASTnode *p, FILE *fp)
{
   char s[100];

	// base cases
	switch (p->type) {
	
		case A_CONSTANT_INT : // if constant, put value into %rax
			sprintf(s,"mov $%d, %rax", p->value);
			emit(fp,"",s,"expr constant");	
			return;
		    break;

		case A_CONSTANT_BOOL : // if constant, put value into %rax
			if (p->value == 1)
				emit(fp,"","mov $1, %rax","expr constant");	
		    if (p->value == 2)
				emit(fp,"","mov $0, %rax","expr constant");	
            return;
			break;
			
		case A_METHOD : // if method call, call function, and put value into %rax
		    handle_function_call(p,fp);
			return;
            break;
			
		case A_VAR_RVALUE : // if variable, find the ident
			emit_ident(p, fp);
			emit(fp,"","mov (%rax), %rax","expr rvalue");	
			return;
			break;
			
		case A_VAR_LVALUE : 
			emit_ident(p, fp); // if variable, find the ident
			return;
			break;
			
			
	} // of switch for base cases

	// we have an expression

    // recursive case LHS
	emit_expr(p->S1,fp); // get left hand expression value into %rax
	
	if(p->S2!=NULL) { // if there is a right hand side
		sprintf(s,"mov %rax, %d(%rsp)", p->symbol->offset * WSIZE);
		emit(fp,"",s,"expr lhs"); // move %rax into a temporary storage

		// recursive case RHS
		emit_expr(p->S2,fp); // get right hand expression value into %rax
		
		// put everything together
		emit(fp,"","mov %rax, %rbx","expr rhs");
		sprintf(s,"mov %d(%rsp), %rax", p->symbol->offset * WSIZE);
		emit(fp,"",s,"expr lhs");	
	}

	// now execute the desired operation 
        switch(p->operator) {
   		case A_PLUS : 
	        emit(fp,"","add %rbx, %rax", "expr add"); // add
            break;
		case A_MINUS : 
	        emit(fp,"","sub %rbx, %rax", "expr sub"); // subtract
            break;
		case A_TIMES : 
	        emit(fp,"","imul %rbx, %rax", "expr times"); // multiply
            break;
		case A_DIVIDE : 
			emit(fp, "","mov $0, %rdx","");
	        emit(fp,"","idiv %rbx", "expr div"); // divide
            break;
   		case A_UMINUS: 
			emit(fp,"","neg %rax", "expr neg"); // negative
             break;
   		case A_MOD:
			emit(fp, "","mov $0, %rdx","");
	        emit(fp,"","idiv %rbx", "expr mod");
			emit(fp,"","mov %rdx, %rax","expr mod"); // modulus	
            break;
   		case A_AND: 
			emit(fp,"","and %rbx, %rax", "expr and"); // and
            break;
   		case A_OR: 
			emit(fp,"","or %rbx, %rax", "expr or"); // or
            break;
   		case A_NOT: 
			emit(fp,"","not %rax", "expr not"); // not
            break;
   		case A_LEQ: 
			emit(fp,"","cmp %rbx, %rax", "expr cmp");
			emit(fp,"","setle %al", "expr lessthaneq");
			emit(fp,"","mov $1, %rbx", "set rbx to one to filter rax");
			emit(fp,"","and %rbx, %rax", "filter RAX"); // less than equal to
            break;
   		case A_LT: 
			emit(fp,"","cmp %rbx, %rax", "expr cmp");
			emit(fp,"","setl %al", "expr lessthan");
			emit(fp,"","mov $1, %rbx", "set rbx to one to filter rax");
			emit(fp,"","and %rbx, %rax", "filter RAX"); // less than
            break;
   		case A_GT: 
			emit(fp,"","cmp %rbx, %rax", "expr cmp");
			emit(fp,"","setg %al", "expr greaterthan");
			emit(fp,"","mov $1, %rbx", "set rbx to one to filter rax");
			emit(fp,"","and %rbx, %rax", "filter RAX"); // greater than equal to
            break;
   		case A_GEQ: 
			emit(fp,"","cmp %rbx, %rax", "expr cmp");
			emit(fp,"","setge %al", "expr greaterthaneq");
			emit(fp,"","mov $1, %rbx", "set rbx to one to filter rax");
			emit(fp,"","and %rbx, %rax", "filter RAX"); // greater than
            break;
   		case A_EQ: 
			emit(fp,"","cmp %rbx, %rax", "expr cmp");
			emit(fp,"","sete %al", "expr eq");
			emit(fp,"","mov $1, %rbx", "set rbx to one to filter rax");
			emit(fp,"","and %rbx, %rax", "filter RAX"); // equal
            break;
   		case A_NEQ: 
			emit(fp,"","cmp %rbx, %rax", "expr cmp");
			emit(fp,"","setne %al", "expr neq");
			emit(fp,"","mov $1, %rbx", "set rbx to one to filter rax");
			emit(fp,"","and %rbx, %rax", "filter RAX"); // not equal to
            break;
		}

	// result is stored into %rax

}

//  Routine to do an assignment statement
//  PRE: PTR to assignment, file pointer *fp
//  POST: GAS code for assignment
void emit_assign(ASTnode *p, FILE *fp)
{
	char s[100];
	
	emit_expr(p->S2,fp); // get left hand expression, store into temporary register
	sprintf(s,"mov %rax, %d(%rsp)", p->symbol->offset * WSIZE); 
	emit(fp,"",s,"assign lhs");	
	emit_expr(p->S1,fp); // get right hand expression
	sprintf(s,"mov %d(%rsp), %rbx", p->symbol->offset * WSIZE);
	emit(fp,"",s,"assign rhs");	
	emit(fp,"","mov %rbx, (%rax)","finish assign\n");  // combine
}


//  If statement generation
//  PRE: PTR to if statement, file pointer *fp
//  POST: GAS code for if statement
void emit_if(ASTnode *p, FILE *fp)
{  
	char * label1; 
	char * label2;
	label1 = genlabel(); // generate labels for if statement
	label2 = genlabel();
	char * s;
	
	s=strdup("JE ");
	emit_expr(p->S1, fp); 
	
	// check if expression is true
	emit(fp,"","cmp $0, %rax","check if true"); 
	emit(fp,"",strcat(s,label1),"true"); // if true, go to this label
	EMITAST(p->S2->S1,fp); // emit inside block
	s=strdup("JMP ");
	emit(fp,"",strcat(s,label2),"end"); 
	emit(fp,strcat(label1,":"),"","ELSE TARGET");
	EMITAST(p->S2->S2,fp); // if there is an else, go here and emit inside block
	emit(fp,strcat(label2,":"),"","END IF"); // if not true, go to this label
}


//  While statement generation
//  PRE: PTR to while statement, file pointer *fp
//  POST: GAS code for while statement
void emit_while(ASTnode *p, FILE *fp)
{    
	char * label1; 
	char * label2;
	label1 = genlabel(); // generate labels for while statement
	label2 = genlabel();
	char * s;
	char * label12;
	char * label22;
	
	label12 = strdup(label1);
	label22 = strdup(label2);
	
	// push labels on to stack for coninue, break
	GOBACK = PushLabel(label1, GOBACK);
	GETOUT = PushLabel(label2, GETOUT);
	
	emit(fp,strcat(label12,":"),"","WHILE top"); // if true go to this label
	s=strdup("JE ");
	emit_expr(p->S1, fp);
	
	// check if expression is true
	emit(fp,"","cmp $0, %rax","check if true"); 
	emit(fp,"",strcat(s,label22),"end while"); 
	EMITAST(p->S2,fp); // emit inside block
	s=strdup("JMP ");
	emit(fp,"",strcat(s,label1),"jump back while"); 
	emit(fp,strcat(label2,":"),"","END WHILE"); // if false, go to this label
	GOBACK=GOBACK->next; // pop labels off of stack
	GETOUT=GETOUT->next;
}


//  Emits the whole AST as GAS code
//  PRE: PTR to AST, file pointer *fp
//  POST: GAS code for all nodes of the tree in *fp file
void EMITAST(ASTnode *p,FILE *fp)
{
   char s[100];
   char * s2;
   int i;
   if (p == NULL ) return;

       switch (p->type) {
        case A_PROGRAM :
            EMITAST(p->S1,fp);
            EMITAST(p->S2,fp);
			emit(fp,"",".ident \"GCC: (SUSE Linux) 7.5.0\"",""); // needed for ending program
			emit(fp,"",".section\t.note.GNU-stack,\"\",@progbits\n","");
            break;

        case A_EXTERN : 
		    EMITAST(p->S1,fp);     
        	break;

		case A_EXTERN_TYPE : 
         	break;

		case A_PACKAGE :   
 	        Emit_Global_headers(fp, p);
		    EMITAST(p->S1,fp); 
		    EMITAST(p->S2,fp); 
		    break;

        case A_VARDEC : 
            break;

        case A_METHODDEC : 
			emit_func_start(p, fp);
            if (p->S1 == NULL ) {}
            else { 
            EMITAST(p->S1,fp);
            }
            EMITAST(p->S2,fp); 
			emit_func_return(p, fp);
			emit(fp,"",".cfi_endproc","STANDARD end function for GAS"); // needed for ending methoddec
			sprintf(s,".size %s, .-%s",p->name,p->name);
			s2 = strdup(s);
			emit(fp,"",s2,"STANDARD end function for GAS\n");
            break;

        case A_METHODID : 
            break;

        case A_EXPR : 
			EMITAST(p->S1,fp);
			if (p->operator != A_NOT) 
                EMITAST(p->S2,fp);
            break;

        case A_BLOCK : 
            EMITAST(p->S1,fp);
            EMITAST(p->S2,fp);
            break;

		case A_BREAK : 
			s2=strdup("JMP ");
			emit(fp,"",strcat(s2,GETOUT->Label),"break"); // in the case of break, jump to current GETOUT label in the stack
		    break;

		case A_RETURN : 
			emit_func_return(p, fp);
		    break;

		case A_CONTINUE : 
			s2=strdup("JMP ");
			emit(fp,"",strcat(s2,GOBACK->Label),"continue"); // in the case of coninue, jump to current GOBACK label in the stack
		    break;

		case A_WHILE : 
			emit_while(p, fp);
		    break;
			 

		case A_METHOD : 
		    handle_function_call(p,fp);
            break;

        case A_METHODARG : 
		    EMITAST(p->S1,fp);
            break;

		case A_IF : 
			emit_if(p,fp);
            break;
			 
		case A_ELSE: 
		    EMITAST(p->S1,fp);
		    if (p->S1 != NULL ) {
		        EMITAST(p->S2,fp);
            }
		    break;

		case A_ASSIGN : 
			emit_assign(p,fp);
            break;

		case A_VAR_RVALUE : 
		    if (p->S1 != NULL ) {
                EMITAST(p->S1,fp);
		    }
		    EMITAST(p->S2,fp);
            break;

		case A_VAR_LVALUE :  
		    if (p->S1 != NULL ) {
                EMITAST(p->S1,fp);
	        }
		    EMITAST(p->S2,fp);
		    break;

		default: printf("%d:  unknown type in EMITAST\n",p->type);
		}
	
    EMITAST(p->next,fp); // always do the next

} // of EMITAST function


// Emits label, action, comment in a specific format for file output
// PRE : file pointer *fp, label, action, comment
// POST: GAS code emitted in a specific format
void emit(FILE *fp, char *label, char *action, char *comment) {
	if (label=="") { // label no
		if(action=="") { // action no
			if(comment=="") { // comment no
				fprintf(fp,"\n");
			}
			else { // comment yes
				fprintf(fp,"# %s\n", comment);
			}
		}
		else { // action yes
			if(comment=="") { // comment no
				fprintf(fp,"\t\t%s\n",action);
			}
			else { // comment yes
				fprintf(fp,"\t\t%s\t# %s\n", action, comment);
			}
		}
	}
	else { // label yes
		if(action=="") { // action no
			if(comment=="") {
				fprintf(fp,"%s\n",label);
			}
			else { // comment yes
				fprintf(fp,"%s\t\t\t# %s\n", label, comment);
			}
		}
		else { // action yes
			if(comment=="") { // comment no
				fprintf(fp,"%s\t\t%s\n", label, action);
			}
			else { // comment yes
				fprintf(fp,"%s\t\t%s\t# %s\n", label, action, comment);
			}
		}
	}
} // of emit function




