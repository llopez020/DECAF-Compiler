#
# Levi Lopez
# December 1, 2022	
#
	
all: lex.l yacc.y ast.c ast.h symtable.h symtable.c emit.c emit.h
	lex lex.l
	yacc -d yacc.y
	gcc y.tab.c lex.yy.c ast.c symtable.c symtable.h emit.c emit.h -o decafcompile -lm

run: decafcompile
	./decafcompile

clean: decafcompile
	rm -f decafcompile
