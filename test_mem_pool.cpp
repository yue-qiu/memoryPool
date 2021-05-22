#include "mem_pool.h"
#include <iostream>

using namespace std;

typedef struct Person {
    char name[20];
    int age;
}Person;

int main(void) {
    MemPool* pool = new MemPool(1 * GB);

    cout << "sizeof Person: 0x" << hex << sizeof(Person) << endl;

    Person* people[10];

    for (int i = 0; i < 10; i++) {
        people[i] = (Person*)pool->Malloc(sizeof(Person));
        cout << "people[" << i << "]'s addr in pool:" <<  people[i] << endl;
    }

    for (int i = 0; i < 10; i++) {
        pool->Free(people[i]);
    }
}