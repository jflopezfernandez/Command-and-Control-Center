
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>

#include <iostream>
#include <string>
#include <string_view>

#include <unistd.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <boost/program_options.hpp>

namespace Options = boost::program_options;

int main(int argc, char *argv[])
{
    Options::options_description generic_options("Generic");
    generic_options.add_options()
        ("help", "Display this help menu and exit")
        ("version", "Display program version information")
    ;

    std::string port_number = "8080";

    Options::options_description runtime_configuration("Runtime Configuration");
    runtime_configuration.add_options()
        ("port", Options::value<std::string>(&port_number)->default_value("8080"), "The port the server should listen for connections on")
    ;
    
    Options::options_description program_options("Program Options");
    program_options
        .add(runtime_configuration)
        .add(generic_options)
    ;

    Options::variables_map variables_map;
    Options::store(Options::parse_command_line(argc, argv, program_options), variables_map);
    Options::notify(variables_map);

    if (variables_map.count("help")) {
        std::cout << program_options << std::endl;
        return EXIT_SUCCESS;
    }

    if (variables_map.count("version")) {
        std::cout << "<Program Version Information>" << std::endl;
        return EXIT_SUCCESS;
    }

    struct addrinfo hints;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* bind_address = NULL;
    getaddrinfo(0, port_number.c_str(), &hints, &bind_address);

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
