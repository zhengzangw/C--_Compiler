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
CFLAGS = -std=c99 -g -Werror

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

syntax: lexical syntax-c
	$(CC) $(CFLAGS) -c $(YFC) -o $(YFO)

lexical: $(LFILE)
	$(FLEX) -o $(LFC) $(LFILE)

syntax-c: $(YFILE)
	$(BISON) -o $(YFC) -d -v $(YFILE)

-include $(patsubst %.o, %.d, $(OBJS))

.PHONY: clean test
test: parser
	./parser ./Test/test1.cmm ./Test/test1.ir
test1: parser
	./parser ./Test/test1.cmm
testall:
	$(CI) ./parser -l 3 --ins
clean:
	rm -f $(SRC)/parser $(SRC)/lex.yy.c $(SRC)/syntax.tab.c $(SRC)/syntax.tab.h $(SRC)/syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
	rm -f $(SRC)/*~
