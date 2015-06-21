LIBTOOL = libtool
VERSION = $(shell cat VERSION)
CFLAGS = -Wall -g -std=c99 -I$(shell pg_config --includedir) -fPIC
LDFLAGS = -L$(shell pg_config --libdir) -lpq -lgit2
SRCS = $(wildcard *.c) $(wildcard */*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

OS = $(shell uname | tr '[:upper:]' '[:lower:]')

include $(OS).mk

LIBS = libgit2-postgresql.a libgit2-postgresql.$(LIBEXT)

all: $(LIBS) main

libgit2-postgresql.a: $(OBJS)
	$(AR) rcs $@ $^
 
%.o: %.d
	$(CC) $*.c $(CFLAGS) -c -o $@
 
%.d: %.c
	$(CC) $(CFLAGS) -MM -MT $*.o $*.c > $*.d
 
-include $(DEPS)
 
clean:
	-find . -name "*.[od]" | xargs rm
	-rm -f $(LIBS)
 
.PHONY: clean all
