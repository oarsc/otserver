#!/bin/sh

if [ -f "/.dockerenv" ] || [ -f "/.dockerinit" ]; then
  OTSERVER_PATH=/otserver
  mkdir -p "$OTSERVER_PATH/build"
  cd "$OTSERVER_PATH/build"
  cmake -DUSE_MYSQL=On ..
  make -j${1:-$(nproc)}
else
  echo " [#] Running command on docker"

  if [ "$(docker inspect -f '{{.State.Running}}' "ots")" = "false" ]; then
    SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
    sh "$SCRIPT_DIR/setup-and-run-docker.sh"
  fi

  docker exec -it ots compile.sh $1
fi
