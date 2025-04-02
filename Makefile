GCC_FLAGS=-Wall -g
GCC_LIB_FLAGS=-fpic -static
CC=gcc
EXE=main_parser

all: $(EXE)

%.o: %.c
	$(CC) $(GCC_FLAGS) -c $^

main_parser : main_parser.o parser.o
	$(CC) $(GCC_FLAGS) -o $@ $^	

main_parser.o: main_parser.c
	$(CC) $(GCC_FLAGS) -c $@ $^

parser.o: parser.c parser.h segment.o
	$(CC) $(GCC_FLAGS) -c $@ $^

segment.o: segment.c segment.h exercice1.o
	$(CC) $(GCC_FLAGS) -c $@ $^

exercice1.o: exercice1.c exercice1.h
	$(CC) $(GCC_FLAGS) -c $@ $^

lib%.a: %.c
	$(CC) $(GCC_FLAGS) $(GCC_LIB_FLAGS) -c -o $@ $^

clean:
	rm -f *.o $(EXE) $(LIB) *~ 
