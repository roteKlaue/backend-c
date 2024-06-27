#include "routes.h"

char *my_index(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client)
{
    return "\"Hello, World\"";
}

char *michi(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client)
{
    return "\"Fuer Fortnite\"";
}

char *get_gamer(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client)
{
    return get_games(mongo_client, "wmc", "games");
}

char *get_game_by_id(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client)
{

}

route_f *create_route_f(route *route, char *(*funcy)(char *path_arg, HashTable *query_params, mongoc_client_t *mongo_client))
{
    route_f *route1 = (route_f *) malloc(sizeof(route_f));
    route1->route = route;
    route1->funcy = funcy;
    return route1;
}
