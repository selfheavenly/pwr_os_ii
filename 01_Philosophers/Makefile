TARGET = dining_philosophers
CC = gcc
CFLAGS = -Wall -Wextra -pthread -std=c99 -O2

all: build

build:
	@echo "Compiling main.c..."
	$(CC) $(CFLAGS) -c main.c -o main.o
	@echo "Compiling philosopher.c..."
	$(CC) $(CFLAGS) -c philosopher.c -o philosopher.o
	@echo "Linking objects..."
	$(CC) $(CFLAGS) -o $(TARGET) main.o philosopher.o
	@echo "Cleaning temporary files..."
	rm -f main.o philosopher.o
	@echo "Build complete!"

clean:
	@echo "Cleaning project..."
	rm -f *.o $(TARGET)
