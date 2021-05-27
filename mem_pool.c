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

static Block* NewBlock(MemPool *mp, Block *prev, Block *next, char *payload, size_t blockSize, size_t dataSize) {
    Block *block;

    if (mp->oldBlockList != NULL) {
        block = mp->oldBlockList;
        mp->oldBlockList = mp->oldBlockList->next;
        if (mp->oldBlockList != NULL) {
            mp->oldBlockList->prev = NULL;
        }
    } else {
        block = (Block*)malloc(sizeof(Block));
    }

    block->prev = prev;
    block->next = next;
    block->payload = payload;
    block->dataSize = dataSize;
    block->blockSize = blockSize;

    return block;
}

static void initBlock(Block *block, Block *prev, Block *next, char *payload, size_t blockSize, size_t dataSize) {
    block->prev = prev;
    block->next = next;
    block->payload = payload;
    block->blockSize = blockSize;
    block->dataSize = dataSize;
}

static void moveToOldNodeList(MemPool *mp, Block *block) {
    if (mp->oldBlockList != NULL) {
        block->next = mp->oldBlockList;
        mp->oldBlockList->prev = block;
    }
    mp->oldBlockList = block;
}
 
static void moveToOldRBNodeList(MemPool *mp, Node *node) {
    if (mp->oldRBNodeList != NULL) {
        node->right = mp->oldRBNodeList;
        mp->oldRBNodeList->parent = node;
    }
    mp->oldRBNodeList = node;
}

MemPool* NewMemPool(size_t size) {
    unsigned long adjust_size = MIN_CACHE_SIZE;
    unsigned long initBlockNum = (size + MIN_CACHE_SIZE - 1) / MIN_CACHE_SIZE;
    if (size > MIN_CACHE_SIZE) {
        adjust_size = MIN_CACHE_SIZE * initBlockNum;
    }
    char* mem = (char*)malloc(adjust_size);  // 申请并缓存一大片内存，大小为 size 向上舍入到最近的 MIN_CHACHE_SIZE 整数倍
    
    MemPool *mp = (MemPool*)malloc(sizeof(MemPool));
    mp->memCount = adjust_size;
    mp->usedTree = create_rbtree();
    mp->usageCount = 0;
    mp->oldRBNodeList = NULL;
    mp->oldBlockList = NULL;

    // 初始化 freeList
    for (int i = 0; i < CHUNKNUM-1; i++) {
        for (unsigned long j = 0; j < initBlockNum; j++) {
            Block *block = NewBlock(mp, NULL, NULL, mem, GET_BLOCKSIZE(i), 0);
            if (mp->freeList[i] != NULL) {
                block->next = mp->freeList[i];
                mp->freeList[i]->prev = block;
            }
            mp->freeList[i] = block;
            mem += GET_BLOCKSIZE(i);
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
        
        // 得到一个自由大小块
        tmp = NewBlock(mp, NULL, NULL, (char*)malloc(size), size, size);
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

                        for (int k = 0; k < newBlockNum; k++) {
                            // 得到一个块大小为 GET_BLOCKSIZE(i) 的固定块
                            Block *block = NewBlock(mp, NULL, NULL, mp->freeList[j]->payload + k * GET_BLOCKSIZE(i), GET_BLOCKSIZE(i), 0);
                            // 插入到 freeList[i] 头部
                            if (mp->freeList[i] != NULL) {
                                block->next = mp->freeList[i];
                                mp->freeList[i]->prev = block;
                            } 
                            mp->freeList[i] = block;
                        }

                        // 被窃取的 freeList[j] 首结点放入 oldBlockList
                        Block *oldNode = mp->freeList[j];
                        mp->freeList[j] = mp->freeList[j]->next;
                        if (mp->freeList[j] != NULL) {
                            mp->freeList[j]->prev = NULL;
                        }
                        initBlock(oldNode, NULL, NULL, (char*)0, 0, 0);
                        moveToOldNodeList(mp, oldNode);

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

            // 得到一个固定块
            tmp = NewBlock(mp, NULL, NULL, (char*)malloc(GET_BLOCKSIZE(i)), GET_BLOCKSIZE(i), size);
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
        node->color = BLACK;
        node->key = NULL;
        node->value = NULL;
        node->parent = NULL;
        node->left = NULL;
        node->right = NULL;
        moveToOldRBNodeList(mp, node);
    }
}

/*
* 把 block 移动到 usedTree。如果 block 是 freeList[freeListIdx] 的首节点，还要修改 freeList[freeListIdx] 
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

    Node *node = NULL;
    if (mp->oldRBNodeList != NULL) {
        node = mp->oldRBNodeList;
        mp->oldRBNodeList = mp->oldRBNodeList->right;
        if (mp->oldRBNodeList != NULL) {
            mp->oldRBNodeList->parent = NULL;
        }
    }
    insert_rbtree(mp->usedTree, block->payload, block, node);
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
    }
    mp->freeList[i] = block;
}

double Usage(MemPool* mp) {
    return (double)mp->usageCount / mp->memCount;
}

void Destroy(MemPool *mp) {
    Block *block = mp->oldBlockList;
    Block *block_next = NULL;
    while (block != NULL) {
        block_next = block->next;
        free(block);
        block = block_next;
    }

    for (int i = 0; i < CHUNKNUM; i++) {
        block = mp->freeList[i];
        while (block != NULL) {
            block_next = block->next;
            free(block);
            block = block_next;    
        }      
    }

    Node *node = mp->oldRBNodeList;
    Node *node_next = NULL;
    while (node != NULL) {
        node_next = node->right;
        free(node);
        node = node_next;
    }
    
    destroy_rbtree(mp->usedTree);
    
    free(mp);
}