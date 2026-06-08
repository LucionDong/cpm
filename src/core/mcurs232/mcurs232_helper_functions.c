#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
/* #include <sys/types.h> */
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/serial.h>
#include <asm/ioctls.h>
#include "termios2.h"

#include "mcurs232_helper_functions.h"

// Common serial port storage functionality
serialPort* pushBack(serialPortVector* vector, const char* key, const char* friendlyName, const char* description, const char* location, const char* serialNumber, int vid, int pid)
{
	// Allocate memory for the new SerialPort storage structure
	if (vector->capacity == vector->length)
	{
		serialPort** newArray = (serialPort**)realloc(vector->ports, ++vector->capacity * sizeof(serialPort*));
		if (newArray)
			vector->ports = newArray;
		else
		{
			vector->capacity--;
			return NULL;
		}
	}
	serialPort* port = (serialPort*)malloc(sizeof(serialPort));
	if (port)
		vector->ports[vector->length++] = port;
	else
		return NULL;

	// Initialize the serial port mutex and condition variables
	memset(port, 0, sizeof(serialPort));
	pthread_mutex_init(&port->eventMutex, NULL);
	pthread_condattr_t conditionVariableAttributes;
	pthread_condattr_init(&conditionVariableAttributes);
#if !defined(__APPLE__) && !defined(__OpenBSD__) && !defined(__ANDROID__)
	pthread_condattr_setclock(&conditionVariableAttributes, CLOCK_REALTIME);
#endif
	pthread_cond_init(&port->eventReceived, &conditionVariableAttributes);
	pthread_condattr_destroy(&conditionVariableAttributes);

	// Initialize the storage structure
	port->handle = -1;
	port->enumerated = 1;
	port->vendorID = vid;
	port->productID = pid;
	port->portPath = (char*)malloc(strlen(key) + 1);
	port->portLocation = (char*)malloc(strlen(location) + 1);
	port->friendlyName = (char*)malloc(strlen(friendlyName) + 1);
	port->serialNumber = (char*)malloc(strlen(serialNumber) + 1);
	port->portDescription = (char*)malloc(strlen(description) + 1);

	// Store port strings
	strcpy(port->portPath, key);
	strcpy(port->portLocation, location);
	strcpy(port->friendlyName, friendlyName);
	strcpy(port->serialNumber, serialNumber);
	strcpy(port->portDescription, description);

	// Return the newly created serial port structure
	return port;
}

serialPort* fetchPort(serialPortVector* vector, const char* key)
{
	for (int i = 0; i < vector->length; ++i)
		if (strcmp(key, vector->ports[i]->portPath) == 0)
			return vector->ports[i];
	return NULL;
}

void removePort(serialPortVector* vector, serialPort* port)
{
	// Clean up memory associated with the port
	free(port->portPath);
	free(port->portLocation);
	free(port->friendlyName);
	free(port->serialNumber);
	free(port->portDescription);
	pthread_cond_destroy(&port->eventReceived);
	pthread_mutex_destroy(&port->eventMutex);

	// Move up all remaining ports in the serial port listing
	for (int i = 0; i < vector->length; ++i)
		if (vector->ports[i] == port)
		{
			for (int j = i; j < (vector->length - 1); ++j)
				vector->ports[j] = vector->ports[j+1];
			vector->length--;
			break;
		}

	// Free the serial port structure memory
	free(port);
}

// Common string storage functionality
typedef struct stringVector
{
   char **strings;
   int length, capacity;
} stringVector;

static void pushBackString(stringVector* vector, const char* string)
{
	// Allocate memory for the new port path prefix storage structure
	if (vector->capacity == vector->length)
	{
		char** newStringArray = (char**)realloc(vector->strings, ++vector->capacity * sizeof(const char*));
		if (newStringArray)
			vector->strings = newStringArray;
		else
		{
			vector->capacity--;
			return;
		}
	}
	char* newString = (char*)malloc(strlen(string) + 1);
	if (newString)
	{
		strcpy(newString, string);
		vector->strings[vector->length++] = newString;
	}
}

static void freeStringVector(stringVector* vector)
{
	for (int i = 0; i < vector->length; ++i)
		free(vector->strings[i]);
	if (vector->strings)
		free(vector->strings);
	vector->strings = NULL;
	vector->length = vector->capacity = 0;
}

static void retrievePhysicalPortPrefixes(stringVector* prefixes)
{
	// Open the TTY drivers file
	FILE *input = fopen("/proc/tty/drivers", "rb");
	if (input)
	{
		// Read the file line by line
		int maxLineSize = 256;
		char *line = (char*)malloc(maxLineSize);
		while (fgets(line, maxLineSize, input))
		{
			// Parse the prefix, name, and type fields
			int i = 0;
			char *token, *remainder = line, *prefix = NULL, *name = NULL, *type = NULL;
			while ((token = strtok_r(remainder, " \t\r\n", &remainder)))
				if (++i == 1)
					name = token;
				else if (i == 2)
					prefix = token;
				else if (i == 5)
					type = token;

			// Add a new prefix to the vector if the driver type and name are both "serial"
			if (prefix && type && name && (strcmp(type, "serial") == 0) && (strcmp(name, "serial") == 0))
				pushBackString(prefixes, prefix);
		}

		// Close the drivers file and clean up memory
		free(line);
		fclose(input);
	}
}

static char isUsbSerialSubsystem(const char *subsystemLink)
{
	// Attempt to read the path that the subsystem link points to
	struct stat fileInfo;
	char *subsystem = NULL, isUsbSerial = 0;
	if (lstat(subsystemLink, &fileInfo) == 0)
	{
		ssize_t bufferSize = fileInfo.st_size ? (fileInfo.st_size + 1) : PATH_MAX;
		subsystem = (char*)malloc(bufferSize);
		if (subsystem)
		{
			ssize_t subsystemSize = readlink(subsystemLink, subsystem, bufferSize);
			if (subsystemSize >= 0)
			{
				subsystem[subsystemSize] = '\0';
				if (strcmp(1 + strrchr(subsystem, '/'), "usb-serial") == 0)
					isUsbSerial = 1;
			}
			free(subsystem);
		}
	}
	return isUsbSerial;
}

static char isPtyDevice(const char *deviceLink)
{
	// Attempt to read the path that the potential PTY device link points to
	struct stat fileInfo;
	char *symlink = NULL, isPtyDevice = 0;
	if ((lstat(deviceLink, &fileInfo) == 0) && fileInfo.st_size && ((fileInfo.st_mode & S_IFMT) == S_IFLNK))
	{
		ssize_t bufferSize = fileInfo.st_size + 1;
		symlink = (char*)malloc(bufferSize);
		if (symlink)
		{
			ssize_t symlinkSize = readlink(deviceLink, symlink, bufferSize);
			if (symlinkSize >= 0)
			{
				symlink[symlinkSize] = '\0';
				if (strstr(symlink, "dev/pt"))
					isPtyDevice = 1;
			}
			free(symlink);
		}
	}
	return isPtyDevice;
}

static void getPortLocation(const char* portDirectory, char* portLocation, int physicalPortNumber)
{
	// Set location of busnum and devpath files
	char* busnumFile = (char*)malloc(strlen(portDirectory) + 16);
	strcpy(busnumFile, portDirectory);
	strcat(busnumFile, "busnum");
	char* devpathFile = (char*)malloc(strlen(portDirectory) + 16);
	strcpy(devpathFile, portDirectory);
	strcat(devpathFile, "devpath");
	int portLocationLength = 0;
	portLocation[0] = '\0';

	// Read the bus number
	FILE *input = fopen(busnumFile, "rb");
	if (input)
	{
		int ch = getc(input);
		while (((char)ch != '\n') && (ch != EOF))
		{
			portLocation[portLocationLength++] = (char)ch;
			ch = getc(input);
		}
		portLocation[portLocationLength++] = '-';
		fclose(input);
	}
	else
	{
		portLocation[portLocationLength++] = '0';
		portLocation[portLocationLength++] = '-';
	}

	// Read the device path
	input = fopen(devpathFile, "rb");
	if (input)
	{
		int ch = getc(input);
		while (((char)ch != '\n') && (ch != EOF))
		{
			portLocation[portLocationLength++] = (char)ch;
			ch = getc(input);
		}
		portLocation[portLocationLength] = '\0';
		fclose(input);
	}
	else
	{
		portLocation[portLocationLength++] = '0';
		portLocation[portLocationLength] = '\0';
	}

	// Append the physical port number if applicable
	if (physicalPortNumber >= 0)
		sprintf(portLocation + portLocationLength, ".%d", physicalPortNumber);

	// Clean up the dynamic memory
	free(devpathFile);
	free(busnumFile);
}

static void assignFriendlyName(const char* portDevPath, char* friendlyName)
{
	// Manually assign a friendly name if the port type is known from its name
	const char *portName = 1 + strrchr(portDevPath, '/');
	if ((strlen(portName) >= 5) && (portName[3] == 'A') && (portName[4] == 'P'))
		strcpy(friendlyName, "Advantech Extended Serial Port");
	else if ((strlen(portName) >= 6) && (portName[0] == 'r') && (portName[1] == 'f') && (portName[2] == 'c') &&
			(portName[3] == 'o') && (portName[4] == 'm') && (portName[5] == 'm'))
		strcpy(friendlyName, "Bluetooth-Based Serial Port");
	else
	{
		// Assign a friendly name based on the serial port driver in use
		char nameAssigned = 0;
		FILE *input = fopen("/proc/tty/drivers", "rb");
		if (input)
		{
			// Read the TTY drivers file line by line
			int maxLineSize = 256;
			char *line = (char*)malloc(maxLineSize);
			while (!nameAssigned && fgets(line, maxLineSize, input))
			{
				// Parse the prefix, name, and type fields
				int i = 0;
				char *token, *remainder = line, *prefix = NULL, *name = NULL, *type = NULL;
				while ((token = strtok_r(remainder, " \t\r\n", &remainder)))
					if (++i == 1)
						name = token;
					else if (i == 2)
						prefix = token;
					else if (i == 5)
						type = token;

				// Assign a friendly name if a matching port prefix was found
				if (prefix && name && type && (strcmp(type, "serial") == 0) && (strstr(portDevPath, prefix) == portDevPath))
				{
					strcpy(friendlyName, "Serial Device (");
					strcat(friendlyName, name);
					strcat(friendlyName, ")");
					nameAssigned = 1;
				}
			}

			// Close the drivers file and clean up memory
			free(line);
			fclose(input);
		}

		// If no driver prefix could be found, just assign a generic friendly name
		if (!nameAssigned)
			strcpy(friendlyName, "USB-Based Serial Port");
	}
}

static void getUsbDetails(const char* fileName, char* basePathEnd, int* vendorID, int* productID, char* serialNumber)
{
	// Attempt to read the Vendor ID
	char *temp = (char*)malloc(8);
	sprintf(basePathEnd, "../idVendor");
	FILE *input = fopen(fileName, "rb");
	if (input)
	{
		fgets(temp, 8, input);
		*vendorID = (int)strtol(temp, NULL, 16);
		fclose(input);
	}

	// Attempt to read the Product ID
	sprintf(basePathEnd, "../idProduct");
	input = fopen(fileName, "rb");
	if (input)
	{
		fgets(temp, 8, input);
		*productID = (int)strtol(temp, NULL, 16);
		fclose(input);
	}
	free(temp);

	// Attempt to read the Serial Number
	strcpy(serialNumber, "Unknown");
	sprintf(basePathEnd, "../serial");
	input = fopen(fileName, "rb");
	if (input)
	{
		fgets(serialNumber, 128, input);
		serialNumber[strcspn(serialNumber, "\r\n")] = 0;
		fclose(input);
	}
}

void searchForComPorts(serialPortVector* comPorts)
{
	// Set up the necessary local variables
	struct stat pathInfo = { 0 };
	stringVector physicalPortPrefixes = { NULL, 0, 0 };
	int fileNameMaxLength = 0, portDevPathMaxLength = 128, maxLineSize = 256;
	char *fileName = NULL, *portDevPath = (char*)malloc(portDevPathMaxLength);
	char *line = (char*)malloc(maxLineSize), *friendlyName = (char*)malloc(256);
	char *portLocation = (char*)malloc(128), *interfaceDescription = (char*)malloc(256);
	char *serialNumber = (char*)malloc(128);

	// Retrieve the list of /dev/ prefixes for all physical serial ports
	retrievePhysicalPortPrefixes(&physicalPortPrefixes);

	// Iterate through the system TTY directory
	DIR *directoryIterator = opendir("/sys/class/tty/");
	if (directoryIterator)
	{
		struct dirent *directoryEntry = readdir(directoryIterator);
		while (directoryEntry)
		{
			// Ensure that the file name buffer is large enough
			if ((64 + strlen(directoryEntry->d_name)) > fileNameMaxLength)
			{
				fileNameMaxLength = 64 + strlen(directoryEntry->d_name);
				fileName = (char*)realloc(fileName, fileNameMaxLength);
			}

			// Check if the entry represents a valid serial port
			strcpy(fileName, "/sys/class/tty/");
			strcat(fileName, directoryEntry->d_name);
			char *basePathEnd = fileName + strlen(fileName);
			sprintf(basePathEnd, "/device/driver");
			char isSerialPort = (stat(fileName, &pathInfo) == 0) && S_ISDIR(pathInfo.st_mode);
			sprintf(basePathEnd, "/dev");
			isSerialPort = isSerialPort && (stat(fileName, &pathInfo) == 0) && S_ISREG(pathInfo.st_mode);
			sprintf(basePathEnd, "/uevent");
			isSerialPort = isSerialPort && (stat(fileName, &pathInfo) == 0) && S_ISREG(pathInfo.st_mode);
			if (!isSerialPort)
			{
				directoryEntry = readdir(directoryIterator);
				continue;
			}

			// Determine the /dev/ path to the device
			isSerialPort = 0;
			FILE *input = fopen(fileName, "r");
			if (input)
			{
				while (fgets(line, maxLineSize, input))
					if (strstr(line, "DEVNAME=") == line)
					{
						isSerialPort = 1;
						strcpy(portDevPath, "/dev/");
						strcat(portDevPath, line + 8);
						portDevPath[strcspn(portDevPath, "\r\n")] = '\0';
					}
				fclose(input);
			}
			if (!isSerialPort)
			{
				directoryEntry = readdir(directoryIterator);
				continue;
			}

			// Check if the device is a physical serial port
			char isPhysical = 0;
			int physicalPortNumber = 0;
			for (int i = 0; !isPhysical && (i < physicalPortPrefixes.length); ++i)
				if (strstr(portDevPath, physicalPortPrefixes.strings[i]) == portDevPath)
				{
					isPhysical = 1;
					physicalPortNumber = atoi(portDevPath + strlen(physicalPortPrefixes.strings[i]));
				}

			// Determine the subsystem and bus location of the port
			int vendorID = -1, productID = -1;
			sprintf(basePathEnd, "/device/subsystem");
			if (isUsbSerialSubsystem(fileName))
			{
				sprintf(basePathEnd, "/device/../");
				basePathEnd += 11;
			}
			else
			{
				sprintf(basePathEnd, "/device/");
				basePathEnd += 8;
			}
			getUsbDetails(fileName, basePathEnd, &vendorID, &productID, serialNumber);
			sprintf(basePathEnd, "../");
			getPortLocation(fileName, portLocation, isPhysical ? physicalPortNumber : -1);

			// Check if the port has already been enumerated
			serialPort *port = fetchPort(comPorts, portDevPath);
			if (port)
			{
				// See if the device has changed locations
				int oldLength = strlen(port->portLocation);
				int newLength = strlen(portLocation);
				if (oldLength != newLength)
				{
					port->portLocation = (char*)realloc(port->portLocation, newLength + 1);
					strcpy(port->portLocation, portLocation);
				}
				else if (memcmp(port->portLocation, portLocation, newLength))
					strcpy(port->portLocation, portLocation);

				// Update descriptors if this is not a physical port
				if (!isPhysical)
				{
					// Update the device's registered friendly name
					friendlyName[0] = '\0';
					sprintf(basePathEnd, "../product");
					FILE *input = fopen(fileName, "rb");
					if (input)
					{
						fgets(friendlyName, 256, input);
						friendlyName[strcspn(friendlyName, "\r\n")] = '\0';
						fclose(input);
					}
					if (friendlyName[0] == '\0')
						assignFriendlyName(portDevPath, friendlyName);
					oldLength = strlen(port->friendlyName);
					newLength = strlen(friendlyName);
					if (oldLength != newLength)
					{
						port->friendlyName = (char*)realloc(port->friendlyName, newLength + 1);
						strcpy(port->friendlyName, friendlyName);
					}
					else if (memcmp(port->friendlyName, friendlyName, newLength))
						strcpy(port->friendlyName, friendlyName);

					// Attempt to read the bus-reported device description
					interfaceDescription[0] = '\0';
					sprintf(basePathEnd, "interface");
					input = fopen(fileName, "rb");
					if (input)
					{
						fgets(interfaceDescription, 256, input);
						interfaceDescription[strcspn(interfaceDescription, "\r\n")] = '\0';
						fclose(input);
					}
					if (interfaceDescription[0] == '\0')
						strcpy(interfaceDescription, friendlyName);
					oldLength = strlen(port->portDescription);
					newLength = strlen(interfaceDescription);
					if (oldLength != newLength)
					{
						port->portDescription = (char*)realloc(port->portDescription, newLength + 1);
						strcpy(port->portDescription, interfaceDescription);
					}
					else if (memcmp(port->portDescription, interfaceDescription, newLength))
						strcpy(port->portDescription, interfaceDescription);
				}

				// Continue port enumeration
				directoryEntry = readdir(directoryIterator);
				port->enumerated = 1;
				continue;
			}

			// Retrieve all available port details based on its type
			if (isPhysical)
			{
				// Probe the physical port to see if it actually exists
				int fd = open(portDevPath, O_RDWR | O_NONBLOCK | O_NOCTTY);
				if (fd >= 0)
				{
					struct serial_struct serialInfo = { 0 };
					if ((ioctl(fd, TIOCGSERIAL, &serialInfo) == 0) && (serialInfo.type != PORT_UNKNOWN))
					{
						// Add the port to the list of available ports
						strcpy(friendlyName, "Physical Port ");
						strcat(friendlyName, directoryEntry->d_name+3);
						pushBack(comPorts, portDevPath, friendlyName, friendlyName, portLocation, "Unknown", -1, -1);
					}
					close(fd);
				}
			}
			else		// Emulated serial port
			{
				// See if the device has a registered friendly name
				friendlyName[0] = '\0';
				sprintf(basePathEnd, "../product");
				FILE *input = fopen(fileName, "rb");
				if (input)
				{
					fgets(friendlyName, 256, input);
					friendlyName[strcspn(friendlyName, "\r\n")] = '\0';
					fclose(input);
				}
				if (friendlyName[0] == '\0')
					assignFriendlyName(portDevPath, friendlyName);

				// Attempt to read the bus-reported device description
				interfaceDescription[0] = '\0';
				sprintf(basePathEnd, "interface");
				input = fopen(fileName, "rb");
				if (input)
				{
					fgets(interfaceDescription, 256, input);
					interfaceDescription[strcspn(interfaceDescription, "\r\n")] = '\0';
					fclose(input);
				}
				if (interfaceDescription[0] == '\0')
					strcpy(interfaceDescription, friendlyName);

				// Add the port to the list of available ports
				pushBack(comPorts, portDevPath, friendlyName, interfaceDescription, portLocation, serialNumber, vendorID, productID);
			}

			// Read next TTY directory entry
			directoryEntry = readdir(directoryIterator);
		}
		closedir(directoryIterator);
	}

	// Search through the system DEV directory for PTY symlinks
	directoryIterator = opendir("/dev/");
	if (directoryIterator)
	{
		struct dirent *directoryEntry = readdir(directoryIterator);
		while (directoryEntry)
		{
			// Ensure that the file name buffer is large enough
			if ((16 + strlen(directoryEntry->d_name)) > fileNameMaxLength)
			{
				fileNameMaxLength = 16 + strlen(directoryEntry->d_name);
				fileName = (char*)realloc(fileName, fileNameMaxLength);
			}

			// Check if the entry represents a valid serial port
			strcpy(fileName, "/dev/");
			strcat(fileName, directoryEntry->d_name);
			if (isPtyDevice(fileName))
				pushBack(comPorts, fileName, "PTY Device", "Pseudo-Terminal Device", "0-0", "Unknown", -1, -1);

			// Read next TTY directory entry
			directoryEntry = readdir(directoryIterator);
		}
		closedir(directoryIterator);
	}

	// Clean up dynamically allocated memory
	freeStringVector(&physicalPortPrefixes);
	if (fileName)
		free(fileName);
	free(interfaceDescription);
	free(serialNumber);
	free(portLocation);
	free(friendlyName);
	free(portDevPath);
	free(line);
}

static baud_rate getBaudRateCode(baud_rate baudRate)
{
	// Translate a raw baud rate into a system-specified one
	switch (baudRate)
	{
		case 50:
			return B50;
		case 75:
			return B75;
		case 110:
			return B110;
		case 134:
			return B134;
		case 150:
			return B150;
		case 200:
			return B200;
		case 300:
			return B300;
		case 600:
			return B600;
		case 1200:
			return B1200;
		case 1800:
			return B1800;
		case 2400:
			return B2400;
		case 4800:
			return B4800;
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
		case 57600:
#ifdef B57600
			return B57600;
#else
			return 0;
#endif
		case 76800:
#ifdef B76800
			return B76800;
#else
			return 0;
#endif
		case 115200:
#ifdef B115200
			return B115200;
#else
			return 0;
#endif
		case 153600:
#ifdef B153600
			return B153600;
#else
			return 0;
#endif
		case 230400:
#ifdef B230400
			return B230400;
#else
			return 0;
#endif
		case 307200:
#ifdef B307200
			return B307200;
#else
			return 0;
#endif
		case 460800:
#ifdef B460800
			return B460800;
#else
			return 0;
#endif
		case 500000:
#ifdef B500000
			return B500000;
#else
			return 0;
#endif
		case 576000:
#ifdef B576000
			return B576000;
#else
			return 0;
#endif
		case 614400:
#ifdef B614400
			return B614400;
#else
			return 0;
#endif
		case 921600:
#ifdef B921600
			return B921600;
#else
			return 0;
#endif
		case 1000000:
#ifdef B1000000
			return B1000000;
#else
			return 0;
#endif
		case 1152000:
#ifdef B1152000
			return B1152000;
#else
			return 0;
#endif
		case 1500000:
#ifdef B1500000
			return B1500000;
#else
			return 0;
#endif
		case 2000000:
#ifdef B2000000
			return B2000000;
#else
			return 0;
#endif
		case 2500000:
#ifdef B2500000
			return B2500000;
#else
			return 0;
#endif
		case 3000000:
#ifdef B3000000
			return B3000000;
#else
			return 0;
#endif
		case 3500000:
#ifdef B3500000
			return B3500000;
#else
			return 0;
#endif
		case 4000000:
#ifdef B4000000
			return B4000000;
#else
			return 0;
#endif
		default:
			return 0;
	}

	return 0;
}


int setConfigOptions(int portFD, baud_rate baudRate, struct termios *options)
{
	// Set options using different methodologies based on availability of baud rate settings
	int retVal = -1;
	baud_rate baudRateCode = getBaudRateCode(baudRate);
	if (baudRateCode)
	{
		// Directly set baud rate and apply all configuration options
		cfsetispeed(options, baudRateCode);
		cfsetospeed(options, baudRateCode);
		retVal = tcsetattr(portFD, TCSANOW, options);
	}
	else
	{
		// Copy termios info into newer termios2 data structure
		struct termios2 options2 = { 0 };
		options2.c_cflag = (options->c_cflag & ~(CBAUD | CBAUDEX)) | BOTHER;
		options2.c_cflag &= ~((CBAUD | CBAUDEX) << IBSHIFT);
		options2.c_cflag |= (BOTHER << IBSHIFT);
		options2.c_iflag = options->c_iflag & ~IBAUD0;
		options2.c_oflag = options->c_oflag;
		options2.c_lflag = options->c_lflag;
		options2.c_line = options->c_line;
		options2.c_ispeed = baudRate;
		options2.c_ospeed = baudRate;
		memcpy(options2.c_cc, options->c_cc, K_NCCS * sizeof(cc_t));
		retVal = ioctl(portFD, T2CSANOW, &options2);
	}
	return retVal;
}
