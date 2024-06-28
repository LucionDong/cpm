#ifndef _H_MCURS232_HELPER_FUNCTIONS_H_
#define _H_MCURS232_HELPER_FUNCTIONS_H_

#include <pthread.h>
#include <termios.h>

// Serial port data structure
typedef struct serialPort
{
	pthread_mutex_t eventMutex;
	pthread_cond_t eventReceived;
	pthread_t eventsThread1, eventsThread2;
	char *portPath, *friendlyName, *portDescription, *portLocation, *serialNumber;
	int errorLineNumber, errorNumber, handle, eventsMask, event, vendorID, productID;
	volatile char enumerated, eventListenerRunning, eventListenerUsesThreads;
} serialPort;

// Common port storage functionality
typedef struct serialPortVector
{
	serialPort **ports;
	int length, capacity;
} serialPortVector;

serialPort* fetchPort(serialPortVector* vector, const char* key);
void removePort(serialPortVector* vector, serialPort* port);

typedef int baud_rate;

// Common Posix functionality
void searchForComPorts(serialPortVector* comPorts);
int setConfigOptions(int portFD, baud_rate baudRate, struct termios *options);

#endif /* ifndef _H_MCURS232_HELPER_FUNCTIONS_H_ */
