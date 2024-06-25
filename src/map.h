#ifndef CMAP_LIBRARY_H
#define CMAP_LIBRARY_H

#define LOAD_FACTOR 0.75

typedef struct Entry
{
    char *key;
    void *value;
    struct Entry *next;
} Entry;

typedef struct
{
    Entry **entries;
    int size;
    int capacity;
} HashTable;

HashTable *create_table(int capacity);
unsigned int hash(const char *key, int capacity);
Entry *create_entry(const char *key, void *value);
void insert(HashTable *table, const char *key, void *value);
void *search(const HashTable *table, const char *key);
void delete(HashTable *table, const char *key);
void resize(HashTable *table);
void free_table(HashTable *table);

#endif // CMAP_LIBRARY_H
