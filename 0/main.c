#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef struct task_t {
    void (*function)(void *arg);
    void *arg;
    struct task_t *next;
} task_t;


typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    task_t *task_queue;
    bool shutdown;
    int thread_count;
    int task_queue_size;
    int tasks_in_queue;
} threadpool_t;


// Include your threadpool header or place the function declarations at the top
// e.g., threadpool.h
// #include "threadpool.h"

// Task function that mimics workload by sleeping for some time
void task_function(void *arg) {
    int task_num = *((int *)arg);
    int work_duration = rand() % 5 + 1; // Sleep between 1 and 5 seconds

    log_message("Task %d is running. Simulating work for %d seconds...", task_num, work_duration);

    sleep(work_duration);  // Simulate workload

    log_message("Task %d has completed.", task_num);
}

int main() {
    srand(time(NULL));  // Seed the random number generator

    // Initialize threadpool with 4 threads and a task queue size of 10
    threadpool_t *pool = threadpool_init(4, 10);
    if (pool == NULL) {
        printf("Failed to initialize threadpool.\n");
        return -1;
    }

    // Add tasks to the pool
    for (int i = 1; i <= 10; i++) {
        int *task_num = malloc(sizeof(int));
        *task_num = i;
        log_message("Adding Task %d to the pool.", i);
        threadpool_add_task(pool, task_function, task_num);
    }

    // Wait for tasks to complete
    sleep(15);  // Allow time for all tasks to finish

    // Shutdown the threadpool
    threadpool_shutdown(pool);

    return 0;
}

