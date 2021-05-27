#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mem_pool.h"


#define random(x) (rand() % (x))

#define TESTCASESNUM (32 * K)

#define MAXREQSIZE ((3 * M) >> 1) 

#define PER_MILLISECOND 1000

// 洗牌算法打乱数组
void shuffle(void* mem[TESTCASESNUM]) {
    for (int i = TESTCASESNUM-1; i > 0; i--) {
        int randIdx = random(i);
        void* tmp = mem[randIdx];
        mem[randIdx] = mem[i];
        mem[i] = tmp;
    }
}

void test_pool() {
    MemPool* pool = NewMemPool(2 * G);
    void* mem[TESTCASESNUM];
    clock_t startTime, endTime;
    startTime = clock();  //计时开始

    for (size_t i = 0; i < TESTCASESNUM; i++) {
        mem[i] = Malloc(pool, random(MAXREQSIZE));
    }

    fprintf(stdout, "Usage: %.2f\n", Usage(pool));

    shuffle(mem); // 模拟随机释放内存
    for (size_t i = 0; i < TESTCASESNUM; i++) {
        Free(pool, mem[i]);
    }

    endTime = clock();  //计时结束
    fprintf(stdout, "The run time with malloc is: %.2f ms\n", (double)(endTime - startTime) / PER_MILLISECOND);  
    Destroy(pool);
}

void test_malloc() {
    void* mem[TESTCASESNUM];
    clock_t startTime, endTime;
    startTime = clock();  //计时开始

    for (size_t i = 0; i < TESTCASESNUM; i++) {
        mem[i] = malloc(random(MAXREQSIZE));
    }

    shuffle(mem);   // 模拟随机释放内存
    for (size_t i = 0; i < TESTCASESNUM; i++) {
        free(mem[i]);
    }

    endTime = clock();  //计时结束
    fprintf(stdout, "The run time with malloc is: %.2f ms\n", (double)(endTime - startTime) / PER_MILLISECOND);  
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "参数错误\n");
        return 0;
    }

    fprintf(stdout, "testcase: %lu, max request size: %lu Bytes\n", TESTCASESNUM, MAXREQSIZE);
    srand((int)time(0));

    if (strcmp(argv[1], "-test=pool") == 0) {
        test_pool();
    } else if (strcmp(argv[1], "-test=malloc") == 0) {
        test_malloc();
    }
    
    return 0;
}
