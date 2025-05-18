#include "connmgr.h"
#include "sbuffer.h"
#include "lib/tcpsock.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1024

static int client_count = 0;
static int max_connections;
static pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 * Structure storing Client Arguments
 *
 * @param buffer SPointer to the Shared Buffer
 * @param client_socket Client Socket struct
 * @param client_id Client Node ID
 */
typedef struct {
    sbuffer_t *buffer;
    tcpsock_t *client_socket;
} client_args_t;

// Client handler thread function
void *handle_client(void *args) {
    client_args_t *client_args = (client_args_t *)args;
    tcpsock_t *client_socket = client_args->client_socket;
    sbuffer_t *buffer = client_args->buffer;
    int client_id = 0;
    int client_id_established = 0;
    sensor_data_t data;

    // Update client counter
    pthread_mutex_lock(&count_mutex);
    client_count++;
    pthread_mutex_unlock(&count_mutex);

    char message[BUFFER_SIZE];

    while (1) {
        int bytes = sizeof(data.id);
        if (tcp_receive(client_socket, &data.id, &bytes) != TCP_NO_ERROR) break;

        bytes = sizeof(data.value);
        if (tcp_receive(client_socket, &data.value, &bytes) != TCP_NO_ERROR) break;

        bytes = sizeof(data.ts);
        if (tcp_receive(client_socket, &data.ts, &bytes) != TCP_NO_ERROR) break;

        // Check if Sensor Node has an already established ID
        if (client_id_established == 0) {
            // Identity Reveal!!
            client_id = data.id;
            client_id_established = 1;
            // LOG
            snprintf(message, BUFFER_SIZE,
                        "Sensor Node %2d has opened a new connection", client_id);
            write_to_pipe(message);
        }

        // LOG
        snprintf(message, BUFFER_SIZE,
                 "Received new data from Sensor Node %d {id: %d, value: %.2f, ts: %ld}",
                 client_id, data.id, data.value, data.ts);
        write_to_pipe(message);

        // Push data to shared buffer
        sbuffer_insert(buffer, &data);
    }

    // LOG
    snprintf(message, BUFFER_SIZE, "Sensor Node %d has closed the connection", client_id);
    write_to_pipe(message);

    tcp_close(&client_socket);
    free(client_args);

    pthread_exit(NULL);
}

void *connmgr_logic(void *arg) {
    write_to_pipe("Connection Manager started.");
    connmgr_args_t *args = (connmgr_args_t *)arg;
    sbuffer_t *buffer = args->buffer;
    int port = args->port;
    max_connections = args->max_connections;

    tcpsock_t *server_socket, *client_socket;
    pthread_t client_thread;

    char message[BUFFER_SIZE];

    // Attempt to open the server socket
    if (tcp_passive_open(&server_socket, port) != TCP_NO_ERROR) {
        // LOG
        snprintf(message, BUFFER_SIZE,
                 "Failed to open server socket on port %5d. Errno: %d (%s)",
                 port, errno, strerror(errno));
        write_to_pipe(message);
        pthread_exit(NULL);
    }

    // LOG
    snprintf(message, BUFFER_SIZE, "Server has just launched on port %5d", port);
    write_to_pipe(message);

    int client_id = 0;

    while (1) {
        // LOG
        snprintf(message, BUFFER_SIZE, "Server is waiting for new Sensor Node connection...");
        write_to_pipe(message);

        if (tcp_wait_for_connection(server_socket, &client_socket) == TCP_NO_ERROR) {
            char *client_ip = NULL;
            int client_port = 0;
            tcp_get_ip_addr(client_socket, &client_ip);
            tcp_get_port(client_socket, &client_port);

            // LOG
            snprintf(message, BUFFER_SIZE,
                     "New connection from %s:%d", client_ip, client_port);
            write_to_pipe(message);

            // Create client handler thread
            client_args_t *client_args = malloc(sizeof(client_args_t));
            if (!client_args) {
                // LOG
                snprintf(message, BUFFER_SIZE, "Memory allocation for Sensor Node %2d failed.", client_id);
                write_to_pipe(message);
                tcp_close(&client_socket);
                continue;
            }
            client_args->buffer = buffer;
            client_args->client_socket = client_socket;

            if (pthread_create(&client_thread, NULL, handle_client, client_args) != 0) {
                // LOG
                write_to_pipe("Failed to create a thread for a new Sensor Node");
                tcp_close(&client_socket);
                free(client_args);
            } else {
                pthread_detach(client_thread);
            }
        } else {
            snprintf(message, BUFFER_SIZE,
                     "Failed to accept Sensor Node connection. Errno: %d (%s)", errno, strerror(errno));
            write_to_pipe(message);
        }

        // Check termination condition
        pthread_mutex_lock(&count_mutex);
        if (client_count >= max_connections) {
            // LOG
            write_to_pipe("Max number of simultanous clients reached.");
            pthread_mutex_unlock(&count_mutex);
            break;
        }
        pthread_mutex_unlock(&count_mutex);
    }

    tcp_close(&server_socket);
    // LOG
    write_to_pipe("Server socket closed.");
    sbuffer_terminate(buffer);
    pthread_exit(NULL);
}
