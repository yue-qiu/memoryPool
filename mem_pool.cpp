#include "mem_pool.h"
#include <malloc.h>

#define GET_BLOCKSIZE(i) (size_t)(1 << ((i) + 1))

#define MAX_FIXED_BLOCK_SIZE (1 * MB)

#define MIN_CACHE_SIZE 2097150


MemPool::MemPool(size_t size) {
    size_t adjust_size = MIN_CACHE_SIZE;
    int initBlockNum = (size + MIN_CACHE_SIZE - 1) / MIN_CACHE_SIZE;
    if (size > MIN_CACHE_SIZE) {
        adjust_size = MIN_CACHE_SIZE * initBlockNum;
    }
    char* mem = (char*)malloc(adjust_size);  // 申请并缓存一大片内存，大小为 size 向上舍入到最近的 MIN_CHACHE_SIZE 整数倍

    // 初始化 freeList
    for (int i = 0; i < CHUNKNUM-1; i++) {
        this->freeList[i] = (Block*)malloc(initBlockNum * sizeof(Block));  // 为 freeList[i] 分配 initBlockNum 个 entry
        Block* oldHead = this->freeList[i];
        for (int j = 0; j < initBlockNum; j++) {  // 把 entry 组织为双向链表
            Block* tmp = oldHead + j;
            tmp->payload = mem;
            tmp->size = GET_BLOCKSIZE(i);
            tmp->prev = NULL;
            tmp->next = NULL;
            mem += GET_BLOCKSIZE(i);
        
            if (tmp != this->freeList[i]) {
                tmp->next = this->freeList[i];
                this->freeList[i]->prev = tmp;
                this->freeList[i] = tmp;
            }
        }
    }

    this->usedList = NULL;
}

void* MemPool::Malloc(size_t size) {
    Block* tmp;
    if (size > MAX_FIXED_BLOCK_SIZE) {  // 请求大于最大固定块，在 freeList[CHUNKNUM] 里查找
        tmp = this->freeList[CHUNKNUM-1];
        while (tmp != NULL) {
            if (size <= tmp->size) {  // 找到大小合适的块
                this->moveToUsedList(tmp, &this->freeList[CHUNKNUM-1]);
                return (void*)tmp->payload;
            }

            tmp = tmp->next;
        }
        // freeList[CHUNKNUM] 找不到合适的空闲块，创建一个
        tmp = (Block*)malloc(sizeof(Block));
        tmp->payload = (char*)malloc(size);
        tmp->size = size;
        tmp->prev = NULL;
        tmp->next = NULL;
        moveToUsedList(tmp, NULL);

        return (void*)tmp->payload;
    }

    // 从固定空闲块里查找
    for (int i = 0; i < CHUNKNUM-1; i++) {
        if (size <= GET_BLOCKSIZE(i)) {
            tmp = this->freeList[i];
            while (tmp != NULL) {  // 找到最小的符合请求的固定块
                this->moveToUsedList(tmp, &this->freeList[i]);
                return (void*)tmp->payload;
            }
            // 找不到合适的空闲块，创建一个
            tmp = (Block*)malloc(sizeof(Block));
            tmp->payload = (char*)malloc(GET_BLOCKSIZE(i));
            tmp->size = GET_BLOCKSIZE(i);
            tmp->prev = NULL;
            tmp->next = NULL;
            moveToUsedList(tmp, NULL);

            return (void*)tmp->payload;
        }
    }

    return NULL;
}

void MemPool::Free(void* ptr) {
    Block* tmp = this->usedList;
    while (tmp != NULL) {
        if (tmp->payload == ptr) {
            moveToFreeList(tmp);
        }

        tmp = tmp->next;
    }
}

/*
* 把 block 移动到 usedList。如果 block 是 freeList[i] 的首节点，还要修改 freeList[i] 
**/
void MemPool::moveToUsedList(Block* block, Block** freeListPtr) {
    if (freeListPtr != NULL && block == *freeListPtr) {
        *freeListPtr = (*freeListPtr)->next;
    }
    if (block->prev != NULL) {
        block->prev->next = block->next;
    }
    if (block->next != NULL) {
        block->next->prev = block->prev;
    }
    block->prev = NULL;
    block->next = NULL;

    if (this->usedList == NULL) {
        this->usedList = block;
    } else {
        block->next = this->usedList;
        this->usedList->prev = block;
        this->usedList = block;
    }
}


/*
*  把 usedList 的 block 移动到 freeList
**/
void MemPool::moveToFreeList(Block* block) {
    if (block == this->usedList) {
        this->usedList = NULL;
    }
    if (block->prev != NULL) {
        block->prev->next = block->next;
    }
    if (block->next != NULL) {
        block->next->prev = block->prev;
    }
    block->prev = NULL;
    block->next = NULL;

    for (int i = 0; i < CHUNKNUM; i++) {
        if (i < CHUNKNUM-1) {
            if (block->size <= GET_BLOCKSIZE(i)) {
                if (this->freeList[i] == NULL) {
                    this->freeList[i] = block;
                } else {
                    block->next = this->freeList[i];
                    this->freeList[i]->prev = block;
                    this->freeList[i] = block;
                }
            }
        } else {
            if (this->freeList[i] == NULL) {
                this->freeList[i] = block;
            } else {
                block->next = this->freeList[i];
                this->freeList[i]->prev = block;
                this->freeList[i] = block;
            }
        }
    }
}