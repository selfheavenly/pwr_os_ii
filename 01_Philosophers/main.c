#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "philosopher.h"

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_philosophers>\n", argv[0]);
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    int num_philosophers = atoi(argv[1]);
    if (num_philosophers < 2) {
        fprintf(stderr, "At least two philosophers are required.\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    start_simulation(num_philosophers);
    return 0;
}
