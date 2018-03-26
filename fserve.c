#include "fserve.h"
#include "fserveTypes.h"
#include "utils.h"
#include "reactor.h"
#include "worker.h"
#include "nodeProtocol.h"

#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <string.h>	
#include <sys/epoll.h>
#include <netdb.h>


// Define globals

GHashTable* pendingKeyMap;
pthread_mutex_t eventTaskQueueMutex = PTHREAD_MUTEX_INITIALIZER;
GQueue* eventTaskQueue;

int NUMBER_OF_WORKERS = 2;

int epollFD;
int listenerSocketFD;
int * workerEpollFDArray;

pthread_mutex_t connectionNumberMutex = PTHREAD_MUTEX_INITIALIZER;
unsigned long connectionNumber = 0;

unsigned long workerTaskNumber = 0;
unsigned long doneWorkerTaskNumber = 0;

int main(int argc, char ** argv)
{	
	
	setbuf(stdout, NULL);
	
	int r;
	
	// Initialise queues
	
	eventTaskQueue = g_queue_new();
	
	// Initialise maps
	
	pendingKeyMap = g_hash_table_new(g_int64_hash, g_int64_equal);
	
	// Worker EPOLL
	
	workerEpollFDArray = malloc(sizeof(int) * NUMBER_OF_WORKERS);
	
	for(r = 0; r < NUMBER_OF_WORKERS; ++r)
	{
		*(workerEpollFDArray+r) = epoll_create1(0);
		
		debug_print("Worker epoll for worker %d created: FD %d", r, *(workerEpollFDArray+r));
	}
	
	// Set up EPOLL FD

	epollFD = epoll_create1(0);

	if (epollFD == -1) { exitWithError("epoll_create1"); }
	
	// Set up listening socket

	
	
	char * port = "8888";
	
	if(argc == 2)
	{
		port = argv[1];
	}
	
	listenerSocketFD = createSocketAndBind(port);
	
	if(listenerSocketFD == -1)
	{
		exitWithError("Unable to bind to port");
	}
	
	r = makeSocketNonBlocking(listenerSocketFD);
	
	if(r == -1)
	{
		exitWithError("Unable to make listening socket non-blocking");
	}
	
	r = listen(listenerSocketFD, 10000);
	
	if(r == -1)
	{
		exitWithError("Unable to listen on socket");
	}
	
	FSConnection * conn = malloc(sizeof(FSConnection));
	conn->id = 0;
	conn->fd = listenerSocketFD;
	conn->stateCode = FS_CONN_RECEIVING_COMMAND;
	conn->type = FS_CONN_TYPE_LISTENER;
	conn->offset = 0;
	conn->length = 0;
	
	struct epoll_event eEventListener;
	eEventListener.data.ptr = conn;
	eEventListener.events = EPOLLIN | EPOLLET;
	
	if(epoll_ctl(epollFD, EPOLL_CTL_ADD, listenerSocketFD, &eEventListener) == -1)
	{
		exitWithError("Unable to use epoll on listening socket FD");
	}
	
	debug_print("Listening socket configured");
	
	// Set up main thread
//	
//	pthread_t eventLoopThread;
//	
//	pthread_create(&eventLoopThread, NULL, &reactorLoop, NULL);
//	pthread_join(eventLoopThread, NULL);
//	sleep(300); exit(0);
	
	reactorLoop(NULL);
	
}
