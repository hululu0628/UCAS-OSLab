#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char **argv)
{
	int a = atoi(argv[1]);
	int b = atoi(argv[2]);
	printf("sum = %d\n",a+b);
	while(1);
	sys_exit();
}