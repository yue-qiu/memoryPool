#pragma once

#include <unistd.h>
#include "rbTree.h"

#define G (size_t)(1 << 30)
#define M (size_t)(1 << 20)
#define K (size_t)(1 << 10)

#define CHUNKNUM 21

typedef struct Block {
    struct Block* prev;
    struct Block* next;
    char* payload;
    size_t blockSize;
    size_t dataSize;
}Block;

typedef struct MemPool {
    Block *freeList[CHUNKNUM];
    RBRoot *usedTree;
    size_t memCount;  // 内存总分配量
    size_t usageCount;  // 实际内存使用量
    Node *oldRBNodeList;  // 以 node->right 为 next 指针，node->parent 为 prev 指针
    Block *oldBlockList;
}MemPool;

MemPool* NewMemPool(size_t size);
void* Malloc(MemPool *mp, size_t size);
void Free(MemPool *mp, void* ptr);
double Usage(MemPool *mp);
void Destroy(MemPool *mp);


