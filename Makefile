all:
	gcc -O3 -Wall -Werror -c reader.c -o reader.o
	gcc -O3 -Wall -Werror -c main.c -o main.o

	gcc -o wiegand2mqtt main.o reader.o -pthread -lwiringPi -lrt
