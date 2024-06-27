#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <mongoc/mongoc.h>
#include "map.h"
#include "routes.h"
#include "mongoc.h"
#include "util.h"
#include "webserver.h"

#define PORT 8080
#define BUFFER_SIZE 1024

bool continue_the_server = true;
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

int match_dynamic_route(const char *route_pattern, const char *actual_path, char **variable_value)
{
    // Find the position of the variable segment in the route pattern
    const char *variable_pos = strchr(route_pattern, ':');
    
    // If there is no variable segment, compare the entire route pattern with the actual path
    if (!variable_pos)
    {
        return strcmp(route_pattern, actual_path) == 0;
    }

    // Calculate the length of the static part of the route pattern (before the variable segment)
    size_t static_part_length = variable_pos - route_pattern;
    
    // Compare the static part of the route pattern with the corresponding part of the actual path
    if (strncmp(route_pattern, actual_path, static_part_length) != 0)
    {
        return 0;
    }

    // If the static parts match, extract the variable value from the actual path
    *variable_value = strdup(actual_path + static_part_length);
    
    // Return 1 to indicate a successful match
    return 1;
}


Entry *find_entry(HashTable *table, const char *key)
{
    unsigned int index = hash(key, table->capacity);
    Entry *entry = table->entries[index];
    while (entry != NULL && strcmp(entry->key, key) != 0)
    {
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
    continue_the_server = false;
    return "\"exiting the game\"";
}

route *handle_default(char *method, char *actual_path, char *path, HashTable *params)
{
    if (request_types == NULL)
    {
        return NULL;
    }

    HashTable *request_path_map = (HashTable *)search(request_types, method);
    if (request_path_map == NULL)
    {
        return NULL;
    }

    char *(*route_func)(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client) = (char *(*)(char *, HashTable *, mongoc_client_t *))search(request_path_map, actual_path);
    if (route_func == NULL)
    {
        return NULL;
    }

    return create_route(route_func(path, params, client), false, JSON);
}

bool the_server_should_stop()
{
    return continue_the_server;
}

int main()
{
    client = setup_mongoc("mongodb://localhost:27017", "wmc-c-backend");

    if (client == NULL) {
        return EXIT_FAILURE;
    }

    request_types = create_table(5);
    HashTable *get_requests = create_table(10);
    HashTable *put_requests = create_table(10);
    HashTable *patch_requests =  create_table(10);
    HashTable *post_requests = create_table(10);
    HashTable *delete_requests = create_table(10);

    insert(request_types, "GET", get_requests);
    insert(request_types, "PUT", put_requests);
    insert(request_types, "POST", post_requests);
    insert(request_types, "PATCH", patch_requests);
    insert(request_types, "DELETE", delete_requests);

    insert(get_requests, "/", &my_index);
    insert(get_requests, "/exit", &exit_the_server);
    insert(get_requests, "/michi", &michi);
    insert(get_requests, "/michi/", &michi);
    insert(get_requests, "/games", &get_gamer);
    insert(get_requests, "/games/", &get_gamer);

    int server_socket = setup_webserver(PORT, BUFFER_SIZE, &handle_default, &not_found_404, &internal_error_500, &the_server_should_stop);

    if (server_socket == -1) {
        fprintf(stderr, "Failed to create server.");
    } else {
        clean_up_webserver(server_socket);
    }

    free_table(get_requests);
    free_table(delete_requests);
    free_table(patch_requests);
    free_table(post_requests);
    free_table(put_requests);
    free_table(request_types);
    clean_up_mongoc(client);
    return 0;
}
