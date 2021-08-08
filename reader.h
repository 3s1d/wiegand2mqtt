#ifndef _READER_H
#define _READER_H

#define READER_PIN_0 0 // GPIO Pin 17 | Green cable | Data0
#define READER_PIN_1 1 // GPIO Pin 18 | White cable | Data1

void wiegandInit(int d0pin, int d1pin);
int wiegandReadData(void* data, int dataMaxLen);
int wiegandGetPendingBitCount(void);


#endif
