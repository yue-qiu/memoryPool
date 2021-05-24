#include "mem_pool.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cstring>

#define random(x) (rand() % (x))

#define TESTCASE (2 * K)

#define MAXDATASIZE (2 * M) 

using namespace std;

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
    MemPool* pool = new MemPool(2 * G);
    void* mem[TESTCASE];
    clock_t startTime, endTime;
    startTime = clock();  //计时开始

    for (size_t i = 0; i < TESTCASE; i++) {
        mem[i] = pool->Malloc(random(MAXDATASIZE));
    }

    
    shuffle(mem); // 模拟随机释放内存
    for (size_t i = 0; i < TESTCASE; i++) {
        pool->Free(mem[i]);
    }

    endTime = clock();  //计时结束
    cout << "The run time with pool is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
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
    cout << "The run time with malloc is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;  
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "参数错误\n");
        return 0;
    }

    fprintf(stdout, "testcase: %ld, max data size: %ld Bytes\n", TESTCASE, MAXDATASIZE);
    srand((int)time(0));

    if (strcmp(argv[1], "-test=pool") == 0) {
        test_pool();
    } else if (strcmp(argv[1], "-test=malloc") == 0) {
        test_malloc();
    }
    
    return 0;
}
