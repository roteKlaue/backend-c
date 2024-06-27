#include <string.h>
#include <stdlib.h>
#include "map.h"

char *strcpy_until_char(char *dest, const char *src, char stop_char)
{
    int i = 0;
    while (src[i] != '\0') {
        if (src[i] == stop_char) {
            break;
        }
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

void parse_url_params(HashTable *table, const char *url)
{
    const char *query_start = strchr(url, '?');
    if (!query_start) {
        return;
    }
    query_start++;

    char *query = strdup(query_start);
    char *pair, *key, *value;

    pair = strtok(query, "&");
    while (pair != NULL) {
        key = strtok(pair, "=");
        value = strtok(NULL, "=");

        if (key && value) {
            insert(table, key, strdup(value));
        }
        pair = strtok(NULL, "&");
    }

    free(query);
}
