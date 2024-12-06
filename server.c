#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>


int setup_socket_server() {
    int opt = 1;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(socket_fd,
                   SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)
        ) == -1) {
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd,
            (struct sockaddr*)&server_addr,
            sizeof(server_addr)
        ) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(socket_fd, 5) < 0) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    return socket_fd;
}

int main() {
    int socket_fd = setup_socket_server();

    for (int i = 0; i < 5; i++) {
        int client_fd;
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        if ((client_fd = accept(socket_fd,
                        (struct sockaddr*) &client_addr,
                        &client_addr_len
            )) < 0) {
            perror("accept error");
            exit(EXIT_FAILURE);
        }
        
        if (getsockname(socket_fd,
                        (struct sockaddr*) &client_addr,
                        &client_addr_len
            ) < 0) {
            perror("getsockname error");
            exit(EXIT_FAILURE);
        }

        char ip_addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), ip_addr, INET_ADDRSTRLEN);
        printf("New connection from: %s\n", ip_addr);

        // receiving data
        char buffer[8192] = { 0 };
        if (read(client_fd, buffer, 8192) < 0) {
            perror("read error");
        }

        printf("%s\n", buffer);

        const char* response_buffer =
            "HTTP/1.1 200 OK\r\n"
            "Server: Simple-Server\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 14\r\n"
            "Connection: close\r\n"
            "\r\n"
            "{\"data\":\"ok\"}\n";

        if (send(client_fd, response_buffer, strlen(response_buffer), 0) < 0) {
            perror("send error");
        }

        // closing the socket.
        close(client_fd);
    }

    close(socket_fd);

    return 0;
}

