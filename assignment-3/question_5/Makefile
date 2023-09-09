INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -pg -g -Wcpp -Wall -lpthread -lrt $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= thread.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	thread

clean:
	-rm -f *.o *.d
	-rm -f thread

thread: thread.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -lrt


depend:

.c.o:
	$(CC) $(CFLAGS) -c $<