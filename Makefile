GCC_FLAGS=-Wall -g
GCC_LIB_FLAGS=-fpic -static
CC=gcc
EXE=main_parser main main_address

all: $(EXE)

%.o: %.c
	$(CC) $(GCC_FLAGS) -c $^

main_address : main_address.o cpu_core.o cpu.o parser.o segment.o exercice1.o
	$(CC) $(GCC_FLAGS) -o $@ $^	

main : main.o cpu_core.o cpu.o parser.o segment.o exercice1.o
	$(CC) $(GCC_FLAGS) -o $@ $^	
	
main_parser : main_parser.o parser.o segment.o exercice1.o
	$(CC) $(GCC_FLAGS) -o $@ $^	

main_address.o: main_address.c
	$(CC) $(GCC_FLAGS) -c $<

main.o: main.c
	$(CC) $(GCC_FLAGS) -c $<

main_parser.o: main_parser.c
	$(CC) $(GCC_FLAGS) -c $<

cpu_core.o: cpu_core.c cpu_core.h cpu.o
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
