#pragma once

#include <unistd.h>

#define GB (1 << 30)
#define MB (1 << 20)
#define KB (1 << 10)

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
        void moveToUsedList(Block*, Block**);
        void moveToFreeList(Block*);

    public:
        MemPool(size_t);
        void* Malloc(size_t);
        void Free(void*);
        ~MemPool();
};


