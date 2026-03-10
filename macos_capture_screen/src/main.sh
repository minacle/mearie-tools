#!/bin/sh
set -eu

FILEPATH="$(mktemp -qu)"
FILEPATH="${FILEPATH}.png"

if ! screencapture -C -T 0 -t png -x "$FILEPATH"
then
	rm -f "$FILEPATH"
	exit 1
fi

echo "$FILEPATH"
