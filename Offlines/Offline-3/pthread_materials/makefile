CC = g++
CFLAGS = -pthread -std=c++11 -Wall
TARGET = peaky_blinders
SOURCE = main.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET) output.txt

run: $(TARGET)
	./$(TARGET) input.txt output.txt

.PHONY: all clean run