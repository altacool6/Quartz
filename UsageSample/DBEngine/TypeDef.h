#ifndef __TYPE_DEF_H__
#define __TYPE_DEF_H__

typedef char Q_BOOL;
enum{
    Q_FALSE,
    Q_TRUE
};/* Q_BOOL Range*/

typedef int Q_RET_CODE;
enum{
    Q_RET_SUCCESS = 0,
    Q_RET_FAILURE = -1
};/* Q_RET_CODE */

#define Q_NULL	(0)

typedef int SEARCH_COND;

enum{
    IS_NULL,
    IS_NOT_NULL,
    EQUAL,
    NOT_EQUAL,
    LIKE,        // this condition can only use in case of String Field
    DISTINCT,
};/* Q_BOOL Range*/

typedef void (*CBFuncDistinct)(int reqID, int tag, int columeType, void* value);
typedef void (*CBFuncNormal)  (int reqID, int tag, int recordID,   int  param);

typedef struct QuartzReq {
    char        t_name[256];
/*
    struct _searchInfo {
        char        c_name[256];

        SEARCH_COND condition;

        union {
            char    val8;
            short   val16;
            int     val32;
            char    valStr[256];
        } value;

        union {
            struct _normal{
                int             reqID;
                int             tag;
                CBFuncNormal    pfCBNormal;
            } normal;
            struct _distinct{
                int             reqID;
                int             tag;
                CBFuncDistinct  pfCBDistinct;
            } distinct;

        } CBInfo;
    } SearchInfo;*/
} QUARTZ_REQ;

#define INIT_QUARTZ_REQ(a)  do{                               \
                                PD_Memset(&(a), sizeof((a))); \
                            }while(0)

typedef void* DB_HANDLE;
typedef int   SEMAPORE_ID;
typedef int   SEMAPORE_RET;

enum{
    SEMA_QUEUE,
    SEMA_RUN,
};/* SEMAPORE_ID Range*/

enum{
    SEMA_FAILURE,
    SEMA_SUCCESS,
};/* SEMAPORE_RET Range*/

#define container_of(ptr, type, member) ((type*)(((char*)ptr) - ((int)(&(((type*)0)->member)))))


#define QDB_ASSERT(COND) do {                                                \
    if (!(COND)) {                                                           \
        int _exit = 0;                                                       \
        PD_Printf("[ASSERT FAIL] %s, %d line\n", __FUNCTION__, __LINE__);    \
        while(!_exit){};                                                     \
    }                                                                        \
} while (0)

#endif  //#ifndef __TYPE_DEF_H__