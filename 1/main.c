#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "threadpool.h" // Include your threadpool header

#define logit(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

extern threadpool_t *threadpool;

void task_function(void *arg) {
    int task_num = *((int *)arg);
    int work_duration = rand() % 10 + 1; // Sleep between 1 and 5 seconds

    logit("[Task: %d] Task is running. Simulating work for %d seconds...", task_num, work_duration);

    sleep(work_duration);  // Simulate workload

    logit("[Task: %d] ############ Task has completed.", task_num);

    free(arg); // Free the allocated memory
}

int main() {
    srand(time(NULL));  // Seed the random number generator

    // Initialize threadpool with 4 threads and a task queue size of 10
    threadpool = threadpool_init(4, 10);
    if (threadpool == NULL) {
        logit("Failed to initialize threadpool.");
        return -1;
    }

    // Add tasks to the pool
    for (int i = 1; i <= 10; i++) {
        int *task_num = malloc(sizeof(int));
        if (task_num == NULL) {
            logit("Failed to allocate memory for task number.");
            threadpool_clean(threadpool);
            return -1;
        }
        *task_num = i;
        logit("[Task: %d] Adding Task to the pool.", i);
        if (threadpool_add_task(threadpool, task_function, task_num) != 0) {
            logit("[Error] Failed to add task %d to the pool.", i);
            free(task_num);
        }
    }

    // Wait for tasks to complete (adjust as needed or use a better synchronization method)

    logit("LOOP.....");
    getchar();

    // Clean the threadpool
    threadpool_clean(threadpool);

    return 0;
}
