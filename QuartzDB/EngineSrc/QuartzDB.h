#ifndef __QUARTZ_DB_H__
#define __QUARTZ_DB_H__

#include "TypeDef.h"

typedef char  COLUME_TYPE;
enum{
    NUMBER_8BIT_TYPE,
    NUMBER_16BIT_TYPE,
    NUMBER_32BIT_TYPE,
    STRING_A_TYPE,
    STRING_W_TYPE
};/* COLUME_TYPE Range*/

typedef struct ColumeInfo {
    char*       c_name;
    COLUME_TYPE c_type;
    Q_BOOL      c_index;
} COLUME_INFO;

static __inline COLUME_INFO GET_COLUME_INFO(char* c_name, COLUME_TYPE c_type, Q_BOOL c_index)
{
    COLUME_INFO colInfo;

    colInfo.c_name  = c_name;
    colInfo.c_type  = c_type;
    colInfo.c_index = c_index;

    return colInfo;
}

extern DB_HANDLE QDB_Open          (char* path);
#define          QDB_CreateTable(handle, t_name, ...)   do {                                                            \
                                                            _QDB_CreateTable((handle), (t_name), ##__VA_ARGS__, Q_NULL);\
                                                        } while(0)
extern int       _QDB_CreateTable   (DB_HANDLE handle, char* t_name, ...);
extern int       QDB_InsertRequest (DB_HANDLE handle, QUARTZ_REQ* pReq);
extern void      QDB_Run           (DB_HANDLE handle);
extern void      QDB_Close         (DB_HANDLE handle);

#endif //#ifndef __QUARTZ_DB_H__