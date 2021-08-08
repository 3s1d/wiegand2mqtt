#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "reader.h"


int main(int argc, char *argv[])
{
	wiegandInit(READER_PIN_0, READER_PIN_1);

	
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
				printf("Token: ");
				for (int i = 0; i < bytes; i++)
                			printf("%02X", (int)data[i]);
				printf("\n");

				//todo
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
			if(pin[pinIdx-1] == 0x0B || poinIdx >= 8 || now.tv_sec - last.tv_sec > 5)
			{
				/* token */
				printf("PIN: ");
				for (int i = 0; i < pinIdx; i++)
					printf("%02X", (int)pin[i]);
				printf("\n");

				//todo
				
				/* reset state */
				pinIdx = 0;
			}
		}
	}


	return 0;
}	
