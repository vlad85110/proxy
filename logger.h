//
// Created by Влад Кирилов on 16.07.2022.
//

#include "list.h"
#include <stdio.h>

#pragma once

#define CLIENT 0
#define SERVER 1

void log_connect(_endpoints *endpoints, _list *list);

void log_disconnect(_endpoints *endpoints, _list *list);

void print_buffer(size_t len, size_t max_len);

void log_send(_endpoints *endpoints, int to);

void log_receive(_endpoints *endpoints, int from);
