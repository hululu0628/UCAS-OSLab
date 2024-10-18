#include <unistd.h>
#include <string.h>

int buf_idx,read_idx;
char buffer[256];

int getchar(void)
{
	int c;
	if(buffer[buf_idx] == '\n' || buffer[buf_idx] == '\r')
	{
		if(read_idx < buf_idx)
		{
			read_idx++;
			return buffer[read_idx - 1];
		}
		else if(read_idx == buf_idx)
		{
			c = read_idx;
			read_idx = 0;
			buf_idx = 0;
			return buffer[c];
		}
		else if(read_idx > buf_idx)
		{
			sys_write("\n\033[31mError:\033[0m read_index > buffer_index\n");
			return -1;
		}
	}
	else
	{
		do
		{
			if(buf_idx < 255)
			{
				while((c = sys_getchar()) == -1);
				if(c == '\b' || c == 127)
				{
					if(buf_idx > 0)
					{
						buf_idx--;
						buffer[buf_idx] = 0;
						sys_putchar(c);
					}
				}
				else
				{
					buffer[buf_idx] = c;
					sys_putchar(c);
					buf_idx++;
					buffer[buf_idx] = 0;
					if(c == '\n' || c == '\r')
					{
						buf_idx--;
						break;
					}
				}
			}
			else
			{
				sys_write("\n\033[31mError:\033[0m Buffer Overflow!\n");
				buf_idx = 0;
				read_idx = 0;
				return -1;
			}
		} while(1);
		
		read_idx++;
		return buffer[read_idx - 1];
	}
	return -1;
}

// 不保证处理数组溢出
int gets(char *str)
{
	int c;
	int i = 0;
	while((c=getchar()) != '\n' && c != '\r')
	{
		str[i++] = c;
	}
	str[i] = 0;
	return i;
}