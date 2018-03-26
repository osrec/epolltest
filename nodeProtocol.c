/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   nodeProtocol.c
 * Author: osrec
 * 
 * Created on 19 March 2018, 20:44
 */

#include "nodeProtocol.h"
#include "fserve.h"
#include "worker.h"
#include "utils.h"

#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>

void handleNodeConnectionRead(FSConnection * conn)
{	
	if(!(conn->stateCode & (FS_CONN_RECEIVING_COMMAND | FS_CONN_READY)))
	{
		debug_print("Not in a valid state to read data (just reading to get rid of data)");
		readAll(conn->fd);
		return;
	}

	if(conn->stateCode == FS_CONN_READY)
	{
		conn->stateCode = FS_CONN_RECEIVING_COMMAND;
	}

	ssize_t count;

	while(1)
	{	

		if(conn->offset >= MAX_BUFFER_SIZE)
		{
			readAll(conn->fd);
			break;
		}

		char * buffer = conn->buffer + conn->offset;
		int readLength = MAX_BUFFER_SIZE - conn->offset;
		count = read(conn->fd, buffer, readLength);
		
		if(count == -1)
		{
			if((errno == EAGAIN) || (errno == EWOULDBLOCK))
			{
				debug_print("Yay! read everything");
			}
			else
			{
				perror("Read Error");
			}

			break;
		}
		
		conn->offset += (long)count;
	}

	// Check if we can interpret length

	if(conn->length == 0 && conn->offset >= 4)
	{
		conn->length = ntohl(*(uint32_t *)conn->buffer);

		// Message must be a minimum of 8 bytes (1 byte for size and 1 byte for instruction)
		// and a maximum of 1024 bytes in length 

		if(conn->length < 8 || conn->length > 1024)
		{
			debug_print("Error: Invalid length %ld. Closing connection.", conn->length);
			handleNodeConnectionError(conn);
		}

		debug_print("Expecting message of length %ld", conn->length);
	}

	debug_print("Read up to %ld bytes", conn->offset);

	if(conn->length > 0 && conn->offset >= conn->length)
	{
		debug_print("Received full command");
//		printf("Received full command #%lu\n", conn->id);

		conn->stateCode = FS_CONN_DOING_TASK;
		
		int taskType = ntohl(*((uint32_t *)conn->buffer + 4));

		char command[1025] = "";

		memset(command, 0, 1025);
		memcpy(command, conn->buffer + 8, 1016);
		memset(command + (conn->length - 8)*sizeof(char), '\0', 1);

		debug_print("Inst: %s", command);
	}
}

void handleNodeConnectionWrite(FSConnection * conn)
{
	if
	(
		conn->length > 0 && 
		conn->offset >= conn->length && 
		conn->stateCode & FS_CONN_DOING_TASK
	)
	{
		conn->length = 0;
		conn->offset = 0;
		conn->stateCode = FS_CONN_AWAITING_RESPONSE;
		debug_print("The client is ready for a response...");
		
		writeAll(conn->fd, "yahoo", 5);
		
	}
}

int closeCount = 0;

void handleNodeConnectionError(FSConnection * conn)
{
	int r = epoll_ctl(epollFD, EPOLL_CTL_DEL, conn->fd, NULL);
	debug_print("Error or hang up, so closing FD %d #%d", conn->fd, ++closeCount);
	close(conn->fd);
	free_FSConnection(conn);
}