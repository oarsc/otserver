#!/bin/sh

# Prevent running as root
if [ "$(id -u)" -eq 0 ]; then
  echo "This script should not be executed as root."
  exit 1
fi

IMAGE_NAME="otsi:latest"
CONTAINER_NAME="ots"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if docker images --format '{{.Repository}}:{{.Tag}}' | grep -q "^${IMAGE_NAME}$"; then
  echo " [ ◆ ] Image \"$IMAGE_NAME\" exists."

  if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo " [ ◆ ] Container \"$CONTAINER_NAME\" exists."

    if docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
      echo " [ ◆ ] Container \"$CONTAINER_NAME\" already running. Skipping."
    else
      echo " [ ◆ ] Running container \"$CONTAINER_NAME\"..."
      docker start "$CONTAINER_NAME"
    fi

  else
    echo " [ ◆ ] Creating and running container \"$CONTAINER_NAME\"..."
    docker run -dit --network=host -v "$SCRIPT_DIR:/otserver" --name "$CONTAINER_NAME" "$IMAGE_NAME" tail -f /dev/null
  fi

else
  echo " [ ◆ ] Creating image \"$IMAGE_NAME\"..."
  docker build -t "$IMAGE_NAME" "$SCRIPT_DIR"

  echo " [ ◆ ] Creating and running container \"$CONTAINER_NAME\"..."
  docker run -dit --network=host -v "$SCRIPT_DIR:/otserver" --name "$CONTAINER_NAME" "$IMAGE_NAME" tail -f /dev/null
fi
