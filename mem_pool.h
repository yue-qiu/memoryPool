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
            size_t blockSize;  // 块大小
            size_t dataSize;  // 块保存的内容的大小
            char* payload;
        };
        Block* freeList[CHUNKNUM];
        Block* usedList;
        void moveToUsedList(Block*, int);
        void moveToFreeList(Block*);
        size_t memCount;  // 内存总分配量
        size_t usageCount;  // 实际内存使用量

    public:
        MemPool(size_t);
        void* Malloc(size_t);
        void Free(void*);
        double Usage();
        ~MemPool();
};


