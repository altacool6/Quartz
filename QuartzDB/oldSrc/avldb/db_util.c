#include "db_util.h"
#include "avldb.h"
#include "malloc.h"


enum
{
	DB_CHECK_ERROR_MEMORY_DISALLOC,
	DB_CHECK_ERROR_MEMORY_ALLOC,
};

enum
{
	DB_UTIL_ORDER_INDIAN,
	DB_UTIL_SAME_INDIAN,
};


//#define WORKING_SYSTEM_INDIAN  DB_UTIL_ORDER_INDIAN
#define WORKING_SYSTEM_INDIAN  DB_UTIL_SAME_INDIAN

#if (TFS4_BASE_STAND_IO==1)
	#include "tfs4_types.h"
	#include "tfs4_api.h"
	#define CHECK_DB_FILE			signed long
	#define FILE_HANDLE_NULL		-1
	#define DICTIONARY_FILE_OPEN	tfs4_open((t_char*)g_pCheck_data->pDicFileName,	O_RDONLY)
	#define DICTIONARY_FILE_CLOSE	tfs4_close(g_pCheck_data->fpDbDictionaryFile)
	#define DATA_FILE_OPEN			tfs4_open((t_char*)g_pCheck_data->pDatFileName, 	O_RDONLY)
	#define DATA_FILE_CLOSE		tfs4_close(g_pCheck_data->fpDbDataFile)
	#define INDEX_FILE_OPEN			tfs4_open((t_char*)g_pCheck_data->pIdxFileName, 	O_RDONLY)
	#define INDEX_FILE_CLOSE		tfs4_close(g_pCheck_data->fpDbIndexFile)

#endif

#if (ANSI_BASE_STAND_IO==1)
	#include "stdio.h"
	#define CHECK_DB_FILE			FILE*
	#define FILE_HANDLE_NULL		0
	#define DICTIONARY_FILE_OPEN	fopen(g_pCheck_data->pDicFileName, "r")
	#define DICTIONARY_FILE_CLOSE	fclose(g_pCheck_data->fpDbDictionaryFile)
	#define DATA_FILE_OPEN			fopen(g_pCheck_data->pDatFileName, "r")
	#define DATA_FILE_CLOSE		fclose(g_pCheck_data->fpDbDataFile)
	#define INDEX_FILE_OPEN			fopen(g_pCheck_data->pIdxFileName, "r")
	#define INDEX_FILE_CLOSE		fclose(g_pCheck_data->fpDbIndexFile)
#endif

struct Check_Data{
	unsigned long 		vDictionarySize;
	unsigned long 		vDataFilePageSize;
	unsigned long 		vIndexFilePageSize;
	char*			pDicStart;
	char*			pDatStart;
	char*			pIdxStart;
	CHECK_DB_FILE	fpDbDictionaryFile;
	CHECK_DB_FILE	fpDbDataFile;
	CHECK_DB_FILE	fpDbIndexFile;
	char*			pDicFileName;
	char*			pDatFileName;
	char*			pIdxFileName;
};

struct Check_Data	*g_pCheck_data = 0;

#if (TFS4_BASE_STAND_IO==1)

#ifndef CONFIG_HDD
extern unsigned char g_MBuf[65536]; //jaihong.kim noncache 사용않함 (E10)
#endif

static void dbUtilFileRead(unsigned long lDbFileHandle, unsigned char *pReadBuf, signed long lTotalReadSize)
{
	unsigned long lReadSize;
	
#ifdef CONFIG_HDD //jaihong.kim noncache 사용않함 (E10)

	tfs4_read(lDbFileHandle, pReadBuf, lTotalReadSize);
	if(lTotalReadSize != lReadSize)
	{
		//error
		//DbgPrintf(("dbFileRead:: READ ERROR \n", lTotalReadSize));	

	}
#else

	while (lTotalReadSize>0)
	{
		if (lTotalReadSize > sizeof(g_MBuf))
			lReadSize = sizeof(g_MBuf);
		else
			lReadSize = lTotalReadSize;	

		tfs4_read(lDbFileHandle, g_MBuf, lReadSize);

		memcpy(pReadBuf, g_MBuf, lReadSize);
		
		pReadBuf += lReadSize;
		lTotalReadSize -= lReadSize;
	}
#endif

}
#endif

void dbTransformationMyIndian(void *pSource, unsigned long vUnitSize, unsigned long vRepeat)
{
#if (WORKING_SYSTEM_INDIAN == DB_UTIL_SAME_INDIAN)	
	//같은 인디언 체계이므로 바꿀것이 없다.
	return;
#else
	//다른인디언 체계이므로 변환을 해야 한다.
	char				TempUnit;
	
	if (vUnitSize == sizeof(char))
	{
		//다른인디언 체계로 변환을 해야 하는데 변환하려는 unit의 사이즈가 1인 경우 인디언 특성을 타지 않으므로 변환을 하지 않는다.
	}
	else if(vUnitSize == sizeof(unsigned short))
	{
		char		*pSwapFront;
		char		*pSwapRear;

		while(vRepeat)
		{
			pSwapFront	= ((char*)pSource) + (vRepeat-1)*vUnitSize;
			pSwapRear	= ((char*)pSource) + (vRepeat*vUnitSize) -1;

			TempUnit		= *pSwapFront;
			*pSwapFront	= *pSwapRear;
			*pSwapRear	= TempUnit;

			vRepeat--;
		}
	}
	else if (vUnitSize == sizeof(unsigned long))
	{
		char		*pSwapFront;
		char		*pSwapRear;

		while(vRepeat)
		{
			pSwapFront	= ((char*)pSource) + (vRepeat-1)*vUnitSize;
			pSwapRear	= ((char*)pSource) + (vRepeat*vUnitSize) -1;

			TempUnit			= *pSwapFront;
			*pSwapFront		= *pSwapRear;
			*pSwapRear		= TempUnit;
			TempUnit			= *(pSwapFront+1);
			*(pSwapFront+1)	= *(pSwapRear-1);
			*(pSwapRear-1)	= TempUnit;
			
			vRepeat--;
		}
	}
	else
	{
		//error
	}
#endif
}


void dbUtilMemcpyFreeIndian(void *pSource, void *pTarget, unsigned long vUnitSize, unsigned long vRepeat)
{
#if (WORKING_SYSTEM_INDIAN == DB_UTIL_SAME_INDIAN)	
	unsigned long vCopySize;

	vCopySize = vUnitSize*vRepeat;
	
	while(vCopySize)
	{
		vCopySize--;
		*(((char*)pTarget) + vCopySize)= *(((char*)pSource) + vCopySize);
	}
#else
	unsigned long vLocalUnitSize;

	while(vRepeat)
	{
		vLocalUnitSize = vUnitSize;
		
		while(vLocalUnitSize)
		{
			*(((char*)pTarget) + vUnitSize - vLocalUnitSize)= *(((char*)pSource) + vLocalUnitSize - 1);

			vLocalUnitSize--;
		}
		
		pSource = ((char*)pSource) + vUnitSize;
		pTarget = ((char*)pTarget) + vUnitSize;

		vRepeat--;
	}
#endif
}


int dbCheckDatabaseFilesOpen(char * pDictionaryFileName, char * pDataFileName, char * pIndexFileName)
{
	//파일 핸들의 값을 초기화 한다.
	g_pCheck_data->fpDbDictionaryFile	= FILE_HANDLE_NULL;
	g_pCheck_data->fpDbDataFile		= FILE_HANDLE_NULL;
	g_pCheck_data->fpDbIndexFile		= FILE_HANDLE_NULL;


	//디비파일을 연다.
	g_pCheck_data->fpDbDictionaryFile	= DICTIONARY_FILE_OPEN;
	g_pCheck_data->fpDbDataFile		= DATA_FILE_OPEN;
	g_pCheck_data->fpDbIndexFile		= INDEX_FILE_OPEN;

	//핸들값이 FILE_HANDLE_NULL보다 작은 값이 존재하면 디비파일을 open하는데 문제가 발생한 경우
	if ((g_pCheck_data->fpDbDictionaryFile < FILE_HANDLE_NULL) 
		||(g_pCheck_data->fpDbDataFile  < FILE_HANDLE_NULL)
		||(g_pCheck_data->fpDbIndexFile < FILE_HANDLE_NULL))
	{
		DICTIONARY_FILE_CLOSE;
		DATA_FILE_CLOSE;
		INDEX_FILE_CLOSE;
		
		g_pCheck_data->fpDbDictionaryFile	= FILE_HANDLE_NULL;
		g_pCheck_data->fpDbDataFile		= FILE_HANDLE_NULL;
		g_pCheck_data->fpDbIndexFile		= FILE_HANDLE_NULL;

		//Error를 리턴한다.
		return DB_FILE_ERROR;
	}
	//핸들값이 FILE_HANDLE_NULL보다 작은 값이 존재하지않는다면 그것은 정상적으로 파일이 존재하고 열린경우이다.
	else
	{
		DICTIONARY_FILE_CLOSE;
		DATA_FILE_CLOSE;
		INDEX_FILE_CLOSE;
		
		g_pCheck_data->fpDbDictionaryFile	= FILE_HANDLE_NULL;
		g_pCheck_data->fpDbDataFile		= FILE_HANDLE_NULL;
		g_pCheck_data->fpDbIndexFile		= FILE_HANDLE_NULL;

		//물리적인 파일에 이상이 없음을 리턴한다.
		return DB_FILE_NO_ERROR;
	}
}

int dbCheckDatabaseDictionaryFile(char* pDictionaryFileName, unsigned long vLogicalDictionaryFileSize, unsigned long vTableCount)
{
	#if (ANSI_BASE_STAND_IO==1)
	int 	vReadingCount;		//ERROR처리를 위한 지역변수
	#endif
	DictionaryHeader	vDictionaryFileHeader;

	memset(&vDictionaryFileHeader, 0, sizeof(vDictionaryFileHeader));

	//=================STEP 1. 딕셔너리 파일을 연다. (BEGIN)=========================================//
	if (g_pCheck_data->fpDbDictionaryFile != FILE_HANDLE_NULL)
	{
		DICTIONARY_FILE_CLOSE;
		g_pCheck_data->fpDbDictionaryFile = FILE_HANDLE_NULL;
	}
	
	g_pCheck_data->fpDbDictionaryFile = DICTIONARY_FILE_OPEN;
	if (g_pCheck_data->fpDbDictionaryFile == FILE_HANDLE_NULL)	return DB_FILE_ERROR;	//error 처리
	//=================STEP 1. 딕셔너리 파일을 연다. (END)===========================================//	


	//=================STEP 2. 딕셔너리 해더를 읽어온다. (BEGIN)=======================================//
	{
		#if (TFS4_BASE_STAND_IO==1)
		dbUtilFileRead((unsigned long)g_pCheck_data->fpDbDictionaryFile, (unsigned char*)(&vDictionaryFileHeader), sizeof(vDictionaryFileHeader));
		#endif

		#if (ANSI_BASE_STAND_IO==1)
		vReadingCount = fread(&vDictionaryFileHeader, sizeof(vDictionaryFileHeader), 1, g_pCheck_data->fpDbDictionaryFile);
		#endif
	}
	dbTransformationMyIndian(&vDictionaryFileHeader, sizeof(unsigned long), 6);
	DICTIONARY_FILE_CLOSE;
	g_pCheck_data->fpDbDictionaryFile = FILE_HANDLE_NULL;

	#if (ANSI_BASE_STAND_IO==1)					//error 처리
	if (vReadingCount!=1)	return DB_FILE_ERROR;	//error 처리
	#endif										//error 처리
	//=================STEP 2. 딕셔너리 해더를 읽어온다. (END)=========================================//


	//=================STEP 3. 딕셔너리 해더의 값을 가지고 Error체크를 시도한다. (BEGIN)(특정 범위 내에 존재하는지만을 검사한다.)====//
	//=================STEP 3. 1. 항상 0의 값을 가져야 하는 부분이 0의 값으로 읽어지는지를 확인한다.
	if ((vDictionaryFileHeader.dictionaryHeaderDeleteNode != 0) ||(vDictionaryFileHeader.dictionaryHeaderDeleteStartPage != 0))
	{
		return DB_FILE_ERROR;
	}
	
	//=================STEP 3. 2. 테이블의 root값이 범위 내에 있는지 검사한다.
	if ((vDictionaryFileHeader.dictionaryHeaderOffsetOfRootTable < sizeof(DictionaryHeader)) 
		|| (vDictionaryFileHeader.dictionaryHeaderOffsetOfRootTable>=vLogicalDictionaryFileSize))
	{
		return DB_FILE_ERROR;
	}

	//=================STEP 3. 3. 딕셔너리 파일의 크기가 인자 값으로 받은 값과 같은지를 판단한다.
	if (vDictionaryFileHeader.dictionaryHeaderSizeOfDictionary!=vLogicalDictionaryFileSize)
	{
		return DB_FILE_ERROR;
	}

	//=================STEP 3. 4. Data 파일의 크기가 범위 내에 있는지 검사한다.
	if (vDictionaryFileHeader.dictionaryHeaderNumberOfDataFileEndPage > 1000) 
	{
		return DB_FILE_ERROR;
	}
	
	//=================STEP 3. 5. Index 파일의 크기가 범위 내에 있는지 검사한다.
	if (vDictionaryFileHeader.dictionaryHeaderNumberOfDataFileEndPage > 1000) 
	{
		return DB_FILE_ERROR;
	}
	//=================STEP 3. 딕셔너리 해더의 값을 가지고 Error체크를 시도한다. (END)(특정 범위 내에 존재하는지만을 검사한다.)====//


	//=================STEP 4. Dictionary File의 논리적인 테이블 갯수를 체크한다. (BEGIN)========================//
	{
		unsigned long TempTableCount;				//디비파일을 검색해서 나온 테이블의 갯수를 저장할 변수
		
		//==============STEP 4. 1. Dictionary Memory에 Dictionary File을 올려 놓는다.
		g_pCheck_data->fpDbDictionaryFile = DICTIONARY_FILE_OPEN;
		{
			#if (TFS4_BASE_STAND_IO==1)
			dbUtilFileRead((unsigned long)g_pCheck_data->fpDbDictionaryFile, 
						    (unsigned char*)(g_pCheck_data->pDicStart), 
						    vDictionaryFileHeader.dictionaryHeaderSizeOfDictionary);
			#endif

			#if (ANSI_BASE_STAND_IO==1)
			vReadingCount = fread(g_pCheck_data->pDicStart, 
								vDictionaryFileHeader.dictionaryHeaderSizeOfDictionary, 1,
								g_pCheck_data->fpDbDictionaryFile);
			#endif
		}
		DICTIONARY_FILE_CLOSE;
		g_pCheck_data->fpDbDictionaryFile = FILE_HANDLE_NULL;
	
		#if (ANSI_BASE_STAND_IO==1)
		if (vReadingCount!=1)					//error 처리
		{									//error 처리
			return DB_FILE_ERROR;			//error 처리
		}									//error 처리
		#endif

		//==============STEP 4. 2. Dictionary File에서 존재하는 테이블의 갯수를 파라메터로 받은 테이블의 갯수와 비교를 해본다.
		TempTableCount=0;
		if (dbCheckTablesCount(	vDictionaryFileHeader.dictionaryHeaderOffsetOfRootTable, 
								&TempTableCount, 
								vTableCount, 
								vLogicalDictionaryFileSize)==DB_FILE_ERROR)
		{
			return DB_FILE_ERROR;
		}
		else
		{
			if (TempTableCount !=vTableCount)
			{
				return DB_FILE_ERROR;
			}
		}
	}
	//=================STEP 4. Dictionary File의 논리적인 테이블 갯수를 체크한다. (END)========================//
	return DB_FILE_NO_ERROR;
}


int dbCheckTablesCount(unsigned long vOffset, unsigned long *pCurrentTableCount, unsigned long TableCount, unsigned long vLogicalDictionaryFileSize)
{
	pTableNode	pTable;
	TableNode	vTable;
		
	if (vOffset == 0)
	{
		return DB_FILE_NO_ERROR;
	}
	else
	{
		(*pCurrentTableCount)++; //Table의 갯수를 증가시킨다.

		if ((*pCurrentTableCount) > TableCount)
		{
			return DB_FILE_ERROR;
		}

		//해당 table을 포인팅 한다.
		pTable = (pTableNode)(g_pCheck_data->pDicStart + vOffset);
		dbUtilMemcpyFreeIndian(pTable, &vTable, sizeof(unsigned long), 6);


		//해당 Table에 대한 처리를 한다.
		if (dbCheckTableRecords(&vTable)==DB_FILE_ERROR)
		{
			return DB_FILE_ERROR;
		}

		

		//트리의 왼쪽 subtree에서 갯수를 센다.
		if (dbCheckTablesCount(	vTable.tableNodeLeft,
								pCurrentTableCount,
								TableCount,
								vLogicalDictionaryFileSize)==DB_FILE_ERROR)
		{
			return DB_FILE_ERROR;
		}
		
		//트리의 오른쪽 subtree에서 갯수를 센다.
		if (dbCheckTablesCount(	vTable.tableNodeRight,
								pCurrentTableCount,
								TableCount,
								vLogicalDictionaryFileSize)==DB_FILE_ERROR)
		{
			return DB_FILE_ERROR;
		}

		return DB_FILE_NO_ERROR;
	}

}

int dbCheckTableRecords(pTableNode pTable)
{
//이 함수는 해당 테이블의 내용이 논리적으로 이상이 없는지를 검사하는 함수이다.
//시간관계상 이 부분은 추후에 구현한다.
//nsigned long vCurrentLoadingPage = pTable->tableNodeStartPage;
return DB_FILE_NO_ERROR;
}

int dbCheckMemoryAlloc(char* pDictionaryFileName, char* pDataFileName, char* pIndexFileName,
						      unsigned long vDictionaryMemorySize, unsigned long vDataPageSize, unsigned long vIndexPageSize)
{
	if (g_pCheck_data == 0)
	{
		g_pCheck_data = (struct Check_Data*)malloc(sizeof(struct Check_Data));

		if (g_pCheck_data == 0)
		{
			return DB_CHECK_ERROR_MEMORY_DISALLOC;
		}
		else
		{
			memset(g_pCheck_data, 0, sizeof(g_pCheck_data));

			g_pCheck_data->vDictionarySize		= vDictionaryMemorySize;
			g_pCheck_data->vDataFilePageSize		= vDataPageSize;
			g_pCheck_data->vIndexFilePageSize	= vIndexPageSize;
			g_pCheck_data->pDicStart 			= (char*)malloc(vDictionaryMemorySize);
			g_pCheck_data->pDatStart 			= (char*)malloc(vDataPageSize);
			g_pCheck_data->pIdxStart 			= (char*)malloc(vIndexPageSize);
			g_pCheck_data->pDicFileName			= pDictionaryFileName;
			g_pCheck_data->pDatFileName			= pDataFileName;
			g_pCheck_data->pIdxFileName			= pIndexFileName;

			if ((g_pCheck_data->pDicStart==0)||(g_pCheck_data->pDatStart==0)||(g_pCheck_data->pIdxStart==0))
			{
				if (g_pCheck_data->pDicStart != 0)
					free(g_pCheck_data->pDicStart);
				if (g_pCheck_data->pDatStart!=0)
					free(g_pCheck_data->pDatStart);
				if (g_pCheck_data->pIdxStart!=0)
					free(g_pCheck_data->pIdxStart);

				free(g_pCheck_data);
				g_pCheck_data = 0;
				
				return DB_CHECK_ERROR_MEMORY_DISALLOC;
			}
			else
			{
				return DB_CHECK_ERROR_MEMORY_ALLOC;
			}
		}
	}
}

void dbCheckMemoryFree(void)
{
	if (g_pCheck_data !=0)
	{
		if (g_pCheck_data->pDicStart != 0)
			free(g_pCheck_data->pDicStart);
		if (g_pCheck_data->pDatStart!=0)
			free(g_pCheck_data->pDatStart);
		if (g_pCheck_data->pIdxStart!=0)
			free(g_pCheck_data->pIdxStart);

		free(g_pCheck_data);
		g_pCheck_data = 0;
	}
}

int dbApiUtilCheckDatabaseFiles(char* pDictionaryFileName, char* pDataFileName, char* pIndexFileName, 
									unsigned long vLogicalDictionaryFileSize, unsigned long vDataPageSize, unsigned long vIndexPageSize,
									unsigned long vTableCount)
{
	int vDictionaryFileError;
	int vDataFileError;
	int vIndexFileError;

	//check 0. Database file을 check하는 루틴이 돌기 위하여 전역변수로 사용하는 구조체의 값을 채운다.
	if (dbCheckMemoryAlloc(	pDictionaryFileName, 
							pDataFileName, 
							pIndexFileName, 
							vLogicalDictionaryFileSize, 
							vDataPageSize, 
							vIndexPageSize) ==DB_CHECK_ERROR_MEMORY_DISALLOC)
	{
		dbCheckMemoryFree();
		return DB_FILE_ERROR;
	}

	
	//check 1. Database File이 물리적으로 존재하는지 열리는지에 대한 Check를 실시한다.
	if (dbCheckDatabaseFilesOpen(pDictionaryFileName, pDataFileName, pIndexFileName) == DB_FILE_ERROR)
	{
		dbCheckMemoryFree();
		return DB_FILE_ERROR;
	}

	//check 2. Dictionary file을 Check한다.
	if (dbCheckDatabaseDictionaryFile(pDictionaryFileName, vLogicalDictionaryFileSize, vTableCount) == DB_FILE_ERROR)
	{
		dbCheckMemoryFree();
		return DB_FILE_ERROR;
	}

	dbCheckMemoryFree();
	return DB_FILE_NO_ERROR;
}

