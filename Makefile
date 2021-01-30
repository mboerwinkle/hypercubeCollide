.PHONY: clean
.DEFAULT: libhypercube.so

ifneq ($(BUILD), debug)
	BUILD = release
endif

CFLAGS = -Wall -DDIM=3
ifeq ($(BUILD), debug)
	CFLAGS += -O0 -g
else
	CFLAGS += -O3 -DNDEBUG
endif

LFLAGS = -lm
CC = gcc

libhypercube.so: *.c *.h
	$(CC) $(CFLAGS) -fPIC -c *.c
	$(CC) -shared *.o -o libhypercube.so $(LFLAGS)

libhypercube.a: *.c *.h
	$(CC) $(CFLAGS) -c *.c
	ar rsv libhypercube.a *.o

clean:
	rm -f *.o *.a *.so
