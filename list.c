#include "list.h"

_endpoints* create_node() {
    static int id = 0;

    _endpoints *node = (_endpoints*)malloc(sizeof(_endpoints));
    if (!node) {
        return NULL;
    }

    node->id = id;

    id++;
    if (id == 1001) {
        id = 0;
    }

    node->client_buf = (char *)malloc(MAX_SIZE);
    node->server_buf = (char *)malloc(MAX_SIZE);

    if(!node->client_buf || !node->server_buf) {
        return NULL;
    }

    node->client_write = 0;
    node->server_write = 0;

    node->client_read = 0;
    node->client_write = 0;

    node->server_len = 0;
    node->client_len = 0;

    return node;
}

_list* create_list() {
    _list *list = (_list*)malloc(sizeof(_list));

    if (!list) {
        return NULL;
    }

    list->first = NULL;
    list->last = NULL;

    return list;
}

void add_to_list(_list *list, _endpoints* endpoints) {
    if (list->last == NULL) {
        list->last = endpoints;
        list->first = endpoints;
    } else {
        list->last->next = endpoints;
    }

    list->last->next = NULL;
}

void delete_by_client(_list *list, int r_socket) {
    _endpoints *it = list->first;
    _endpoints *tmp;

    if (it->client_socket == r_socket) {
        if (list ->first == list->last) {
            list->last = NULL;
        }

        list->first = it->next;

        free(it);
        return;
    }

    while (it->next != NULL) {
        if (it->next->client_socket == r_socket) {
            tmp = it->next;
            it->next = it->next->next;
            free(tmp);
            return;
        }

        it = it->next;
    }
}

void delete_by_s(_list *list, int s_socket) {
    _endpoints *it = list->first;
    _endpoints *tmp;

    if (it->server_socket == s_socket) {
        tmp = it->next;
        it->next = it->next->next;
        free(tmp);
        return;
    }

    while (it->next != NULL) {
        if (it->next->server_socket == s_socket) {
            tmp = it->next;
            it->next = it->next->next;
            free(tmp);
            return;
        }

        it = it->next;
    }
}

_endpoints* find_by_r(_list *list, int r_socket) {
    _endpoints *it = list->first;
    _endpoints *tmp;

    if (it->client_socket == r_socket) {
        return it;
    }

    while (it->next != NULL) {
        if (it->next->client_socket == r_socket) {
            return it->next;
        }

        it = it->next;
    }

    return NULL;
}

bool is_empty(_list *list) {
    if (list->first == NULL) {
        return true;
    }

    return false;
}

void delete_list(_list *list) {
    _endpoints *it = list->first, *prev;

    while (it->next != NULL) {
        prev = it;
        it = it->next;
        free(prev->server_buf);
        free(prev->client_buf);
        free(prev);
    }

    free(it->server_buf);
    free(it->client_buf);

    free(it);
    free(list);
}

void print_list(_list *list) {
    _endpoints *it = list->first;

    while (it != NULL) {
        fprintf(stderr, "%d -> ", it->id);

        it = it->next;
    }
}

