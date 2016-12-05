PROG       	= compiler
SRCS       	= main.c syntax.tab.c lex.yy.c ast.c codegen.c interpreter.c
CFLAGS     	=
FLEX_FLAGS 	=
BISON_FLAGS	= -d -v

TEST_FILES 	= $(wildcard test/*.pl0)
TEST_FLAGS 	= -tcro

all: $(PROG)

$(PROG): $(SRCS)
	clang -o $@ $(CFLAGS) $(SRCS)

lex.yy.c: lexical.l syntax.tab.h ast.h
	flex $(FLEX_FLAGS) $<

syntax.tab.c syntax.tab.h: syntax.y ast.h
	bison $(BISON_FLAGS) $<

.PHONY: clean test

clean:
	-rm -f *.yy.c *.tab.c *.tab.h *.output test/*.ast test/*.pcode test/*.hex
	-rm -f $(PROG)

test:
	@for file in $(TEST_FILES); do printf "\nTesting $$file\n"; ./$(PROG) $(TEST_FLAGS) $$file; done
