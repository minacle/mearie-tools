#!/bin/sh
set -eu

printf '{"@mearie":{"action":"image.attach","data":%s}}\n' "$MEARIE__image_urls"
