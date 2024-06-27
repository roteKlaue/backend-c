//
// Created by xxrot on 26.06.2024.
//

#ifndef WMCCBACKEND_WEBSERVER_H
#define WMCCBACKEND_WEBSERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "map.h"
#include "util.h"

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "Ws2_32.lib")
    typedef int socklen_t;
    #define close closesocket
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

enum ContentType {
    JSON,
    TEXT,
    HTML,
    XML,
    CSS,
    JS
};

typedef struct route {
    bool clean_up;
    enum ContentType content_type;
    char *content;
} route;

int setup_webserver(int port, int buffer_size, route *(*path_func)(char *method, char *actual_path, char *path, HashTable *params), void (*not_found_404)(int client_socket), void (*internal_error_500)(int client_socket), bool (*stop_server)());
void clean_up_webserver(int server_socket);
route *create_route(char *content, bool clean_up, enum ContentType content_type);
void free_route(route *route);

#endif //WMCCBACKEND_WEBSERVER_H
