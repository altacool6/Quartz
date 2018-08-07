#ifndef __QUARTZ_DB_H__
#define __QUARTZ_DB_H__

#include "TypeDef.h"

DB_HANDLE   QDB_Open          (char* path);
extern int  QDB_InsertRequest (DB_HANDLE handle, QUARTZ_REQ* pReq);
extern void QDB_Run           (DB_HANDLE handle);

#endif //#ifndef __QUARTZ_DB_H__