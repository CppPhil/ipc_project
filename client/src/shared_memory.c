#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include <arpa/inet.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "shared_memory.h"
#include "read_u32.h"

int sharedMemory()
{
    static const int projectId = 1;
    key_t            key       = ftok("server", projectId);

    if (key == -1) {
        fprintf(stderr, "Client: ftok failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    static const size_t sharedMemorySize
        = (3 * sizeof(uint32_t)) + sizeof(pthread_mutexattr_t)
          + sizeof(pthread_mutex_t) + sizeof(bool);

    const int sharedMemoryId
        = shmget(key, sharedMemorySize, 0666);

    if (sharedMemoryId == -1) {
        fprintf(stderr, "Client: shmget failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    void* memory = shmat(sharedMemoryId, NULL, 0);

    if (memory == (void*) -1) {
        fprintf(stderr, "Client: shmat failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Please enter x:");
    bool     ok;
    uint32_t x = readU32(&ok);

    if (!ok) {
        fprintf(stderr, "client: could not read x.\n");
        shmdt(memory);
        return EXIT_FAILURE;
    }

    printf("Please enter y:");
    uint32_t y = readU32(&ok);

    if (!ok) {
        fprintf(stderr, "client: could not read y.\n");
        shmdt(memory);
        return EXIT_FAILURE;
    }

    x = htonl(x);
    y = htonl(y);

    uint32_t* px = memory;
    uint32_t* py = px + 1;
    uint32_t* presult = py + 1;
    pthread_mutexattr_t *mutexAttr = (pthread_mutexattr_t *) (presult + 1);
    pthread_mutex_t *    mutex     = (pthread_mutex_t *) (mutexAttr + 1);
    bool *               hasClientFinished = (bool *) (mutex + 1);
    (void)mutexAttr;

    int statusCode = pthread_mutex_lock(mutex);

    if (statusCode != 0) {
        fprintf(stderr, "Client: could not lock mutex: %s\n", strerror(statusCode));
        shmdt(memory);
        return EXIT_FAILURE;
    }

    *px = x;
    *py = y;

    *hasClientFinished = true;

    statusCode = pthread_mutex_unlock(mutex);

    if (statusCode != 0) {
        fprintf(stderr, "Client: could not unlock: %s\n", strerror(statusCode));
        shmdt(memory);
        return EXIT_FAILURE;
    }


    if (shmdt(memory) == -1) {
        fprintf(stderr, "Client could not detach from the shared memory segment: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
