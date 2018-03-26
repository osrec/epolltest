#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#include "utils.h"

void exitWithError(char *errMsg)
{
	perror(errMsg);
	exit(-1);
}

int createSocketAndBind (char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset (&hints, 0, sizeof (struct addrinfo));
	
	hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
	hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
	hints.ai_flags = AI_PASSIVE;     /* All interfaces */

	s = getaddrinfo (NULL, port, &hints, &result);
	
	if (s != 0)
	{
		fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		
		if (sfd == -1) { continue; }

		s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
		
		if (s == 0)
		{
			break; // Managed to bind successfully
		}

		close (sfd);
	}

	if (rp == NULL)
	{
		fprintf (stderr, "Could not bind\n");
		return -1;
	}

	freeaddrinfo (result);

	printf("Listening on port %s\n", port);
	
	return sfd;
}

int makeSocketNonBlocking (int sfd)
{
	fcntl(sfd, F_SETFL, O_NONBLOCK);
	
//    int flags;
//	int s;
//
//    flags = fcntl (sfd, F_GETFL, 0);
//
//    if (flags == -1)
//    {
//        perror ("fcntl");
//        return -1;
//    }
//
//    flags |= O_NONBLOCK;
//    s = fcntl (sfd, F_SETFL, flags);
//
//    if (s == -1)
//    {
//        perror ("fcntl");
//        return -1;
//    }
//	
	int iSetOption = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &iSetOption, sizeof(iSetOption));
//	setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (char*)&iSetOption, sizeof(iSetOption));

	return 0;
}

int writeAll(int fd, char *buf, int len)
{
    int total = 0;        // how many bytes we've sent
    int bytesLeft = len; // how many we have left to send
    int n;

    while(total < len) 
	{
        n = write(fd, buf + total, bytesLeft);
        if (n == -1) { break; }
        total += n;
        bytesLeft -= n;
    }

    len = total; // return number actually sent here

    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
} 

int readAll(int fd)
{
	int count;
	char buffer[1024];
	
	while(1)
	{
		count = read(fd, buffer, 1023);

		if(count == -1)
		{
			if(!(errno == EAGAIN || errno == EWOULDBLOCK))
			{
				perror("Read Error");
			}

			break;
		}
		
		debug_print("readAll count: %d", count);
		buffer[count] = '\0';
		debug_print("readAll: %s", buffer);
		if(count < 1024) { break; }
		
	}
}

void free_FSConnection(FSConnection * conn)
{
	debug_print("Free connection %p, with buffer %p", conn, conn->buffer);
	
	free(conn->buffer);
	free(conn);
}

void free_FSEvent(FSEvent * event)
{
	switch(event->eventType)
	{
		case FS_EVT_NODE_ACCEPT:
		case FS_EVT_NODE_COMM:
			free(event->params);
			break;
	}
	
	free(event);
}

void free_FSTask(FSTask * task)
{
	switch(task->taskType)
	{
		case FS_TSK_STREAM_FILE:
		case FS_TSK_DELETE_FILE:
			free(task->data);
			break;
	}
	
	free(task);
}
