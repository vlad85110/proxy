#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

#define _endpoints struct endpoints
#define _list struct list
#define MAX_SIZE 1024 * 100

_endpoints {
    int id;

    int client_socket;
    int server_socket;

    char *client_buf;
    char *server_buf;

    ssize_t client_write;
    ssize_t server_write;

    ssize_t client_read;
    ssize_t server_read;

    bool client_closed;
    bool server_closed;

    size_t client_len;
    size_t server_len;

    _endpoints* next;
};

struct list {
    _endpoints *first;
    _endpoints *last;
};

_endpoints* create_node();

_list* create_list();

void add_to_list(_list *list, _endpoints* endpoints);

void delete_by_client(_list *list, int r_socket);

bool is_empty(_list *list);

void delete_list(_list *list);

void print_list(_list *list);