#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#include "shared_memory.h"

int sharedMemory()
{
    static const int projectId = 1;
    key_t            key       = ftok("server", projectId);

    if (key == -1) {
        fprintf(stderr, "Server: ftok failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    static const size_t sharedMemorySize
        = (3 * sizeof(uint32_t)) + sizeof(pthread_mutexattr_t)
          + sizeof(pthread_mutex_t) + sizeof(bool);

    const int sharedMemoryId
        = shmget(key, sharedMemorySize, IPC_CREAT | IPC_EXCL | 0666);

    if (sharedMemoryId == -1) {
        fprintf(stderr, "Server: shmget failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    /* If shmaddr is NULL, the system chooses a suitable (unused) address at
     * which to attach the segment. */
    static const void *chooseAddress = NULL;
    static const int   noFlags       = 0;

    void *memory = shmat(
        /* shmid */ sharedMemoryId,
        /* shmaddr */ chooseAddress,
        /* shmflg */ noFlags);

    if (memory == (void *) -1) {
        fprintf(stderr, "Server: shmat failed: %s\n", strerror(errno));

        /* If cmd is IPC_RMID then the buf argument is ignored. */
        shmctl(/* shmid */ sharedMemoryId, /* cmd */ IPC_RMID, /* buf */ NULL);

        return EXIT_FAILURE;
    }

    uint32_t *           x         = memory;
    uint32_t *           y         = x + 1;
    uint32_t *           result    = y + 1;
    pthread_mutexattr_t *mutexAttr = (pthread_mutexattr_t *) (result + 1);
    pthread_mutex_t *    mutex     = (pthread_mutex_t *) (mutexAttr + 1);
    bool *               hasClientFinished = (bool *) (mutex + 1);

    int statusCode = pthread_mutexattr_init(mutexAttr);

    if (statusCode != 0) {
        fprintf(
            stderr,
            "Server: couldn't initialize mutex attribute: %s\n",
            strerror(statusCode));
        shmdt(memory);
        shmctl(sharedMemoryId, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }

    /* permit a mutex to be operated upon by any thread that has access to the
     * memory where the mutex is allocated, even if the mutex is allocated in
     * memory that is shared by multiple processes */
    statusCode
        = pthread_mutexattr_setpshared(mutexAttr, PTHREAD_PROCESS_SHARED);

    if (statusCode != 0) {
        fprintf(
            stderr,
            "Server: couldn't set PTHREAD_PROCESS_SHARED: %s\n",
            strerror(statusCode));
        pthread_mutexattr_destroy(mutexAttr);
        shmdt(memory);
        shmctl(sharedMemoryId, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }

    statusCode = pthread_mutex_init(mutex, mutexAttr);

    if (statusCode != 0) {
        fprintf(
            stderr,
            "Server: couldn't initialize mutex: %s\n",
            strerror(statusCode));
        pthread_mutexattr_destroy(mutexAttr);
        shmdt(memory);
        shmctl(sharedMemoryId, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }

    *hasClientFinished = false;

    for (;;) {
        statusCode = pthread_mutex_lock(mutex);

        if (statusCode != 0) {
            fprintf(
                stderr,
                "Server: could not lock mutex: %s\n",
                strerror(statusCode));
            pthread_mutex_destroy(mutex);
            pthread_mutexattr_destroy(mutexAttr);
            shmdt(memory);
            shmctl(sharedMemoryId, IPC_RMID, NULL);
            return EXIT_FAILURE;
        }

        if (*hasClientFinished) {
            break;
        }

        statusCode = pthread_mutex_unlock(mutex);

        if (statusCode != 0) {
            fprintf(
                stderr,
                "Server: could not unlock mutex: %s\n",
                strerror(statusCode));
            pthread_mutex_destroy(mutex);
            pthread_mutexattr_destroy(mutexAttr);
            shmdt(memory);
            shmctl(sharedMemoryId, IPC_RMID, NULL);
            return EXIT_FAILURE;
        }

        statusCode = usleep(100);

        if (statusCode == -1) {
            fprintf(
                stderr,
                "Server: failure to invoke usleep: %s\n",
                strerror(errno));
            pthread_mutex_destroy(mutex);
            pthread_mutexattr_destroy(mutexAttr);
            shmdt(memory);
            shmctl(sharedMemoryId, IPC_RMID, NULL);
            return EXIT_FAILURE;
        }
    }

    *x = ntohl(*x);
    *y = ntohl(*y);

    *result = *x + *y;

    printf(
        "Server: %" PRIu32 " + %" PRIu32 " = %" PRIu32 "\n", *x, *y, *result);

    statusCode = pthread_mutex_destroy(mutex);

    if (statusCode != 0) {
        fprintf(
            stderr,
            "Server could not destroy mutex: %s\n",
            strerror(statusCode));
        pthread_mutexattr_destroy(mutexAttr);
        shmdt(memory);
        shmctl(sharedMemoryId, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }

    statusCode = pthread_mutexattr_destroy(mutexAttr);

    if (statusCode != 0) {
        shmdt(memory);
        shmctl(sharedMemoryId, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }

    if (shmdt(memory) == -1) {
        fprintf(
            stderr,
            "Server: could not detach from shared memory segment: %s\n",
            strerror(errno));
        shmctl(sharedMemoryId, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }

    if (shmctl(sharedMemoryId, IPC_RMID, NULL) == -1) {
        fprintf(
            stderr,
            "Server couldn't delete shared memory segment: %s\n",
            strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
