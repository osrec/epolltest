#ifndef __FSERVE__
#define __FSERVE__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/aio_abi.h>
#include <glib.h>

#define MAX_KEY_LENGTH 20
#define MAX_BUFFER_SIZE 1024

extern GHashTable* pendingKeyMap;
extern pthread_mutex_t eventTaskQueueMutex;
extern GQueue* eventTaskQueue;

extern int NUMBER_OF_WORKERS;

extern int epollFD;
extern int listenerSocketFD;
extern int * workerEpollFDArray;

extern pthread_mutex_t connectionNumberMutex;
extern unsigned long connectionNumber;

extern unsigned long workerTaskNumber;
extern unsigned long doneWorkerTaskNumber;

// Read from disk

int readFromDisk(char * fileName, size_t numberOfBytes, off_t offset);

// Write to disk

int writeToDisk(char *fileName, size_t numberOfBytes, off_t offset, void *data);

// Accept socket connection on the listening FD

//int acceptConnection();

// Read from socket

int readFromSocket(int fd);

// Write to socket

int writeToSocket(int fd);

// Close socket

int closeSocket(int fd);

// Connect to another host

int connectToHost(char * host, int port);

// Stream a file to an FD

int streamFile(int fd, char * fileName);


#endif //__FSERVE__
