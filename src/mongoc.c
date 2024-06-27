#include <mongoc/mongoc.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

mongoc_client_t *setup_mongoc(const char *uri_string, const char *application_name)
{
    mongoc_init();

    bson_error_t error;
    mongoc_uri_t *uri = mongoc_uri_new_with_error(uri_string, &error);

    if (!uri) {
        fprintf (stderr,
                 "failed to parse URI: %s\n"
                 "error message:       %s\n",
                 uri_string,
                 error.message);
        return NULL;
    }

    mongoc_client_t *client = mongoc_client_new_from_uri(uri);
    if (!client) {
        return NULL;
    }

    mongoc_client_set_appname(client, application_name);

    mongoc_uri_destroy (uri);
    return client;
}

char *get_games(mongoc_client_t *client, const char *db_name, const char *collection_name)
{
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    bson_t query;
    size_t json_len = 2;
    char *json_result = malloc(json_len);
    if (!json_result) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }
    strcpy(json_result, "[");
    bool first = true;

    collection = mongoc_client_get_collection(client, db_name, collection_name);
    if (!collection) {
        fprintf(stderr, "Failed to get collection.\n");
        free(json_result);
        return NULL;
    }
    bson_init(&query);

    cursor = mongoc_collection_find_with_opts(collection, &query, NULL, NULL);
    if (!cursor) {
        fprintf(stderr, "Failed to create cursor.\n");
        bson_destroy(&query);
        mongoc_collection_destroy(collection);
        free(json_result);
        return NULL;
    }

    while (mongoc_cursor_next(cursor, &doc)) {
        char *str = bson_as_canonical_extended_json(doc, NULL);
        size_t str_len = strlen(str);

        json_result = realloc(json_result, json_len + str_len + (first ? 0 : 1));
        if (!json_result) {
            fprintf(stderr, "Memory reallocation failed.\n");
            bson_free(str);
            mongoc_cursor_destroy(cursor);
            mongoc_collection_destroy(collection);
            return NULL;
        }

        if (!first) {
            strcat(json_result, ",");
        } else {
            first = false;
        }

        strcat(json_result, str);
        json_len += str_len + 1;

        bson_free(str);
    }

    bson_error_t error;
    if (mongoc_cursor_error(cursor, &error)) {
        fprintf(stderr, "Cursor error: %s\n", error.message);
    }

    strcat(json_result, "]");

    bson_destroy(&query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);

    return json_result;
}

char *get_game(mongoc_client_t *client, const char *db_name, const char *collection_name, int id)
{
    mongoc_collection_t *collection = mongoc_client_get_collection(client, db_name, collection_name);
    char *json_str = NULL;
    const bson_t *doc;

    if (!collection) {
        fprintf(stderr, "Failed to get collection\n");
        mongoc_client_destroy(client);
        mongoc_cleanup();
        return NULL;
    }

    bson_t *query = bson_new();
    BSON_APPEND_INT32(query, "id", id);

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);

    if (mongoc_cursor_next(cursor, &doc)) {
        json_str = bson_as_canonical_extended_json(doc, NULL);
    } else {
        fprintf(stderr, "No document found with id: %d\n", id);
    }

    char *json_str_copy = NULL;
    if (json_str != NULL) {
        json_str_copy = malloc(strlen(json_str) + 1);
        if (json_str_copy == NULL) {
            fprintf(stderr, "Failed to allocate memory for JSON string copy\n");
            bson_free(json_str);
            bson_destroy(query);
            mongoc_cursor_destroy(cursor);
            mongoc_collection_destroy(collection);
            return NULL;
        }
        strcpy(json_str_copy, json_str);
    }

    bson_free(json_str);
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);

    return json_str_copy;
}

void clean_up_mongoc(mongoc_client_t *client)
{
    mongoc_client_destroy(client);
    mongoc_cleanup();
}
