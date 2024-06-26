#include <mongoc/mongoc.h>
#include "./map.h"

char *my_index(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client)
{
    return "Hello, World";
}

char *michi(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client)
{
    return "Fuer Fortnite";
}

char *get_gamer(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client)
{
    return get_games(mongo_client, "wmc", "games");
}