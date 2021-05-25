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

在利用率（Usage，总请求字节数 / 内存池占用字节数）为 0.39 的时候，内存池的响应速度是原生的 `60%`；利用率为 0.53 的时候，内存池的响应速度是原生的 `80%`；以后内存池表现不如原生。

**你不该期望从内存缓存池里获取的地址一定是连续的！**


