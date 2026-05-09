CC = clang
CFLAGS = -Wall -Wextra -pedantic -std=c17 -g
TARGET = heap
SRC = src/main.c

.PHONY: all run debug clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	lldb -- ./$(TARGET)

clean:
	rm -f $(TARGET)