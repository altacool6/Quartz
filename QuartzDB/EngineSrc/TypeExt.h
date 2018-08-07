#ifndef __TYPE_EXT_H__
#define __TYPE_EXT_H__

#include "List.h"
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
    unsigned  long Magic;
    LIST_NODE      head;
    int            count;
} QUARTZ_Q;

#define QUARTZ_QUEUE_MAGIC (0xCCCCAAAA)

#define INIT_QUARTZ_Q(a)    do{                                 \
                                (a).Magic = QUARTZ_QUEUE_MAGIC; \
                                INIT_LIST_NODE((a).head);       \
                                (a).count = 0;                  \
                            }while(0)

typedef struct QuartzCtx{
    unsigned  long Magic;
    char	  FilePath[256];
    QUARTZ_Q  ReqQueue;
} QUARTZ_CTX;

#define QUARTZ_CTX_MAGIC (0xCAFACFBC)

#define INIT_QUARTZ_CTX(a) do{                                  \
                                PD_Memset(&a, sizeof((a)));     \
                                (a).Magic = QUARTZ_CTX_MAGIC;   \
                                INIT_QUARTZ_Q((a).ReqQueue);    \
                            }while(0)

#endif  //#ifndef __TYPE_EXT_H__