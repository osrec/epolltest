/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fserveTypes.h
 * Author: osrec
 *
 * Created on 19 March 2018, 21:32
 */

#ifndef FSERVETYPES_H
#define FSERVETYPES_H

// Defines the various types of tasks we can perform

typedef enum 
{
	FS_EVT_NODE_ACCEPT					= 1,
	FS_EVT_NODE_COMM					= 2
			
} FSEventType;


typedef enum 
{
	FS_TSK_STREAM_FILE					= 1,
	FS_TSK_DELETE_FILE					= 2
			
} FSTaskType;

typedef enum 
{
	FS_CONN_CONNECTING			= 0x1 << 0,
	FS_CONN_AUTHENTICATING		= 0x1 << 1,
	FS_CONN_READY				= 0x1 << 2,
	FS_CONN_STREAMINGOUT		= 0x1 << 3,
	FS_CONN_STREAMINGIN			= 0x1 << 4,
	FS_CONN_RECEIVING_COMMAND	= 0x1 << 5,
	FS_CONN_AWAITING_RESPONSE	= 0x1 << 6,
	FS_CONN_DOING_TASK			= 0x1 << 7
		

} FSConnectionState;


typedef enum 
{
	FS_CONN_TYPE_CLIENT			= 0x1 << 0,
	FS_CONN_TYPE_NODE			= 0x1 << 1,
	FS_CONN_TYPE_HTTP_CLIENT	= 0x1 << 2,
	FS_CONN_TYPE_LISTENER		= 0x1 << 3,
	FS_CONN_TYPE_DUMMY			= 0x1 << 4

} FSConnectionType;


typedef struct FSEvent
{
	FSEventType eventType;
	void *params;
	
} FSEvent;


typedef struct FSNetworkEventParams
{
	unsigned long connNumber;
	
} FSNetworkEventParams;


// Task related structs

typedef struct FSTask
{
	unsigned long connNumber;
	FSTaskType taskType;
	void * data;
	
} FSTask;

typedef struct FSConnection
{
	int fd;
	FSConnectionType type;
	FSConnectionState stateCode;
	long offset;
	long length;
	char * buffer;
	unsigned long id;
	int workerNumber;
	
} FSConnection;

#endif /* FSERVETYPES_H */

