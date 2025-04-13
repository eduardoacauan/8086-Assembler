#include "asm.h"
#include <assert.h>

typedef struct CBlock CBlock;

struct CBlock {
    byte   *base;
    byte   *avail;
    byte   *limit;
    CBlock *next;
};

static CBlock *arena[4] = {NULL, NULL, NULL, NULL};

static void *MAlloc(size_t size)
{
    void *ptr;

    assert(size);

    ptr = malloc(size);

    assert(ptr);

    return ptr;
}

static void Free(void *ptr)
{
    if(!ptr)
        return;
    free(ptr);
}

static CBlock *NewBlock(size_t size)
{
    CBlock *tmpb;

    if(!size)
        return NULL;

    tmpb        = (CBlock*)MAlloc(sizeof(CBlock));
    tmpb->base  = (byte*)MAlloc(sizeof(byte) * size);
    tmpb->avail = tmpb->base;
    tmpb->limit = tmpb->avail + size;
    tmpb->next  = NULL;

    return tmpb;
}

void *Alloc(size_t size, size_t idx)
{
    CBlock *tmpb;
    size_t  alignment;

    tmpb      = arena[idx];
    alignment = (size + (sizeof(union align) - 1)) & ~(sizeof(union align) - 1);

    while(tmpb) {
        if(tmpb->avail + alignment <= tmpb->limit) {
            tmpb->avail += alignment;
            return tmpb->avail - alignment;
        }
        tmpb = tmpb->next;
    }

    size = (size > ARENA_SIZE) ? size : ARENA_SIZE;

    tmpb = NewBlock(size);

    tmpb->next = arena[idx];

    arena[idx] = tmpb;

    tmpb->avail += alignment;

    return tmpb->avail - alignment;
}

static void FreeArena(size_t idx)
{
    assert(idx >= ARENA_1 && idx <= ARENA_4);
    while(arena[idx]) {
        CBlock *tmpb = arena[idx]->next;
        Free(arena[idx]->base);
        Free(arena[idx]);
        arena[idx] = tmpb;
    }

    arena[idx] = NULL;
}

void AFree(void)
{
    FreeArena(ARENA_1);
    FreeArena(ARENA_2);
    FreeArena(ARENA_3);
    FreeArena(ARENA_4);
}
