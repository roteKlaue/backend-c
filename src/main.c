#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <mongoc/mongoc.h>
#include "./map.h"
#include "./routes.h"
#include "./mongoc.h"
#include "./util.h"
#include "./webserver.h"

#define PORT 8080
#define BUFFER_SIZE 1024

bool exitMe = true;

HashTable *request_types = NULL;
mongoc_client_t *client = NULL;

void not_found_404(int client_socket)
{
    const char *response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 3\r\n"
        "\r\n"
        "404";
    send(client_socket, response, (int)strlen(response), 0);
    close(client_socket);
}

int match_dynamic_route(const char *route_pattern, const char *actual_path, char **variable_value) {
    const char *variable_pos = strchr(route_pattern, ':');
    if (!variable_pos) {
        return strcmp(route_pattern, actual_path) == 0;
    }

    size_t static_part_length = variable_pos - route_pattern;
    if (strncmp(route_pattern, actual_path, static_part_length) != 0) {
        return 0;
    }

    *variable_value = strdup(actual_path + static_part_length);
    return 1;
}

Entry *find_entry(HashTable *table, const char *key) {
    unsigned int index = hash(key, table->capacity);
    Entry *entry = table->entries[index];
    while (entry != NULL && strcmp(entry->key, key) != 0) {
        entry = entry->next;
    }
    return entry;
}

void internal_error_500(int client_socket)
{
    const char *response =
        "HTTP/1.1 500 Internal Server Error\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 3\r\n"
        "\r\n"
        "500";
    send(client_socket, response, (int)strlen(response), 0);
    close(client_socket);
}

char *exit_the_server(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client)
{
    exitMe = false;
    return "\"exiting the game\"";
}

char *handle_default(char *method, char *actual_path, char *path, HashTable *params)
{
    if (request_types == NULL) {
        return NULL;
    }

    HashTable *request_path_map = (HashTable *)search(request_types, method);
    if (request_path_map == NULL) {
        return NULL;
    }

    char *(*path_func)(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client) = (char *(*)(char *, HashTable *, mongoc_client_t *))search(request_path_map, actual_path);
    if (path_func == NULL) {
        return NULL;
    }

    return path_func(path, params, client);
}

bool the_server_should_stop() {
    return exitMe;
}

int main(int argc, char *argv[])
{
    client = setup_mongoc("mongodb://localhost:27017", "wmc-c-backend");

    if (client == NULL) {
        return EXIT_FAILURE;
    }

    request_types = create_table(5);
    HashTable *getRequests = create_table(10);
    HashTable *putRequests = create_table(10);
    HashTable *patchRequests =  create_table(10);
    HashTable *postRequests = create_table(10);
    HashTable *deleteRequests = create_table(10);

    insert(request_types, "GET", getRequests);
    insert(request_types, "PUT", putRequests);
    insert(request_types, "POST", postRequests);
    insert(request_types, "PATCH", patchRequests);
    insert(request_types, "DELETE", deleteRequests);

    insert(getRequests, "/", &my_index);
    insert(getRequests, "/exit", &exit_the_server);
    insert(getRequests, "/michi", &michi);
    insert(getRequests, "/michi/", &michi);
    insert(getRequests, "/games", &get_gamer);
    insert(getRequests, "/games/", &get_gamer);

    int server_socket = setup_webserver(PORT, BUFFER_SIZE, &handle_default, &not_found_404, &internal_error_500, &the_server_should_stop);

    free_table(getRequests);
    free_table(deleteRequests);
    free_table(patchRequests);
    free_table(postRequests);
    free_table(putRequests);
    free_table(request_types);
    clean_up_webserver(server_socket);
    return 0;
}
