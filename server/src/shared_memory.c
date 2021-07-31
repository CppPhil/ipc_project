#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <semaphore.h>
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
        = (3 * sizeof(uint32_t)) + sizeof(sem_t);

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

    uint32_t *x         = memory;
    uint32_t *y         = x + 1;
    uint32_t *result    = y + 1;
    sem_t *   semaphore = (sem_t *) (result + 1);

    int statusCode
        = sem_init(/* __sem */ semaphore, /* __pshared */ 1, /* __value */ 0);

    if (statusCode == -1) {
        fprintf(
            stderr,
            "Server: could not initialize semaphore: %s\n",
            strerror(errno));
        shmdt(memory);
        shmctl(sharedMemoryId, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }

    statusCode = sem_wait(semaphore);

    if (statusCode == -1) {
        fprintf(
            stderr,
            "Server: couldn't wait on semaphore: %s\n",
            strerror(errno));
        sem_destroy(semaphore);
        shmdt(memory);
        shmctl(sharedMemoryId, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }

    *x = ntohl(*x);
    *y = ntohl(*y);

    *result = *x + *y;

    printf(
        "Server: %" PRIu32 " + %" PRIu32 " = %" PRIu32 "\n", *x, *y, *result);

    statusCode = sem_destroy(semaphore);

    if (statusCode == -1) {
        fprintf(
            stderr,
            "Server: failed to destroy semaphore: %s\n",
            strerror(errno));
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
