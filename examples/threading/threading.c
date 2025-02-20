#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // DONE: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    thread_func_args->thread_complete_success = false;

    // Wait for specified time.
    usleep(thread_func_args->wait_to_obtain_ms * 1000);

    // Obtain mutex.
    int mutex_lock_err = pthread_mutex_lock(thread_func_args->mutex);

    // If no error in mutex lock.
    if (!mutex_lock_err) {
        DEBUG_LOG("Mutex lock successful.");

        // Wait for specified release time.
        usleep(thread_func_args->wait_to_release_ms * 1000);

        // Release mutex.
        int mutex_unlock_err = pthread_mutex_unlock(thread_func_args->mutex);

        // If no error in mutex unlock.
        if (!mutex_unlock_err != 0) {
            DEBUG_LOG("Mutex unlock successful.");
            thread_func_args->thread_complete_success = true;
        }
        else { ERROR_LOG("Error during mutex unlock with code %d", mutex_unlock_err); }
    }
    else { ERROR_LOG("Error during mutex lock with code %d", mutex_lock_err); }

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * DONE: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    // Allocate memory for the thread data struct.
    struct thread_data *new_thread_data = (struct thread_data *)malloc(sizeof(struct thread_data));
    if (new_thread_data == NULL) {
        ERROR_LOG("Unsuccessful malloc for thread data.");
        return false;
    }

    // Initialise the struct with the required data.
    new_thread_data->mutex = mutex;
    new_thread_data->wait_to_obtain_ms = wait_to_obtain_ms;
    new_thread_data->wait_to_release_ms = wait_to_release_ms;
    new_thread_data->thread_complete_success = false;

    // Create the thread.
    int thread_err = pthread_create(thread, NULL, threadfunc, new_thread_data);

    // Log error if any when creating thread.
    if (thread_err) {
        ERROR_LOG("Error in creating thread with code %d", thread_err);
        free(new_thread_data);
        return false;
    }

    // Handle post thread completion.
    DEBUG_LOG("Thread successful.");
    return true;
}

