
AR	= ar
CC	= gcc

CFLAGS	= -Wall -g
LDFLAGS	= -L. -lminibar -L./pthread_compat -lpthread_compat -lm -pthread

PROGS	= libminibar.a testbar1 testbar threadbar

all: $(PROGS)

%.o: %.c minibar.h
	$(CC) -c $(CFLAGS) $<

pthread_compat/libpthread_compat.a:
	make -C pthread_compat_src libpthread_compat.a
	@-mkdir pthread_compat
	cp pthread_compat_src/*.h ./pthread_compat/
	cp pthread_compat_src/*.a ./pthread_compat/
	make -C pthread_compat_src clean

libminibar.a: minibar.h minibar.o
	$(AR) rc libminibar.a minibar.o

testbar1: pthread_compat/libpthread_compat.a libminibar.a testbar1.o
	$(CC) -o $@ $@.o $(LDFLAGS)

testbar: pthread_compat/libpthread_compat.a libminibar.a testbar.o
	$(CC) -o $@ $@.o $(LDFLAGS)

threadbar: pthread_compat/libpthread_compat.a libminibar.a threadbar.o
	$(CC) -o $@ $@.o $(LDFLAGS)

clean:
	rm -f *.o $(PROGS)

cleanall: clean
	rm -rf ./pthread_compat
