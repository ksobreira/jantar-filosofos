CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu11 -pthread -Iinclude
SRC     = $(wildcard src/*.c)

all: hierarquia

hierarquia: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o filosofos_hierarquia

clean:
	rm -f filosofos_hierarquia
