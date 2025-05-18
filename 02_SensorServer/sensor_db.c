#include "sensor_db.h"
#include "sbuffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define CSV_FILE "data.csv"
#define BUFFER_SIZE 1024

void *sensor_db_logic(void *arg) {
    char message[BUFFER_SIZE];
    write_to_pipe("Storage Manager started.");

    sbuffer_t *buffer = (sbuffer_t *)arg;

    FILE *csv_file = fopen(CSV_FILE, "w");
    if (!csv_file) {
        write_to_pipe("ERROR: Unable to open CSV file.");
        return NULL;
    }
    else {
        write_to_pipe("A new data.csv file has been created.");
    }

    // CSV Header
    fprintf(csv_file, "SensorID,Value,Timestamp\n");

    sensor_data_t data;
    while (1) {
        // Check if the buffer has already been terminated
        if (sbuffer_is_terminated(buffer)) {
            break;
        }

        // Read processed data (processed_flag == 1)
        if (sbuffer_read_unprocessed(buffer, &data, 1) == 0) {
            // Save to CSV
            fprintf(csv_file, "%d,%.2f,%ld\n", data.id, data.value, data.ts);
            fflush(csv_file);

            // Remove from buffer, as it's already saved to csv
            sbuffer_remove(buffer, &data);

            // LOG
            snprintf(message, BUFFER_SIZE,
                     "Data insertion from sensor %d succeeded.",
                     data.id);
            write_to_pipe(message);
        } else {
            sleep(1);
        }
    }

    fclose(csv_file);
    // LOG
    snprintf(message, BUFFER_SIZE, "The data.csv file has been closed.");
    write_to_pipe(message);
    
    write_to_pipe("Storage Manager exited.");
    return NULL;
}
