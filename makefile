main:
	gcc -o main main.c daemon.h

daemon:
	gcc -o daemon daemon.c daemon.h

all: main daemon