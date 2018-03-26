/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   reactor.h
 * Author: osrec
 *
 * Created on 19 March 2018, 23:01
 */

#ifndef REACTOR_H
#define REACTOR_H

#include "fserveTypes.h"

unsigned long incrementConnectionNumber();

FSTask * getReactorTask();

int putReactorTask(void * task);

void doPendingReactorTasks(int maxTasks);

void * reactorLoop(void * args);

void acceptConnections(int listenerSocketFD);

#endif /* REACTOR_H */
