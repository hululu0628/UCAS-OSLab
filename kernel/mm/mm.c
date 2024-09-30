#include <os/mm.h>

static ptr_t kernMemCurr = FREEMEM_KERNEL;
static ptr_t userMemCurr = FREEMEM_USER;

ptr_t allocKernelPage(int numPage)
{
    // align PAGE_SIZE
    // the first page is 0x50501000~0x50502000
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
