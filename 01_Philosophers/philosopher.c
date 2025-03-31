#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "philosopher.h"

typedef struct philosopher {
    int id;
    pthread_mutex_t *left_fork;
    pthread_mutex_t *right_fork;
} philosopher_t;

static pthread_mutex_t *forks;

static void *philosopher_thread(void *arg) {
    philosopher_t *phil = (philosopher_t *)arg;
    setvbuf(stdout, NULL, _IONBF, 0);
    while (1) {
        printf("Philosopher %d is thinking.\n", phil->id);
        fflush(stdout);
        usleep((rand() % 800 + 200) * 1000);  // Think for 0.2 - 0.8 sec
        printf("Philosopher %d is hungry.\n", phil->id);
        fflush(stdout);
        
        // Even and odd philosophers pick forks in opposite order to avoid deadlock.
        if (phil->id % 2 == 0) {
            pthread_mutex_lock(phil->left_fork);
            pthread_mutex_lock(phil->right_fork);
        } else {
            pthread_mutex_lock(phil->right_fork);
            pthread_mutex_lock(phil->left_fork);
        }
        
        printf("Philosopher %d is eating.\n", phil->id);
        fflush(stdout);
        usleep((rand() % 800 + 200) * 1000);  // Eat for 0.2 - 0.8 sec
        
        pthread_mutex_unlock(phil->left_fork);
        pthread_mutex_unlock(phil->right_fork);
    }
    return NULL;
}

void start_simulation(int num_philosophers) {
    // Initialize forks and philosophers
    pthread_t *threads = malloc(sizeof(pthread_t) * num_philosophers);
    philosopher_t *philosophers = malloc(sizeof(philosopher_t) * num_philosophers);
    forks = malloc(sizeof(pthread_mutex_t) * num_philosophers);

    for (int i = 0; i < num_philosophers; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }
    
    // Assign forks to philosophers, create threads
    for (int i = 0; i < num_philosophers; i++) {
        philosophers[i].id = i;
        philosophers[i].left_fork = &forks[i];
        philosophers[i].right_fork = &forks[(i + 1) % num_philosophers];
        pthread_create(&threads[i], NULL, philosopher_thread, &philosophers[i]);
    }
    
    // Wait for threads to finish
    for (int i = 0; i < num_philosophers; i++) {
        pthread_join(threads[i], NULL);
    }

    // Clean up
    for (int i = 0; i < num_philosophers; i++) {
        pthread_mutex_destroy(&forks[i]);
    }
    
    free(forks);
    free(threads);
    free(philosophers);
}
