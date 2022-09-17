#include <string.h>
#include <sys/fcntl.h>
#include "proxy.h"
#include "list.h"


_args convert_args(char **argv) {
    _args args;
    args.receive_port = strtoul(argv[1], NULL, 10);
    args.address = inet_addr(argv[2]);
    args.send_port = strtoul(argv[3], NULL, 10);
    return args;
}

int get_connect_limit() {
    struct rlimit limit;
    getrlimit(RLIMIT_NOFILE, &limit);
    return ((int)limit.rlim_cur - 5) / 2;
}

int connect_to_server(int* socket_fd, _args args) {
    *socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (*socket_fd == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = args.send_port;
    address.sin_addr.s_addr = args.address;

    if (connect(*socket_fd, (struct sockaddr*)&address, sizeof(struct sockaddr_in)) == -1) {
        perror("connection error");
        return -1;
    }

    return 0;
}

void create_proxy_socket(int* socket_fd, _args args, int connect_limit) {
    *socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (*socket_fd == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in in;
    struct in_addr ad = {INADDR_ANY};
    in.sin_port = args.receive_port;
    in.sin_family = AF_INET;
    in.sin_addr = ad;

    if (bind(*socket_fd, (struct sockaddr *)&in, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(*socket_fd, connect_limit) == -1) {
        perror("listen");
        exit(1);
    }

    if (fcntl(*socket_fd, F_SETFL, O_NONBLOCK) == -1) {
        perror("fcntl");
        exit(1);
    }
}

void close_session(_endpoints* endpoints, fd_set *read_set, fd_set *write_set) {
    close(endpoints->client_socket);
    close(endpoints->server_socket);

    FD_CLR(endpoints->client_socket, read_set);
    FD_CLR(endpoints->server_socket, read_set);

    FD_CLR(endpoints->client_socket, write_set);
    FD_CLR(endpoints->server_socket, write_set);
}

void start_proxy_server(int connect_socket, int limit, _args args) {
    _list* list = create_list();
    if (!list) {
        perror("malloc");
        exit(1);
    }

    _endpoints* endpoints;

    size_t free_place;

    fd_set read_set, read_master, write_set, write_master;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_SET(connect_socket, &read_set);
    _endpoints *tmp;

    read_master = read_set;
    write_master = write_set;

    while (select(FD_SETSIZE, &read_master, &write_master, NULL, NULL) != -1) {
        if (FD_ISSET(connect_socket, &read_master)) {
            endpoints = create_node();
            if (!endpoints) {
                perror("malloc");
                exit(1);
            }

            endpoints->client_socket = accept(connect_socket, NULL, NULL);

            if (connect_to_server(&endpoints->server_socket, args) == -1) {
                perror("connect");
                close_session(endpoints, &read_set, &write_set);
                free(endpoints->client_buf);
                free(endpoints->server_buf);
                free(endpoints);
                continue;
            }

            FD_SET(endpoints->server_socket, &read_set);
            FD_SET(endpoints->client_socket, &read_set);

            add_to_list(list, endpoints);

            log_connect(endpoints, list);
        }

        endpoints = list->first;
        while (endpoints != NULL) {
            if (FD_ISSET(endpoints->server_socket, &read_master)) {
                free_place = MAX_SIZE - endpoints->server_len;

                if (free_place != 0 && !endpoints->server_closed) {
                    endpoints->server_read = read(endpoints->server_socket,
                                                  endpoints->server_buf + endpoints->server_len, free_place);

                    switch (endpoints->server_read) {
                        case -1:
                        case 0:
                            endpoints->server_closed = true;
                            fprintf(stderr, "server disconnected\n");
                            if (FD_ISSET(endpoints->server_socket, &write_set)) {
                                FD_CLR(endpoints->server_socket, &write_set);
                            }

                            if (endpoints->server_len != 0) {
                                FD_SET(endpoints->client_socket, &write_set);
                            }
                            break;

                        default:
                            endpoints->server_len += endpoints->server_read;
                            log_receive(endpoints, SERVER);
                            FD_SET(endpoints->client_socket, &write_set);
                            break;
                    }
                } else {
                    FD_CLR(endpoints->server_socket, &read_set);
                }
            }

            if (FD_ISSET(endpoints->client_socket, &write_master)) {
                free_place = MAX_SIZE - endpoints->server_len;

                if (endpoints->server_len > 0 && !endpoints->client_closed) {
                    endpoints->server_write = write(endpoints->client_socket,
                                                    endpoints->server_buf, endpoints->server_len);

                    switch (endpoints->server_write) {
                        case 0:
                        case -1:
                            FD_CLR(endpoints->client_socket, &write_set);
                            break;
                        default:
                            if (endpoints->server_write == endpoints->server_len) {
                                if (FD_ISSET(endpoints->client_socket, &write_set)) {
                                    FD_CLR(endpoints->client_socket, &write_set);
                                }
                            } else {
                                offset_to_begin(endpoints->server_buf,
                                                endpoints->server_write,
                                                endpoints->server_len - endpoints->server_write);
                            }

                            endpoints->server_len -= endpoints->server_write;

                            if (!FD_ISSET(endpoints->server_socket, &read_set)) {
                                FD_SET(endpoints->server_socket, &read_set);
                            }

                            log_send(endpoints, SERVER);
                            break;
                    }
                } else {
                    FD_CLR(endpoints->client_socket, &write_set);
                }
            }

            if (FD_ISSET(endpoints->client_socket, &read_master)) {
                free_place = MAX_SIZE - endpoints->client_len;

                if (free_place != 0 && !endpoints->client_closed) {
                    endpoints->client_read = read(endpoints->client_socket,
                                                  endpoints->client_buf + endpoints->client_len, free_place);

                    switch (endpoints->client_read) {
                        case -1:
                        case 0:
                            endpoints->client_closed = true;
                            fprintf(stderr, "client disconnected\n");
                            if (FD_ISSET(endpoints->client_socket, &write_set)) {
                                FD_CLR(endpoints->client_socket, &write_set);
                            }

                            FD_SET(endpoints->server_socket, &write_set);
                            break;

                        default:
                            endpoints->client_len += endpoints->client_read;
                            log_receive(endpoints, CLIENT);
                            FD_SET(endpoints->server_socket, &write_set);
                            break;
                    }
                } else {
                    FD_CLR(endpoints->client_socket, &read_set);
                }
            }

            if (FD_ISSET(endpoints->server_socket, &write_master)) {
                free_place = MAX_SIZE - endpoints->client_len;

                if (endpoints->client_len > 0 && !endpoints->server_closed) {
                    signal(SIGPIPE, SIG_IGN);
                    endpoints->client_write = write(endpoints->server_socket,
                                                    endpoints->client_buf, endpoints->client_len);
                    signal(SIGPIPE, SIG_DFL);

                    switch (endpoints->client_write) {
                        case 0:
                        case -1:
                            FD_CLR(endpoints->server_socket, &write_set);
                            break;
                        default:
                            if (endpoints->client_write == endpoints->client_len) {
                                if (FD_ISSET(endpoints->server_socket, &write_set)) {
                                    FD_CLR(endpoints->server_socket, &write_set);
                                }
                            } else {
                                offset_to_begin(endpoints->client_buf,
                                                endpoints->client_write,
                                                endpoints->client_len - endpoints->client_write);
                            }

                            endpoints->client_len -= endpoints->client_write;

                            if (!FD_ISSET(endpoints->client_socket, &read_set)) {
                                    FD_SET(endpoints->client_socket, &read_set);
                            }

                            log_send(endpoints, CLIENT);
                            break;
                    }
                } else {
                    FD_CLR(endpoints->server_socket, &write_set);
                }
            }

            if ((endpoints->server_closed && endpoints->server_len == 0) ||
                    (endpoints->client_closed && endpoints->client_len ==0) ||
                    (endpoints->server_closed && endpoints->client_closed)) {
                tmp = endpoints;
                endpoints = endpoints->next;

                delete_by_client(list, tmp->client_socket);
                close_session(tmp, &read_set, &write_set);

                read_master = read_set;
                write_master = write_set;

                log_disconnect(tmp, list);
                continue;
            }

            endpoints = endpoints->next;

            read_master = read_set;
            write_master = write_set;
        }
    }

    delete_list(list);
}

void offset_to_begin(char* buffer, size_t start_point, size_t len) {
    for (size_t i = start_point, j = 0; i < len; ++i, ++j) {
        buffer[j] = buffer[i];
    }
    
}