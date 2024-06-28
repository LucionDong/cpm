/* #include <esvcpm/utils/log.h> */
/* #include <esvcpm/plugin.h> */
/* #include <stdint.h> */
/* #include <stdlib.h> */
#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include "esvcpm/utils/log.h"
/* #include <sys/time.h> */
/* #include <time.h> */
#include <linux/serial.h>
#include <unistd.h>

#include "../outside_service_manager.h"
#include "mcurs232_helper_functions.h"
#include "serial_port.h"

char portsEnumerated = 0;
static pthread_mutex_t criticalSection = PTHREAD_MUTEX_INITIALIZER;

serialPortVector serialPorts = {NULL, 0, 0};

int lastErrorLineNumber = 0, lastErrorNumber = 0;

bool clearDTR(serialPort *port);
bool clearRTS(serialPort *port);

bool disableConfig = false;
bool disableExclusiveLock = false;
bool autoFlushIOBuffers = false;
int baudRate = 115200, byteSizeInt = 8, stopBitsInt = SerialPort_ONE_STOP_BIT, parityInt = SerialPort_NO_PARITY,
    eventsToMonitor = 0;
int timeoutMode = SerialPort_TIMEOUT_NONBLOCKING, readTimeout = 0, writeTimeout = 0,
    flowControl = SerialPort_FLOW_CONTROL_DISABLED;
int sendDeviceQueueSize = 4096, receiveDeviceQueueSize = 4096;
int safetySleepTimeMS = 200, rs485DelayBefore = 0, rs485DelayAfter = 0;
unsigned char xonStartChar = 17, xoffStopChar = 19;
bool rs485ModeEnabled = false, rs485ActiveHigh = true, rs485RxDuringTx = false, rs485EnableTermination = false;
bool isRtsEnabled = false, isDtrEnabled = false;

static void enumeratePorts() {
    // Reset the enumerated flag on all non-open serial ports
    for (int i = 0; i < serialPorts.length; ++i) serialPorts.ports[i]->enumerated = (serialPorts.ports[i]->handle > 0);

    // Enumerate serial ports on this machine
    searchForComPorts(&serialPorts);

    // Remove all non-enumerated ports from the serial port listing
    for (int i = 0; i < serialPorts.length; ++i)
        if (!serialPorts.ports[i]->enumerated) {
            removePort(&serialPorts, serialPorts.ports[i]);
            i--;
        }
    portsEnumerated = 1;

    /* for (int i = 0; i < serialPorts.length; ++i) { */
    /* 	plog_info(plugin, "serial device%d: (%s)--->(%s)", i, serialPorts.ports[i]->friendlyName,
     * serialPorts.ports[i]->portPath); */
    /* } */
    /* plog_info(plugin, "enumerate posrts done"); */
}

int config_port(serialPort *serial_port) {

    // Quickly set the desired RTS/DTR line status immediately upon opening
    clearDTR(serial_port);
    clearRTS(serial_port);

    // Configure port parameters if not explicitly disabled
    struct termios options = {0};
    tcgetattr(serial_port->handle, &options);
    if (!disableConfig) {
        // Clear any serial port flags and set up raw non-canonical port parameters
        options.c_cc[VSTART] = (unsigned char) xonStartChar;
        options.c_cc[VSTOP] = (unsigned char) xoffStopChar;
        options.c_iflag &=
            ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | INPCK | IGNPAR | IGNCR | ICRNL | IXON | IXOFF | IXANY);
        options.c_oflag &= ~OPOST;
        options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        options.c_cflag &= ~(CSIZE | PARENB | CMSPAR | PARODD | CSTOPB | CRTSCTS);

        // Update the user-specified port parameters
        tcflag_t byteSize = (byteSizeInt == 5) ? CS5 : (byteSizeInt == 6) ? CS6 : (byteSizeInt == 7) ? CS7 : CS8;
        tcflag_t parity = (parityInt == SerialPort_NO_PARITY) ? 0
            : (parityInt == SerialPort_ODD_PARITY)            ? (PARENB | PARODD)
            : (parityInt == SerialPort_EVEN_PARITY)           ? PARENB
            : (parityInt == SerialPort_MARK_PARITY)           ? (PARENB | CMSPAR | PARODD)
                                                              : (PARENB | CMSPAR);
        options.c_cflag |= (byteSize | parity | CLOCAL | CREAD);
        if (!isDtrEnabled || !isRtsEnabled)
            options.c_cflag &= ~HUPCL;
        if (!rs485ModeEnabled)
            options.c_iflag |= BRKINT;
        if (stopBitsInt == SerialPort_TWO_STOP_BITS)
            options.c_cflag |= CSTOPB;
        if (byteSizeInt < 8)
            options.c_iflag |= ISTRIP;
        if (parityInt != 0)
            options.c_iflag |= (INPCK | IGNPAR);
        if (((flowControl & SerialPort_FLOW_CONTROL_CTS_ENABLED) > 0) ||
            ((flowControl & SerialPort_FLOW_CONTROL_RTS_ENABLED) > 0))
            options.c_cflag |= CRTSCTS;
        if ((flowControl & SerialPort_FLOW_CONTROL_XONXOFF_IN_ENABLED) > 0)
            options.c_iflag |= IXOFF;
        if ((flowControl & SerialPort_FLOW_CONTROL_XONXOFF_OUT_ENABLED) > 0)
            options.c_iflag |= IXON;

        // Attempt to set the transmit buffer size, closing wait time, and latency flags
        struct serial_struct serInfo = {0};
        if (!ioctl(serial_port->handle, TIOCGSERIAL, &serInfo)) {
            serInfo.closing_wait = 250;
            serInfo.xmit_fifo_size = sendDeviceQueueSize;
            serInfo.flags |= ASYNC_LOW_LATENCY;
            ioctl(serial_port->handle, TIOCSSERIAL, &serInfo);
        }

        // Retrieve the driver-reported transmit buffer size
        if (!ioctl(serial_port->handle, TIOCGSERIAL, &serInfo))
            sendDeviceQueueSize = serInfo.xmit_fifo_size;
        receiveDeviceQueueSize = sendDeviceQueueSize;
        nlog_info("sendDeviceQueueSize: %d", sendDeviceQueueSize);
        /* (*env)->SetIntField(env, obj, sendDeviceQueueSizeField, sendDeviceQueueSize); */
        /* if (checkJniError(env, __LINE__ - 1)) return JNI_FALSE; */
        /* (*env)->SetIntField(env, obj, receiveDeviceQueueSizeField, receiveDeviceQueueSize); */
        /* if (checkJniError(env, __LINE__ - 1)) return JNI_FALSE; */

        // Attempt to set the requested RS-485 mode
        struct serial_rs485 rs485Conf = {0};
        if (!ioctl(serial_port->handle, TIOCGRS485, &rs485Conf)) {
            if (rs485ModeEnabled)
                rs485Conf.flags |= SER_RS485_ENABLED;
            else
                rs485Conf.flags &= ~SER_RS485_ENABLED;
            if (rs485ActiveHigh) {
                rs485Conf.flags |= SER_RS485_RTS_ON_SEND;
                rs485Conf.flags &= ~(SER_RS485_RTS_AFTER_SEND);
            } else {
                rs485Conf.flags &= ~(SER_RS485_RTS_ON_SEND);
                rs485Conf.flags |= SER_RS485_RTS_AFTER_SEND;
            }
            if (rs485RxDuringTx)
                rs485Conf.flags |= SER_RS485_RX_DURING_TX;
            else
                rs485Conf.flags &= ~(SER_RS485_RX_DURING_TX);
            /* if (rs485EnableTermination) */
            /* 	rs485Conf.flags |= SER_RS485_TERMINATE_BUS; */
            /* else */
            /* 	rs485Conf.flags &= ~(SER_RS485_TERMINATE_BUS); */
            rs485Conf.delay_rts_before_send = rs485DelayBefore / 1000;
            rs485Conf.delay_rts_after_send = rs485DelayAfter / 1000;
            ioctl(serial_port->handle, TIOCSRS485, &rs485Conf);
        }
    }

    // Configure the serial port read and write timeouts
    int flags = 0;
    serial_port->eventsMask = eventsToMonitor;
    if ((eventsToMonitor & SerialPort_LISTENING_EVENT_DATA_RECEIVED) > 0) {
        // Force specific read timeouts if we are monitoring data received
        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = 10;
    } else if (((timeoutMode & SerialPort_TIMEOUT_READ_SEMI_BLOCKING) > 0) &&
               (readTimeout > 0))  // Read Semi-blocking with timeout
    {
        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = readTimeout / 100;
    } else if ((timeoutMode & SerialPort_TIMEOUT_READ_SEMI_BLOCKING) > 0)  // Read Semi-blocking without timeout
    {
        options.c_cc[VMIN] = 1;
        options.c_cc[VTIME] = 0;
    } else if (((timeoutMode & SerialPort_TIMEOUT_READ_BLOCKING) > 0) &&
               (readTimeout > 0))  // Read Blocking with timeout
    {
        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = readTimeout / 100;
    } else if ((timeoutMode & SerialPort_TIMEOUT_READ_BLOCKING) > 0)  // Read Blocking without timeout
    {
        options.c_cc[VMIN] = 1;
        options.c_cc[VTIME] = 0;
    } else if ((timeoutMode & SerialPort_TIMEOUT_SCANNER) > 0)  // Scanner Mode
    {
        options.c_cc[VMIN] = 1;
        options.c_cc[VTIME] = 1;
    } else  // Non-blocking
    {
        flags = O_NONBLOCK;
        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = 0;
    }

    // Apply changes
    if (fcntl(serial_port->handle, F_SETFL, flags)) {
        serial_port->errorLineNumber = lastErrorLineNumber = __LINE__ - 2;
        serial_port->errorNumber = lastErrorNumber = errno;
        return -1;
    }
    if (setConfigOptions(serial_port->handle, baudRate, &options)) {
        serial_port->errorLineNumber = lastErrorLineNumber = __LINE__ - 2;
        serial_port->errorNumber = lastErrorNumber = errno;
        return -1;
    }

    return 0;
}

serialPort *open_port(char *port_path) {

    pthread_mutex_lock(&criticalSection);
    if (!portsEnumerated) {
        enumeratePorts();
    }
    serialPort *port = fetchPort(&serialPorts, port_path);
    pthread_mutex_unlock(&criticalSection);

    if (!port || port->handle > 0) {
        return NULL;
    }

    int portHandle = open(port_path, O_RDWR | O_NOCTTY | O_NONBLOCK | O_CLOEXEC | O_SYNC);
    if (portHandle > 0) {
        // Set the newly opened port handle in the serial port structure
        pthread_mutex_lock(&criticalSection);
        port->handle = portHandle;
        pthread_mutex_unlock(&criticalSection);

        // Ensure that multiple root users cannot access the device simultaneously
        if (!disableExclusiveLock && flock(port->handle, LOCK_EX | LOCK_NB)) {
            port->errorLineNumber = lastErrorLineNumber = __LINE__ - 2;
            port->errorNumber = lastErrorNumber = errno;
            while (close(port->handle) && (errno == EINTR)) errno = 0;
            pthread_mutex_lock(&criticalSection);
            port->handle = -1;
            pthread_mutex_unlock(&criticalSection);
        } else if (config_port(port)) {
            // Close the port if there was a problem setting the parameters
            fcntl(port->handle, F_SETFL, O_NONBLOCK);
            while (close(port->handle) && (errno == EINTR)) errno = 0;
            pthread_mutex_lock(&criticalSection);
            port->handle = -1;
            pthread_mutex_unlock(&criticalSection);
        }
        /* else if (autoFlushIOBuffers) */
        /* { */
        /* 	// Sleep to workaround kernel bug about flushing immediately after opening */
        /* 	const struct timespec sleep_time = { 0, 10000000 }; */
        /* 	nanosleep(&sleep_time, NULL); */
        /* 	Java_SerialPort_flushRxTxBuffers(env, obj, (jlong)(intptr_t)port); */
        /* } */
    } else {
        port->errorNumber = lastErrorNumber = errno;
    }

    return (port->handle > 0) ? port : NULL;
}

bool clearRTS(serialPort *port) {
    const int modemBits = TIOCM_RTS;
    port->errorLineNumber = __LINE__ + 1;
    if (ioctl(port->handle, TIOCMBIC, &modemBits)) {
        port->errorNumber = errno;
        return false;
    }
    return true;
}

bool clearDTR(serialPort *port) {
    const int modemBits = TIOCM_DTR;
    port->errorLineNumber = __LINE__ + 1;
    if (ioctl(port->handle, TIOCMBIC, &modemBits)) {
        port->errorNumber = errno;
        return false;
    }
    return true;
}

int close_port(serialPort *port) {
    // Force the port to enter non-blocking mode to ensure that any current reads return
    struct termios options = {0};
    tcgetattr(port->handle, &options);
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0;
    fcntl(port->handle, F_SETFL, O_NONBLOCK);
    tcsetattr(port->handle, TCSANOW, &options);

    // Unblock, unlock, and close the port
    fdatasync(port->handle);
    tcflush(port->handle, TCIOFLUSH);
    flock(port->handle, LOCK_UN | LOCK_NB);
    while (close(port->handle) && (errno == EINTR)) errno = 0;
    pthread_mutex_lock(&criticalSection);
    port->handle = -1;
    pthread_mutex_unlock(&criticalSection);
    return 0;
}

int bytesAvailable(serialPort *serialPortPointer) {
    // Retrieve bytes available to read
    serialPort *port = (serialPort *) serialPortPointer;
    int numBytesAvailable = -1;
    port->errorLineNumber = __LINE__ + 1;
    ioctl(port->handle, FIONREAD, &numBytesAvailable);
    port->errorNumber = errno;
    return numBytesAvailable;
}

int readBytes(serialPort *serialPortPointer, unsigned char **buf, int *bufSize) {
    int bytesToRead = bytesAvailable(serialPortPointer);
    if (bytesToRead == 0) {
        *buf = NULL;
        *bufSize = 0;
        return 0;
    }

    *buf = calloc(sizeof(char), bytesToRead);
    *bufSize = bytesToRead;
    int bytesRead = read(serialPortPointer->handle, *buf, bytesToRead);
    return bytesRead;
}

/* void* eventReadingThread2(void *serialPortPointer) */
/* { */
/* 	// Make this thread immediately and asynchronously cancellable */
/* 	int oldValue; */
/* 	serialPort *port = (serialPort*)serialPortPointer; */
/* 	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldValue); */
/* 	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldValue); */
/* 	struct serial_icounter_struct oldSerialLineInterrupts, newSerialLineInterrupts; */
/* 	ioctl(port->handle, TIOCGICOUNT, &oldSerialLineInterrupts); */

/* 	// Loop forever while open */
/* 	while (port->eventListenerRunning && port->eventListenerUsesThreads) */
/* 	{ */
/* 		// Initialize the polling variables */
/* 		int pollResult; */
/* 		short pollEventsMask = ((port->eventsMask & SerialPort_LISTENING_EVENT_DATA_AVAILABLE) || (port->eventsMask &
 * SerialPort_LISTENING_EVENT_DATA_RECEIVED)) ? (POLLIN | POLLERR) : POLLERR; */
/* 		struct pollfd waitingSet = { port->handle, pollEventsMask, 0 }; */

/* 		// Wait for a serial port event */
/* 		do */
/* 		{ */
/* 			waitingSet.revents = 0; */
/* 			pollResult = poll(&waitingSet, 1, 1000); */
/* 		} */
/* 		while ((pollResult == 0) && port->eventListenerRunning && port->eventListenerUsesThreads); */

/* 		// Return the detected port events */
/* 		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldValue); */
/* 		pthread_mutex_lock(&port->eventMutex); */
/* 		if (waitingSet.revents & POLLHUP) */
/* 			port->event |= SerialPort_LISTENING_EVENT_PORT_DISCONNECTED; */
/* 		else if (waitingSet.revents & POLLIN) */
/* 			port->event |= SerialPort_LISTENING_EVENT_DATA_AVAILABLE; */
/* 		if (waitingSet.revents & POLLERR) */
/* 			if (!ioctl(port->handle, TIOCGICOUNT, &newSerialLineInterrupts)) */
/* 			{ */
/* 				if (oldSerialLineInterrupts.frame != newSerialLineInterrupts.frame) */
/* 					port->event |= SerialPort_LISTENING_EVENT_FRAMING_ERROR; */
/* 				if (oldSerialLineInterrupts.brk != newSerialLineInterrupts.brk) */
/* 					port->event |= SerialPort_LISTENING_EVENT_BREAK_INTERRUPT; */
/* 				if (oldSerialLineInterrupts.overrun != newSerialLineInterrupts.overrun) */
/* 					port->event |= SerialPort_LISTENING_EVENT_FIRMWARE_OVERRUN_ERROR; */
/* 				if (oldSerialLineInterrupts.parity != newSerialLineInterrupts.parity) */
/* 					port->event |= SerialPort_LISTENING_EVENT_PARITY_ERROR; */
/* 				if (oldSerialLineInterrupts.buf_overrun != newSerialLineInterrupts.buf_overrun) */
/* 					port->event |= SerialPort_LISTENING_EVENT_SOFTWARE_OVERRUN_ERROR; */
/* 				memcpy(&oldSerialLineInterrupts, &newSerialLineInterrupts, sizeof(newSerialLineInterrupts)); */
/* 			} */
/* 		if (port->event) */
/* 			pthread_cond_signal(&port->eventReceived); */
/* 		pthread_mutex_unlock(&port->eventMutex); */
/* 		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldValue); */
/* 	} */
/* 	return NULL; */
/* } */

/* void setEventListeningStatus(serialPort *serialPortPointer, bool eventListenerRunning) */
/* { */
/* 	// Create or cancel a separate event listening thread if required */
/* 	serialPort *port = serialPortPointer; */
/* 	port->eventListenerRunning = eventListenerRunning; */
/* 	if (eventListenerRunning && ((port->eventsMask & SerialPort_LISTENING_EVENT_CARRIER_DETECT) || (port->eventsMask &
 * SerialPort_LISTENING_EVENT_CTS) || */
/* 			(port->eventsMask & SerialPort_LISTENING_EVENT_DSR) || (port->eventsMask &
 * SerialPort_LISTENING_EVENT_RING_INDICATOR))) */
/* 	{ */
/* 		port->event = 0; */
/* 		/1* if (!port->eventsThread1) *1/ */
/* 		/1* { *1/ */
/* 		/1* 	if (!pthread_create(&port->eventsThread1, NULL, eventReadingThread1, port)) *1/ */
/* 		/1* 		pthread_detach(port->eventsThread1); *1/ */
/* 		/1* 	else *1/ */
/* 		/1* 		port->eventsThread1 = 0; *1/ */
/* 		/1* } *1/ */
/* 		if (!port->eventsThread2) */
/* 		{ */
/* 			if (!pthread_create(&port->eventsThread2, NULL, eventReadingThread2, port)) */
/* 				pthread_detach(port->eventsThread2); */
/* 			else */
/* 				port->eventsThread2 = 0; */
/* 		} */
/* 		port->eventListenerUsesThreads = 1; */
/* 	} */
/* 	else if (port->eventListenerUsesThreads) */
/* 	{ */
/* 		port->eventListenerUsesThreads = 0; */
/* 		pthread_cancel(port->eventsThread1); */
/* 		port->eventsThread1 = 0; */
/* 		pthread_cancel(port->eventsThread2); */
/* 		port->eventsThread2 = 0; */
/* 	} */
/* } */
