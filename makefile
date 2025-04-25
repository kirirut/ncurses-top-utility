CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LDFLAGS = -lncurses

SOURCES = src/main.c src/graph.c src/process.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = ncurses_top_utility

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
