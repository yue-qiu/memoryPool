## Introduce

这是一个基于分离适配法（sagregated fit）实现的内存池，暂不考虑并发的情况。初始化内存池时如果指定缓存参数小于 2097150 则会至少缓存 2097150 个字节，否则缓存大小为指定缓存参数向上舍入到最近的 2097150 的整数倍个字节。

一共有 21 个大小类（size class），范围分别为 (0, 2], (2, 4], (4, 8], (8, 16]...(512K, 1M], (1M, inf]。大小类内部以双向链表的形式组织。

内存池 `MemPool` 提供两个接口 `Malloc()` 与 `Free()`，表现与系统调用 `malloc()` 和 `free()` 一致。

提供一个测试实例 `test_mem_pool.cpp`，执行如下命令查看测试结果：

```shell
$ make
$ ./test_mem_pool -test=malloc  # 测试原生 malloc 与 free
$ ./test_mem_pool -test=pool    # 测试内存池的 Malloc 与 Free
```

预定义宏：`K`，`M`，`G`。

**你不该期望从内存缓存池里获取的地址一定是连续的！**

## BenchMark

定义`利用率(Usage) = 总请求字节数 / 总缓存字节数`表征池子里可用内存的数量。内存池以块（block）的形式组织，会产生一定的“内存浪费”，即不是所有被缓存起来的内存都可以用于响应请求，因此**利用率永远小于 1**。

创建一个初始容量为 2G 的内存池，与原生 malloc/free 比较：

| testcase | max request size | 内存池响应时间 | malloc/free 响应时间 | Usage |
|  ----  | ----  | ---- | ---- | ---- |
| 8k | 16k | 0.003988s | 0.016443s | 0.03 | 
| 16k | 16k | 0.009377s | 0.038889s | 0.06 |
| 32k | 16k | 0.023470s | 0.068538s | 0.12 |
| 8k | 512k | 0.010936s | 0.051649s | 0.73 |
| 16k | 512k | 0.038987s | 0.121583s | 0.74 |
| 32k | 512k | 0.110380s | 0.238870s | 0.75 |
| 8k | 1.5M | 0.018760s | 0.065249s | 0.84 |
| 16k | 1.5M | 0.054910s | 0.141231s | 0.86 |
| 32k | 1.5M | 0.110323s | 0.274101s | 0.87 |



## Todo

- ~~用红黑树保存非空闲块，提高 `Free()` 执行效率~~
- 保证线程安全


