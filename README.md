## Introduce

这是一个基于分离适配法（sagregated fit）实现的内存池，暂不考虑并发的情况。初始化时如果指定缓存字节数小于 2097150 则会至少缓存 2097150 个字节，否则缓存大小为初始化参数向上舍入到最近的 2097150 整数倍个字节。

一共有 21 个大小类（size class），范围分别为 (0, 2], (2, 4], (4, 8], (8, 16]...(512K, 1M], (1M, inf]。大小类内部以双向链表的形式组织。如果总请求字节量超过了初始容量，内存池会自动扩容。

`MemPool` 提供两个接口 `Malloc()` 与 `Free()`，表现与系统调用 `malloc()` 和 `free()` 一致。

提供一个测试实例 `test_mem_pool.cpp`，执行如下命令查看测试结果：

```shell
$ make
$ ./test_mem_pool -test=malloc  # 测试原生 malloc 与 free
$ ./test_mem_pool -test=pool    # 测试内存池的 Malloc 与 Free
```

预定义宏：`K`，`M`，`G`。

**你不该期望从内存池里获取的地址一定是连续的！**

## BenchMark

定义`利用率(Usage) = 总请求字节数 / 总缓存字节数`表征池子里可用内存的数量。内存池以块（block）的形式组织，会产生一定的“内存浪费”，即不是所有被缓存起来的内存都可以用于响应请求，因此**利用率永远小于 1**。

创建一个初始容量为 2G 的内存池，与原生 malloc/free 比较：

| Number of testcases | max request size | 内存池响应时间(ms) | malloc/free 响应时间(ms) | Usage |
|  ----  | ----  | ---- | ---- | ---- |
| 8k | 16k | 3.73 | 15.00 | 0.03 | 
| 32k | 16k | 28.89 | 84.22 | 0.12 |
| 64k | 16k | 64.78 | 239.75 | 0.25 |
| 8k | 512k | 11.76 | 66.22 | 0.73 |
| 32k | 512k | 136.50 | 271.14 | 0.75 |
| 64k | 512k | 382.90 | 660.68 | 0.75 |
| 8k | 1.5M |  22.91 | 82.55 | 0.84 |
| 32k | 1.5M | 148.39 | 321.98 | 0.87 |

## 接口列表

```c
MemPool* NewMemPool(size_t size);  // 创建一个初始容量为 size 的内存池
void* Malloc(MemPool *mp, size_t size);  // 向内存池请求 size 个字节的内存
void Free(MemPool *mp, void* ptr);  // 释放内存 
double Usage(MemPool *mp);  // 查看利用率
void Destroy(MemPool *mp);  // 销毁内存池
```

## Todo

- ~~用红黑树保存非空闲块，提高 `Free()` 执行效率~~
- ~~提供内存池销毁方法~~
- 保证线程安全


