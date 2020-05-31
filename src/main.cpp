
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <iostream>

#include <unistd.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

int main(void)
{
    struct addrinfo hints;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* bind_address = NULL;
    getaddrinfo(0, "8080", &hints, &bind_address);

    int listener_socket = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

    if (listener_socket == -1) {
        fprintf(stderr, "socket() failed\n");
        return EXIT_FAILURE;
    }

    int option_reuse_socket_address = 1;

    if (setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR, (void *) &option_reuse_socket_address, sizeof (option_reuse_socket_address)) == -1) {
        fprintf(stderr, "setsockopt() failed\n");
        return EXIT_FAILURE;
    }

    if (bind(listener_socket, bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed\n");
        return EXIT_FAILURE;
    }

    freeaddrinfo(bind_address);

    if (listen(listener_socket, 10) == -1) {
        fprintf(stderr, "listen() failed\n");
        return EXIT_FAILURE;
    }

    while (true) {
        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof (client_address);
        int client_socket = accept(listener_socket, (struct sockaddr *) &client_address, &client_len);

        if (client_socket == -1) {
            fprintf(stderr, "accept() failed\n");
            return EXIT_FAILURE;
        }

        char request_buffer[BUFSIZ];
        memset(request_buffer, 0, BUFSIZ);

        int bytes_received = recv(client_socket, request_buffer, 1024, 0);
        printf("Received %d bytes.\n", bytes_received);

        /* Log request to the console */
        printf("%.*s\n", bytes_received, request_buffer);

        const char* response =
            "HTTP/1.1 200 OK\r\n"
            "Connection: close\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<p>Welcome to the server.</p>\r\n";

        int bytes_sent = send(client_socket, response, strlen(response), 0);

        close(client_socket);
    }

    close(listener_socket);

    return EXIT_SUCCESS;
}
