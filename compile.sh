#!/bin/sh

if [ -f "/.dockerenv" ] || [ -f "/.dockerinit" ]; then
  OTSERVER_PATH=/otserver
  mkdir -p "$OTSERVER_PATH/build"
  cd "$OTSERVER_PATH/build"
  cmake -DUSE_MYSQL=On ..
  make -j${1:-$(nproc)}
else
  echo " [ ◆  ] Running command on docker"
  SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
  sh "$SCRIPT_DIR/do" compile.sh "$@"
fi
