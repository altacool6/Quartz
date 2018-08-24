#ifndef __QUARTZ_PAGE_H__
#define __QUARTZ_PAGE_H__

#include "List.h"
#include "QuartzCfg.h"

typedef signed int QUARTZ_PAGE_TYPE;

enum{
    QUARTZ_PAGE_UNDEDICATED,
    QUARTZ_PAGE_DICTIONARY,
    QUARTZ_PAGE_DATA,
    QUARTZ_PAGE_INDEX,
};/* Quartz Page Type */

typedef struct QuartzPageHeader{
    signed int          pageNum;
    QUARTZ_PAGE_TYPE    type;
} QUARTZ_PAGE_HEADER;

#define INIT_QUARTZ_PAGE_HEADER(a, p1, p2)  do{                     \
                                                (a).pageNum = (p1); \
                                                (a).type    = (p2); \
                                            } while(0)

typedef struct QuartzPageObj{
    struct QuartzPageObj* pLeft;    // for Tree
    struct QuartzPageObj* pRight;   // for Tree
    signed int            height;   // for Balance Factor
    signed int            pageNum;  // Logical Page Number
    LIST_NODE             stLink;   // for LRU List
    char*                 pPage;    // physical Memory
    Q_BOOL                bDirty;   // Logical Page Status
} QUARTZ_PAGE_OBJ;

#define INIT_QUARTZ_PAGE_OBJ(a, page)  do{                          \
                                PD_Memset(&(a), sizeof((a)));       \
                                (a).pLeft   = Q_NULL;               \
                                (a).pRight  = Q_NULL;               \
                                (a).height  = -1;                   \
                                (a).pageNum = -1;                   \
                                INIT_LIST_NODE((a).stLink);      \
                                (a).pPage   = (page);               \
                                PD_Memset((page), QUARTZ_PAGE_SIZE);\
                                (a).bDirty  = 0;                    \
                            }while(0)

extern QUARTZ_PAGE_OBJ* _TouchPage(QUARTZ_PAGE_OBJ** ppRoot, LIST_NODE* pHead, signed int pageNum);

/*debuging function*/
extern void _PrintLRU(LIST_NODE* pHead);
extern void _PrintTree(QUARTZ_PAGE_OBJ* pTreeRootNode);
/*debuging function*/


#endif //#ifndef __QUARTZ_PAGE_H__