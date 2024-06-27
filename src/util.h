#ifndef CUTIL_LIBRARY_H
#define CUTIL_LIBRARY_H

#include "map.h"

char *strcpy_until_char(char *dest, const char *src, char stop_char);
void parse_url_params(HashTable *table, const char *url);

#endif // CUTIL_LIBRARY_H
