#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "run_socket_server.h"

static bool receiveData(int clientSocket, uint32_t *dataRead);

int runSocketServer()
{
    /* UNIX domain sockets are used for interprocess communication (IPC).
     * They're not internet sockets.
     */

    /* The name (identifier) for our UNIX domain socket */
    static const char socketName[] = "myFancySocket";

    /* Subtract one, because we don't want to count the terminating '\0' (0x00)
     * byte */
    static const size_t socketNameLength = sizeof(socketName) - 1;

    /* Use a UNIX domain socket. */
    static const int unixDomain = AF_UNIX;

    /* Use a streaming socket (comparable to TCP) */
    static const int streamType = SOCK_STREAM;

    /* Use an unspecified default protocol appropriate for the socket type */
    static const int defaultProtocol = 0;

    printf("Server launched in socket mode.\n");

    /* Use sockaddr_un rather than sockaddr_in to create a UNIX domain socket.
     */
    struct sockaddr_un sockaddrUn;
    sockaddrUn.sun_family = unixDomain;
    assert(
        socketNameLength < (sizeof(sockaddrUn.sun_path) - 1)
        && "The socket name is waaay too long.");
    strcpy(sockaddrUn.sun_path, socketName);

    /* Try to create the socket */
    int serverSocket = socket(unixDomain, streamType, defaultProtocol);

    if (serverSocket == -1) {
        fprintf(
            stderr,
            "Server: could not create socket. Error: \"%s\"\n",
            strerror(errno));
        return EXIT_FAILURE;
    }

    /* Remove the socket in case it's already there */
    unlink(socketName);

    int statusCode = bind(
        serverSocket, (struct sockaddr *) &sockaddrUn, sizeof(sockaddrUn));

    if (statusCode == -1) {
        fprintf(
            stderr,
            "Server: couldn't bind socket! Socket name: \"%s\". Error: "
            "\"%s\"\n",
            socketName,
            strerror(errno));
        shutdown(serverSocket, SHUT_RDWR); /* SHUT_RDWR: Disables further send
                                              and receive operations. */
        close(serverSocket); /* Close the socket file descriptor */
        unlink(socketName);
        return EXIT_FAILURE;
    }

    static const int backlog = 1; /* How many clients can queue */
    statusCode               = listen(serverSocket, backlog);

    if (statusCode == -1) {
        fprintf(
            stderr,
            "Server: listen failed with a backlog of %d. Socket name: \"%s\". "
            "Error: \"%s\"\n",
            backlog,
            socketName,
            strerror(errno));
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        unlink(socketName);
        return EXIT_FAILURE;
    }

    struct sockaddr_un sockaddrUnClient;
    socklen_t          clientSockAddrLen = sizeof(sockaddrUnClient);
    int                clientSocket      = accept(
        serverSocket,
        (struct sockaddr *) &sockaddrUnClient,
        &clientSockAddrLen);

    if (clientSocket == -1) {
        fprintf(
            stderr,
            "Server: could not accept client socket! Socket name: \"%s\". "
            "Error: \"%s\"\n",
            socketName,
            strerror(errno));
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        unlink(socketName);
        return EXIT_FAILURE;
    }

    uint32_t x;

    if (!receiveData(clientSocket, &x)) {
        fprintf(stderr, "Server: couldn't read x\n");
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        unlink(socketName);
        return EXIT_FAILURE;
    }

    uint32_t y;

    if (!receiveData(clientSocket, &y)) {
        fprintf(stderr, "Server: couldn't read x\n");
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        unlink(socketName);
        return EXIT_FAILURE;
    }

    const uint32_t sum = x + y;
    printf("Server: %" PRIu32 " + %" PRIu32 " = %" PRIu32 ".\n", x, y, sum);

    statusCode = shutdown(clientSocket, SHUT_RDWR);

    if (statusCode == -1) {
        fprintf(
            stderr,
            "Server: could not shut down client socket: \"%s\"\n",
            strerror(errno));
        close(clientSocket);
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        unlink(socketName);
        return EXIT_FAILURE;
    }

    statusCode = close(clientSocket);

    if (statusCode == -1) {
        fprintf(
            stderr,
            "Server: could not close client socket: \"%s\"\n",
            strerror(errno));
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        unlink(socketName);
        return EXIT_FAILURE;
    }

    statusCode = shutdown(serverSocket, SHUT_RDWR);

    if (statusCode == -1) {
        fprintf(
            stderr,
            "Server: could not shut down server socket: \"%s\"\n",
            strerror(errno));
        close(serverSocket);
        unlink(socketName);
        return EXIT_FAILURE;
    }

    statusCode = close(serverSocket);

    if (statusCode == -1) {
        fprintf(
            stderr,
            "Server: could not close server socket: \"%s\"\n",
            strerror(errno));
        return EXIT_FAILURE;
    }

    statusCode = unlink(socketName);

    if (statusCode == -1) {
        fprintf(stderr, "Server: could not unlink \"%s\".\n", socketName);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static bool receiveData(int clientSocket, uint32_t *dataRead)
{
    /* MSG_WAITALL: Requests that the function block until the full amount of
     * data requested can be returned. The function may return a smaller amount
     * of data if a signal is caught, if the connection is terminated, if
     * MSG_PEEK was specified, or if an error is pending for the socket. */
    static const int flag = MSG_WAITALL;

    ssize_t bytesReceived
        = recv(clientSocket, dataRead, sizeof(*dataRead), flag);

    /* Error */
    if (bytesReceived == -1) {
        fprintf(
            stderr, "Server: recv failed. Error: \"%s\"\n", strerror(errno));
        return false;
    }
    /* Client shut down */
    else if (bytesReceived == 0) {
        fprintf(
            stderr, "Server: recv failed. Client shut down the connection.\n");
        return false;
    }
    /* Too few bytes */
    else if (bytesReceived != sizeof(*dataRead)) {
        fprintf(
            stderr,
            "Server: recv: too few bytes received. Received %zu but expected "
            "%zu.\n",
            (size_t) bytesReceived,
            sizeof(*dataRead));
        return false;
    }

    *dataRead = ntohl(*dataRead);
    return true;
}
