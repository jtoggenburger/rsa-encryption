CC = clang
CFLAGS = -Wall -Wpedantic -Werror -Wextra $(shell pkg-config --cflags gmp)
LFLAGS = -lm -g $(shell pkg-config --libs gmp)

all: keygen encrypt decrypt

keygen: keygen.o rsa.o numtheory.o randstate.o
	$(CC) -o keygen keygen.o rsa.o numtheory.o randstate.o $(LFLAGS)

encrypt: encrypt.o rsa.o numtheory.o randstate.o
	$(CC) -o encrypt encrypt.o rsa.o numtheory.o randstate.o $(LFLAGS)

decrypt: decrypt.o rsa.o numtheory.o randstate.o
	$(CC) -o decrypt decrypt.o rsa.o numtheory.o randstate.o $(LFLAGS)

keygen.o: keygen.c
	$(CC) $(CFLAGS) -c keygen.c

encrypt.o: encrypt.c
	$(CC) $(CFLAGS) -c encrypt.c

decrypt.o: decrypt.c
	$(CC) $(CFLAGS) -c decrypt.c

rsa.o: rsa.c
	$(CC) $(CLFAGS) -c rsa.c

numtheory.o: numtheory.c
	$(CC) $(CFLAGS) -c numtheory.c

randstate.o: randstate.c
	$(CC) $(CFLAGS) -c randstate.c

clean:
	rm -f *.o keygen encrypt decrypt

format:
	clang-format -i -style=file *.[ch]




