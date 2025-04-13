#include "asm.h"
#include <memory.h>
typedef struct CEntry CEntry;

struct CHashTable {
    size_t      item_count;
    struct CEntry {
        const char   *key;
        void         *item;
        CEntry       *next;
    }*buckets[256];
};

void NewTable(CHashTable **ht)
{
    if(!ht)
        return;

    *ht = (CHashTable*)Alloc(sizeof(CHashTable), ARENA_1);

    memset(*ht, 0, sizeof(CHashTable));
}

void Insert(CHashTable *ht, const char *key, void *item)
{
    CEntry  *entry;
    unsigned hash;

    if(!ht || !key)
        return;

    hash = ((unsigned)key << 1) & 0xFF;

    entry = (CEntry*)Alloc(sizeof(CEntry), ARENA_1);

    entry->key  = key;
    entry->item = item;
    entry->next = ht->buckets[hash];

    ht->buckets[hash] = entry;

    ht->item_count++;
}

void *Get(CHashTable *ht, const char *key)
{
    unsigned hash;
    CEntry  *entry;

    if(!ht || !key)
        return NULL;

    hash = ((unsigned)key << 1) & 0xFF;

    for(entry = ht->buckets[hash]; entry; entry = entry->next){
        if(entry->key != key)
            continue;
        return entry->item;
    }

    return NULL;
}
