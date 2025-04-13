GCC_FLAGS=-Wall -g
GCC_LIB_FLAGS=-fpic -static
CC=gcc
EXE=main_parser main_cpu

all: $(EXE)

%.o: %.c
	$(CC) $(GCC_FLAGS) -c $^
main_cpu : main_cpu.o cpu.o parser.o segment.o exercice1.o
	$(CC) $(GCC_FLAGS) -o $@ $^	
	
main_cpu.o: main_cpu.c
	$(CC) $(GCC_FLAGS) -c $<
	
main_parser : main_parser.o parser.o segment.o exercice1.o
	$(CC) $(GCC_FLAGS) -o $@ $^	

main_parser.o: main_parser.c
	$(CC) $(GCC_FLAGS) -c $<

addressing.o: addressing.c addressing.h cpu.o
	$(CC) $(GCC_FLAGS) -c $<

cpu.o: cpu.c cpu.h parser.o
	$(CC) $(GCC_FLAGS) -c $<
	
parser.o: parser.c parser.h segment.o
	$(CC) $(GCC_FLAGS) -c $<

segment.o: segment.c segment.h exercice1.o
	$(CC) $(GCC_FLAGS) -c $<

exercice1.o: exercice1.c exercice1.h
	$(CC) $(GCC_FLAGS) -c $<

lib%.a: %.c
	$(CC) $(GCC_FLAGS) $(GCC_LIB_FLAGS) -c -o $@ $^

clean:
	rm -f *.o $(EXE) *~ 
