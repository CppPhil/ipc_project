#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "named_pipe.h"
#include "read_u32.h"

int namedPipe()
{
    static const char pipeName[] = "IPC_NAMED_PIPE";

    printf("client application is in named_pipe mode.\n");
    bool retry;

    do {
        retry = false;
        struct stat buf;
        int         statReturnCode = stat(pipeName, &buf);

        if (statReturnCode == -1) { // On failure
            if (errno == ENOENT) {  // If it's not there -> retry.
                retry = true;
                usleep(100000);
            }
            else {
                // Otherwise it's some other error.
                fprintf(
                    stderr,
                    "client: Failure to wait for named pipe (\"%s\"), "
                    "error: %s\n",
                    pipeName,
                    strerror(errno));
                return EXIT_FAILURE;
            }
        }
        else { // Success.
            if (!S_ISFIFO(buf.st_mode)) {
                // If it is not a fifo -> that's an error.
                fprintf(
                    stderr,
                    "client: \"%s\" exists but is not of type FIFO.\n",
                    pipeName);
                return EXIT_FAILURE;
            }
        }
    } while (retry);

    printf("client: waited for named pipe \"%s\" to exist.\n", pipeName);

    const int fileDescriptor = open(pipeName, O_WRONLY);

    if (fileDescriptor == -1) {
        fprintf(stderr, "client: could not open \"%s\"\n", pipeName);
        return EXIT_FAILURE;
    }

    printf("Please enter x:");
    bool     ok;
    uint32_t x = readU32(&ok);

    if (!ok) {
        fprintf(stderr, "client: could not read x.\n");
        close(fileDescriptor);
        return EXIT_FAILURE;
    }

    printf("Please enter y:");
    uint32_t y = readU32(&ok);

    if (!ok) {
        fprintf(stderr, "client: could not read y.\n");
        close(fileDescriptor);
        return EXIT_FAILURE;
    }

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
        fprintf(
            stderr,
            "client: could not close file descriptor for \"%s\"\n",
            pipeName);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
