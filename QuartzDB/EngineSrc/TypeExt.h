#ifndef __TYPE_EXT_H__
#define __TYPE_EXT_H__

#include "List.h"
#include "_QuartzPage.h"
#include "QuartzDB.h"

typedef struct QuartzMsg{
    QUARTZ_REQ req;
    LIST_NODE  link;
} QUARTZ_MSG;

#define INIT_QUARTZ_MSG(a)  do{                             \
                                INIT_QUARTZ_REQ((a).req);   \
                                INIT_LIST_NODE((a).link);   \
                            }while(0)

typedef struct QuartzQ{
    unsigned  long magic;
    LIST_NODE      head;
    int            count;
} QUARTZ_Q;

#define QUARTZ_QUEUE_MAGIC (0xCCCCAAAA)

#define INIT_QUARTZ_Q(a)    do{                                 \
                                (a).magic = QUARTZ_QUEUE_MAGIC; \
                                INIT_LIST_NODE((a).head);       \
                                (a).count = 0;                  \
                            }while(0)

typedef struct QuartzCtx{
    unsigned  long magic;

    char           filePath[256];
    signed int     pageCnt;

    struct Mmu { /* virtual memory management unit */
        QUARTZ_PAGE_OBJ  pool[QUARTZ_MAX_LOAD_PAGE_COUNT];
        char             page[QUARTZ_MAX_LOAD_PAGE_COUNT][QUARTZ_PAGE_SIZE];
        LIST_NODE        stLRU;    //LRU List
        QUARTZ_PAGE_OBJ* pRoot;
    } mmu;

    QUARTZ_Q  reqQueue;
} QUARTZ_CTX;

#define INIT_MMU(a) do{                                                     \
                        int i;                                              \
                        (a).pRoot = Q_NULL;                                 \
                        INIT_LIST_NODE((a).stLRU);                          \
                        for (i = 0 ; i < QUARTZ_MAX_LOAD_PAGE_COUNT ; i++){ \
                            INIT_QUARTZ_PAGE_OBJ((a).pool[i], (a).page[i]); \
                            LIST_AddLast(&(a).stLRU, &(a).pool[i].stLink);  \
                        }                                                   \
                    } while(0)

#define QUARTZ_CTX_MAGIC (0xCAFACFBC)

#define INIT_QUARTZ_CTX(a)  do{                                     \
                                PD_Memset(&a, sizeof((a)));         \
                                (a).magic   = QUARTZ_CTX_MAGIC;     \
                                (a).pageCnt = 0;                    \
                                INIT_MMU((a).mmu);                  \
                                INIT_QUARTZ_Q((a).reqQueue);        \
                            }while(0)

#endif  //#ifndef __TYPE_EXT_H__