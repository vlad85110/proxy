#pragma once

#include <stdio.h>
#include <sys/resource.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#include "logger.h"

#define _args struct args

struct args {
    in_addr_t address;
    size_t receive_port;
    size_t send_port;
};

_args convert_args(char **argv);

int get_connect_limit();

int connect_to_server(int* socket_fd, _args args);

void create_proxy_socket(int* socket_fd, _args args, int connect_limit);

void start_proxy_server(int connect_socket, int limit, _args args);

void offset_to_begin(char* buffer, size_t start_point, size_t len);


