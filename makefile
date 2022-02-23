CC=clang-11
TARGET=main

all:	main.o 
	$(CC) main.c -o $(TARGET)

clean:
	rm *.o $(TARGET)
