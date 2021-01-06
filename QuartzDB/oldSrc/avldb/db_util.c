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
extern unsigned char g_MBuf[65536]; //jaihong.kim noncache ������ (E10)
#endif

static void dbUtilFileRead(unsigned long lDbFileHandle, unsigned char *pReadBuf, signed long lTotalReadSize)
{
	unsigned long lReadSize;
	
#ifdef CONFIG_HDD //jaihong.kim noncache ������ (E10)

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
	//���� �ε�� ü���̹Ƿ� �ٲܰ��� ����.
	return;
#else
	//�ٸ��ε�� ü���̹Ƿ� ��ȯ�� �ؾ� �Ѵ�.
	char				TempUnit;
	
	if (vUnitSize == sizeof(char))
	{
		//�ٸ��ε�� ü��� ��ȯ�� �ؾ� �ϴµ� ��ȯ�Ϸ��� unit�� ����� 1�� ��� �ε�� Ư���� Ÿ�� �����Ƿ� ��ȯ�� ���� �ʴ´�.
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
	//���� �ڵ��� ���� �ʱ�ȭ �Ѵ�.
	g_pCheck_data->fpDbDictionaryFile	= FILE_HANDLE_NULL;
	g_pCheck_data->fpDbDataFile		= FILE_HANDLE_NULL;
	g_pCheck_data->fpDbIndexFile		= FILE_HANDLE_NULL;


	//��������� ����.
	g_pCheck_data->fpDbDictionaryFile	= DICTIONARY_FILE_OPEN;
	g_pCheck_data->fpDbDataFile		= DATA_FILE_OPEN;
	g_pCheck_data->fpDbIndexFile		= INDEX_FILE_OPEN;

	//�ڵ鰪�� FILE_HANDLE_NULL���� ���� ���� �����ϸ� ��������� open�ϴµ� ������ �߻��� ���
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

		//Error�� �����Ѵ�.
		return DB_FILE_ERROR;
	}
	//�ڵ鰪�� FILE_HANDLE_NULL���� ���� ���� ���������ʴ´ٸ� �װ��� ���������� ������ �����ϰ� ��������̴�.
	else
	{
		DICTIONARY_FILE_CLOSE;
		DATA_FILE_CLOSE;
		INDEX_FILE_CLOSE;
		
		g_pCheck_data->fpDbDictionaryFile	= FILE_HANDLE_NULL;
		g_pCheck_data->fpDbDataFile		= FILE_HANDLE_NULL;
		g_pCheck_data->fpDbIndexFile		= FILE_HANDLE_NULL;

		//�������� ���Ͽ� �̻��� ������ �����Ѵ�.
		return DB_FILE_NO_ERROR;
	}
}

int dbCheckDatabaseDictionaryFile(char* pDictionaryFileName, unsigned long vLogicalDictionaryFileSize, unsigned long vTableCount)
{
	#if (ANSI_BASE_STAND_IO==1)
	int 	vReadingCount;		//ERRORó���� ���� ��������
	#endif
	DictionaryHeader	vDictionaryFileHeader;

	memset(&vDictionaryFileHeader, 0, sizeof(vDictionaryFileHeader));

	//=================STEP 1. ��ųʸ� ������ ����. (BEGIN)=========================================//
	if (g_pCheck_data->fpDbDictionaryFile != FILE_HANDLE_NULL)
	{
		DICTIONARY_FILE_CLOSE;
		g_pCheck_data->fpDbDictionaryFile = FILE_HANDLE_NULL;
	}
	
	g_pCheck_data->fpDbDictionaryFile = DICTIONARY_FILE_OPEN;
	if (g_pCheck_data->fpDbDictionaryFile == FILE_HANDLE_NULL)	return DB_FILE_ERROR;	//error ó��
	//=================STEP 1. ��ųʸ� ������ ����. (END)===========================================//	


	//=================STEP 2. ��ųʸ� �ش��� �о�´�. (BEGIN)=======================================//
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

	#if (ANSI_BASE_STAND_IO==1)					//error ó��
	if (vReadingCount!=1)	return DB_FILE_ERROR;	//error ó��
	#endif										//error ó��
	//=================STEP 2. ��ųʸ� �ش��� �о�´�. (END)=========================================//


	//=================STEP 3. ��ųʸ� �ش��� ���� ������ Errorüũ�� �õ��Ѵ�. (BEGIN)(Ư�� ���� ���� �����ϴ������� �˻��Ѵ�.)====//
	//=================STEP 3. 1. �׻� 0�� ���� ������ �ϴ� �κ��� 0�� ������ �о��������� Ȯ���Ѵ�.
	if ((vDictionaryFileHeader.dictionaryHeaderDeleteNode != 0) ||(vDictionaryFileHeader.dictionaryHeaderDeleteStartPage != 0))
	{
		return DB_FILE_ERROR;
	}
	
	//=================STEP 3. 2. ���̺��� root���� ���� ���� �ִ��� �˻��Ѵ�.
	if ((vDictionaryFileHeader.dictionaryHeaderOffsetOfRootTable < sizeof(DictionaryHeader)) 
		|| (vDictionaryFileHeader.dictionaryHeaderOffsetOfRootTable>=vLogicalDictionaryFileSize))
	{
		return DB_FILE_ERROR;
	}

	//=================STEP 3. 3. ��ųʸ� ������ ũ�Ⱑ ���� ������ ���� ���� �������� �Ǵ��Ѵ�.
	if (vDictionaryFileHeader.dictionaryHeaderSizeOfDictionary!=vLogicalDictionaryFileSize)
	{
		return DB_FILE_ERROR;
	}

	//=================STEP 3. 4. Data ������ ũ�Ⱑ ���� ���� �ִ��� �˻��Ѵ�.
	if (vDictionaryFileHeader.dictionaryHeaderNumberOfDataFileEndPage > 1000) 
	{
		return DB_FILE_ERROR;
	}
	
	//=================STEP 3. 5. Index ������ ũ�Ⱑ ���� ���� �ִ��� �˻��Ѵ�.
	if (vDictionaryFileHeader.dictionaryHeaderNumberOfDataFileEndPage > 1000) 
	{
		return DB_FILE_ERROR;
	}
	//=================STEP 3. ��ųʸ� �ش��� ���� ������ Errorüũ�� �õ��Ѵ�. (END)(Ư�� ���� ���� �����ϴ������� �˻��Ѵ�.)====//


	//=================STEP 4. Dictionary File�� ������ ���̺� ������ üũ�Ѵ�. (BEGIN)========================//
	{
		unsigned long TempTableCount;				//��������� �˻��ؼ� ���� ���̺��� ������ ������ ����
		
		//==============STEP 4. 1. Dictionary Memory�� Dictionary File�� �÷� ���´�.
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
		if (vReadingCount!=1)					//error ó��
		{									//error ó��
			return DB_FILE_ERROR;			//error ó��
		}									//error ó��
		#endif

		//==============STEP 4. 2. Dictionary File���� �����ϴ� ���̺��� ������ �Ķ���ͷ� ���� ���̺��� ������ �񱳸� �غ���.
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
	//=================STEP 4. Dictionary File�� ������ ���̺� ������ üũ�Ѵ�. (END)========================//
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
		(*pCurrentTableCount)++; //Table�� ������ ������Ų��.

		if ((*pCurrentTableCount) > TableCount)
		{
			return DB_FILE_ERROR;
		}

		//�ش� table�� ������ �Ѵ�.
		pTable = (pTableNode)(g_pCheck_data->pDicStart + vOffset);
		dbUtilMemcpyFreeIndian(pTable, &vTable, sizeof(unsigned long), 6);


		//�ش� Table�� ���� ó���� �Ѵ�.
		if (dbCheckTableRecords(&vTable)==DB_FILE_ERROR)
		{
			return DB_FILE_ERROR;
		}

		

		//Ʈ���� ���� subtree���� ������ ����.
		if (dbCheckTablesCount(	vTable.tableNodeLeft,
								pCurrentTableCount,
								TableCount,
								vLogicalDictionaryFileSize)==DB_FILE_ERROR)
		{
			return DB_FILE_ERROR;
		}
		
		//Ʈ���� ������ subtree���� ������ ����.
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
//�� �Լ��� �ش� ���̺��� ������ �������� �̻��� �������� �˻��ϴ� �Լ��̴�.
//�ð������ �� �κ��� ���Ŀ� �����Ѵ�.
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

	//check 0. Database file�� check�ϴ� ��ƾ�� ���� ���Ͽ� ���������� ����ϴ� ����ü�� ���� ä���.
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

	
	//check 1. Database File�� ���������� �����ϴ��� ���������� ���� Check�� �ǽ��Ѵ�.
	if (dbCheckDatabaseFilesOpen(pDictionaryFileName, pDataFileName, pIndexFileName) == DB_FILE_ERROR)
	{
		dbCheckMemoryFree();
		return DB_FILE_ERROR;
	}

	//check 2. Dictionary file�� Check�Ѵ�.
	if (dbCheckDatabaseDictionaryFile(pDictionaryFileName, vLogicalDictionaryFileSize, vTableCount) == DB_FILE_ERROR)
	{
		dbCheckMemoryFree();
		return DB_FILE_ERROR;
	}

	dbCheckMemoryFree();
	return DB_FILE_NO_ERROR;
}

