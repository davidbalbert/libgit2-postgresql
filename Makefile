CC = clang
CFLAGS = -Wall -g -std=c99 -I$(shell pg_config --includedir)
LDFLAGS = -L$(shell pg_config --libdir) -lpq -lgit2
SRCS = $(wildcard *.c) $(wildcard */*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
 
main: $(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@
 
%.o: %.d
	$(CC) $*.c $(CFLAGS) -c -o $@
 
%.d: %.c
	$(CC) $(CFLAGS) -MM -MT $*.o $*.c > $*.d
 
-include $(DEPS)
 
clean:
	-find . -name "*.[od]" | xargs rm
	-rm -f main
 
.PHONY: clean
