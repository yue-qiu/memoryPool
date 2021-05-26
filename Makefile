CXX=gcc
CFLAGS= -Wall -O2
TARGET=test_mem_pool

$(TARGET): test_mem_pool.c mem_pool.c mem_pool.h
	$(CXX) $(CFLAGS) test_mem_pool.c mem_pool.c -o $(TARGET)

clean:
	rm -rf $(TARGET)
