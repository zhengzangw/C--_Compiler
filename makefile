SRC = Code
UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	LEX_FLAGS = -lfl
endif
ifeq ($(UNAME_S), Darwin)
	LEX_FLAGS = -ll
endif
CI = ./CI4C--_Compiler/autotest.sh

CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -std=c99 -g -Werror -fsanitize=address

CFILES = $(shell find $(SRC)/ -name "*.c")
OBJS = $(CFILES:.c=.o)
LFILE = $(shell find $(SRC)/ -name "*.l")
YFILE = $(shell find $(SRC)/ -name "*.y")
LFC = $(shell find $(SRC)/ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
YFC = $(shell find $(SRC)/ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
LFO = $(LFC:.c=.o)
YFO = $(YFC:.c=.o)

parser: syntax $(filter-out $(LFO),$(OBJS))
	$(CC) $(CFLAGS) -o parser $(filter-out $(LFO),$(OBJS)) $(LEX_FLAGS) -ly
	@cp parser $(SRC)

main_1: $(SRC)/main.c $(SRC)/common.h
	$(CC) $(CFLAGS) -DLab=1 -c -o $(SRC)/main.o $(SRC)/main.c
main_2: $(SRC)/main.c $(SRC)/common.h
	$(CC) $(CFLAGS) -DLab=2 -c -o $(SRC)/main.o $(SRC)/main.c
main_3: $(SRC)/main.c $(SRC)/common.h
	$(CC) $(CFLAGS) -DLab=3 -c -o $(SRC)/main.o $(SRC)/main.c
main_4: $(SRC)/main.c $(SRC)/common.h
	$(CC) $(CFLAGS) -DLab=4 -c -o $(SRC)/main.o $(SRC)/main.c


syntax: lexical syntax-c
	$(CC) $(CFLAGS) -c $(YFC) -o $(YFO)

lexical: $(LFILE)
	$(FLEX) -o $(LFC) $(LFILE)

syntax-c: $(YFILE)
	$(BISON) -o $(YFC) -d -v $(YFILE)

-include $(patsubst %.o, %.d, $(OBJS))

.PHONY: clean test
test: clean parser
	./parser ./Test/test1.cmm ./Test/test1.s
test1: clean main_1 parser
	$(CI) ./parser -l 1 -q
test2: clean main_2 parser
	$(CI) ./parser -l 2 -q
test3: clean main_3 parser
	$(CI) ./parser -l 3 -t 1 --ins -q
test4: clean main_4 parser
	$(CI) ./parser -l 4 -t 2 -q
testall:
	make test1
	make test2
	make test3
	make test4
clean:
	rm -f $(SRC)/parser $(SRC)/lex.yy.c $(SRC)/syntax.tab.c $(SRC)/syntax.tab.h $(SRC)/syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
	rm -f $(SRC)/*~

