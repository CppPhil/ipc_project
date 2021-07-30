#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(
            stderr,
            "client called with an incorrect count of command line arguments. "
            "expected: %d, actual: %d\n",
            2,
            argc);
        return EXIT_FAILURE;
    }

    const char pipeName[] = "IPC_NAMED_PIPE";

    
    if (strcmp(argv[1], "named_pipe") == 0) {
        const int fileDescriptor = open(pipeName, O_WRONLY);

        if (fileDescriptor == -1) {
            fprintf(stderr, "client: could not open \"%s\"\n", pipeName);
            return EXIT_FAILURE;
        }        

        printf("Please enter x and y:");
        fflush(stdout);
        uint32_t x;
        uint32_t y;
        scanf("%" PRIu32 " %" PRIu32 "\n", &x, &y); // TODO: Replace this with something safer.

        x = htonl(x);
        y = htonl(y);

        ssize_t bytesWritten = write(fileDescriptor, &x, sizeof(x));

        if (bytesWritten != sizeof(x)) {
            fprintf(stderr, "client: could not write x\n");
            close(fileDescriptor);
            return EXIT_FAILURE;
        }

        bytesWritten = write(fileDescriptor, &y, sizeof(y));

        if (bytesWritten != sizeof(y)) {
            fprintf(stderr, "client: could not write y\n");
            close(fileDescriptor);
            return EXIT_FAILURE;
        }

        if (close(fileDescriptor) == -1) {
            fprintf(stderr, "client: could not close file descriptor for \"%s\"\n", pipeName);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
