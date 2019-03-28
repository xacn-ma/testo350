
CC = gcc
CFLAGS = -Wall -I /usr/local/include/libusb-1.0 
LDFLAGS = -L /usr/local/lib
objs := $(patsubst %c, %o, $(shell ls *.c))

all: $(objs) 
	cc -o testo350 $(objs) $(CFLAGS) $(LDFLAGS) -lusb-1.0
main.o: main.c testo350.h
	$(CC) -c main.c $(CFLAGS)
testo350.o: testo350.c testo350.h
	$(CC) -c testo350.c $(CFLAGS) $(LDFLAGS) -lusb-1.0

clean:
	rm -f testo350  *.o *~
