#ifndef SBUFFER_H
#define SBUFFER_H

#include "config.h"
#include <pthread.h>

#define SBUFFER_FAILURE -1
#define SBUFFER_SUCCESS 0
#define SBUFFER_NO_DATA 1

typedef struct sbuffer sbuffer_t;

/**
 * Allocates and initializes a new shared buffer
 * \param buffer a double pointer to the buffer that needs to be initialized
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
sbuffer_t *sbuffer_init();

/**
 * All allocated resources are freed and cleaned up
 * \param buffer a double pointer to the buffer that needs to be freed
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_free(sbuffer_t *buffer);

/**
 * Inserts the sensor data in 'data' at the end of 'buffer' (at the 'tail')
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to sensor_data_t data, that will be copied into the buffer
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
*/
int sbuffer_insert(sbuffer_t *buffer, const sensor_data_t *data);

/**
 * Removes the first sensor data in 'buffer' (at the 'head') and returns this sensor data as '*data'
 * If 'buffer' is empty, the function doesn't block until new sensor data becomes available but returns SBUFFER_NO_DATA
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to pre-allocated sensor_data_t space, the data will be copied into this structure. No new memory is allocated for 'data' in this function.
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data);

/**
 * Reads the next unprocessed node (used by Data Manager)
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to sensor_data_t data, that will be copied into the buffer
 * \param processed_flag 0 for unprocessed, 1 for processed data
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_read_unprocessed(sbuffer_t *buffer, sensor_data_t *data, int processed_flag);

/**
 * Marks the data as processed (used by Data Manager)
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to sensor_data_t data, that will be marked as processed
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_mark_processed(sbuffer_t *buffer, const sensor_data_t *data);

/**
 * Checks if the buffer is empty
 * \param buffer a pointer to the buffer that is used
 * \return 1 if empty, 0 if not empty
 */
int sbuffer_is_empty(sbuffer_t *buffer);

/**
 * Signals that no more data will be inserted
 * \param buffer a pointer to the buffer that is used
 * \return void
 */
void sbuffer_terminate(sbuffer_t *buffer);

/**
 * Signals that no more data will be inserted
 * \param buffer a pointer to the buffer that is used
 * \return 1 if terminated, 0 if not
 */
int sbuffer_is_terminated(sbuffer_t *buffer);


#endif // SBUFFER_H
