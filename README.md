# UCAS 2024-2025秋季学期操作系统研讨课

### 实验项目一
- task1已完成
- task2已完成
- task3已完成
- task4已完成
- task5已完成
----

### 实验项目二

- task1已完成
- task2已完成
- task3已完成
- task4已完成
- task5已完成
----

### 实验项目三

task1：
1. 新的内存分配算法
2. 滚屏	// how?
3. history

task2：
1. 目前IPC所有资源的处理都是不受保护的，在实现双核的时候需要通过粗粒度或者细粒度锁进行保护。

task3:
1. 采用全局调度队列，只有两个核，认为队列更新的消耗不是不可接受的
2. 从核初始化tp，stvec

task4:
1. 未进行各种命令的参数规范性检查

task5:
不需要保护的数据结构：
main.c: tasks, 初始化发生在从核未被启动时，双核之后tasks只读
current_running, 应该保证每个核持有的current_running指向的数据不同，这样对两个核pcb的修改可以同步进行

需要保护的数据结构：
pcbs；同步原语；进程调度相关队列；new_screen；old_screen；可分配内存空间