#!/bin/sh

# Script to run commands in docker. Execute it without parameters to build image.

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

if [ $# -eq 0 ]; then
    exec docker buildx build -t otsi "$SCRIPT_DIR"
    exit $?
fi

exec docker run -it --rm -v "$SCRIPT_DIR:/otserver" --network=host otsi "$@"
