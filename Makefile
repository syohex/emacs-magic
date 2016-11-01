EMACS_ROOT ?= ../..
EMACS ?= emacs

CC      = gcc
LD      = gcc
CPPFLAGS = -I$(EMACS_ROOT)/src
CFLAGS = -std=gnu99 -ggdb3 -Wall -fPIC $(CPPFLAGS)

.PHONY : test

all: magic-core.so

magic-core.so: magic-core.o
	$(LD) -shared $(LDFLAGS) -o $@ $^ -lmagic

magic-core.o: magic-core.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f magic-core.so magic-core.o

test:
	$(EMACS) -Q -batch -L . $(LOADPATH) \
		-l test/test.el \
		-f ert-run-tests-batch-and-exit
