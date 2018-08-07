#include "TypeDef.h"
#include "TypeExt.h"
#include "QuartzDB.h"

/******************************************************/
static void _RunMsg(QUARTZ_REQ* pReq);
/******************************************************/


DB_HANDLE QDB_Open(char* path)
{
	return (DB_HANDLE)0;
}

int QDB_InsertRequest(DB_HANDLE handle, QUARTZ_REQ* pReq)
{
    QUARTZ_CTX *pCtx = (QUARTZ_CTX*)handle;
    QUARTZ_MSG *pMsg = Q_NULL;

    if (!pReq)
        goto OUT_RETURN;

    pMsg = (QUARTZ_MSG*)PD_Malloc(sizeof(QUARTZ_MSG));

    pMsg->req = *pReq;

    INIT_LIST_NODE(pMsg->link);

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