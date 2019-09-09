CC=cc
CFLAGS=-I.

all: b64enc b64dec b32enc b32dec b16enc b16dec

b64enc: base64.o
	$(CC) b64enc.c base64.o -o b64enc

b64dec: base64.o
	$(CC) b64dec.c base64.o -o b64dec

b16enc: base64.o
	$(CC) b16enc.c base64.o -o b16enc

b16dec: base64.o
	$(CC) b16dec.c base64.o -o b16dec

b32enc: base64.o
	$(CC) b32enc.c base64.o -o b32enc

b32dec: base64.o
	$(CC) b32dec.c base64.o -o b32dec

base64.o: base64.c
	$(CC) -c base64.c

.PHONY: clean
clean:
	rm -f *.o b64dec b64enc b32enc b32dec b16enc b16dec
