CC=gcc

INCLUDEDIR=include
SRCDIR=src
BINDIR=bin
OBJDIR=obj
TESTDIR=test

CFLAGS=-Wall -I$(INCLUDEDIR) -g -DDEBUG

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

TESTBIN=$(BINDIR)/run_tests
DYNAMIC_BIN=$(BINDIR)/libsubproc.so

all: $(DYNAMIC_BIN)

$(DYNAMIC_BIN): $(OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: all $(TESTBIN)
	LD_LIBRARY_PATH=./bin ./$(TESTBIN)

$(TESTBIN): $(TESTDIR)/test.c
	$(CC) $(CFLAGS) $< -L./$(BINDIR) -lsubproc -o $@

clean:
	rm -f $(OBJDIR)/* $(BINDIR)/*
