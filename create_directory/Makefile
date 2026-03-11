SRC := src/main.c
OUT := run
CC := clang
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -O2

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

.PHONY: all clean

all: $(OUT)

clean:
	rm -f run run.*
