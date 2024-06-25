#include <mongoc/mongoc.h>

mongoc_client_t *setup_mongoc() {
    mongoc_init();

    const char *uri_string = "mongodb://localhost:27017";
    bson_error_t error;
    mongoc_uri_t *uri = mongoc_uri_new _with_error(uri_string, &error);

    if (!uri) {
        fprintf (stderr,
                 "failed to parse URI: %s\n"
                 "error message:       %s\n",
                 uri_string,
                 error.message);
        return NULL;
    }

    mongoc_client_t *client = mongoc_client_new_from_uri (uri);
    if (!client) {
        return NULL;
    }

    mongoc_client_set_appname (client, "wmc-c-backend");
    database = mongoc_client_get_database (client, "db_name");

    return client;
}
