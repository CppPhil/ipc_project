#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "named_pipe.h"
#include "run_socket_client.h"

int main(int argc, char *argv[])
{
    int exitCode = EXIT_SUCCESS;

    if (argc != 2) {
        fprintf(
            stderr,
            "client called with an incorrect count of command line arguments. "
            "expected: %d, actual: %d\n",
            2,
            argc);
        exitCode |= EXIT_FAILURE;
        return exitCode;
    }

    printf("client application launched.\n");

    if (strcmp(argv[1], "named_pipe") == 0) {
        exitCode |= namedPipe();
    }
    else if (strcmp(argv[1], "socket") == 0) {
        exitCode |= runSocketClient();
    }
    else if (strcmp(argv[1], "shared_memory") == 0)
    {
        exitCode |= sharedMemory();    
    }
    else {
        // TODO: Print usage.
        exitCode |= EXIT_FAILURE;
    }

    return exitCode;
}
