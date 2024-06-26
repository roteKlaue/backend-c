#include <mongoc/mongoc.h>

mongoc_client_t *setup_mongoc(const char *uri_string, const char *application_name);
char *get_games(mongoc_client_t *client, const char *db_name, const char *collection_name);
void clean_up_mongoc(mongoc_client_t *client);
