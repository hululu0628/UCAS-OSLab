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
	
	int max = i;
	for(i=0;i<max;i++)
	{
		//测试程序最多只有10个单词，所以假装这里有一个itoa
		if(i==9)
			bp_putstr("10: ");
		else  
		{
			bp_putchar(i+'1');
			bp_putchar(':');
			bp_putchar(' ');
		}

		bp_putstr(buf[i]);
		bp_putchar('\n');
		bp_putchar('\r');
	}
	bp_putstr("Xianan Zhu\n\r");
	
	return 0;
}