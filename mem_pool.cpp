#include "mem_pool.h"
#include <malloc.h>

#define GET_BLOCKSIZE(i) (size_t)(1 << ((i) + 1))

#define MAX_FIXED_BLOCK_SIZE (1 * M)

#define MIN_CACHE_SIZE (2097150)


MemPool::MemPool(size_t size) {
    size_t adjust_size = MIN_CACHE_SIZE;
    size_t initBlockNum = (size + MIN_CACHE_SIZE - 1) / MIN_CACHE_SIZE;
    if (size > MIN_CACHE_SIZE) {
        adjust_size = MIN_CACHE_SIZE * initBlockNum;
    }
    char* mem = (char*)malloc(adjust_size);  // 申请并缓存一大片内存，大小为 size 向上舍入到最近的 MIN_CHACHE_SIZE 整数倍
    memCount = adjust_size;

    // 初始化 freeList
    for (int i = 0; i < CHUNKNUM-1; i++) {
        freeList[i] = (Block*)malloc(initBlockNum * sizeof(Block));  // 为 freeList[i] 分配 initBlockNum 个 entry
        memCount += (initBlockNum * sizeof(Block));

        Block* oldHead = freeList[i];
        for (unsigned j = 0; j < initBlockNum; j++) {  // 把 entry 组织为双向链表
            Block* tmp = oldHead + j;
            tmp->payload = mem;
            tmp->blockSize = GET_BLOCKSIZE(i);
            tmp->dataSize = 0;
            tmp->prev = nullptr;
            tmp->next = nullptr;
            mem += GET_BLOCKSIZE(i);
        
            if (tmp != freeList[i]) {
                tmp->next = freeList[i];
                freeList[i]->prev = tmp;
                freeList[i] = tmp;
            }
        }
    }

    usedList = nullptr;
    usageCount = 0;
}

void* MemPool::Malloc(size_t size) {
    Block* tmp;

    usageCount += size;

    if (size > MAX_FIXED_BLOCK_SIZE) {  // 请求大于最大固定块，在 freeList[CHUNKNUM] 里查找
        tmp = freeList[CHUNKNUM-1];
        while (tmp != nullptr) {
            if (size <= tmp->blockSize) {  // 找到大小合适的块
                moveToUsedList(tmp, CHUNKNUM-1);
                tmp->dataSize = size;

                return (void*)tmp->payload;
            }

            tmp = tmp->next;
        }
        // 创建一个自由大小块
        tmp = (Block*)malloc(sizeof(Block));
        tmp->payload = (char*)malloc(size);
        tmp->blockSize = size;
        tmp->dataSize = size;
        tmp->prev = nullptr;
        tmp->next = nullptr;
        moveToUsedList(tmp, -1);
        memCount += (sizeof(Block) + size);

        return (void*)tmp->payload;
    }

    // 从固定空闲块里查找
    for (int i = 0; i < CHUNKNUM-1; i++) {
        if (size <= GET_BLOCKSIZE(i)) { // 最小的符合请求的空闲固定块
            tmp = freeList[i];

            // 当前大小的空闲固定块耗尽，尝试从更大的空闲固定块里窃取空间
            if (tmp == nullptr) {
                for (int j = CHUNKNUM-1; j > i; j--) {  // 向右查找最远的非空 freeList
                    if (freeList[j] != nullptr) { 
                        int newBlockNum = GET_BLOCKSIZE(j) / GET_BLOCKSIZE(i);  // 确定这次窃取可以为 freeList[i] 得到几个新的固定空闲块
                        Block* mem = (Block*)malloc(newBlockNum * sizeof(Block));
                        memCount += (newBlockNum * sizeof(Block));

                        // 头插法把新固定空闲块放到 freeList[i]
                        for (int k = 0; k < newBlockNum; k++) {
                            mem->blockSize = GET_BLOCKSIZE(i);
                            mem->dataSize = 0;
                            mem->payload = freeList[j]->payload + mem->blockSize * k;
                            mem->prev = nullptr;
                            mem->next = freeList[i];
                            if (freeList[i] == nullptr) {
                                freeList[i] = mem;
                            } else {
                                freeList[i]->prev = mem;
                                freeList[i] = mem;
                            }

                            mem += 1;
                        }

                        // 被窃取的 freeList[j] 释放首结点 
                        freeList[j] = freeList[j]->next;
                        if (freeList[j] != nullptr) {
                            freeList[j]->prev = nullptr;
                        }

                        // 现在 freeList[i] 非空了
                        tmp = freeList[i];
                        break;
                    }
                }
            }

            if (tmp != nullptr) {  
                moveToUsedList(tmp, i);
                tmp->dataSize = size;

                return (void*)tmp->payload;
            }
            


            // 创建一个新的固定块
            tmp = (Block*)malloc(sizeof(Block));
            tmp->payload = (char*)malloc(GET_BLOCKSIZE(i));
            tmp->blockSize = GET_BLOCKSIZE(i);
            tmp->dataSize = size;
            tmp->prev = nullptr;
            tmp->next = nullptr;
            moveToUsedList(tmp, -1);
            memCount += GET_BLOCKSIZE(i);

            return (void*)tmp->payload;
        }
    }

    return nullptr;
}

void MemPool::Free(void* ptr) {
    Block* tmp = usedList;
    while (tmp != nullptr) {
        Block* tmpNext = tmp->next;
        if (tmp->payload == ptr) {
            usageCount -= tmp->dataSize;
            moveToFreeList(tmp);
        }
        tmp = tmpNext;
    }
}

/*
* 把 block 移动到 usedList。如果 block 是 freeList[i] 的首节点，还要修改 freeList[i] 
**/
void MemPool::moveToUsedList(Block* block, int freeListIdx) {
    if (freeListIdx >= 0 && block == freeList[freeListIdx]) {
        freeList[freeListIdx] = freeList[freeListIdx]->next;
    }
    if (block->prev != nullptr) {
        block->prev->next = block->next;
    }
    if (block->next != nullptr) {
        block->next->prev = block->prev;
    }
    block->prev = nullptr;
    block->next = nullptr;

    if (usedList == nullptr) {
        usedList = block;
    } else {
        block->next = usedList;
        usedList->prev = block;
        usedList = block;
    }
}


/*
*  把 usedList 的 block 移动到 freeList
**/
void MemPool::moveToFreeList(Block* block) {
    if (block == usedList) {
        usedList = usedList->next;
    }
    if (block->prev != nullptr) {
        block->prev->next = block->next;
    }
    if (block->next != nullptr) {
        block->next->prev = block->prev;
    }
    block->prev = nullptr;
    block->next = nullptr;

    int i = CHUNKNUM-1;
    if (block->blockSize <= MAX_FIXED_BLOCK_SIZE) {
        i = 0;
        while (GET_BLOCKSIZE(i) != block->blockSize) {
            ++i;
        }
    }

    if (freeList[i] != nullptr) {
        block->next = freeList[i];
        freeList[i]->prev = block;
        freeList[i] = block;
    } else {
        freeList[i] = block;
    }
}

double MemPool::Usage() {
    return double(usageCount) / memCount;
}