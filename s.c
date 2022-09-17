#include <stdio.h>
#include <sys/resource.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

int main() {
    int sock_fd, new_sock_fd;
    char c;
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = 7001;
    server.sin_addr.s_addr = INADDR_ANY;
    static struct sigaction act;

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (bind(sock_fd, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sock_fd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    if ((new_sock_fd = accept(sock_fd, NULL, NULL)) == -1) {
        perror("accept");
    }
    char buff[100];

    fd_set set, master;
    FD_ZERO(&set);
    FD_SET(new_sock_fd, &set);

    int cnt;
    while (1) {
        cnt = read(new_sock_fd, buff, 1000);
        write(new_sock_fd, "1234567", 7);
    }

    close(new_sock_fd);

    return 0;
}