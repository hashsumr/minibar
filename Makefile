
AR	= ar
CC	= gcc

CFLAGS	= -Wall -g
LDFLAGS	= -L. -lminibar -lm -pthread

PROGS	= libminibar.a testbar1 testbar

all: $(PROGS)

%.o: %.c minibar.h
	$(CC) -c $(CFLAGS) $<

libminibar.a: minibar.h minibar.o
	$(AR) rc libminibar.a minibar.o

testbar1: libminibar.a testbar1.o
	$(CC) -o $@ $@.o $(LDFLAGS)

testbar: libminibar.a testbar.o
	$(CC) -o $@ $@.o $(LDFLAGS)

clean:
	rm -f *.o $(PROGS)
