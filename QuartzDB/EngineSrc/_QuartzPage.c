#include "TypeDef.h"
#include "_QuartzPage.h"

static Q_BOOL __InitPage(QUARTZ_PAGE_OBJ* pPageObj, signed int pageNum, QUARTZ_PAGE_TYPE pageType)
{
    QUARTZ_PAGE_HEADER* pPageHeader;

    PD_Memset(pPageObj->pPage, QUARTZ_PAGE_SIZE);

    pPageHeader = (QUARTZ_PAGE_HEADER*)pPageObj->pPage;

    INIT_QUARTZ_PAGE_HEADER(*pPageHeader, pageNum, pageType);

    return Q_TRUE;
}

static Q_BOOL __LoadPage(QUARTZ_PAGE_OBJ* pPageObj, signed int pageNum)
{
    PD_Memset(pPageObj->pPage, QUARTZ_PAGE_SIZE);

    PD_Printf("Need Implement Load Page#%d\n", pageNum);

    return Q_TRUE;
}

static Q_BOOL __SavePage(QUARTZ_PAGE_OBJ* pPageObj)
{
    PD_Printf("Need Implement Save Page#%d\n", pPageObj->pageNum);

    return Q_TRUE;
}


static signed int __CalHeight(QUARTZ_PAGE_OBJ* pPageObj)
{
    signed int height;
    signed int heightL = 0;
    signed int heightR = 0;

    QDB_ASSERT(pPageObj);

    if (pPageObj->pLeft != Q_NULL)
        heightL = pPageObj->pLeft->height;
    
    if (pPageObj->pRight != Q_NULL)
        heightR = pPageObj->pRight->height;

    height = (heightL > heightR) ? heightL + 1 : heightR + 1;

    return height;
}


static signed int __CalBalanceFactor(QUARTZ_PAGE_OBJ* pPageObj)
{
    signed int bf;
    signed int heightL = 0;
    signed int heightR = 0;

    QDB_ASSERT(pPageObj);

    if (pPageObj->pLeft != Q_NULL)
        heightL = pPageObj->pLeft->height;
    
    if (pPageObj->pRight != Q_NULL)
        heightR = pPageObj->pRight->height;

    bf = heightL - heightR;

    return bf;
}

int RRCnt = 0;
int RLCnt = 0;
int LRCnt = 0;
int LLCnt = 0;

static void __RotationRR(QUARTZ_PAGE_OBJ** ppPageObj)
{
    QUARTZ_PAGE_OBJ *pBig, *pMid, *pSmall;

    pBig   = (*ppPageObj)->pRight->pRight;
    pMid   = (*ppPageObj)->pRight;
    pSmall = (*ppPageObj);

    *ppPageObj = pMid;

    pSmall->pRight = pMid->pLeft;

    pMid->pLeft  = pSmall;
    pMid->pRight = pBig;

    RRCnt++;
}

static void __RotationRL(QUARTZ_PAGE_OBJ** ppPageObj)
{
    QUARTZ_PAGE_OBJ *pBig, *pMid, *pSmall;

    pBig   = (*ppPageObj)->pRight;
    pMid   = (*ppPageObj)->pRight->pLeft;
    pSmall = (*ppPageObj);

    *ppPageObj = pMid;

    pSmall->pRight = pMid->pLeft;
    pBig->pLeft    = pMid->pRight;
    
    pMid->pLeft  = pSmall;
    pMid->pRight = pBig;

    RLCnt++;
}

static void __RotationLR(QUARTZ_PAGE_OBJ** ppPageObj)
{
    QUARTZ_PAGE_OBJ *pBig, *pMid, *pSmall;

    pBig   = (*ppPageObj);
    pMid   = (*ppPageObj)->pLeft->pRight;
    pSmall = (*ppPageObj)->pLeft;

    *ppPageObj = pMid;

    pSmall->pRight = pMid->pLeft;
    pBig->pLeft    = pMid->pRight;

    pMid->pLeft  = pSmall;
    pMid->pRight = pBig;

    LRCnt++;
}

static void __RotationLL(QUARTZ_PAGE_OBJ** ppPageObj)
{
    QUARTZ_PAGE_OBJ *pBig, *pMid, *pSmall;

    pBig   = (*ppPageObj);
    pMid   = (*ppPageObj)->pLeft;
    pSmall = (*ppPageObj)->pLeft->pLeft;

    *ppPageObj = pMid;

    pBig->pLeft = pMid->pRight;

    pMid->pLeft  = pSmall;
    pMid->pRight = pBig;

    LLCnt++;
}

static QUARTZ_PAGE_OBJ* __FindPreloadedPageTree(QUARTZ_PAGE_OBJ* pPageObj, signed int pageNum){
    QDB_ASSERT(pageNum >= 0
    );
    if (pPageObj == Q_NULL)
        goto OUT_RETURN;

    if (pPageObj->pageNum == pageNum)
        goto OUT_RETURN;

    pPageObj = __FindPreloadedPageTree((pPageObj->pageNum > pageNum) ? 
                                        pPageObj->pLeft : pPageObj->pRight, 
                                        pageNum);
OUT_RETURN:
    return pPageObj;
}

static void __RemovePreloadedPageTree(QUARTZ_PAGE_OBJ** ppCurPageObj, QUARTZ_PAGE_OBJ* pPageObj)
{
    QDB_ASSERT(ppCurPageObj > 0);

    if (*ppCurPageObj == pPageObj) {

        QDB_ASSERT(pPageObj->height > 0);

        if (pPageObj->height == 1) { //leaf node case
            pPageObj->height = -1;
            *ppCurPageObj = Q_NULL;
        }
        else {
            signed int bf = __CalBalanceFactor(*ppCurPageObj);

            QDB_ASSERT((-1<=bf) && (bf<=1));

            switch(bf)
            {
            case -1:/* _GetMinNode((*ppCurPageObj)->pRight) */  break;
            case 0:     break;
            case 1:     break;
            }
        }
        goto OUT_RETURN;
    }

    if ((*ppCurPageObj)->pageNum > pPageObj->pageNum)
        __RemovePreloadedPageTree(&(*ppCurPageObj)->pLeft, pPageObj);
    else
        __RemovePreloadedPageTree(&(*ppCurPageObj)->pRight, pPageObj);

OUT_RETURN:
    return;
}

static void __ADDPreloadedPageTree(QUARTZ_PAGE_OBJ** ppCurPageObj, QUARTZ_PAGE_OBJ* pPageObj)
{
    signed int bf;

    if (*ppCurPageObj == Q_NULL) {
        *ppCurPageObj = pPageObj;
        goto OUT_RETURN;
    }

    // Tree traverse : Recursive call part 
    if ((*ppCurPageObj)->pageNum > pPageObj->pageNum)
        __ADDPreloadedPageTree(&(*ppCurPageObj)->pLeft, pPageObj);
    else
        __ADDPreloadedPageTree(&(*ppCurPageObj)->pRight, pPageObj);

    // Tree Rebalencing Step
    bf = __CalBalanceFactor(*ppCurPageObj);
    QDB_ASSERT((-2<=bf) && (bf<=2));

    if (bf==0 || bf==-1 || bf==1)
        goto OUT_RETURN;

    // Tree Rotation
    if (bf == -2){
        signed int rightChildBf = __CalBalanceFactor((*ppCurPageObj)->pRight);

        if (rightChildBf == 1)
            __RotationRL(ppCurPageObj); // RL rotation
        else
            __RotationRR(ppCurPageObj); // RR rotation
    }

    if (bf == 2){
        signed int leftChildBf = __CalBalanceFactor((*ppCurPageObj)->pLeft);

        if (leftChildBf == -1)
            __RotationLR(ppCurPageObj); // LR rotation
        else
            __RotationLL(ppCurPageObj); // LL rotation
    }

    if ((*ppCurPageObj)->pLeft)
        (*ppCurPageObj)->pLeft->height = __CalHeight((*ppCurPageObj)->pLeft);

    if ((*ppCurPageObj)->pRight)
        (*ppCurPageObj)->pRight->height = __CalHeight((*ppCurPageObj)->pRight);

OUT_RETURN:
    (*ppCurPageObj)->height = __CalHeight(*ppCurPageObj);
    return;
}

void _PrintLRU(LIST_NODE* pHead)
{
    LIST_NODE *pNode;
    QUARTZ_PAGE_OBJ *pPageObj;

    pNode = LIST_FIRST(*pHead);

    PD_Printf("[_PrintLRU]\n");
    while(pHead != pNode) {
        pPageObj = container_of(pNode, QUARTZ_PAGE_OBJ, stLink);

        if (pPageObj->pageNum >= 0)
            PD_Printf("#%d(h%d)%s", pPageObj->pageNum, pPageObj->height, 
                                (pNode->next != pHead)?"->":"");

        pNode = pNode->next;
    }
    PD_Printf("\n");
}

static void __PrintTree(QUARTZ_PAGE_OBJ* pPageObj)
{
    if (!pPageObj)
        return;

    __PrintTree(pPageObj->pLeft);

    PD_Printf("#%d(h%d) ", pPageObj->pageNum, pPageObj->height);

    __PrintTree(pPageObj->pRight);
}

void _PrintTree(QUARTZ_PAGE_OBJ* pTreeRootNode)
{
    PD_Printf("[_PrintTree]\n");
    __PrintTree(pTreeRootNode);
}


QUARTZ_PAGE_OBJ* _TouchPage(QUARTZ_PAGE_OBJ** ppRootPageObj, LIST_NODE* pHead, signed int pageNum)
{
    QUARTZ_PAGE_OBJ* pPageObj = Q_NULL;
    LIST_NODE*       pNode    = Q_NULL;

    QDB_ASSERT(LIST_IsEmpty(pHead) == Q_FALSE);

    pPageObj = __FindPreloadedPageTree(*ppRootPageObj, pageNum);

    if (pPageObj == Q_NULL) {
        pNode    = LIST_LAST(*pHead);
        pPageObj = container_of(pNode, QUARTZ_PAGE_OBJ, stLink);
    }

    LIST_Remove(&pPageObj->stLink);

    if (pPageObj->pageNum != pageNum) {
        if (pPageObj->height > 0)
            __RemovePreloadedPageTree(ppRootPageObj, pPageObj);

        if (pPageObj->bDirty)
            __SavePage(pPageObj);

        __LoadPage(pPageObj, pageNum);

        pPageObj->pageNum = pageNum;

        __ADDPreloadedPageTree(ppRootPageObj, pPageObj);
    }

    LIST_AddFirst(pHead, &pPageObj->stLink);

    return pPageObj;
}