#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "posix_sockets.h"
#include "mqtt.h"
#include "reader.h"

const char *hostname = "apollo.local";	//"localhost";
const char *port = "1883";
const char *topicToken = "/out/deckNorth/token";
const char *topicPin = "/out/deckNorth/pin";

void* client_refresher(void* client)
{
	while(1) 
    	{
        	mqtt_sync((struct mqtt_client*) client);
        	usleep(200000U);
    	}
    	return NULL;
}

void publish_callback(void** unused, struct mqtt_response_publish *published) 
{
    /* not used */
}

void exit_prog(int status, int sockfd, pthread_t *client_daemon)
{
	if (sockfd != -1) 
		close(sockfd);
	if (client_daemon != NULL) 
		pthread_cancel(*client_daemon);
    	exit(status);
}

int main(int argc, char *argv[])
{
	wiegandInit(READER_PIN_0, READER_PIN_1);

	/* MQTT */
	/* open the non-blocking TCP socket (connecting to the broker) */
	int sockfd = open_nb_socket(hostname, port);
	//note: mqtt server may not be ready, yet
	if(sockfd == -1)
	{
		printf("Server not yey ready?\n");
		sleep(5);
		sockfd = open_nb_socket(hostname, port);
	}
	if (sockfd == -1) 
	{
	        perror("Failed to open socket: ");
        	exit_prog(EXIT_FAILURE, sockfd, NULL);
	}

	/* setup a client */
	struct mqtt_client client;
    	uint8_t sendbuf[2048]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
    	uint8_t recvbuf[1024]; /* recvbuf should be large enough any whole mqtt message expected to be received */
    	mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback);
    	/* Create an anonymous session */
    	const char* client_id = NULL;
    	/* Ensure we have a clean session */
    	uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    	/* Send connection request to the broker. */
    	mqtt_connect(&client, client_id, NULL, NULL, 0, NULL, NULL, connect_flags, 400);

    	/* check that we don't have any errors */
    	if (client.error != MQTT_OK) 
	{
        	fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
       		exit_prog(EXIT_FAILURE, sockfd, NULL);
    	}

    	/* start a thread to refresh the client (handle egress and ingree client traffic) */
    	pthread_t client_daemon;
    	if(pthread_create(&client_daemon, NULL, client_refresher, &client))
       	{
       		fprintf(stderr, "Failed to start client daemon.\n");
        	exit_prog(EXIT_FAILURE, sockfd, NULL);
	}
	
	printf("Ready!\n");

	uint8_t pin[32];
	int pinIdx = 0;
	struct timespec last;
	while (1)
	{
		int bitLen = wiegandGetPendingBitCount();
        	if (bitLen == 0)
	       	{
            		usleep(10000);	//org 5000
        	}
	       	else
	       	{
            		uint8_t data[100];
            		bitLen = wiegandReadData((void *)data, sizeof(data));
			int bytes = bitLen / 8 + 1;

			if(bytes > 1)
			{
				/* token */
				char token[32];
				for (int i = 0; i < bytes && i < sizeof(token)/2 - 1; i++)
                			sprintf(&token[i*2], "%02X", (int)data[i]);
				printf("Token %s\n", token);

				/* publish */
        			mqtt_publish(&client, topicToken, token, strlen(token) + 1, MQTT_PUBLISH_QOS_1);
			}
			else
			{
				/* key */
				printf("Key %02X @ %d\n", data[0], pinIdx);

				if(pinIdx < sizeof(pin))
					pin[pinIdx++] = data[0];
    				clock_gettime(CLOCK_MONOTONIC_COARSE, &last);
			}
		}

		if(pinIdx > 0)
		{
			struct timespec now;
			clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
			if(pin[pinIdx-1] == 0x0B || pinIdx >= 8 || now.tv_sec - last.tv_sec > 5)
			{
				/* token */
				char pinStr[32];
				for (int i = 0; i < pinIdx && i < sizeof(pin)/2 - 1; i++)
					sprintf(&pinStr[i*2], "%02X", (int)pin[i]);
				printf("PIN: %s\n", pinStr);

				/* publish */
        			mqtt_publish(&client, topicPin, pinStr, strlen(pinStr) + 1, MQTT_PUBLISH_QOS_1);
				
				/* reset state */
				pinIdx = 0;
			}
		}
	}

   	/* exit */ 
    	exit_prog(EXIT_SUCCESS, sockfd, &client_daemon);
	return 0;
}	
