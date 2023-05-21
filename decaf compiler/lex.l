/*   

     Levi Lopez
     December 1, 2022

*/

%{

#include <math.h>
#include <stdio.h>
#include "ast.h"

int aHexToInt(const char * x); /* returns an integer value from a hex value input */
int line(); /* returns line count */

int linecount = 1; /* records current line */
int debugsw = 0; /* debug switch */
#include "y.tab.h"

%}


/* tokens */
%% 

\/\/.*				{if (debugsw) fprintf(stderr,"Comment return\n");}

"&&"				{if (debugsw) fprintf(stderr,"T_AND found\n"); return(T_AND);}
"="				{if (debugsw) fprintf(stderr,"T_ASSIGN found\n"); return(T_ASSIGN);}
bool				{if (debugsw) fprintf(stderr,"T_BOOLTYPE found\n"); return(T_BOOLTYPE);}
break				{if (debugsw) fprintf(stderr,"T_BREAK found\n"); return(T_BREAK);}
continue			{if (debugsw) fprintf(stderr,"T_CONTINUE found\n"); return(T_CONTINUE);}
else				{if (debugsw) fprintf(stderr,"T_ELSE found\n"); return(T_ELSE);}
"=="				{if (debugsw) fprintf(stderr,"T_EQ found\n"); return(T_EQ);}
extern				{if (debugsw) fprintf(stderr,"T_EXTERN found\n"); return(T_EXTERN);}
false				{if (debugsw) fprintf(stderr,"T_FALSE found\n"); return(T_FALSE);}
func				{if (debugsw) fprintf(stderr,"T_FUNC found\n"); return(T_FUNC);}
">="				{if (debugsw) fprintf(stderr,"T_GEQ found\n"); return(T_GEQ);}
">"				{if (debugsw) fprintf(stderr,"T_GT found\n"); return(T_GT);}
"<"				{if (debugsw) fprintf(stderr,"T_LT found\n"); return(T_LT);}
if				{if (debugsw) fprintf(stderr,"T_IF found\n"); return(T_IF);}
int				{if (debugsw) fprintf(stderr,"T_INTTYPE found\n"); return(T_INTTYPE);}
"<<"				{if (debugsw) fprintf(stderr,"T_LEFTSHIFT found\n"); return(T_LEFTSHIFT);}
"<="				{if (debugsw) fprintf(stderr,"T_LEQ found\n"); return(T_LEQ);}
"!="				{if (debugsw) fprintf(stderr,"T_NEQ found\n"); return(T_NEQ);}
"||"				{if (debugsw) fprintf(stderr,"T_OR found\n"); return(T_OR);}
package				{if (debugsw) fprintf(stderr,"T_PACKAGE found\n"); return(T_PACKAGE);}
return				{if (debugsw) fprintf(stderr,"T_RETURN found\n"); return(T_RETURN);}
">>"				{if (debugsw) fprintf(stderr,"T_RIGHTSHIFT found\n"); return(T_RIGHTSHIFT);}
["]([^"\n]|\\(.|\n))*["]	{if (debugsw) fprintf(stderr,"T_STRINGCONSTANT found\n"); yylval.string=strdup(yytext); return(T_STRINGCONSTANT);}
string				{if (debugsw) fprintf(stderr,"T_STRINGTYPE found\n"); return(T_STRINGTYPE);}
true				{if (debugsw) fprintf(stderr,"T_TRUE found\n"); return(T_TRUE);}
var				{if (debugsw) fprintf(stderr,"T_VAR found\n"); return(T_VAR);}
void				{if (debugsw) fprintf(stderr,"T_VOID found\n"); return(T_VOID);}
while				{if (debugsw) fprintf(stderr,"T_AND found\n"); return(T_WHILE);}

[A-Za-z][0-9A-Za-z_]*	        {if (debugsw) fprintf(stderr,"T_ID found\n"); yylval.string=strdup(yytext); return(T_ID);}
0[xX][0-9A-Fa-f]*	        {if (debugsw) fprintf(stderr,"Hexadecimal Digit found\n"); yylval.value=aHexToInt(yytext); return(T_INTCONSTANT);}
[0-9][0-9]*	                {if (debugsw) fprintf(stderr,"Digit found\n"); yylval.value=atoi((const char *)yytext); return(T_INTCONSTANT);}
[ \t]		                {if (debugsw) fprintf(stderr,"Whitespace found\n");}
[;,\"=\-+*/%&|(){}\[\]!] 	{if (debugsw) fprintf(stderr,"Return a token %c\n",*yytext); return (*yytext);}
\n 		                {if (debugsw) fprintf(stderr,"Cariage return %c\n",*yytext); linecount++;}

%%


/* int AHexToInt (const char *) */
/* PRE: character string representing hexadecimal */
/* POST: integer based off hexadecimal value */
int aHexToInt(const char * x) {
	int total = 0; /* integer to return */
	int base = strlen(x)-3; /* find base power to exponentiate */

	for(int i=2; i < strlen(x); i++) {

		/* if character is a number 0-9 */
		if (x[i]=='0' || x[i]=='1' || x[i]=='2' || x[i]=='3' || x[i]=='4' || x[i]=='5' || x[i]=='6' || x[i]=='7' || x[i]=='8' || x[i]=='9') {
			total = total + (int) ((x[i]-'0') * pow(16, base)); /* add number to total */
		} else { /* if character is a letter */
			if(x[i]=='A')
				total = total + (int) (10 * pow(16, base));
			if(x[i]=='B')
				total = total + (int) (11 * pow(16, base));
			if(x[i]=='C')
				total = total + (int) (12 * pow(16, base));
			if(x[i]=='D')
				total = total + (int) (13 * pow(16, base));
			if(x[i]=='E')
				total = total + (int) (14 * pow(16, base));
			if(x[i]=='F')
				total = total + (int) (15 * pow(16, base));
		}
		base--;
	}

	return total; /* return integer */
}


/* int line() */
/* PRE: nothing */
/* POST: integer linecount */
int line() {
	return linecount; /* return recorded linecount (from \n) */
}


int yywrap(void)
{ return 1;}


