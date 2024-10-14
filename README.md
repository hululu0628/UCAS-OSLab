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

为了完成p2的C-core，在`kernel`文件夹中新增了`workload.c`，在`include/os`文件夹中增加了对应的头文件`workload.h`。

C-core的具体调度算法以注释的形式在`workload.c`中描述，同时为了完成C-core，对于do_scheduler进行了专门的更改。

当前版本回退到了A-core，区别在于恢复到了A-core的do_scheduler()。