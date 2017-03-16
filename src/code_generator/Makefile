conv: conv.l conv.yy ast.h ast.cc context.h context.cc
	bison -d conv.yy
	flex conv.l
	g++ -g -std=c++0x -o $@ context.cc ast.cc conv.tab.cc lex.yy.c

.PHONY: clean
clean:
	-rm conv.tab.cc lex* *.hh
