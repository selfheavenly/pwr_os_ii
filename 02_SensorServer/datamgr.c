#include "datamgr.h"
#include "sbuffer.h"
#include "lib/dplist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define RUN_AVG_LENGTH 5
#define MIN_TEMP 10.0
#define MAX_TEMP 16.5

/**
 * Structure storing Client Arguments
 *
 * @param sensor_id Sensort ID
 * @param room_id Room id for which the average is counted
 * @param running_avg Array storing source values
 * @param avg_index Client Index of average
 * @param current_avg Current Average value
 * @param last_modified Last modified timestamp
 */
typedef struct {
    uint16_t sensor_id;
    uint16_t room_id;
    double running_avg[RUN_AVG_LENGTH];
    int avg_index;
    double current_avg;
    time_t last_modified;
} sensor_node_t;

// Function prototypes
void *datamgr_logic(void *arg);
sensor_node_t *create_sensor_node(uint16_t sensor_id, uint16_t room_id);
void update_running_avg(sensor_node_t *node, double new_value);
void free_sensor_node(void **element);
int sensor_node_compare(void *x, void *y);

// Free sensor node
void free_sensor_node(void **element) {
    if (element && *element) {
        free(*element);
        *element = NULL;
    }
}

// Compare sensor nodes
int sensor_node_compare(void *x, void *y) {
    sensor_node_t *node_x = (sensor_node_t *)x;
    sensor_node_t *node_y = (sensor_node_t *)y;
    return (node_x->sensor_id - node_y->sensor_id);
}

void *datamgr_logic(void *arg) {
    write_to_pipe("Data Manager started.");

    sbuffer_t *buffer = (sbuffer_t *)arg;
    dplist_t *sensor_list = dpl_create(NULL, free_sensor_node, sensor_node_compare);

    // Load room-sensor mapping
    FILE *map_file = fopen("room_sensor.map", "r");
    if (!map_file) {
        write_to_pipe("[ERROR] Unable to open room_sensor.map.");
        return NULL;
    }

    uint16_t room_id, sensor_id;
    while (fscanf(map_file, "%hu %hu", &room_id, &sensor_id) == 2) {
        sensor_node_t *node = create_sensor_node(sensor_id, room_id);
        dpl_insert_at_index(sensor_list, node, 0, false);
    }
    fclose(map_file);

    sensor_data_t data;
    while (1) {
        if (sbuffer_is_terminated(buffer)) {
            break;
        }

        // Read unprocessed sensor data
        if (sbuffer_read_unprocessed(buffer, &data, 0) == 0) {
            // Find the sensor node in the list
            sensor_node_t key = { .sensor_id = data.id };
            int index = dpl_get_index_of_element(sensor_list, &key);

            if (index == -1) {
                // Log if sensor ID is not found
                char message[BUFFER_SIZE];
                snprintf(message, BUFFER_SIZE, "Received sensor data with invalid sensor node ID %d", data.id);
                write_to_pipe(message);
                usleep(20);
                sbuffer_mark_processed(buffer, &data);
                continue;
            }

            sensor_node_t *node = dpl_get_element_at_index(sensor_list, index);

            // Update the running average
            update_running_avg(node, data.value);

            // Check if the running average is out of bounds
            if (node->current_avg < MIN_TEMP) {
                char message[BUFFER_SIZE];
                snprintf(message, BUFFER_SIZE, "Sensor node %d reports it's too cold (avg temp = %f)", node->sensor_id, node->current_avg);
                write_to_pipe(message);
            } else if (node->current_avg > MAX_TEMP) {
                char message[BUFFER_SIZE];
                snprintf(message, BUFFER_SIZE, "Sensor node %d reports it's too hot (avg temp = %f)", node->sensor_id, node->current_avg);
                write_to_pipe(message);
            }

            // Log the processing
            char message[BUFFER_SIZE];
            snprintf(message, BUFFER_SIZE,
                     "Processed sensor data {id: %d, value: %.2f, avg: %.2f, ts: %ld}",
                     data.id, data.value, node->current_avg, data.ts);
            write_to_pipe(message);

            // Mark the data as processed
            sbuffer_mark_processed(buffer, &data);
        } else {
            // No unprocessed data, wait
            sleep(1);
        }
    }

    write_to_pipe("Data Manager exited.");
    dpl_free(&sensor_list, true);
    return NULL;
}

// Create a new sensor node
sensor_node_t *create_sensor_node(uint16_t sensor_id, uint16_t room_id) {
    sensor_node_t *node = malloc(sizeof(sensor_node_t));
    node->sensor_id = sensor_id;
    node->room_id = room_id;
    memset(node->running_avg, 0, sizeof(node->running_avg));
    node->avg_index = 0;
    node->current_avg = 0.0;
    node->last_modified = 0;
    return node;
}

// Update the running average
void update_running_avg(sensor_node_t *node, double new_value) {
    node->running_avg[node->avg_index] = new_value;
    node->avg_index = (node->avg_index + 1) % RUN_AVG_LENGTH;

    double sum = 0.0;
    for (int i = 0; i < RUN_AVG_LENGTH; i++) {
        sum += node->running_avg[i];
    }
    node->current_avg = sum / RUN_AVG_LENGTH;
}
