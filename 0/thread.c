#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>

#define LOG_FILE "/tmp/threadpool.log"

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

void log_message(const char *format, ...) {
    FILE *logfile = fopen(LOG_FILE, "a");
    if (logfile == NULL) {
        perror("Failed to open log file");
        return;
    }

    va_list args;
    va_start(args, format);
    vfprintf(logfile, format, args);
    va_end(args);

    fprintf(logfile, "\n");
    fclose(logfile);
}

int threadpool_add_task(threadpool_t *pool, void (*function)(void *), void *arg) {
    pthread_mutex_lock(&(pool->lock));

    if (pool->shutdown) {
        pthread_mutex_unlock(&(pool->lock));
        log_message("Threadpool is shutting down. Cannot add new task.");
        return -1;
    }

    task_t *new_task = (task_t *)malloc(sizeof(task_t));
    if (new_task == NULL) {
        pthread_mutex_unlock(&(pool->lock));
        log_message("Failed to allocate memory for new task.");
        return -1;
    }
    new_task->function = function;
    new_task->arg = arg;
    new_task->next = NULL;

    task_t *last_task = pool->task_queue;
    if (last_task == NULL) {
        pool->task_queue = new_task;
    } else {
        while (last_task->next != NULL) {
            last_task = last_task->next;
        }
        last_task->next = new_task;
    }

    pool->tasks_in_queue++;

    log_message("Task added to the queue. Waiting to be picked up. Tasks in queue: %d", pool->tasks_in_queue);

    pthread_cond_signal(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));
    return 0;
}

task_t *threadpool_get_task(threadpool_t *pool) {
    if (pool->tasks_in_queue == 0) {
        return NULL;
    }

    task_t *task = pool->task_queue;
    pool->task_queue = task->next;
    pool->tasks_in_queue--;

    log_message("Task removed from the queue. Tasks left in queue: %d", pool->tasks_in_queue);

    return task;
}

void *threadpool_worker(void *arg) {
    threadpool_t *pool = (threadpool_t *)arg;

    while (1) {
        pthread_mutex_lock(&(pool->lock));

        while (pool->tasks_in_queue == 0 && !pool->shutdown) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if (pool->shutdown) {
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }

        task_t *task = threadpool_get_task(pool);
        pthread_mutex_unlock(&(pool->lock));

        if (task != NULL) {
            log_message("Task picked up by a worker thread. Running...");

            task->function(task->arg);

            log_message("Task completed execution.");
            free(task);
        }
    }

    return NULL;
}

threadpool_t *threadpool_init(int num_threads, int queue_size) {
    threadpool_t *pool = (threadpool_t *)malloc(sizeof(threadpool_t));
    if (pool == NULL) {
        log_message("Failed to allocate memory for threadpool.");
        return NULL;
    }

    pool->thread_count = num_threads;
    pool->task_queue_size = queue_size;
    pool->tasks_in_queue = 0;
    pool->shutdown = false;
    pool->task_queue = NULL;

    pthread_mutex_init(&(pool->lock), NULL);
    pthread_cond_init(&(pool->notify), NULL);

    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    if (pool->threads == NULL) {
        log_message("Failed to allocate memory for threadpool threads.");
        free(pool);
        return NULL;
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&(pool->threads[i]), NULL, threadpool_worker, pool) != 0) {
            log_message("Failed to create thread.");
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }

    log_message("Threadpool initialized with %d threads.", num_threads);
    return pool;
}

void threadpool_shutdown(threadpool_t *pool) {
    pthread_mutex_lock(&(pool->lock));
    pool->shutdown = true;
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);

    while (pool->task_queue != NULL) {
        task_t *task = pool->task_queue;
        pool->task_queue = task->next;
        free(task);
    }

    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));

    free(pool);
    log_message("Threadpool shutdown complete.");
}

