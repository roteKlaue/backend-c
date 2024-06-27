#ifndef ROUTE_H
#define ROUTE_H

#include <mongoc/mongoc.h>
#include "./webserver.h"
#include "./map.h"

typedef struct route_f {
    route *route;
    char *(*funcy)(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client);
} route_f;

char *my_index(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client);
char *michi(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client);
char *get_gamer(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client);
char *get_game_by_id(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client);
route_f *create_route_f(route *route,
    char *(*funcy)(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client));

#endif // ROUTE_H
