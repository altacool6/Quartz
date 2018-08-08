#include "TypeDef.h"
#include "TypeExt.h"
#include "QuartzDB.h"
#include "String.h"
#include "stdarg.h"

/******************************************************/
static void _RunMsg(QUARTZ_REQ* pReq);
/******************************************************/


DB_HANDLE QDB_Open(char* path)
{
    QUARTZ_CTX *pCtx = Q_NULL;

    pCtx = (QUARTZ_CTX*)PD_Malloc(sizeof(QUARTZ_CTX));
    QDB_ASSERT(pCtx);

    INIT_QUARTZ_CTX(*pCtx);
    strcpyA(pCtx->FilePath, path);

    return (DB_HANDLE)pCtx;
}

int _QDB_CreateTable (DB_HANDLE handle, char* t_name, ...)
{
    int         status = 0;
    va_list     args;
    COLUME_INFO cInfo;

    va_start(args, t_name);

    do{
        cInfo = va_arg(args, COLUME_INFO);
        if (cInfo.c_name == Q_NULL)
            break;

        PD_Printf("[%s][%s]\n", t_name, cInfo.c_name);
    } while(1);

    va_end(args);

    return status;
}

void QDB_Close(DB_HANDLE handle)
{
    QUARTZ_CTX *pCtx = handle;

	QDB_ASSERT(pCtx->Magic == QUARTZ_CTX_MAGIC);
	pCtx->Magic = 0;

    PD_Free(pCtx);
}

int QDB_InsertRequest(DB_HANDLE handle, QUARTZ_REQ* pReq)
{
    QUARTZ_CTX *pCtx = (QUARTZ_CTX*)handle;
    QUARTZ_MSG *pMsg = Q_NULL;

	QDB_ASSERT(pCtx->Magic == QUARTZ_CTX_MAGIC);

    if (!pReq)
        goto OUT_RETURN;

    pMsg = (QUARTZ_MSG*)PD_Malloc(sizeof(QUARTZ_MSG));
    QDB_ASSERT(pMsg);

    INIT_QUARTZ_MSG(*pMsg);
    pMsg->req = *pReq;

    while(PD_ObtainSemapore(SEMA_QUEUE) != SEMA_SUCCESS)
        PD_Sleep(100);

    /*Enqueue*/
    LIST_AddLast(&pCtx->ReqQueue.head, &pMsg->link);
    pCtx->ReqQueue.count++;

    PD_ReleaseSemapore(SEMA_QUEUE);

OUT_RETURN:
	return;
}

void QDB_Run(DB_HANDLE handle)
{
    QUARTZ_CTX *pCtx  = (QUARTZ_CTX*)handle;
    LIST_NODE  *pLink = Q_NULL;
    QUARTZ_MSG *pMsg  = Q_NULL;

	QDB_ASSERT(pCtx->Magic == QUARTZ_CTX_MAGIC);

    while(PD_ObtainSemapore(SEMA_QUEUE) != SEMA_SUCCESS)
        PD_Sleep(100);

    if (pCtx->ReqQueue.count != 0){
        pLink = LIST_FIRST(pCtx->ReqQueue.head);
        LIST_Remove(pLink);
        pCtx->ReqQueue.count--;
    }

    PD_ReleaseSemapore(SEMA_QUEUE);

    if (pLink){
        pMsg = container_of(pLink, QUARTZ_MSG, link);
        _RunMsg(&pMsg->req);
        PD_Free(pMsg);
    }
}

static void _RunMsg(QUARTZ_REQ* pReq){

}