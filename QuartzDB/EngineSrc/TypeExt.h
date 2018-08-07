#ifndef __TYPE_EXT_H__
#define __TYPE_EXT_H__

#include "List.h"
#include "QuartzDB.h"

typedef struct QuartzMsg{
    QUARTZ_REQ req;
    LIST_NODE  link;
} QUARTZ_MSG;

typedef struct QuartzQ{
    LIST_NODE   head;
    int         count;
} QUARTZ_Q;

typedef struct QuartzCtx{
    char	  FilePath[256];
    QUARTZ_Q  ReqQueue;
} QUARTZ_CTX;

#endif  //#ifndef __TYPE_EXT_H__