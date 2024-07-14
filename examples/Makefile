CC=gcc
INCLUDE_DIR=../include
LIBRARY_PATH=../bin
CFLAGS=-Wall -I$(INCLUDE_DIR) -L./$(LIBRARY_PATH) -lsubproc

BINS=redirection_example interaction_example buffering_example

all: $(BINS)

%: %.c
	$(CC) $< $(CFLAGS) -o $@
	@echo "\033[32m     " built $@. run with: LD_LIBRARY_PATH=$(LIBRARY_PATH) ./$@ "\033[0m"

clean:
	rm -f $(BINS)
