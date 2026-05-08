CC = gcc
CFLAGS = -Wall -g

main: main.o 
	$(CC) $(CFLAGS) -o main main.o include/dict.o include/A.o