#ifndef ROUTE_H
#define ROUTE_H

#include <mongoc/mongoc.h>
#include "./map.h"

char *my_index(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client);
char *michi(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client);
char *get_gamer(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client);

#endif // ROUTE_H
