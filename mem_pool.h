#pragma once

#include <unistd.h>

#define G (size_t)(1 << 30)
#define M (size_t)(1 << 20)
#define K (size_t)(1 << 10)

static const int CHUNKNUM = 21;

class MemPool {
    private:
        struct Block {
            Block* prev;
            Block* next;
            size_t size;
            char* payload;
            // bool alloc;
        };
        Block* freeList[CHUNKNUM];
        Block* usedList;
        void moveToUsedList(Block*, int);
        void moveToFreeList(Block*);

    public:
        MemPool(size_t);
        void* Malloc(size_t);
        void Free(void*);
        ~MemPool();
};


