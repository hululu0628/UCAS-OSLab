#include <pipe.h>
#include <string.h>
#define MAXWORD 10
#define MAXLENGTH 50
int main()
{
	char buf[MAXWORD][MAXLENGTH];	//测试，最多10个单词
	int ch;
	int i = 0, j = 0;
	int status = 0;
	int max;
	int temp;
	int array[MAXWORD] = {0,1,2,3,4,5,6,7,8,9};
	while(1)
	{
		ch = bp_getchar();
		if((ch != -1) && (ch != '\n') && (ch != '\r'))
		{
			status = 0;
			buf[i][j++] = ch;
			//bios_putchar(ch);		//debug
		}
		else
		{
			if(status)
				break;
			else if(ch == '\r')
			{
				//bios_putchar('\n');	//debug
				//bios_putchar('\r');	//debug
				status = 1;
				buf[i][j] = '\0';
				i++;
				j = 0;
			}
		}
	}
	max = i;
	for(i=max-1;i>0;i--)
	{
		for(j=0;j<i;j++)
		{
			if(strcmp(buf[array[j]],buf[array[j+1]]) > 0)
			{
				temp = array[j];
				array[j] = array[j+1];
				array[j+1] = temp;
			}
		}
	}
	for(i=0;i<max;i++)
	{
		bp_putstr(buf[array[i]]);
		bp_putchar('\n');
		bp_putchar('\r');
	}

	return 0;
}