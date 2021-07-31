#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "named_pipe.h"
#include "run_socket_server.h"
#include "shared_memory.h"

static void printUsage()
{
    static const char appName[] = "./server";
    FILE *const       stream    = stderr;

    fprintf(stream, "usage: %s named_pipe|socket|shared_memory\n\n", appName);
    fprintf(stream, "Example:\n");
    fprintf(stream, "  %s named_pipe\n\n", appName);
}

int main(int argc, char *argv[])
{
    int exitCode = EXIT_SUCCESS;

    if (argc != 2) {
        fprintf(
            stderr,
            "server called with an incorrect count of command line arguments. "
            "expected: %d, actual: %d\n",
            2,
            argc);
        exitCode |= EXIT_FAILURE;
        printUsage();
        return exitCode;
    }

    printf("Server application launched.\n");

    if (strcmp(argv[1], "named_pipe") == 0) {
        exitCode |= namedPipe();
    }
    else if (strcmp(argv[1], "socket") == 0) {
        exitCode |= runSocketServer();
    }
    else if (strcmp(argv[1], "shared_memory") == 0) {
        exitCode |= sharedMemory();
    }
    else {
        printUsage();
        exitCode |= EXIT_FAILURE;
    }

    return exitCode;
}
