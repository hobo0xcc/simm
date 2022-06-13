CC=gcc
SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)
BIN=simm
RM=rm -rf
CFLAGS=-Wall -Wextra -g

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) $(BIN) $(OBJ)
