#include <string.h>
#include <stdlib.h>

char *strcpy_until_char(char *dest, const char *src, char stop_char) {
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

void parse_url_params(HashTable *table, const char *url) {
    const char *query_start = strchr(url, '?');
    if (!query_start) {
        return;
    }
    query_start++;

    char *query = strdup(query_start);
    char *pair, *key, *value, *saveptr1, *saveptr2;

    for (pair = strtok_r(query, "&", &saveptr1); pair; pair = strtok_r(NULL, "&", &saveptr1)) {
        key = strtok_r(pair, "=", &saveptr2);
        value = strtok_r(NULL, "=", &saveptr2);

        if (key && value) {
            insert(table, key, strdup(value));
        }
    }

    free(query);
}
