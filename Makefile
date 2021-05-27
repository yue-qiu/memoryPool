CXX=gcc
CFLAGS= -Wall -O2
TARGET=test_mem_pool

$(TARGET): test_mem_pool.c mem_pool.c mem_pool.h rbTree.c rbTree.h
	$(CXX) $(CFLAGS) test_mem_pool.c mem_pool.c rbTree.c -o $(TARGET)

thread_safe:
	$(CXX) $(CFLAGS) test_mem_pool.c mem_pool.c rbTree.c -D_THREAD_SAFE_ -lpthread -o $(TARGET)

clean:
	rm -rf $(TARGET)
