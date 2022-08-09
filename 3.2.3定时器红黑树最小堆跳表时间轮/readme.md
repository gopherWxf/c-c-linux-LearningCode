

### 编译

#### 最小堆

```shell
# 关联文件 mh-timer.c mh-timer.h minheap.h minheap.c
gcc mh-timer.c minheap.c -o mh -I./
```

#### 红黑树

```shell
# 关联文件 rbt-timer.c rbt-timer.h rbtree.c rbtree.h
gcc rbt-timer.c rbtree.c -o rbt -I./
```

#### 跳表

```shell
# 关联文件 skiplist.h skiplist.c skl-timer.c
gcc skiplist.c skl-timer.c -o skl -I./
```

#### 单层级时间轮

```shell
gcc single_tw.c -o stw
```

#### 多层级时间轮

```shell
# 关联文件 timewheel.h timewheel.c tw-timer.c spinlock.h
gcc timewheel.c tw-timer.c -o tw -I./ -lpthread
```

