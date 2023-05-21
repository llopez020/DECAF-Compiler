/*

     Levi Lopez
     December 1, 2022

*/

//  Subprograms which output GAS code.
//
//  Shaun Cooper Spring 2017
//  Updated Spring 2017 to utilize the data segement portion of NASM instead of advancing the Stack and Global pointers
//  to accomodate global variables.

// NASM is called on linux as
// gcc foo.s 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef EMIT_H
#define EMIT_H

#include "ast.h"
#include "symtable.h"
#define WSIZE 8  // number of bytes in a word
#define LOGWSIZE 3  // number of shifts to get to WSIZE
FILE *fp;

char *genlabel(); // needed in YACC for T_STRING_LIT

void Emit_Global_headers(FILE *fp, ASTnode *program);

void EMITAST(ASTnode *p,FILE *fp);
void emit_expr(ASTnode *p, FILE *fp);

void emit( FILE *fp, char *label, char *action, char *comment);

void emit_default_extern_methods(FILE *fp);


void Emit_Write_Global_Strings(FILE *fp, ASTnode *program);

char *ER(char *S, int offset,char *r);
char *ES(char *s, int offset);

void emit_ident(ASTnode *p, FILE *fp);

void emit_header( FILE *fp, int NUM, int offset);

void handle_function_call(ASTnode *p, FILE *fp);

void handle_extern_function_call(ASTnode *p, FILE *fp);

void emit_func_start(ASTnode *p, FILE *fp);


void emit_func_return(ASTnode *p, FILE *fp);

void emit_expr(ASTnode *p, FILE *fp);


void emit_assign(ASTnode *p, FILE *fp);

void emit_if(ASTnode *p, FILE *fp);
void emit_while(ASTnode *p, FILE *fp);

void Emit_Global(FILE *fp, ASTnode *p);
void EMITAST(ASTnode *p,FILE *fp);

#endif  // of EMIT.h
