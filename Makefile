all:
	gcc -O3 -Wall -Werror -c reader.c -o reader.o
	gcc -O3 -Wall -Werror -c mqtt.c -o mqtt.o
	gcc -O3 -Wall -Werror -c mqtt_pal.c -o mqtt_pal.o
	gcc -O3 -Wall -Werror -c main.c -o main.o

	gcc -o wiegand2mqtt main.o reader.o mqtt.o mqtt_pal.o -pthread -lwiringPi -lrt
