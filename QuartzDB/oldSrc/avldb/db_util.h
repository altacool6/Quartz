#ifndef __DB_UTIL_H__
#define __DB_UTIL_H__
#include "db3_basictype_define.h"

enum
{
	DB_FILE_ERROR,
	DB_FILE_NO_ERROR
};


void dbTransformationMyIndian(void *pSource, unsigned long vUnitSize, unsigned long vRepeat);
void dbUtilMemcpyFreeIndian(void *pSource, void *pTarget, unsigned long vUnitSize, unsigned long vRepeat);
int dbCheckDatabaseFilesOpen(char * pDictionaryFileName, char * pDataFileName, char * pIndexFileName);
int dbCheckDatabaseDictionaryFile(char* pDictionaryFileName, unsigned long vLogicalDictionaryFileSize, unsigned long vTableCount);
int dbCheckTablesCount(unsigned long vOffset, unsigned long *pCurrentTableCount, unsigned long TableCount, unsigned long vLogicalDictionaryFileSize);
int dbCheckTableRecords(pTableNode pTable);
int dbCheckMemoryAlloc(char* pDictionaryFileName, char* pDataFileName, char* pIndexFileName,
						      unsigned long vDictionaryMemorySize, unsigned long vDataPageSize, unsigned long vIndexPageSize);

int dbCheckMemoryAlloc(char* pDictionaryFileName, char* pDataFileName, char* pIndexFileName,
						      unsigned long vDictionaryMemorySize, unsigned long vDataPageSize, unsigned long vIndexPageSize);

void dbCheckMemoryFree(void);
int dbApiUtilCheckDatabaseFiles(char* pDictionaryFileName, char* pDataFileName, char* pIndexFileName, 
									unsigned long vLogicalDictionaryFileSize, unsigned long vDataPageSize, unsigned long vIndexPageSize,
									unsigned long vTableCount);
#endif
