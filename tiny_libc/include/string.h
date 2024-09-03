#ifndef INCLUDE_STRING_H_
#define INCLUDE_STRING_H_


void memcpy(unsigned char *dest, const unsigned char *src, unsigned len);
void memset(void *dest, unsigned char val, unsigned len);
void bzero(void *dest, unsigned len);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, unsigned n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, int n);
char *strcat(char *dest, const char *src);
int strlen(const char *src);


void memcpy(unsigned char *dest, const unsigned char *src, unsigned len)
{
    for (; len != 0; len--) {
        *dest++ = *src++;
    }
}

void memset(void *dest, unsigned char val, unsigned len)
{
    unsigned char *dst = (unsigned char *)dest;

    for (; len != 0; len--) {
        *dst++ = val;
    }
}

void bzero(void *dest, unsigned len)
{
    memset(dest, 0, len);
}

int strlen(const char *src)
{
    int i = 0;
    while (src[i] != '\0') {
        i++;
    }
    return i;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return (*str1) - (*str2);
        }
        ++str1;
        ++str2;
    }
    return (*str1) - (*str2);
}

int strncmp(const char *str1, const char *str2, unsigned n)
{
    for (unsigned i = 0; i < n; ++i)
        if (str1[i] == '\0' || str1[i] != str2[i])
            return str1[i] - str2[i];
    return 0;
}


char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;

    while (*src) {
        *dest++ = *src++;
    }

    *dest = '\0';

    return tmp;
}

char *strncpy(char *dest, const char *src, int n)
{
    char *tmp = dest;

    while (*src && n-- > 0) {
        *dest++ = *src++;
    }

    while (n-- > 0) {
        *dest++ = '\0';
    }

    return tmp;
}

char *strcat(char *dest, const char *src)
{
    char *tmp = dest;

    while (*dest != '\0') {
        dest++;
    }
    while (*src) {
        *dest++ = *src++;
    }

    *dest = '\0';

    return tmp;
}



#endif
