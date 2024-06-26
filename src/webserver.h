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

int setup_webserver(int port, int buffer_size, char *(*path_func)(char *method, char *actual_path, char *path, HashTable *params), void (*notfound404)(int client_socket), void (*internalError500)(int client_socket), bool (*stop_server)());
void clean_up_webserver(int server_socket);

#endif //WMCCBACKEND_WEBSERVER_H
