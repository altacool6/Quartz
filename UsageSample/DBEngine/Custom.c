#include "stdio.h"
#include "stdarg.h"
#include "malloc.h"
#include "QuartzDB.h"
#include "Windows.h"

void PD_Printf(const char *fmt, ...)
{
	static char buf[1024];

	va_list args;

	va_start(args, fmt);

	vsprintf(buf, fmt, args);
	
	va_end(args);

	printf(buf);
}

void* PD_Malloc(int size)
{
	return malloc(size);
}

void PD_Free(void* ptr)
{
	free(ptr);
}

void PD_Sleep(int ms)
{
	Sleep(ms);
}

SEMAPORE_RET PD_CretateSemapore(SEMAPORE_ID id)
{
    SEMAPORE_RET ret = SEMA_FAILURE;

    switch(id)
    {
    case SEMA_QUEUE:
        ret = SEMA_SUCCESS;
        break;
    case SEMA_RUN:
        ret = SEMA_SUCCESS;
        break;
    }

    return ret;
}

SEMAPORE_RET PD_ObtainSemapore(SEMAPORE_ID id)
{
    SEMAPORE_RET ret = SEMA_FAILURE;
    
    switch(id)
    {
    case SEMA_QUEUE:
        ret = SEMA_SUCCESS;
        break;
    case SEMA_RUN:
        ret = SEMA_SUCCESS;
        break;
    }

    return ret;
}

SEMAPORE_RET PD_ReleaseSemapore(SEMAPORE_ID id)
{
    SEMAPORE_RET ret = SEMA_FAILURE;
    
    switch(id)
    {
    case SEMA_QUEUE:
        ret = SEMA_SUCCESS;
        break;
    case SEMA_RUN:
        ret = SEMA_SUCCESS;
        break;
    }

    return ret;
}
