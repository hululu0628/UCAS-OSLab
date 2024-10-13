#include <os/mm.h>

static ptr_t kernMemCurr = FREEMEM_KERNEL;
static ptr_t userMemCurr = FREEMEM_USER;

// ε=( o｀ω′)ノ
ptr_t allocKernelPage(int numPage)
{
    // align PAGE_SIZE
    ptr_t ret = ROUND(kernMemCurr, PAGE_SIZE);
    kernMemCurr = ret + numPage * PAGE_SIZE;
    return ret;
}

ptr_t allocUserPage(int numPage)
{
    // align PAGE_SIZE
    ptr_t ret = ROUND(userMemCurr, PAGE_SIZE);
    userMemCurr = ret + numPage * PAGE_SIZE;
    return ret;
}

// Stack grows from the high address to the low address
ptr_t allocKernelStack(int numPage)
{
	allocKernelPage(numPage);
	return kernMemCurr;
}

ptr_t allocUserStack(int numPage)
{
	allocUserPage(numPage);
	return userMemCurr;
}