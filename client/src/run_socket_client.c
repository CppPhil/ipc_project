#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "read_u32.h"
#include "run_socket_client.h"

static bool sendData(int sock, uint32_t dataToSend);

int runSocketClient()
{
    static const char socketName[] = "myFancySocket";

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sock == -1) {
        fprintf(
            stderr, "Client: socket failed. Error: \"%s\"\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    assert(
        (sizeof(socketName) - 1) < (sizeof(address.sun_path) - 1)
        && "The socket name is waaay too long.");
    strcpy(address.sun_path, socketName);

    int statusCode
        = connect(sock, (struct sockaddr *) &address, sizeof(address));

    if (statusCode == -1) {
        fprintf(
            stderr,
            "Client: failure to connect. Error: \"%s\"\n",
            strerror(errno));
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Client: established socket connection.\n");

    printf("Please enter x:");
    bool     ok;
    uint32_t x = readU32(&ok);

    if (!ok) {
        fprintf(stderr, "client: could not read x.\n");
        shutdown(sock, SHUT_RDWR);
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Please enter y:");
    uint32_t y = readU32(&ok);

    if (!ok) {
        fprintf(stderr, "client: could not read y.\n");
        shutdown(sock, SHUT_RDWR);
        close(sock);
        return EXIT_FAILURE;
    }

    x = htonl(x);
    y = htonl(y);

    if (!sendData(sock, x)) {
        fprintf(stderr, "Client: failure to send x\n");
        shutdown(sock, SHUT_RDWR);
        close(sock);
        return EXIT_FAILURE;
    }

    if (!sendData(sock, y)) {
        fprintf(stderr, "Client: failure to send y\n");
        shutdown(sock, SHUT_RDWR);
        close(sock);
        return EXIT_FAILURE;
    }

    statusCode = shutdown(sock, SHUT_RDWR);

    if (statusCode == -1) {
        fprintf(
            stderr,
            "Client: failure to shut down socket: \"%s\".\n",
            strerror(errno));
        close(sock);
        return EXIT_FAILURE;
    }

    statusCode = close(sock);

    if (statusCode == -1) {
        fprintf(
            stderr,
            "Client: Could not close socket. Error: \"%s\".\n",
            strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static bool sendData(int sock, uint32_t dataToSend)
{
    static int    noFlags = 0;
    const ssize_t bytesSent
        = send(sock, &dataToSend, sizeof(dataToSend), noFlags);

    if (bytesSent == -1) {
        fprintf(
            "Client: failure to send %" PRIX32 " (%" PRIX32
            " in host byte order). Error: \"%s\"\n",
            dataToSend,
            ntohl(dataToSend),
            strerror(errno));
        return false;
    }
    else if (bytesSent != (sizeof(dataToSend))) {
        fprintf(
            "Client: sent %zu bytes but it should've been %zu bytes.\n",
            (size_t) bytesSent,
            sizeof(dataToSend));
        return false;
    }

    return true;
}
