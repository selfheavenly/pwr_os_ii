#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>
#include <sys/wait.h>

#include "config.h"
#include "sbuffer.h"
#include "connmgr.h"
#include "datamgr.h"
#include "sensor_db.h"

#define READ_END 0
#define WRITE_END 1

#define LOGGER_TIMEOUT_S 4
#define LOGGER_RETRIES_LIMIT 4

#define LOG_FILE "gateway.log"
#define BUFFER_SIZE 1024

// Buffer Sync
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_cond = PTHREAD_COND_INITIALIZER;

// Pipe Sync
pthread_mutex_t pipe_mutex = PTHREAD_MUTEX_INITIALIZER;

int PIPE_READ, PIPE_WRITE;

int write_to_pipe(const char *message) {
    pthread_mutex_lock(&pipe_mutex);

    // Allocate a buffer to include a newline and null terminator
    char formatted_message[BUFFER_SIZE];
    snprintf(formatted_message, sizeof(formatted_message), "%s\n", message);

    ssize_t bytes_written = write(PIPE_WRITE, formatted_message, strlen(formatted_message));
    if (bytes_written < 0) {
        perror("[ERROR] Writing to pipe failed");
        pthread_mutex_unlock(&pipe_mutex);
        return -1;
    }

    pthread_mutex_unlock(&pipe_mutex);
    return 0;
}

ssize_t read_from_pipe(char *buffer, ssize_t size, int timeout_sec) {
    struct timeval timeout;
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(PIPE_READ, &read_fds);

    int retval = select(PIPE_READ + 1, &read_fds, NULL, NULL, &timeout);
    if (retval == -1) {
        perror("[ERROR] Select failed");
        return -1;
    } else if (retval == 0) {
        // Timeout occurred
        return 0;
    }

    return read(PIPE_READ, buffer, size);
}

void main_process(int port, int max_clients) {
    printf("Main process started. Port: %d, Max Clients: %d\n", port, max_clients);

    // Shared buffer initialization
    sbuffer_t *shared_buffer = sbuffer_init();
    if (!shared_buffer) {
        perror("[ERROR] Failed to initialize shared buffer");
        exit(EXIT_FAILURE);
    }

    // Connection Manager Arguments
    connmgr_args_t connmgr_args = {
        .buffer = shared_buffer,
        .port = port,
        .max_connections = max_clients
    };

    // Threads
    pthread_t connmgr_tid, datamgr_tid, storagemgr_tid;

    // Create threads with error handling
    if (pthread_create(&connmgr_tid, NULL, connmgr_logic, &connmgr_args) != 0 ||
        pthread_create(&datamgr_tid, NULL, datamgr_logic, shared_buffer) != 0 ||
        pthread_create(&storagemgr_tid, NULL, sensor_db_logic, shared_buffer) != 0) {
        perror("[ERROR] Failed to create threads");
        sbuffer_free(shared_buffer);
        exit(EXIT_FAILURE);
    }

    // Wait for threads to complete
    pthread_join(connmgr_tid, NULL);
    pthread_join(datamgr_tid, NULL);
    pthread_join(storagemgr_tid, NULL);

    // Cleanup
    sbuffer_free(shared_buffer);
    pthread_mutex_destroy(&pipe_mutex);

    printf("Main process exited.\n");
}

void logger_process(void) {
    printf("Logger Process started.\n");
    FILE *log_file = fopen(LOG_FILE, "w");
    if (!log_file) {
        perror("[ERROR] Unable to open log file");
        return;
    }

    char buffer[BUFFER_SIZE];
    char *line;
    int sequence_number = 0;
    int retries_count = 0;

    while (1) {
        ssize_t bytes_read = read_from_pipe(buffer, sizeof(buffer) - 1, LOGGER_TIMEOUT_S);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Null-terminate the string

            // Process each line individually
            line = strtok(buffer, "\n");
            while (line != NULL) {
                time_t now = time(NULL);
                struct tm *timeinfo = localtime(&now);

                char timestamp[20];
                strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

                fprintf(log_file, "%6d %s %s\n", sequence_number++, timestamp, line);
                fflush(log_file);

                line = strtok(NULL, "\n"); // Get the next line
            }
            retries_count = 0; // Reset retries count after successful read
        } else if (bytes_read == 0) {
            // Timeout occurred
            if (retries_count < LOGGER_RETRIES_LIMIT) {
                fprintf(stderr, "Logger timeout [%d/%d]: No data received. Retry in %d seconds...\n",
                        ++retries_count, LOGGER_RETRIES_LIMIT, LOGGER_TIMEOUT_S);
                sleep(LOGGER_TIMEOUT_S); // Enforce timeout
            } else {
                fprintf(stderr, "Logger timeout of %d retries reached. Shutting down.\n", LOGGER_RETRIES_LIMIT);
                break;
            }
        } else if (bytes_read < 0) {
            // Pipe closed or error
            fprintf(stderr, "Logger: Pipe closed or read error. Exiting...\n");
            break;
        }
    }

    fclose(log_file);
    printf("Logger process exited.\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Gateway Init Hint: %s <port> <max_clients>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    int max_clients = atoi(argv[2]);

    if (port <= 0 || max_clients <= 0) {
        fprintf(stderr, "Error: Invalid port or max_clients value.\n");
        return EXIT_FAILURE;
    }

    printf("Sensor_gateway program started.\n");
    int pipe_fd[2];

    if (pipe(pipe_fd) < 0) {
        perror("ERROR: Pipe couldn't be created");
        return EXIT_FAILURE;
    }

    PIPE_WRITE = pipe_fd[WRITE_END];
    PIPE_READ = pipe_fd[READ_END];

    int process_id = fork();
    if (process_id < 0) {
        perror("ERROR: Fork unsuccessful");
        return EXIT_FAILURE;
    }

    // CHILD Process (Logger)
    if (process_id == 0) {
        close(PIPE_WRITE);
        logger_process();
        close(PIPE_READ);
        return EXIT_SUCCESS;
    }

    // PARENT Process (Main)
    else {
        close(PIPE_READ);
        main_process(port, max_clients);
        close(PIPE_WRITE);
        // Wait for the CHILD process to exit
        if (waitpid(process_id, NULL, 0) == -1) {
            perror("ERROR: Failed to wait for logger process");
        }
    }

    printf("Sensor_gateway program exited.\n");
    return EXIT_SUCCESS;
}
