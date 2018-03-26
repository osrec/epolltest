/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fserveGlobals.h
 * Author: osrec
 *
 * Created on 19 March 2018, 22:18
 */

#ifndef FSERVEGLOBALS_H
#define FSERVEGLOBALS_H

extern int ioEventFD;
extern int epollFD;
extern int listenerSocketFD;

extern int workerEpollFD;
extern int workerEventFD;

extern aio_context_t ioContext;

extern GHashTable* activeNodeConnectionFDMap;
extern GHashTable* pendingKeyMap;

extern pthread_mutex_t eventTaskQueueMutex;
extern GQueue* eventTaskQueue;

extern pthread_mutex_t reactorTaskMutex;
extern GQueue* reactorTaskQueue;

extern int NUMBER_OF_WORKERS;

extern pthread_mutex_t connectionNumberMutex;
extern unsigned long connectionNumber;

extern unsigned long workerTaskNumber;
extern unsigned long doneWorkerTaskNumber;

#endif /* FSERVEGLOBALS_H */

