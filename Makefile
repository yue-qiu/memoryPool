CXX=g++
CFLAGS= -Wall -O2
TARGET=test_mem_pool

$(TARGET): test_mem_pool.cpp mem_pool.cpp mem_pool.h
	$(CXX) $(CFLAGS) test_mem_pool.cpp mem_pool.cpp -o $(TARGET)

clean:
	rm -rf $(TARGET)
