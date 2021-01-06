#ifndef __DB_DESIGN_UTIL_H__
#define __DB_DESIGN_UTIL_H__

#include "avldbupgrade.h"

enum
{
	FILE_PATH,
	FILE_PATH_AND_FILE_NAME
};
#define DEVICE_DIRECTORY_DEPTH	20



#ifdef DB_SEARCH_CHANGED
unsigned long GetRowidOfRefferanceTableByParentUid(unsigned long vUid, unsigned long *vRowids, unsigned long vRowidsArraySize);
unsigned long GetRowidOfRefferanceTableByChildUid(unsigned long vUid, unsigned long *vRowids, unsigned long vRowidsArraySize);
#else
unsigned long GetRowidOfRefferanceTableByParentUid(unsigned long vUid, unsigned long *vRowids);
unsigned long GetRowidOfRefferanceTableByChildUid(unsigned long vUid, unsigned long *vRowids);
#endif



unsigned long GetRowidOfMusicTableByUid(unsigned long vUid);
unsigned long GetRowidOfAlbumTableByUid(unsigned long vUid);
unsigned long GetRowidOfPhotoTableByUid(unsigned long vUid);
unsigned long GetRowidOfObjectsTableByUid(unsigned long vUid);


int CheckObjectUid(unsigned long uId, unsigned long * rowId); 
int FindUid(unsigned short* fileFullPathName, unsigned long* uId);
int FillObjectPath(unsigned long uId, unsigned short* targetBuffer, unsigned long pathOnlyOrFullPath);
int FillObjectName(unsigned long uId, unsigned short* targetBuffer);
#endif
