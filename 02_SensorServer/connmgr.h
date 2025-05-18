#ifndef CONNMGR_H
#define CONNMGR_H

#include "lib/tcpsock.h"
#include "sbuffer.h"
#include <pthread.h>

/**
 * Connection Manager Arguments
 *
 * @param buffer A pointer to the shared buffer
 * @param port Port for TCP communication
 * @param max_connections Max number of simultanous connections
 */
typedef struct {
    sbuffer_t *buffer;
    int port;
    int max_connections;
} connmgr_args_t;

/**
 * Connection Manager Thread Logic
 *
 * @param arg Set of arguments
 * @return void
 */
void *connmgr_logic(void *arg);

#endif // CONNMGR_H
