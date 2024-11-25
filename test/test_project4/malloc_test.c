#include <mm.h>
#include <assert.h>
#include <unistd.h>
// 感谢GPT-4o开源
int main(void)
{
	sys_move_cursor(0, 0);
	printf("=== Testing malloc and free ===\n");

	// 1. 测试基本分配和释放
	int *ptr = (int *)malloc(sizeof(int) * 10); // 分配10个整数的空间
	if (ptr == 0) {
		printf("Error: malloc failed to allocate memory!\n");
		assert(0);
	}
	printf("Basic allocation: Success\n");

	// 初始化分配的内存
	for (int i = 0; i < 10; i++) {
		ptr[i] = i * 10; // 写入数据
	}

	// 验证数据是否正确
	int success = 1;
	for (int i = 0; i < 10; i++) {
		if (ptr[i] != i * 10) {
		printf("Error: Memory corruption detected at index %d!\n", i);
		success = 0;
		break;
		}
	}
	if (success) {
		printf("Memory write and read: Success\n");
	}

	// 释放内存
	free(ptr);
	printf("Free operation: Success\n");
	printf("============= End =============\n");

	return 0;
}