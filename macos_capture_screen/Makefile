SRC := src/main.sh
OUT := run.sh

$(OUT): $(SRC)
	cp "$<" "$@"
	chmod +x "$@"

.PHONY: all clean

all: $(OUT)

clean:
	rm -f "$(OUT)"
