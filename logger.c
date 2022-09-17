//
// Created by Влад Кирилов on 16.07.2022.
//

#include "logger.h"

static int cnt = 0;

void log_connect(_endpoints *endpoints, _list *list) {
    fprintf(stderr, "new connect with id %d\n", endpoints->id);
    print_list(list);
}

void log_disconnect(_endpoints *endpoints, _list *list) {
    fprintf(stderr, "disconnect with id %d\n", endpoints->id);
    print_list(list);
}

void print_buffer(size_t len, size_t max_len) {
    float fill_counts;

    fill_counts = (float)len / (float)max_len * 100;

    fprintf(stderr, "buffer is %.0f percents full ", fill_counts);

    char view[53];
    view[0] = '[';
    view[51] = ']';
    view[52] = '\0';

    for (int i = 1; i < 51; ++i) {
        if ((float)i <= fill_counts / 2) {
            view[i] = '=';
        } else {
            view[i] = ' ';
        }
    }

    fprintf(stderr, "%s\n", view);
}

void log_send(_endpoints *endpoints, int to) {
    ssize_t send = 0;
    size_t len = 0;

    cnt++;

    switch (to) {
        case CLIENT:
            fprintf(stderr, "%d.to server ", cnt);
            send = endpoints->client_write;
            len = endpoints->client_len;
            break;
        case SERVER:
            fprintf(stderr, "%d.to client ", cnt);
            send = endpoints->server_write;
            len = endpoints->server_len;
            break;
        default: break;
    }

    fprintf(stderr, "id %d %ld bytes ", endpoints->id, send);
    print_buffer(len, MAX_SIZE);
}

void log_receive(_endpoints *endpoints, int from) {
    ssize_t receive = 0;
    size_t len = 0;

    cnt++;

    switch (from) {
        case CLIENT:
            fprintf(stderr, "%d.from client ", cnt);
            receive = endpoints->client_read;
            len = endpoints->client_len;
            break;
        case SERVER:
            fprintf(stderr, "%d.from server ", cnt);
            receive = endpoints->server_read;
            len = endpoints->server_len;
            break;
        default: break;
    }

    fprintf(stderr, "id %d %ld bytes ", endpoints->id, receive);
    print_buffer(len, MAX_SIZE);
}