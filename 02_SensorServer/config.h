/**
 * \author {AUTHOR}
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <sys/types.h>
#include <time.h>

typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

/**
 * Structure storing Sensor Data
 *
 * @param id Sensor ID
 * @param value Measurement value
 * @param ts Measurement timestamp
 */
typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
} sensor_data_t;

/**
 * Thread-safe wrapper for writing to the pipe (across all main threads)
 *
 * @param pipe_fd The file descriptor for the pipe's write end.
 * @param message The message to write.
 * @return 0 on success, -1 on failure.
 */
int write_to_pipe(const char *message);

/**
 * Thread-safe wrapper for reading from the pipe (reserved for the Pipe Process)
 *
 * @param pipe_fd The file descriptor for the pipe's read end.
 * @param buffer The buffer to store the read data.
 * @param size The size of the buffer.
 * @param timeout_sec The timeout in seconds for the read operation.
 * @return Number of bytes read on success, 0 on timeout, -1 on failure.
 */
ssize_t read_from_pipe(char *buffer, ssize_t size, int timeout_sec);

#endif /* _CONFIG_H_ */
