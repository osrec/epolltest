/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   nodeProtocol.h
 * Author: osrec
 *
 * Created on 19 March 2018, 20:44
 */

#ifndef NODEPROTOCOL_H
#define NODEPROTOCOL_H

#include "fserveTypes.h"

void handleNodeConnectionRead(FSConnection * conn);

void handleNodeConnectionWrite(FSConnection * conn);

void handleNodeConnectionError(FSConnection * conn);

#endif /* NODEPROTOCOL_H */
