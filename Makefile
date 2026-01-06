
AR	= ar
CC	= gcc

CFLAGS	= -Wall -g -I./pthread_compat
LDFLAGS	= -lm -pthread

PROGS	= libminibar.a testbar1 testbar threadbar

PTHREAD_COMPAT_OBJS	= pthread_compat/pthread_barrier.o pthread_compat/pthread_win32.o

all: $(PROGS)

pthread_compat/%.o: pthread_compat/%.c
	$(CC) -c -o $@ $(CFLAGS) $<

%.o: %.c minibar.h
	$(CC) -c $(CFLAGS) $<

libminibar.a: minibar.h minibar.o
	$(AR) rc libminibar.a minibar.o

testbar1: testbar1.o minibar.o $(PTHREAD_COMPAT_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

testbar: testbar.o minibar.o $(PTHREAD_COMPAT_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

threadbar: threadbar.o minibar.o $(PTHREAD_COMPAT_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o $(PROGS)
