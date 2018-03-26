/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   reactor.c
 * Author: osrec
 * 
 * Created on 19 March 2018, 23:01
 */

#include "reactor.h"
#include "fserve.h"
#include "utils.h"
#include "nodeProtocol.h"

#define _GNU_SOURCE

#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

int eventFD;

unsigned long incrementConnectionNumber()
{
	//pthread_mutex_lock( &connectionNumberMutex );
	int n = ++connectionNumber;
	//pthread_mutex_unlock( &connectionNumberMutex );
	return n;
}

void * reactorLoop(void * args)
{
	// Initialise workers

	int i, r;
//	pthread_t workerThreads[NUMBER_OF_WORKERS];
//
//	debug_print("Starting event loop thread");
//
//	for (i = 0; i < NUMBER_OF_WORKERS; ++i)
//	{
//		int *workerNumber = malloc(sizeof(i));
//		*workerNumber = i;
//		pthread_create(&workerThreads[i], NULL, &workerLoop, workerNumber);
//	}

	eventFD = eventfd(0, EFD_NONBLOCK);
	
	FSConnection * conn = malloc(sizeof(FSConnection));
	conn->type = FS_CONN_TYPE_DUMMY;
	
	struct epoll_event ee;
	ee.data.ptr = conn;
	ee.events = EPOLLIN | EPOLLET;
	
	epoll_ctl(epollFD, EPOLL_CTL_ADD, eventFD, &ee);
	
	debug_print("Getting core event loop started");
	
	// Initialise Event Loop
	int readyFileDescriptorCount;
	int MAX_EVENTS = 64;

	struct epoll_event epollEventsArray[MAX_EVENTS];

	for(;;)
	{
		debug_print("### Awaiting events ...");
		
		do
		{
			readyFileDescriptorCount = epoll_wait(epollFD, epollEventsArray, MAX_EVENTS, 5000);
		}
		while (readyFileDescriptorCount < 0 && errno == EINTR);
		
		
//		debug_print("## readyFileDescriptorCount: %d", readyFileDescriptorCount);

		if (readyFileDescriptorCount == -1)
		{
			exitWithError("reactor epoll_wait");
		}

		debug_print("### Got %d events ...", readyFileDescriptorCount);
		
		for (int n = 0; n < readyFileDescriptorCount; ++n)
		{
			
			FSConnection * conn = (FSConnection *)epollEventsArray[n].data.ptr;
			
			if(epollEventsArray[n].events & EPOLLIN) { debug_print("EPOLLIN %lu %d", conn->id, conn->fd); }
			if(epollEventsArray[n].events & EPOLLPRI) { debug_print("EPOLLPRI %lu %d", conn->id, conn->fd); }
			if(epollEventsArray[n].events & EPOLLOUT) { debug_print("EPOLLOUT %lu %d", conn->id, conn->fd); }
			if(epollEventsArray[n].events & EPOLLRDNORM) { debug_print("EPOLLRDNORM %lu %d", conn->id, conn->fd); }
			if(epollEventsArray[n].events & EPOLLRDBAND) { debug_print("EPOLLRDBAND %lu %d", conn->id, conn->fd); }
			if(epollEventsArray[n].events & EPOLLWRNORM) { debug_print("EPOLLWRNORM %lu %d", conn->id, conn->fd); }
			if(epollEventsArray[n].events & EPOLLWRBAND) { debug_print("EPOLLWRBAND %lu %d", conn->id, conn->fd); }
			if(epollEventsArray[n].events & EPOLLMSG) { debug_print("EPOLLMSG %lu %d", conn->id, conn->fd); }
			if(epollEventsArray[n].events & EPOLLERR) { debug_print("##################### EPOLLERR %lu %d", conn->id, conn->fd); }
			if(epollEventsArray[n].events & EPOLLHUP) { debug_print("EPOLLHUP %lu %d", conn->id, conn->fd); }
			if(epollEventsArray[n].events & EPOLLRDHUP) { debug_print("EPOLLRDHUP %lu %d", conn->id, conn->fd); }
			
			if(conn->type & FS_CONN_TYPE_LISTENER)
			{
				acceptConnections(listenerSocketFD);
			}
			if(conn->type & FS_CONN_TYPE_DUMMY)
			{
				uint64_t u;
				read(eventFD, &u, sizeof(u));
				acceptConnections(listenerSocketFD);
			}
			else if(conn->type & FS_CONN_TYPE_NODE)
			{
				
				if(epollEventsArray[n].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR) )
				{
//					debug_print("Ready for close");
					handleNodeConnectionError(conn);

				}
				else
				{		
				
					if(epollEventsArray[n].events & EPOLLIN)
					{
	//					debug_print("Ready for read");
						handleNodeConnectionRead(conn);
					}

					if(epollEventsArray[n].events & EPOLLOUT)
					{
	//					debug_print("Ready for write");
						handleNodeConnectionWrite(conn);
					}
				}
				
			}
		
		}
	}
}


void acceptConnections(int listenerSocketFD)
{
	
	int r = 0;
	int maxAcceptances = 100;
	uint64_t u = 1;
	
	while(maxAcceptances--)
	{
		struct sockaddr incomingAddress;
		socklen_t incomingAddressLength;
		int incomingFD;

		incomingAddressLength = sizeof incomingAddress;
		incomingFD = accept(listenerSocketFD, &incomingAddress, &incomingAddressLength);

		if (incomingFD == -1)
		{
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
			{
				// We have processed all incoming connections
				//debug_print("No more conns");
				break;
			}
			else
			{
				perror ("Error accepting connections");
				break;
			}
		}

		r = makeSocketNonBlocking(incomingFD);

		if (r == -1) { break; }
		
		FSConnection * conn = malloc(sizeof(FSConnection));
		conn->id = incrementConnectionNumber();
		conn->fd = incomingFD;
		conn->stateCode = FS_CONN_RECEIVING_COMMAND;
		conn->type = FS_CONN_TYPE_NODE;
		conn->offset = 0;
		conn->length = 0;
		conn->buffer = malloc(MAX_BUFFER_SIZE);
		conn->workerNumber = conn->id % NUMBER_OF_WORKERS;

		struct epoll_event eEvent;
		eEvent.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLHUP | EPOLLERR | EPOLLET;
		eEvent.data.ptr = conn;

		debug_print("Accepted connection %lu %d with buffer %p", conn->id, conn->fd, conn->buffer);

//				printf("Accepted connection %lu\n", conn->id);

		r = epoll_ctl(epollFD, EPOLL_CTL_ADD, incomingFD, &eEvent);

		if(r == -1) 
		{
			debug_print("Worker epoll error");
			continue; 
		}
	}
	
	if(maxAcceptances == 0)
	{
		write(eventFD, &u, sizeof(uint64_t));
	}
	
}