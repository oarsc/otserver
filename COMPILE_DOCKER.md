## Creating Docker Image

Create the docker image by using the Dockerfile with:
```sh
docker buildx build -t otsi .
```

## Creating a container from the image

Run the next command and close the bash console afterwards:
```sh
docker run --network=host -itv $PWD:/otserver --name ots otsi
```

## Running the container

In order to compile, or to run the server, the container must be started.
```sh
docker start ots
```

To run a simple bash terminal run this.
```sh
docker exec -it ots sh
```

Remember to stop the container once stop using it to free system resources.
```sh
docker stop ots
```

## Compiling and running the code

In order to compile the code, simpy run
```sh
./compile.sh
# or
docker exec -it ots compile.sh
```

To run the server:
```sh
./otserv
# or
docker exec -it ots otserv
```
