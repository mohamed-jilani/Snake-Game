CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lSDL2 -lm
TARGET = snake
SOURCES = snake.c game.c graphics.c
HEADERS = snake.h game.h graphics.h

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: clean