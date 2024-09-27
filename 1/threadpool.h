#include <stdio.h>
#include <stdbool.h>

// Structure to define a task in the thread pool
typedef struct task_t {
    void (*function)(void *arg);  // Function to execute
    void *arg;                    // Argument to pass to the function
    struct task_t *next;          // Pointer to the next task in the queue
} task_t;

// Structure to define the thread pool
typedef struct {
    pthread_mutex_t lock;         // Mutex to protect shared data
    pthread_cond_t notify;        // Condition variable to signal worker threads
    pthread_t *threads;           // Array of worker threads
    task_t *task_queue;           // Head of the task queue
    bool shutdown;                // Flag to indicate if the pool is shutting down
    int thread_count;             // Number of worker threads
    int task_queue_size;          // Maximum size of the task queue
    int tasks_in_queue;           // Current number of tasks in the queue
} threadpool_t;

//void logit(const char *format, ...);
threadpool_t *threadpool_init(int num_threads, int queue_size);
int threadpool_add_task(threadpool_t *pool, void (*function)(void *), void *arg);
void threadpool_clean(threadpool_t *pool);
