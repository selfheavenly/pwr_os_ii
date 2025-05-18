#ifndef DATAMGR_H
#define DATAMGR_H

#include "sbuffer.h"
#include <pthread.h>


/**
 * Data Manager Thread Logic
 * \param arg a pointer to the arguments
 * \return void
 */
void *datamgr_logic(void *arg);

#endif // DATAMGR_H
