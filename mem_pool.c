#include <malloc.h>
#include "mem_pool.h"
#include "rbTree.h"


#define GET_BLOCKSIZE(i) (size_t)(1 << ((i) + 1))

#define MAX_FIXED_BLOCK_SIZE (1 * M)
#define MIN_CACHE_SIZE (2097150)

MemPool* NewMemPool(size_t);
void* Malloc(MemPool*, size_t);
void Free(MemPool*, void* ptr);
void moveToUsedTree(MemPool*, Block*, int);
void moveToFreeList(MemPool*, Block*);
double Usage(MemPool*);

MemPool* NewMemPool(size_t size) {
    size_t adjust_size = MIN_CACHE_SIZE;
    size_t initBlockNum = (size + MIN_CACHE_SIZE - 1) / MIN_CACHE_SIZE;
    if (size > MIN_CACHE_SIZE) {
        adjust_size = MIN_CACHE_SIZE * initBlockNum;
    }
    char* mem = (char*)malloc(adjust_size);  // 申请并缓存一大片内存，大小为 size 向上舍入到最近的 MIN_CHACHE_SIZE 整数倍
    
    MemPool *mp = (MemPool*)malloc(sizeof(MemPool));
    mp->memCount = adjust_size;
    mp->usedTree = create_rbtree();
    mp->usageCount = 0;

    // 初始化 freeList
    for (int i = 0; i < CHUNKNUM-1; i++) {
        mp->freeList[i] = (Block*)malloc(initBlockNum * sizeof(Block));  // 为 freeList[i] 分配 initBlockNum 个 entry

        Block* oldHead = mp->freeList[i];
        for (unsigned long j = 0; j < initBlockNum; j++) {  // 把 entry 组织为双向链表
            Block* tmp = oldHead + j;
            tmp->payload = mem;
            tmp->blockSize = GET_BLOCKSIZE(i);
            tmp->dataSize = 0;
            tmp->prev = NULL;
            tmp->next = NULL;
            mem += GET_BLOCKSIZE(i);
        
            if (tmp != mp->freeList[i]) {
                tmp->next = mp->freeList[i];
                mp->freeList[i]->prev = tmp;
                mp->freeList[i] = tmp;
            }
        }
    }

    return mp;
}

void* Malloc(MemPool *mp, size_t size) {
    Block* tmp;

    mp->usageCount += size;

    if (size > MAX_FIXED_BLOCK_SIZE) {  // 请求大于最大固定块，在 freeList[CHUNKNUM] 里查找
        tmp = mp->freeList[CHUNKNUM-1];
        while (tmp != NULL) {
            if (size <= tmp->blockSize) {  // 找到大小合适的块
                moveToUsedTree(mp, tmp, CHUNKNUM-1);
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
        tmp->prev = NULL;
        tmp->next = NULL;
        moveToUsedTree(mp, tmp, -1);
        mp->memCount += tmp->blockSize;

        return (void*)tmp->payload;
    }

    // 从固定空闲块里查找
    for (int i = 0; i < CHUNKNUM-1; i++) {
        if (size <= GET_BLOCKSIZE(i)) { // 最小的符合请求的空闲固定块
            tmp = mp->freeList[i];

            // 当前大小的空闲固定块耗尽，尝试从更大的空闲固定块里窃取空间
            if (tmp == NULL) {
                for (int j = CHUNKNUM-2; j > i; j--) {  // 向右查找最远的非空 freeList
                    if (mp->freeList[j] != NULL) { 
                        int newBlockNum = GET_BLOCKSIZE(j) / GET_BLOCKSIZE(i);  // 确定这次窃取可以为 freeList[i] 得到几个新的固定空闲块
                        Block* mem = (Block*)malloc(newBlockNum * sizeof(Block));

                        // 头插法把新固定空闲块放到 freeList[i]
                        for (int k = 0; k < newBlockNum; k++) {
                            mem->blockSize = GET_BLOCKSIZE(i);
                            mem->dataSize = 0;
                            mem->payload = mp->freeList[j]->payload + mem->blockSize * k;
                            mem->prev = NULL;
                            mem->next = mp->freeList[i];
                            if (mp->freeList[i] == NULL) {
                                mp->freeList[i] = mem;
                            } else {
                                mp->freeList[i]->prev = mem;
                                mp->freeList[i] = mem;
                            }

                            mem += 1;
                        }

                        // 被窃取的 freeList[j] 释放首结点 
                        mp->freeList[j] = mp->freeList[j]->next;
                        if (mp->freeList[j] != NULL) {
                            mp->freeList[j]->prev = NULL;
                        }

                        // 现在 freeList[i] 非空了
                        tmp = mp->freeList[i];
                        break;
                    }
                }
            }

            if (tmp != NULL) {  
                moveToUsedTree(mp, tmp, i);
                tmp->dataSize = size;

                return (void*)tmp->payload;
            }
            


            // 创建一个新的固定块
            tmp = (Block*)malloc(sizeof(Block));
            tmp->payload = (char*)malloc(GET_BLOCKSIZE(i));
            tmp->blockSize = GET_BLOCKSIZE(i);
            tmp->dataSize = size;
            tmp->prev = NULL;
            tmp->next = NULL;
            moveToUsedTree(mp, tmp, -1);
            mp->memCount += tmp->blockSize;

            return (void*)tmp->payload;
        }
    }

    return NULL;
}

void Free(MemPool *mp, void* ptr) {
    Node* node =  delete_rbtree(mp->usedTree, ptr);
    if (node != NULL) {
        moveToFreeList(mp, (Block*)node->value);
        free(node);
    }
}

/*
* 把 block 移动到 usedList。如果 block 是 freeList[i] 的首节点，还要修改 freeList[i] 
**/
void moveToUsedTree(MemPool *mp, Block *block, int freeListIdx) {
    if (freeListIdx >= 0 && block == mp->freeList[freeListIdx]) {
        mp->freeList[freeListIdx] = mp->freeList[freeListIdx]->next;
    }
    if (block->prev != NULL) {
        block->prev->next = block->next;
    }
    if (block->next != NULL) {
        block->next->prev = block->prev;
    }
    block->prev = NULL;
    block->next = NULL;

    insert_rbtree(mp->usedTree, block->payload, block);
}


/*
*  把 block 移动到 freeList
**/
void moveToFreeList(MemPool *mp, Block *block) {
    int i = CHUNKNUM-1;
    if (block->blockSize <= MAX_FIXED_BLOCK_SIZE) {
        i = 0;
        while (GET_BLOCKSIZE(i) != block->blockSize) {
            ++i;
        }
    }

    if (mp->freeList[i] != NULL) {
        block->next = mp->freeList[i];
        mp->freeList[i]->prev = block;
        mp->freeList[i] = block;
    } else {
        mp->freeList[i] = block;
    }
}

double Usage(MemPool* mp) {
    return (double)mp->usageCount / mp->memCount;
}