#include <errno.h>
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

#include "read_u32.h"
#include "shared_memory.h"

int sharedMemory()
{
    printf("client application is in shared_memory mode.\n");

    static const int projectId = 1;
    key_t            key       = ftok("server", projectId);

    if (key == -1) {
        fprintf(stderr, "Client: ftok failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    static const size_t sharedMemorySize
        = (3 * sizeof(uint32_t)) + sizeof(sem_t);

    const int sharedMemoryId = shmget(key, sharedMemorySize, 0666);

    if (sharedMemoryId == -1) {
        fprintf(stderr, "Client: shmget failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    void *memory = shmat(sharedMemoryId, NULL, 0);

    if (memory == (void *) -1) {
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

    uint32_t *px        = memory;
    uint32_t *py        = px + 1;
    uint32_t *presult   = py + 1;
    sem_t *   semaphore = (sem_t *) (presult + 1);

    *px = x;
    *py = y;

    if (sem_post(semaphore) == -1) {
        fprintf(stderr, "Client: sem_post failed: %s\n", strerror(errno));
        shmdt(memory);
        return EXIT_FAILURE;
    }

    if (shmdt(memory) == -1) {
        fprintf(
            stderr,
            "Client could not detach from the shared memory segment: %s\n",
            strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
