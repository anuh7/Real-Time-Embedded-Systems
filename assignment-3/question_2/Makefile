INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -pg -g -Wcpp -Wall -lpthread -lrt $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= threads1.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	threads1

clean:
	-rm -f *.o *.d
	-rm -f threads1

threads1: threads1.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -lrt


depend:

.c.o:
	$(CC) $(CFLAGS) -c $<