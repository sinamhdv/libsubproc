CC=gcc
INCLUDE_DIR=../include
LIBRARY_PATH=../bin
CFLAGS=-Wall -I$(INCLUDE_DIR) -L./$(LIBRARY_PATH) -lsubproc -static

BINS=redirection_example interaction_example buffering_example

all: $(BINS)

%: %.c
	$(CC) $< $(CFLAGS) -o $@

clean:
	rm -f $(BINS)
