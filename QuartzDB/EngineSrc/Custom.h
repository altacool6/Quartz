#ifndef __CUSTOM_H__
#define __CUSTOM_H__

extern void  PD_Printf(...);

extern void* PD_Malloc(int size);
extern void  PD_Free(void* ptr);

extern void  PD_Memset(void* p, int size);
extern void  PD_Memcpy(void* dst, void* src, int size);

extern void  PD_Sleep(int ms);

extern SEMAPORE_RET PD_CretateSemapore(SEMAPORE_ID id);
extern SEMAPORE_RET PD_ObtainSemapore(SEMAPORE_ID id);
extern SEMAPORE_RET PD_ReleaseSemapore(SEMAPORE_ID id);

#endif	//#ifndef __CUSTOM_H__
