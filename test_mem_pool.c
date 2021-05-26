#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mem_pool.h"


#define random(x) (rand() % (x))

#define TESTCASE (8 * K)

#define MAXDATASIZE (512 * K) 

// 洗牌算法打乱数组
void shuffle(void* mem[TESTCASE]) {
    for (int i = TESTCASE-1; i > 0; i--) {
        int randIdx = random(i);
        void* tmp = mem[randIdx];
        mem[randIdx] = mem[i];
        mem[i] = tmp;
    }
}

void test_pool() {
    MemPool* pool = NewMemPool(2 * G);
    void* mem[TESTCASE];
    clock_t startTime, endTime;
    startTime = clock();  //计时开始

    for (size_t i = 0; i < TESTCASE; i++) {
        mem[i] = Malloc(pool, random(MAXDATASIZE));
    }

    fprintf(stdout, "Usage: %.2f\n", Usage(pool));

    shuffle(mem); // 模拟随机释放内存
    for (size_t i = 0; i < TESTCASE; i++) {
        Free(pool, mem[i]);
    }

    endTime = clock();  //计时结束
    fprintf(stdout, "The run time with malloc is: %fs\n", (double)(endTime - startTime) / CLOCKS_PER_SEC);  
}

void test_malloc() {
    void* mem[TESTCASE];
    clock_t startTime, endTime;
    startTime = clock();  //计时开始

    for (size_t i = 0; i < TESTCASE; i++) {
        mem[i] = malloc(random(MAXDATASIZE));
    }

    shuffle(mem);   // 模拟随机释放内存
    for (size_t i = 0; i < TESTCASE; i++) {
        free(mem[i]);
    }

    endTime = clock();  //计时结束
    fprintf(stdout, "The run time with malloc is: %fs\n", (double)(endTime - startTime) / CLOCKS_PER_SEC);  
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "参数错误\n");
        return 0;
    }

    fprintf(stdout, "testcase: %ld, max request size: %ld Bytes\n", TESTCASE, MAXDATASIZE);
    srand((int)time(0));

    if (strcmp(argv[1], "-test=pool") == 0) {
        test_pool();
    } else if (strcmp(argv[1], "-test=malloc") == 0) {
        test_malloc();
    }
    
    return 0;
}
