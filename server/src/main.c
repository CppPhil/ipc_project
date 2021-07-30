#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// TODO: add program name to output.
int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "server called with an incorrect count of command line arguments. expected: %d, actual: %d\n", 2, argc);
        return EXIT_FAILURE;
    }

    const char pipeName[] = "IPC_NAMED_PIPE";

    if (strcmp(argv[1], "named_pipe") == 0) {
        int statusCode = mkfifo(pipeName, 0666);

        if (statusCode == -1) {
            fprintf(stderr, "Could not create pipe with name \"%s\".\n", pipeName);
            return EXIT_FAILURE;
        }

        const int fileDescriptor
            = open(pipeName, O_RDONLY);

        if (fileDescriptor == -1) {
            fprintf(stderr, "Could not open \"%s\"\n", pipeName);
            unlink(pipeName);
            return EXIT_FAILURE;
        }

        uint32_t x;
        uint32_t y;

        ssize_t bytesRead
            = read(fileDescriptor, &x, sizeof(x));

        if (bytesRead != sizeof(x)) {
            fprintf(stderr, "Could not read x.\n");
            close(fileDescriptor);
            unlink(pipeName);
            return EXIT_FAILURE;
        }

        bytesRead
            = read(fileDescriptor, &y, sizeof(y));

        if (bytesRead != sizeof(y)) {
            fprintf(stderr, "Could not read y.\n");
            close(fileDescriptor);
            unlink(pipeName);
            return EXIT_FAILURE;
        }

        x = ntohl(x);
        y = ntohl(y);

        const uint32_t result = x + y;
        printf("server: %" PRIu32 " + %" PRIu32 " = %" PRIu32 "\n", x, y, result);

        statusCode = close(fileDescriptor);

        if (statusCode == -1) {
            fprintf(stderr, "Could not close file description of \"%s\"\n", pipeName);
            unlink(pipeName);
            return EXIT_FAILURE;
        }

        statusCode = unlink(pipeName);

        if (statusCode == -1) {
            fprintf(stderr, "Could not unlink \"%s\"\n", pipeName);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}