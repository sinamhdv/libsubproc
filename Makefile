CC=gcc

# edit the install prefix here
INSTALL_PREFIX=/usr

INSTALL_INCLUDES=$(INSTALL_PREFIX)/include/subproc
INSTALL_LIBS=$(INSTALL_PREFIX)/lib

INCLUDEDIR=include
SRCDIR=src
BINDIR=bin
OBJDIR=obj
TESTDIR=test

CFLAGS=-Wall -I$(INCLUDEDIR) -fPIC

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

HEADERS=$(wildcard $(INCLUDEDIR)/subproc/*.h)

TESTBIN=$(BINDIR)/run_tests
DYNAMIC_BIN=$(BINDIR)/libsubproc.so
STATIC_BIN=$(BINDIR)/libsubproc.a

$(shell mkdir -p $(BINDIR) $(OBJDIR))

all: $(DYNAMIC_BIN) $(STATIC_BIN)

debug: CFLAGS += -g -DDEBUG
debug: all

$(DYNAMIC_BIN): $(OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJS)

$(STATIC_BIN): $(OBJS)
	ar rcs $@ $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: all $(TESTBIN)
	LD_LIBRARY_PATH=./bin ./$(TESTBIN)

$(TESTBIN): $(TESTDIR)/test.c
	$(CC) $(CFLAGS) $< -L./$(BINDIR) -lsubproc -o $@

install: all
	mkdir -p $(INSTALL_INCLUDES)
	mkdir -p $(INSTALL_LIBS)
	install -m 755 -o $(shell whoami) $(DYNAMIC_BIN) $(INSTALL_LIBS)
	install -m 755 -o $(shell whoami) $(STATIC_BIN) $(INSTALL_LIBS)
	install -m 644 -o $(shell whoami) -t $(INSTALL_INCLUDES) $(HEADERS)

clean:
	rm -f $(OBJDIR)/* $(BINDIR)/*
