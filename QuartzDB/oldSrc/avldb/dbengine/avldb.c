#include	"db3_basictype_define.h"
#include	"avldb.h"
#include	"db3_error_code.h"
#include "uart_api.h"
#include "ffilesys.h"
//#include <ucos_ii.h>
#include 	"..\..\usb\mtpdevice\mtpconfig.h"

///////////////////////////////////////////////////////////////
struct  mString{ 
	unsigned char		NumChars;
	unsigned short	StringChars[255];
};
////////////////////////////////////////////////////////////////

struct DB_SEARCH_DATA
{
	unsigned long dbSearchCount;
	unsigned long *ResultArray;
	unsigned long ResultArraySize;
	unsigned long vRemainRecordsCount;
	unsigned char refMemory;
};
//#include	"malloc.h"


//########## �������� ����(begin) ##########//
char			*gDbFileNameDictionaryFileName;	//��񿡼� ����ϴ� ��ųʸ� ������ �̸�
char			*gDbFileNameDataFileName;		//��񿡼� ����ϴ� ������ ������ �̸�
char			*gDbFileNameIndexFileName;		//��񿡼� ����ϴ� �ε��� ������ �̸�


unsigned long	gDbAreaSizeDictionaryAreaSize;	//����� Area�߿� ��ųʸ��� ����ϴ� �޸��� ũ��
unsigned long	gDbAreaSizeDataAreaSize;		//����� Area�߿� �����Ͱ� ����ϴ� �޸��� ũ��
unsigned long	gDbAreaSizeIndexAreaSize;		//����� Area�߿� �ε����� ����ϴ� �޸��� ũ��

unsigned long	gDbPageSizeDataFilePageSize;	//��񿡼� ����ϴ� ������ ������ Page Size
unsigned long	gDbPageSizeIndexFilePageSize;	//��񿡼� ����ϴ� �ε��� ������ Page Size


char			*gDbAreaDictionaryAreaStart;	//��񿡼� ����ϴ� Area�� Dictionary Area�� Start Address
char			*gDbAreaDataAreaStart;			//��񿡼� ����ϴ� Area�� Data Area�� Start Address
char			*gDbAreaIndexAreaStart;			//��񿡼� ����ϴ� Area�� Index Area�� Start Address
char			*gDbMmuAreaDataMmuStart;		//��񿡼� ����ϴ� Area�� Data MMU Area�� Start Address
char			*gDbMmuAreaIndexMmuStart;		//��񿡼� ����ϴ� Area�� Index MMU Area�� Start Address

unsigned char	gDbSameIndian;					//��� ������ �ý����� �ε�� ü��
pMMNODE	gDbMmuDataPage;					//��񿡼� ����ϴ� �޸� ���� ������ ������ Page�� �����ϴ� MMU�� ���� ������
pMMNODE	gDbMmuIndexPage;				//��񿡼� ����ϴ� �޸� ���� ������ �ε��� Page�� �����ϴ� MMU�� ���� ������
unsigned char	gDbStartUpDB = DB_DO_NOT_RUNNING;	//��� ���� ���������� �ƴ����� �����ϴ� ����

unsigned long	dbSearchCount;				//�˻��� ����� ������ ����
unsigned long	*dbSearchArray;
unsigned char	gDbDictionaryChanged;		//��ųʸ��� ���濩�� �����ϴ� ���º���

void 		(*gFpMemcpy)(void*, void*, unsigned long, unsigned long);

struct DB_SEARCH_DATA gDbSearchData;
//########## �������� ����( end ) ##########//


#define  	AVLDB_DEBUG  		0 
#define     DB_FILE_DEBUG		0			// 060123woody
//#define 	DB_DEBUG	1

#ifdef	DB_DEBUG
void showDicMemory(unsigned long userDefine)
{
	char* baseAddress;
	unsigned long i;
	
	baseAddress = gDbAreaDictionaryAreaStart;

	DbgPrintf(("=========== ��ųʸ� �޸��� ��========== "));
	for(i=0;i<userDefine; i++)
	{
		if (i%16==0) DbgPrintf(("\n[%x] ", baseAddress));
		if ((*baseAddress)>15) DbgPrintf((" %x ,", *baseAddress));
		else DbgPrintf(("  %x ,", *baseAddress));
		baseAddress++;
	}
}

void showDatMemory(unsigned long userDefine)
{
	char* baseAddress;
	unsigned long i;
	
	baseAddress = gDbAreaDataAreaStart;

	DbgPrintf(("===========����Ÿ �޸��� ��========== "));
	for(i=0;i<userDefine; i++)
	{
		if (i%16==0) DbgPrintf(("\n[%x] ", baseAddress));
		if ((*baseAddress)>15) DbgPrintf((" %x ,", *baseAddress));
		else DbgPrintf(("  %x ,", *baseAddress));
		baseAddress++;
	}
}


void showTwoByteString(char *pMessage  ,void *source)
{
	unsigned short	vBuffer;
	char				*pSource;
	pSource	= (char*)source;

	dbMemcpyFreeIndian(pSource, &vBuffer, 2, 1);

	DbgPrintf(("%s\n", pMessage));
	
	if (source==0)
	{
		//DbgPrintf(("������ ��� �ֽ��ϴ�."));
	}
	else
	{
	    while(vBuffer)
	    {
		    DbgPrintf((" %x ", vBuffer));	
		    pSource+=2;
		    dbMemcpyFreeIndian(pSource, &vBuffer, 2, 1);
	    }
	}
	DbgPrintf(("\n"));
}

#endif


void dbInitialSetting(void)
{
	gDbFileNameDictionaryFileName	= DB_NULL;	//��񿡼� ����ϴ� ��ųʸ� ������ �̸�
	gDbFileNameDataFileName		= DB_NULL;	//��񿡼� ����ϴ� ������ ������ �̸�
	gDbFileNameIndexFileName		= DB_NULL;	//��񿡼� ����ϴ� �ε��� ������ �̸�


	gDbAreaSizeDictionaryAreaSize	= DB_NULL;	//����� Area�߿� ��ųʸ��� ����ϴ� �޸��� ũ��
	gDbAreaSizeDataAreaSize		= DB_NULL;	//����� Area�߿� �����Ͱ� ����ϴ� �޸��� ũ��
	gDbAreaSizeIndexAreaSize		= DB_NULL;	//����� Area�߿� �ε����� ����ϴ� �޸��� ũ��

	gDbPageSizeDataFilePageSize	= DB_NULL;	//��񿡼� ����ϴ� ������ ������ Page Size
	gDbPageSizeIndexFilePageSize	= DB_NULL;	//��񿡼� ����ϴ� �ε��� ������ Page Size


	gDbAreaDictionaryAreaStart		= DB_NULL;	//��񿡼� ����ϴ� Area�� Dictionary Area�� Start Address
	gDbAreaDataAreaStart			= DB_NULL;	//��񿡼� ����ϴ� Area�� Data Area�� Start Address
	gDbAreaIndexAreaStart			= DB_NULL;	//��񿡼� ����ϴ� Area�� Index Area�� Start Address
	gDbMmuAreaDataMmuStart		= DB_NULL;	//��񿡼� ����ϴ� Area�� Data MMU Area�� Start Address
	gDbMmuAreaIndexMmuStart		= DB_NULL;	//��񿡼� ����ϴ� Area�� Index MMU Area�� Start Address

	gDbSameIndian					= DB_NULL;				//��� ������ �ý����� �ε�� ü��
	gDbMmuDataPage				= DB_NULL;				//��񿡼� ����ϴ� �޸� ���� ������ ������ Page�� �����ϴ� MMU�� ���� ������
	gDbMmuIndexPage				= DB_NULL;				//��񿡼� ����ϴ� �޸� ���� ������ �ε��� Page�� �����ϴ� MMU�� ���� ������
	gDbStartUpDB					= DB_DO_NOT_RUNNING;	//��� ���� ���������� �ƴ����� �����ϴ� ����

	dbSearchCount					= DB_NULL;				//�˻��� ����� ������ ����
	dbSearchArray 					= DB_NULL;
	gDbDictionaryChanged			= DB_UNCHANGED;		//��ųʸ��� ���濩�� �����ϴ� ���º���
}


int dbApiGetDictionaryFileSize(unsigned long *pFileSize)
{
	if (gDbStartUpDB == DB_DO_RUNNING)
	{
		//DB�� Startup�� �� ��Ȳ������ �����Ѵ�.
		pDictionaryHeader		pdicHeader;
		DictionaryHeader		vDicHeader;

		//��ųʸ� �ش��� �޾ƿ´�.
		pdicHeader = dbGetDictionaryHeader();
		dbMemcpyFreeIndian(pdicHeader, &vDicHeader, sizeof(unsigned long), 6);
		
		*pFileSize = vDicHeader.dictionaryHeaderSizeOfDictionary;
		
		return DB_FUNCTION_SUCCESS;
	}
	else
	{
		return DB_FUNCTION_FAIL;
	}
}

int dbApiGetDataFileSize(unsigned long *pFileSize)
{
	if (gDbStartUpDB == DB_DO_RUNNING)
	{
		//DB�� Startup�� �� ��Ȳ������ �����Ѵ�.
		pDictionaryHeader		pdicHeader;
		DictionaryHeader		vDicHeader;

		//��ųʸ� �ش��� �޾ƿ´�.
		pdicHeader = dbGetDictionaryHeader();
		dbMemcpyFreeIndian(pdicHeader, &vDicHeader, sizeof(unsigned long), 6);
		
		*pFileSize = vDicHeader.dictionaryHeaderNumberOfDataFileEndPage*gDbPageSizeDataFilePageSize;

		return DB_FUNCTION_SUCCESS;
	}
	else
	{
		return DB_FUNCTION_FAIL;
	}

}

int dbApiGetIndexFileSize(unsigned long *pFileSize)
{
	if (gDbStartUpDB == DB_DO_RUNNING)
	{
		//DB�� Startup�� �� ��Ȳ������ �����Ѵ�.
		pDictionaryHeader		pdicHeader;
		DictionaryHeader		vDicHeader;

		//��ųʸ� �ش��� �޾ƿ´�.
		pdicHeader = dbGetDictionaryHeader();
		dbMemcpyFreeIndian(pdicHeader, &vDicHeader, sizeof(unsigned long), 6);
		
		*pFileSize = vDicHeader.dictionaryHeaderNumberOfDataFileEndPage*gDbPageSizeIndexFilePageSize;

		return DB_FUNCTION_SUCCESS;
	}
	else
	{
		return DB_FUNCTION_FAIL;
	}


}

int dbApiGetFileTotalSize(unsigned long *pFileSize)
{
	unsigned long vTempSize;

	//Dictionary file�� Size�� �޾ƿ´�.
	if (dbApiGetDictionaryFileSize(&vTempSize) ==DB_FUNCTION_FAIL)
	{
		return DB_FUNCTION_FAIL;
	}
	*pFileSize = vTempSize;

	//Data file�� Size�� �޾ƿ´�.	
	if (dbApiGetDataFileSize(&vTempSize) ==DB_FUNCTION_FAIL)
	{
		return DB_FUNCTION_FAIL;
	}
	*pFileSize += vTempSize;
	
	//Index file�� Size�� �޾ƿ´�.
	if (dbApiGetIndexFileSize(&vTempSize) ==DB_FUNCTION_FAIL)
	{
		return DB_FUNCTION_FAIL;
	}
	*pFileSize += vTempSize;
	
	return DB_FUNCTION_SUCCESS;

}

unsigned long  dbApiStartUp(char			*dbDictionaryFileName,
							char			*dbDataFileName,
							char			*dbIndexFileName,
							char			*dbAreaDictionaryAreaStart,
							char			*dbAreaDataAreaStart,
							char			*dbAreaIndexAreaStart,
							char			*dbMmuAreaDataMmuStart,
							char			*dbMmuAreaIndexMmuStart,
							unsigned long	dbAreaSizeDictionaryAreaSize,
							unsigned long	dbAreaSizeDataAreaSize,
							unsigned long	dbAreaSizeIndexAreaSize,
							unsigned long	dbPageSizeDataFilePageSize,
							unsigned long	dbPageSizeIndexFilePageSize,
							unsigned char	dbSameIndian,
							unsigned long vDictionaryLogicalSize)
{
	#if DB_SHOW_ERROR_CODE			//Error Check
	gDbErrorCode = DB_ERROR_NO_ERROR;	//Error Check
	#endif								//Error Check

	if (gDbStartUpDB == DB_DO_NOT_RUNNING)
	{
		//Database�� ����� Indian ü�迡 ���� �Լ��� Mapping�Ѵ�.(being)
		gDbSameIndian = dbSameIndian;

		if (gDbSameIndian == DB_ORDER_INDIAN)
		{
			gFpMemcpy = dbMemcpyOrderIndian;
		}
		else if(gDbSameIndian == DB_SAME_INDIAN)
		{
			gFpMemcpy = dbMemcpySameIndian;
		}
		else
		{
			#if DB_SHOW_ERROR_CODE					//Error Check
			gDbErrorCode = DB_ERROR_INDIAN_INCORRECT;	//Error Check
			#endif										//Error Check
		}
		//Database�� ����� Indian ü�迡 ���� �Լ��� Mapping�Ѵ�.( end )
		
		//������ ���̽��� ����� ȭ���� �����Ѵ�.
		gDbFileNameDictionaryFileName	= dbDictionaryFileName;
		gDbFileNameDataFileName		= dbDataFileName;
		gDbFileNameIndexFileName		= dbIndexFileName;

		//������ ���̽��� Area�� ���� Address����
		gDbAreaDictionaryAreaStart		= dbAreaDictionaryAreaStart;
		gDbAreaDataAreaStart			= dbAreaDataAreaStart;
		gDbAreaIndexAreaStart			= dbAreaIndexAreaStart;
		gDbMmuAreaDataMmuStart		= dbMmuAreaDataMmuStart;
		gDbMmuAreaIndexMmuStart		= dbMmuAreaIndexMmuStart;


		//����� Area�� ũ�⸦ �����Ѵ�.
		gDbAreaSizeDictionaryAreaSize	= dbAreaSizeDictionaryAreaSize;
		gDbAreaSizeDataAreaSize		= dbAreaSizeDataAreaSize;
		gDbAreaSizeIndexAreaSize		= dbAreaSizeIndexAreaSize;


		//������ ���̽����� ����ϴ� ������ ���ϰ� �ε��� ������ Page Size�� �����Ѵ�.
		gDbPageSizeDataFilePageSize	= dbPageSizeDataFilePageSize;
		gDbPageSizeIndexFilePageSize	= dbPageSizeIndexFilePageSize;


		//������ ���̽��� ����ϴ� �޸𸮿� ���� ������ �Ѵ�.
		//dbInitMemorySetting(&gDbAreaDictionaryAreaStart,
		//					&gDbAreaDataAreaStart,
		//					&gDbAreaIndexAreaStart, 
		//					&gDbMmuAreaDataMmuStart,
		//					&gDbMmuAreaIndexMmuStart);
		//�޸� ���� ������ �����ϰ� �����Ѵ�.
		dbMmuCreate(&gDbMmuDataPage,			&gDbMmuIndexPage, 
					gDbMmuAreaDataMmuStart,	gDbMmuAreaIndexMmuStart,
					gDbAreaDataAreaStart,		gDbAreaIndexAreaStart);

		//������ ���̽� ������ Load�Ѵ�.
		if (dbLoadDatabaseFiles(gDbFileNameDictionaryFileName, gDbFileNameDataFileName,  gDbFileNameIndexFileName, vDictionaryLogicalSize) == DB_FUNCTION_FAIL)
		{
			//DBFile Loading�� ���и� �� ���
			return DB_DO_NOT_RUNNING;
		}
		gDbDictionaryChanged	= DB_UNCHANGED;
		gDbStartUpDB 			= DB_DO_RUNNING;
	}
	else
	{
		#if DB_SHOW_ERROR_CODE					//Error Check
		gDbErrorCode = DB_ERROR_ALREADY_RUNNING;	//Error Check
		#endif										//Error Check
	}
	return gDbStartUpDB;
}


pDictionaryHeader dbGetDictionaryHeader(void)
{
	return (pDictionaryHeader)gDbAreaDictionaryAreaStart;
}





void dbInitMemorySetting(char	**dbAreaDictionaryAreaStart,
						 char	**dbAreaDataAreaStart,
						 char	**dbAreaIndexAreaStart,
						 char	**dbMmuAreaDataMmuStart,
						 char	**dbMmuAreaIndexMmuStart)
{

	//Memory�� ���������͸� �Ҵ��Ѵ�.
//	*dbAreaDictionaryAreaStart	= (char*)malloc(gDbAreaSizeDictionaryAreaSize);
	
//	*dbAreaDataAreaStart		= (char*)malloc(gDbAreaSizeDataAreaSize);
	
//	*dbAreaIndexAreaStart		= (char*)malloc(gDbAreaSizeIndexAreaSize);
	
//	*dbMmuAreaDataMmuStart		= (char*)malloc(sizeof(MMNODE)*(gDbAreaSizeDataAreaSize/gDbPageSizeDataFilePageSize));
	
//	*dbMmuAreaIndexMmuStart		= (char*)malloc(sizeof(MMNODE)*(gDbAreaSizeIndexAreaSize/gDbPageSizeIndexFilePageSize));
/*
	// porting ���̵�//
	*dictionaryAreaStart	= (char*)DB_DICTIONARY_AREA_START;
	
	*dataAreaStart		= (char*)DB_DATA_AREA_SATAT;
	
	*indexAreaStart		= (char*)DB_INDEX_AREA_START;
	
	*mmuDatStart			= (char*)DB_GLOBAL_AREA_DATA_MMU_START;
	
	*mmuIdxStart			= (char*)DB_GLOBAL_AREA_INDEX_MMU_START;
*/
}

 
void dbMmuCreate(pMMNODE	*dbMmuDataPage,				//data area�� MMU�� ������ �������� ����
				 pMMNODE	*dbMmuIndexPage,			//index area�� MMU�� ������ �������� ����
				 char		*dbMmuAreaDataMmuStart,		//data area�� MMU�� ����Ǳ� ���� ������ ���۹���
				 char		*dbMmuAreaIndexMmuStart,	//index area�� MMU�� ����Ǳ� ���� ������ ���۹���
				 char		*dbAreaDataAreaStart,		//mmu�� ���� ������ data area�� ���۹���
				 char		*dbAreaIndexAreaStart)		//mmu�� ���� ������ index area�� ���۹���
{
	//�� �Լ��� �޸𸮸� �����ϱ� ���� MMU���� �����ϴ� ����̴�.
	unsigned long	i;					//counter�� ���� �ӽú���
	unsigned long	dbMmuDataMmuNum;	//DATA AREA�� �޸� ���� ���� ������ ������ ����
	unsigned long	dbMmuIndexMmuNum;	//INDEX AREA�� �޸� ���� ���� ������ ������ ����

	//Memory�� page�� �����ϱ� ���Ͽ� Index�� Data page�� � �ö�ü� �ִ��� ����Ѵ�.
	dbMmuDataMmuNum	= (gDbAreaSizeDataAreaSize)/(gDbPageSizeDataFilePageSize);
	dbMmuIndexMmuNum	= (gDbAreaSizeIndexAreaSize)/(gDbPageSizeIndexFilePageSize);

	*dbMmuDataPage  = (pMMNODE)dbMmuAreaDataMmuStart;
	*dbMmuIndexPage = (pMMNODE)dbMmuAreaIndexMmuStart;
	
	//Data Mmu�� �����Ѵ�.
	for (i = 0; i < dbMmuDataMmuNum; i++ )
	{
		((*dbMmuDataPage) + i)->mmuPageNumber		= 0;
		((*dbMmuDataPage) + i)->mmuChanged			= DB_UNCHANGED;
		((*dbMmuDataPage) + i)->mmuMemoryAddress	= dbAreaDataAreaStart + (i*(gDbPageSizeDataFilePageSize));
		((*dbMmuDataPage) + i)->mmuNext				= (i == (dbMmuDataMmuNum - 1)) ? 0 : ((*dbMmuDataPage) + i + 1);
	}

	//Index Mmu�� �����Ѵ�.
	for (i = 0; i < dbMmuIndexMmuNum; i++ )
	{
		((*dbMmuIndexPage) + i)->mmuPageNumber		= 0;
		((*dbMmuIndexPage) + i)->mmuChanged			= DB_UNCHANGED;
		((*dbMmuIndexPage) + i)->mmuMemoryAddress	= dbAreaIndexAreaStart + (i*(gDbPageSizeIndexFilePageSize));
		((*dbMmuIndexPage) + i)->mmuNext			= (i == (dbMmuIndexMmuNum - 1)) ? 0 : ((*dbMmuIndexPage) + i + 1);
	}
}



//################### ���ο� �ڵ��� ��� ��ȯ�ϸ鼭 �۾��ϴ� �κ� ( end )########################//

unsigned char dbGetLruListPage(pMMNODE	**start, 
								unsigned long	pageNumber)
{
	//MMU�� List���� �ش� �������� ã���� 1 ��ã���� 0�� �����Ѵ�.
	if ((**start) != DB_NULL)
	{
		while(1)
		{
			if ( (**start)->mmuPageNumber == 0)			return 0;
			
			if ( (**start)->mmuPageNumber == pageNumber) return 1;
			
			if ( (**start)->mmuNext == DB_NULL)				return 0;
			
			*start = &((**start)->mmuNext);
		}
	}
	return 0;
}

void dbMaintainLruList(pMMNODE	*start, 
					   pMMNODE	*lastAccessPage)
{
	//MMU�� ����Ʈ���� lastAccessPage�� ����Ʈ�� ���� �տ� ��ġ�Ѵ�.
	pMMNODE firstpage	= *lastAccessPage;
	*lastAccessPage	= (*lastAccessPage)->mmuNext;
	firstpage->mmuNext	= *start;
	*start				= firstpage;
}


void dbTouch(unsigned long offset, 
		      unsigned long filetype)
{
//==================================================================================================//
//	dbTouch �Լ��� �ش� offset�� ����Ǿ� �ִ� data page �Ǵ� index page�� �޸𸮿� load�����ְ�		//
//	������ �޸� ���� ��ũ�� ����Ʈ�� ����տ� ��ġ ���������ν�, datapage�� indexpage��� �κ�����//
//	�ڽ��� �޸𸮺��� �� ū index file�� data file����� �����ϰ� ���ش�.							//
//==================================================================================================//
	pMMNODE		*temp,
					*target;
	unsigned	long		pageNumber;	//���� ã���� �ϴ� offset�� ����Ǿ� �ִ� page��ȣ�� ������ ����


	if (filetype == DB_DATA_FILE_LOAD)		//datafile
	{
		pageNumber	= (offset/gDbPageSizeDataFilePageSize) + 1;
		temp		= &gDbMmuDataPage;			//datapage���� �׻� ã�� memory offset�� ���ذ��� ����.
		target		= &gDbMmuDataPage;
	}
	else if (filetype == DB_INDEX_FILE_LOAD)	//indexfile
	{
		pageNumber = (offset/gDbPageSizeIndexFilePageSize) + 1;
		temp		= &gDbMmuIndexPage;			//datapage���� �׻� ã�� memory offset�� ���ذ��� ����.
		target		= &gDbMmuIndexPage;
	}
	else{}


	if (dbGetLruListPage(&temp,pageNumber))
	{	//*temp���� ã�� Node�� ����.
		//memory page���� ã�� Page�� ������� : �����ϱ� ������ LRU List�� �����ϸ� ��
		dbMaintainLruList(target, temp);
	}
	else
	{
		//memory page���� ã�� Page�� ������� : LRU List�� �����ϰ� , page��ü�� �ؾ��Ѵ�.
		dbMaintainLruList(target, temp);
		
		if ((*target)->mmuChanged == DB_UNCHANGED)	//��ü�� �Ϸ��� Page�� ����� ������ ���� ���
		{
			(*target)->mmuPageNumber = pageNumber;
			dbSetFileLoadMemorypage(pageNumber, (*target)->mmuMemoryAddress, filetype);
			(*target)->mmuChanged= DB_UNCHANGED;
		}
		else	//��ü�� �Ϸ��� Page�� ����� ������ �ִ� ���
		{
			//�̸� �ö�� �����ϰ�
			if (dbSetFileStoreMemorypage((*target)->mmuPageNumber, (*target)->mmuMemoryAddress, filetype)==DB_FUNCTION_SUCCESS)
			{
				//page�� load�Ѵ�.
				(*target)->mmuPageNumber = pageNumber;
				dbSetFileLoadMemorypage(pageNumber, (*target)->mmuMemoryAddress, filetype);
			}
			else
			{
				dbTouch(offset,  filetype);
			}
		}
	}
}



void dbMemcpyFreeIndian(void		*source, 
						void			*target, 
						unsigned long	unitSize, 
						unsigned long	repeat)
{
	(*gFpMemcpy)(source, target, unitSize, repeat);
}

void dbMemcpyOrderIndian(void			*pSource, 
							   void			*pTarget, 
							   unsigned long	dbUnitSize, 
							   unsigned long	dbRepeat)
{
	unsigned long vUnitSize;

	while(dbRepeat)
	{
		vUnitSize = dbUnitSize;
		
		while(vUnitSize)
		{
			*(((char*)pTarget) + dbUnitSize - vUnitSize)= *(((char*)pSource) + vUnitSize - 1);

			vUnitSize--;
		}
		
		pSource = ((char*)pSource) + dbUnitSize;
		pTarget = ((char*)pTarget) + dbUnitSize;

		dbRepeat--;
	}
}




void dbMemcpySameIndian(void		     	*pSource, 
							   void			*pTarget, 
							   unsigned long	dbUnitSize, 
							   unsigned long	dbRepeat)
{
	unsigned long vCopySize;

	vCopySize = dbUnitSize*dbRepeat;
	
	while(vCopySize)
	{
		vCopySize--;
		*(((char*)pTarget) + vCopySize)= *(((char*)pSource) + vCopySize);
	}
}

void dbApiFileFullMemoryLoad(unsigned long vDataPageTotalNum, unsigned long vIndexPageTotalNum)
{
	pDictionaryHeader		pdicHeader;
	DictionaryHeader		vDicHeader;
	unsigned long			vDataOffset		= 0;
	unsigned long			vIndexOffset	= 0;
	
	if (gDbStartUpDB != DB_DO_RUNNING)
	{
		#if DB_SHOW_ERROR_CODE
			gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;
		#endif
		return;
	}

	//��ųʸ� �ش��� �޾ƿ´�.
	pdicHeader = dbGetDictionaryHeader();

	dbMemcpyFreeIndian(pdicHeader, &vDicHeader, sizeof(unsigned long), 6);

	//Memory�� Load�� �� �ִ� Page�� ���� ������ �����Ǿ� �ִ� Page�� ���߿� ���� ���� ���Ѵ�.
	{
		if (vDicHeader.dictionaryHeaderNumberOfDataFileEndPage < vDataPageTotalNum)
		{
			vDataPageTotalNum = vDicHeader.dictionaryHeaderNumberOfDataFileEndPage;
		}
		if (vDicHeader.dictionaryHeaderNumberOfIndexFileEndPage < vIndexPageTotalNum)
		{
			vIndexPageTotalNum = vDicHeader.dictionaryHeaderNumberOfIndexFileEndPage;
		}
	}

	//Data file�� Index file�� load�Ѵ�
	{
		while(vDataPageTotalNum)
		{
			vDataPageTotalNum--;
			vDataOffset	= gDbPageSizeDataFilePageSize*vDataPageTotalNum 
						+ (gDbPageSizeDataFilePageSize/2) ;
			dbTouch(vDataOffset, DB_DATA_FILE_LOAD);
		}
		
		while(vIndexPageTotalNum)
		{
			vIndexPageTotalNum--;
			vIndexOffset  	= gDbPageSizeIndexFilePageSize*vIndexPageTotalNum 
						+ (gDbPageSizeIndexFilePageSize/2) ;
			dbTouch(vIndexOffset, DB_INDEX_FILE_LOAD);
		}
	}
}

void dbApiCreateTable(DB_VARCHAR *tablename)
{
	pDictionaryHeader		pdicHeader;

#if DB_SHOW_ERROR_CODE
	gDbErrorCode = DB_ERROR_NO_ERROR;
#endif

//==============ERROR Code (Begin)==============//
if (gDbStartUpDB != DB_DO_RUNNING)
{
	#if DB_SHOW_ERROR_CODE
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;
	#endif
	return;
}
//==============ERROR Code ( end )==============//


	//��ųʸ� �ش��� �޾ƿ´�.
	pdicHeader = dbGetDictionaryHeader();
	
	//�̰��� ������ ���� ��带 �������Ŀ� �� ���� ptable�� �����ϸ�ȴ�.
	dbTableSetCreateTab(&(pdicHeader->dictionaryHeaderOffsetOfRootTable), tablename);

	//dictionary�� ������ �߻��� ���� üũ
	gDbDictionaryChanged = DB_CHANGED;
}

void dbTableSetNode(char				*target, 
					   DB_VARCHAR		*tablename)
{
	TableNode				TABLE_NODE;

	//���̺� ��带 �����Ѵ�.
	TABLE_NODE.tableNodeComplexIndexOffset	= 0;
	TABLE_NODE.tableNodeLeft				= 0;
	TABLE_NODE.tableNodeRight				= 0;
	TABLE_NODE.tableNodeColumnOffset		= 0;
	TABLE_NODE.tableNodeStartPage			= 0;
	TABLE_NODE.tableNodeHeight				= 1;

	//���̺� ��带 ����Ѵ�.
	dbMemcpyFreeIndian(&TABLE_NODE, target, sizeof(unsigned long), 6);

	//���̺��� �̸��� ����Ѵ�.
	dbStringCopy((DB_VARCHAR*)(target + sizeof(TableNode)), tablename);
}

unsigned long dbStringCopy(DB_VARCHAR* target, 
							 DB_VARCHAR* string)
{
	unsigned long		vStringSize = 0;
	unsigned short	vChar;
	
	//�� �Լ��� �������� String�� �����ʹ� aline�� �´ٴ� ������ �����̴�.
	while(1)
	{
		vStringSize += DB_VARCHAR_SIZE;
		
		dbMemcpyFreeIndian(string, target, DB_VARCHAR_SIZE, 1);
		dbMemcpyFreeIndian(string, &vChar, DB_VARCHAR_SIZE, 1);
		
		if (vChar == DB_END_OF_STRING) break;
		
		string++;
		target++;
	}
	return vStringSize;
}

unsigned long dbStringGetSize(DB_VARCHAR* string)
{
	unsigned long stringSize = 0;
	unsigned short vChar;

	//�� �Լ��� �������� String�� �����ʹ� aline�� �´ٴ� ������ �����̴�.
	while(1)
	{
		dbMemcpyFreeIndian(string, &vChar, DB_VARCHAR_SIZE, 1);
		
		if (vChar == 0) break;
		string++;
		stringSize += 2;
	}
	return stringSize;
}

unsigned long dbStringGetSize_AREA(DB_VARCHAR* string)
{
	DB_VARCHAR		uniChar;
	unsigned long	stringSize = 0;

	//�� �Լ��� �������� String�� �����ʹ� aline�� �´ٴ� ������ �����̴�.
	while(1)
	{
		dbMemcpyFreeIndian(string, &uniChar, DB_VARCHAR_SIZE, 1);

		if ((uniChar) == 0) break;
		string++;
		stringSize += 2;
	}
	return stringSize;
}

void dbTableSetCreateTab(unsigned long	*tableOffset, 
				   DB_VARCHAR		*tableName)
{
	//�� �Լ��� ũ�� �ΰ��� ���� �Ѵ�.
	//ù��°���� ���Ե� ���� ã�� ���� ������ �ϴ� ���̰�,
	//�������� ������ �����ϱ� ���� ã�ư����� ���ƿ��鼭 Ʈ���� �籸���ϴ� ���̴�.
	
	pDictionaryHeader		pdicHeader;	
	unsigned	long		vOffset;
	signed		long		vBalanceFactor;

	pdicHeader		= dbGetDictionaryHeader();
	
	dbMemcpyFreeIndian(tableOffset, &vOffset, sizeof(unsigned long), 1);	//���� ��带 ����Ű�� offset�� ��

	if (vOffset == 0)		//������ �Ͼ�� �ϴ� ������ ������ ó��
	{
		unsigned	long		needsize;
		unsigned	long		vSizeOfDictionary;
		
		//��ųʸ��� ���� ũ�⸦ �о����.
		dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderSizeOfDictionary), &vSizeOfDictionary, sizeof(unsigned long), 1);
		
		//���̺��� ���ԵǱ� ���Ͽ� �ʿ��� ����� ����Ѵ�.
		needsize	= sizeof(TableNode) + dbStringGetSize(tableName) + DB_VARCHAR_SIZE;
		
		
		//���̺��� ��带 �����Ѵ�.
		dbTableSetNode(((char*)pdicHeader) + vSizeOfDictionary, tableName);
		
		
		
		//������ ���� �����ϰ�.
		dbMemcpyFreeIndian(&vSizeOfDictionary, tableOffset, sizeof(unsigned long), 1);

		//��ųʸ��� ���� ũ�⸦ ������Ų��..
		vSizeOfDictionary += needsize;
		dbMemcpyFreeIndian(&vSizeOfDictionary, &(pdicHeader->dictionaryHeaderSizeOfDictionary), sizeof(unsigned long), 1);
			
		//��ųʸ��� ������ �ٲ������ �˸���.
		gDbDictionaryChanged = DB_CHANGED;
	}
	else	//������ �Ͼ�� ���� ã�ư��� ������ ã���� ó��
	{
		pTableNode					ptable;		//��ųʸ� �󿡼� ���� �񱳸� �Ϸ��� �ϴ� ��� 
		signed		char		result;		//���� �������ϰ� �ִ� ���� �����Ϸ��� ���̺� �̸��� �񱳰���� ������ ����
		unsigned	long		*tableOffsetBackup;

		//dictionary ���� ���̺� ��带 ������ �Ѵ�.
		ptable = (pTableNode)(((char*)pdicHeader) + vOffset);
		
		result = dbStringCompare_AREAVs_STACK( (DB_VARCHAR*)(ptable + 1), tableName);
		
		tableOffsetBackup = tableOffset;
		
		if (result == 0)	//�ش� ���ڿ��� ��ġ�ϴ� ���
		{
			return;		//������ �̸��� Table�� Database�� �����Ѵ�.
		}
		else if (result == -1)
		{
			tableOffset = &(ptable->tableNodeLeft);
		}
		else
		{
			tableOffset = &(ptable->tableNodeRight);
		}

		//########### recursive����κ�(����) ##########//
		dbTableSetCreateTab(tableOffset, tableName);
		//########### recursive����κ� (��)  ##########//


		//�ش� ����� balanceFactor�� ����Ѵ�.
		vBalanceFactor = dbTableGetBalanceFactor(vOffset);
		
		if (vBalanceFactor == 2)		//�ش� ����� balanceFactor�� �������
		{
			unsigned long leftSubOffset;
			unsigned long leftSubBalanceFactor;

			dbMemcpyFreeIndian(&(ptable->tableNodeLeft), &leftSubOffset, sizeof(unsigned long), 1);
			
			leftSubBalanceFactor = dbTableGetBalanceFactor(leftSubOffset);

			if (leftSubBalanceFactor == -1)
			{
				dbTableRotationLR(tableOffsetBackup);
			}
			else
			{
				dbTableRotationLL(tableOffsetBackup);
			}
		}
		else if (vBalanceFactor == -2)	//�ش� ����� balanceFactor�� �������
		{
			unsigned long rightSubOffset;
			unsigned long rightSubBalanceFactor;

			dbMemcpyFreeIndian(&(ptable->tableNodeRight), &rightSubOffset, sizeof(unsigned long), 1);
			
			rightSubBalanceFactor = dbTableGetBalanceFactor(rightSubOffset);

			if (rightSubBalanceFactor == 1)
			{
				dbTableRotationRL(tableOffsetBackup);
			}
			else
			{
				dbTableRotationRR(tableOffsetBackup);
			}
		}
		else			// �ش� ����� balanceFactor�� �ȱ������
		{
			//������ ����.
		}	

		//�ش� ����� ���̸� �ٽ� ����Ѵ�.
		dbTableSetHeight(ptable);
	}
}

unsigned long dbTableGetHeight(unsigned long tableOffset)
{
	unsigned long		tableHeight;
	
	if (tableOffset == 0)
	{
		tableHeight = 0;
	}
	else
	{
		pDictionaryHeader	pdicHeader;
		pTableNode				ptable;
		
		pdicHeader = dbGetDictionaryHeader();
		ptable		= (pTableNode)(((char*)pdicHeader) + tableOffset);
		
		dbMemcpyFreeIndian(&(ptable->tableNodeHeight), &tableHeight, sizeof(unsigned long), 1);
	}

	return tableHeight;
}

signed long dbTableGetBalanceFactor(unsigned long tableOffset)
{
	signed long	balanceFactor;

	if (tableOffset == 0)	
	{
		balanceFactor = 0;		//�̷��� ��� table�� �������� �����Ƿ� balance factor�� �������� �ʴ´�.
	}
	else
	{
		pDictionaryHeader	pdicHeader;
		pTableNode				ptable;
		unsigned long		leftOffset, rightOffset;
		
		pdicHeader = dbGetDictionaryHeader();
		ptable		= (pTableNode)(((char*)pdicHeader) + tableOffset);

		dbMemcpyFreeIndian(&(ptable->tableNodeLeft),  &leftOffset,  sizeof(unsigned long), 1);
		dbMemcpyFreeIndian(&(ptable->tableNodeRight), &rightOffset, sizeof(unsigned long), 1);

		balanceFactor = (signed long)(dbTableGetHeight(leftOffset) - dbTableGetHeight(rightOffset));
	}
	return balanceFactor;
}


signed char dbStringCompareAreaVsStack(DB_VARCHAR		*dbAreaMemory, 
									   DB_VARCHAR		*dbStackMemory)
{
	DB_VARCHAR			AREA_CHAR;

	while(1)
	{
		dbMemcpyFreeIndian(dbAreaMemory, &AREA_CHAR, DB_VARCHAR_SIZE, 1);
		

		if ((AREA_CHAR == DB_END_OF_STRING) && ((*dbStackMemory) == DB_END_OF_STRING))
		{
			return 0;
		}
		else
		{
			if ( AREA_CHAR == (*dbStackMemory) )
			{
				dbAreaMemory++ ;
				dbStackMemory++ ;
			}
			else if( AREA_CHAR > (*dbStackMemory) )
			{
				return -1;
			}
			else
			{
				return 1;
			}
		}
	}
}


signed char dbStringCompareAreaVsArea(DB_VARCHAR		*dbAreaMemory1, 
									  DB_VARCHAR		*dbAreaMemory2)
{
	DB_VARCHAR			AREA_CHAR1, AREA_CHAR2;

	while(1)
	{
		dbMemcpyFreeIndian(dbAreaMemory1, &AREA_CHAR1, DB_VARCHAR_SIZE, 1);
		dbMemcpyFreeIndian(dbAreaMemory2, &AREA_CHAR2, DB_VARCHAR_SIZE, 1);
		

		if ((AREA_CHAR1 == DB_END_OF_STRING) && (AREA_CHAR2 == DB_END_OF_STRING)) return 0;

		if ( AREA_CHAR1 == AREA_CHAR2 )
		{
			dbAreaMemory1++ ;
			dbAreaMemory2++ ;
		}
		else if( AREA_CHAR1 > AREA_CHAR2 )
		{
			return -1;
		}
		else
		{
			return 1;
		} 
	}
}


unsigned short* dbStringRTrimStack(DB_VARCHAR *dbStackMemoryString)
{
	if (((*dbStackMemoryString) == ' ')||((*dbStackMemoryString) == '"'))
	{
		while(1)
		{
			dbStackMemoryString++;
			
			if (((*dbStackMemoryString) == ' ') ||((*dbStackMemoryString) == '"'))
			{
				//���� ����.
				;
			}
			else
			{
				break;
			}
		}
	}
	
	return dbStackMemoryString;
}


unsigned short* dbStringRTrimArea(DB_VARCHAR *dbAreaMemoryString)
{
	unsigned short	vCharater;

	dbMemcpyFreeIndian(dbAreaMemoryString, &vCharater, sizeof(unsigned short), 1);

	if ((vCharater == ' ')||(vCharater == '"'))
	{
		while(1)
		{
			dbAreaMemoryString++;

			dbMemcpyFreeIndian(dbAreaMemoryString, &vCharater, sizeof(unsigned short), 1);
			
			if ((vCharater == ' ')||(vCharater == '"'))
			{}
			else
			{
				break;
			}
		}
	}
	
	return dbAreaMemoryString;
}

unsigned short* dbStringWithoutArticleArea(DB_VARCHAR	*dbAreaMemoryString)
{
	//�� �Լ��� ��� Area�� �ִ� ��Ʈ�������� ���縦 ���� ������ ��Ʈ������ �������ϵ��� �Ѵ�.
	unsigned short	vLowerCaseMask = 0x20;
	unsigned short	vChar1, vChar2, vChar3, vChar4;

	dbAreaMemoryString = dbStringRTrimArea(dbAreaMemoryString); //���鹮�ڸ� �ǳʶڴ�..
	
	dbMemcpyFreeIndian(dbAreaMemoryString,		&vChar1,	sizeof(unsigned short),		1);
	dbMemcpyFreeIndian(dbAreaMemoryString + 1,	&vChar2,	sizeof(unsigned short),		1);
	dbMemcpyFreeIndian(dbAreaMemoryString + 2,	&vChar3,	sizeof(unsigned short),		1);
	dbMemcpyFreeIndian(dbAreaMemoryString + 3,	&vChar4,	sizeof(unsigned short),		1);

	if (vChar1!=0)
	{
		vChar1 = vChar1|vLowerCaseMask;
	}
	if (vChar2!=0)
	{
		vChar2 = vChar2|vLowerCaseMask;
	}
	if (vChar3!=0)
	{
		vChar3 = vChar3|vLowerCaseMask;
	}
	if (vChar4!=0)
	{
		vChar4 = vChar4|vLowerCaseMask;
	}
	
	if ((vChar1 == 'a') && (vChar2 == ' '))
	{
		//���� a�� �ִ� ���
		//dbAreaMemoryString = dbStringRTrimStack(dbAreaMemoryString + 2);
		dbAreaMemoryString = dbStringRTrimArea(dbAreaMemoryString + 2);
	}
	else if ((vChar1 == 'a') && (vChar2 == 'n') && (vChar3 == ' '))
	{
		//���� an�� �ִ� ���
		//dbAreaMemoryString = dbStringRTrimStack(dbAreaMemoryString + 3);
		dbAreaMemoryString = dbStringRTrimArea(dbAreaMemoryString + 3);
	}
	else if ((vChar1 == 't') && (vChar2 == 'h') && (vChar3 == 'e') && (vChar4 == ' '))
	{
		//���� the�� �ִ� ���
		//dbAreaMemoryString = dbStringRTrimStack(dbAreaMemoryString + 4);
		dbAreaMemoryString = dbStringRTrimArea(dbAreaMemoryString + 4);
	}
	else
	{}

	return dbAreaMemoryString;
}


unsigned short* dbStringWithoutArticleStack(DB_VARCHAR	*dbStackMemoryString)
{
	//�� �Լ��� ��� Area�� �ִ� ��Ʈ�������� ���縦 ���� ������ ��Ʈ������ �������ϵ��� �Ѵ�.
	unsigned short vLowerCaseMask = 0x20;

	dbStackMemoryString = dbStringRTrimStack(dbStackMemoryString); //���鹮�ڸ� �ǳʶڴ�..

	if ((((*dbStackMemoryString)|vLowerCaseMask) == 'a') 
		&& (((*(dbStackMemoryString + 1))|vLowerCaseMask) == ' ')
		&&((*(dbStackMemoryString + 1))!=0))
	{
		//���� a�� �ִ� ���
		dbStackMemoryString = dbStringRTrimStack(dbStackMemoryString + 2);
	}
	else if ((((*dbStackMemoryString)|vLowerCaseMask) == 'a') 
		&& (((*(dbStackMemoryString+1))|vLowerCaseMask) == 'n') 
		&& (((*(dbStackMemoryString+2))|vLowerCaseMask) == ' ')
		&&((*(dbStackMemoryString + 2))!=0))
	{
		//���� an�� �ִ� ���
		dbStackMemoryString = dbStringRTrimStack(dbStackMemoryString + 3);
	}
	else if ((((*dbStackMemoryString)|vLowerCaseMask) == 't') 
		&& (((*(dbStackMemoryString+1))|vLowerCaseMask) == 'h') 
		&& (((*(dbStackMemoryString+2))|vLowerCaseMask) == 'e') 
		&& (((*(dbStackMemoryString+3))|vLowerCaseMask) == ' ')
		&&((*(dbStackMemoryString + 3))!=0))
	{
		//���� the�� �ִ� ���
		dbStackMemoryString = dbStringRTrimStack(dbStackMemoryString + 4);
	}
	else
	{}

	return dbStackMemoryString;
}

signed char dbStringCompareAreaVsAreaMS(DB_VARCHAR		*dbAreaMemory1, 
									    DB_VARCHAR		*dbAreaMemory2)
{
	DB_VARCHAR			*dbAreaMemory1Bak;
	DB_VARCHAR			*dbAreaMemory2Bak;
	DB_VARCHAR			AREA_CHAR1, AREA_CHAR2;
	DB_VARCHAR			vLowerCaseMask = 0x20;

	dbAreaMemory1Bak	= dbAreaMemory1;
	dbAreaMemory2Bak	= dbAreaMemory2;

//DbgPrintf(("���� �����ϱ� ���� �ΰ��� ���ڿ��� ���� Address{%x, %x}\n", dbAreaMemory1Bak, dbAreaMemory2Bak));
	dbAreaMemory1	= dbStringWithoutArticleArea(dbAreaMemory1);
	dbAreaMemory2	= dbStringWithoutArticleArea(dbAreaMemory2);
//DbgPrintf(("���� ������ ���� �ΰ��� ���ڿ��� ���� Address{%x, %x}\n", dbAreaMemory1Bak, dbAreaMemory2Bak));
//showTwoByteString("���縦 ������ dbAreaMemory1", dbAreaMemory1);
//showTwoByteString("���縦 ������ dbAreaMemory2", dbAreaMemory2);
	while(1)
	{
		dbMemcpyFreeIndian(dbAreaMemory1, &AREA_CHAR1, DB_VARCHAR_SIZE, 1);
		dbMemcpyFreeIndian(dbAreaMemory2, &AREA_CHAR2, DB_VARCHAR_SIZE, 1);
		

		{//������ ������ ��� ���ڶ�� �ҹ��ڷ� �����.
			if ((AREA_CHAR1>='A')&&(AREA_CHAR1<='Z'))
			{
				AREA_CHAR1 = AREA_CHAR1 | vLowerCaseMask;
			}

			if ((AREA_CHAR2>='A')&&(AREA_CHAR2<='Z'))
			{
				AREA_CHAR2 = AREA_CHAR2 | vLowerCaseMask;
			}
			
		}

		if ((AREA_CHAR1 == DB_END_OF_STRING) && (AREA_CHAR2 == DB_END_OF_STRING))
		{
//DbgPrintf(("�ι��ڿ��� ��ġ�Ͽ� �ٽ� ��ҹ��ڱ����Ͽ� ���ϴ� ��ƾ���� �����Ѵ�."));
			return dbStringCompareAreaVsArea(dbAreaMemory1Bak, dbAreaMemory2Bak);
		}
		else
		{
			if ( AREA_CHAR1 == AREA_CHAR2 )
			{
				dbAreaMemory1++ ;
				dbAreaMemory2++ ;
			}
			else if( AREA_CHAR1 > AREA_CHAR2 )
			{
				return -1;
			}
			else
			{
				return 1;
			}
		}
	}
}

signed char dbStringCompareAreaVsStackMS(DB_VARCHAR		*dbAreaMemory, 
										 DB_VARCHAR		*dbStackMemory)
{
	DB_VARCHAR			*dbAreaMemoryBak;
	DB_VARCHAR			*dbStackMemoryBak;
	DB_VARCHAR			vAreaChar;
	DB_VARCHAR			vStackChar;
	DB_VARCHAR			vAreaLowerChar;
	DB_VARCHAR			vStackLowerChar;
	DB_VARCHAR			vLowerCaseMask = 0x20;


	dbAreaMemoryBak		= dbAreaMemory;
	dbStackMemoryBak	= dbStackMemory;

	dbAreaMemory	= dbStringWithoutArticleArea(dbAreaMemory);
	dbStackMemory	= dbStringWithoutArticleStack(dbStackMemory);

	while(1)
	{
		dbMemcpyFreeIndian(dbAreaMemory, &vAreaChar, DB_VARCHAR_SIZE, 1);
		vStackChar = *dbStackMemory;

		{//������ ������ ��� ���ڶ�� �ҹ��ڷ� �����.
			if ((vAreaChar>='A')&&(vAreaChar<='Z'))
			{
				vAreaLowerChar = vAreaChar | vLowerCaseMask;
			}
			else
			{
				vAreaLowerChar = vAreaChar;
			}


			if ((vStackChar>='A')&&(vStackChar<='Z'))
			{
				vStackLowerChar = vStackChar | vLowerCaseMask;
			}
			else
			{
				vStackLowerChar = vStackChar;
			}
		}


		if ((vAreaLowerChar == DB_END_OF_STRING) && (vStackLowerChar == DB_END_OF_STRING))
		{
			return dbStringCompareAreaVsStack(dbAreaMemoryBak, dbStackMemoryBak);
		}
		else
		{
			if ( vAreaLowerChar == vStackLowerChar )
			{
				dbAreaMemory++ ;
				dbStackMemory++ ;
			}
			else if( vAreaLowerChar > vStackLowerChar )
			{
				return -1;
			}
			else
			{
				return 1;
			}
		}
	}
}


signed char dbStringCompare_AREAVs_STACK(DB_VARCHAR		*areaMemory, 
										DB_VARCHAR		*stackMemory)
{
	DB_VARCHAR			AREA_CHAR;

	while(1)
	{
		dbMemcpyFreeIndian(areaMemory, &AREA_CHAR, DB_VARCHAR_SIZE, 1);
		

		if ((AREA_CHAR == DB_END_OF_STRING) && ((*stackMemory) == DB_END_OF_STRING)) return 0;

		if ( AREA_CHAR == (*stackMemory) )
		{
			areaMemory++ ;
			stackMemory++ ;
		}
		else if( AREA_CHAR > (*stackMemory) )
		{
			return -1;
		}
		else
		{
			return 1;
		} 
	}
}
void dbTableSetHeight(pTableNode ptable)
{
	TableNode					TABLE_NODE;

	unsigned	long		CURRENT_NODE_HEIGHT, 
							LEFT_SUB_HEIGHT, 
							RIGHT_SUB_HEIGHT;
	
	dbMemcpyFreeIndian(ptable, &TABLE_NODE, sizeof(unsigned long), 6);
	
	LEFT_SUB_HEIGHT  = dbTableGetHeight(TABLE_NODE.tableNodeLeft);
	RIGHT_SUB_HEIGHT = dbTableGetHeight(TABLE_NODE.tableNodeRight);

	CURRENT_NODE_HEIGHT = (LEFT_SUB_HEIGHT > RIGHT_SUB_HEIGHT) ? LEFT_SUB_HEIGHT + 1 : RIGHT_SUB_HEIGHT + 1;
	
	dbMemcpyFreeIndian(&CURRENT_NODE_HEIGHT, &(ptable->tableNodeHeight), sizeof(unsigned long), 1);
}

void dbTableRotationLR(unsigned long *source)
{
	pDictionaryHeader		pdicHeader;

	pTableNode					pMin, 
							pMid, 
							pMax;

	TableNode					MIN_NODE, 
							MID_NODE, 
							MAX_NODE;
	
	unsigned	long		minOffset, 
							midOffset, 
							maxOffset;
	
	pdicHeader = dbGetDictionaryHeader();

	dbMemcpyFreeIndian(source, &(maxOffset), sizeof(unsigned long), 1);
	pMax = (pTableNode)(((char*)pdicHeader) + maxOffset);
	dbMemcpyFreeIndian(pMax, &MAX_NODE, sizeof(unsigned long), 6);

	minOffset = MAX_NODE.tableNodeLeft;
	pMin = (pTableNode)(((char*)pdicHeader) + minOffset);
	dbMemcpyFreeIndian(pMin, &MIN_NODE, sizeof(unsigned long), 6);

	midOffset = MIN_NODE.tableNodeRight;
	pMid = (pTableNode)(((char*)pdicHeader) + midOffset);
	dbMemcpyFreeIndian(pMid, &MID_NODE, sizeof(unsigned long), 6);

	dbMemcpyFreeIndian(&midOffset, source, sizeof(unsigned long), 1);
	
	MAX_NODE.tableNodeLeft	= MID_NODE.tableNodeRight;
	MIN_NODE.tableNodeRight	= MID_NODE.tableNodeLeft;
	MID_NODE.tableNodeLeft	= minOffset;
	MID_NODE.tableNodeRight	= maxOffset;

	dbMemcpyFreeIndian(&(MAX_NODE.tableNodeLeft),  &(pMax->tableNodeLeft),  sizeof(unsigned long), 1);
	dbMemcpyFreeIndian(&(MIN_NODE.tableNodeRight), &(pMin->tableNodeRight), sizeof(unsigned long), 1);
	dbMemcpyFreeIndian(&(MID_NODE.tableNodeLeft),  &(pMid->tableNodeLeft),  sizeof(unsigned long), 1);
	dbMemcpyFreeIndian(&(MID_NODE.tableNodeRight), &(pMid->tableNodeRight), sizeof(unsigned long), 1);

	dbTableSetHeight(pMin);
	dbTableSetHeight(pMax);
	dbTableSetHeight(pMid);
}

signed char dbIndexGetBalanceFactor(unsigned long leftSubOffset, 
										unsigned long rightSubOffset)
{
	signed long balanceFactor;
	signed long leftSubHeight;
	signed long rightSubHeight;

	leftSubHeight		= (signed long)dbIndexGetHeight(leftSubOffset);
	rightSubHeight	= (signed long)dbIndexGetHeight(rightSubOffset);
	
	balanceFactor = (leftSubHeight - rightSubHeight);
	
	return (signed char)((signed long)balanceFactor);
}

unsigned long dbIndexGetHeight(unsigned long indexOffset)
{
	pIndexNode			pindexNode;
	unsigned long	indexHeight;

	if (indexOffset == DB_NULL)
	{
		indexHeight = 0;
	}
	else
	{
		dbTouch(indexOffset, DB_INDEX_FILE_LOAD);
		pindexNode = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + indexOffset%gDbPageSizeIndexFilePageSize);
		dbMemcpyFreeIndian(&(pindexNode->indexNodeHeight), &indexHeight, sizeof(unsigned long), 1);
	}
	return indexHeight;
}

unsigned long dbIndexGetCalculateHeight(unsigned long leftSubOffset, unsigned long rightSubOffset)
{
	unsigned long	leftSubHeight, rightSubHeight;
	unsigned long	indexHeight;

	leftSubHeight		= dbIndexGetHeight(leftSubOffset);
	rightSubHeight	= dbIndexGetHeight(rightSubOffset);

	indexHeight = (leftSubHeight > rightSubHeight) ? leftSubHeight: rightSubHeight;
	
	return indexHeight + 1;
}
void dbIndexRotationLR(unsigned long		*l1, 
						  unsigned long		*l2, 
						  unsigned long		*l3)
{
	pIndexNode		pMax, pMid, pMin;
	IndexNode		MAX_NODE, MIN_NODE, MID_NODE;	
	
	// ==== MAX_NODE ó�� ==== //
	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;		//����� ���̹Ƿ� �̸� ����
	pMax=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMax, &MAX_NODE, sizeof(unsigned long), 4);
	
	// ==== MIN_NODE ó�� ==== //
	dbTouch(*l2, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;		//����� ���̹Ƿ� �̸� ����
	pMin=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l2)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMin, &MIN_NODE, sizeof(unsigned long), 4);
	
	// ==== MID_NODE ó�� ==== //
	dbTouch(*l3, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;		//����� ���̹Ƿ� �̸� ����
	pMid=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l3)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMid, &MID_NODE, sizeof(unsigned long), 4);

	MIN_NODE.indexNodeRight	= MID_NODE.indexNodeLeft;
	MAX_NODE.indexNodeLeft	= MID_NODE.indexNodeRight;
	MID_NODE.indexNodeRight	= *l1;
	MID_NODE.indexNodeLeft	= *l2;

	dbMemcpyFreeIndian(&MID_NODE, pMid, sizeof(unsigned long), 4);	
	dbMemcpyFreeIndian(&MAX_NODE, pMax, sizeof(unsigned long), 4);	
	dbMemcpyFreeIndian(&MIN_NODE, pMin, sizeof(unsigned long), 4);	

	
	//min�� Ʈ������ �ٽð��
	MIN_NODE.indexNodeHeight = dbIndexGetCalculateHeight(MIN_NODE.indexNodeLeft, MIN_NODE.indexNodeRight);
	
	dbTouch(*l2, DB_INDEX_FILE_LOAD);
	pMin = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l2)%gDbPageSizeIndexFilePageSize);
	
	dbMemcpyFreeIndian(&(MIN_NODE.indexNodeHeight), &(pMin->indexNodeHeight), sizeof(unsigned long), 1);
	
	//max�� Ʈ�� ���� �ٽð��
	MAX_NODE.indexNodeHeight = dbIndexGetCalculateHeight(MAX_NODE.indexNodeLeft, MAX_NODE.indexNodeRight);
	
	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	pMax = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	
	dbMemcpyFreeIndian(&(MAX_NODE.indexNodeHeight), &(pMax->indexNodeHeight), sizeof(unsigned long), 1);

	//root�� *l1�� ����
	*l1 = *l3;
}

void dbIndexRotationLL(unsigned long *l1, 
					 unsigned long *l2, 
					 unsigned long *l3)
{
	pIndexNode			pMax, pMid;
	IndexNode			MAX_NODE, MID_NODE;

	// ==== MAX_NODE ó�� ==== //
	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;		//����� ���̹Ƿ� �̸� ����
	pMax = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMax, &MAX_NODE, sizeof(unsigned long), 4);
	
	// ==== MID_NODE ó�� ==== //
	dbTouch(*l2, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;		//����� ���̹Ƿ� �̸� ����
	pMid = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l2)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMid, &MID_NODE, sizeof(unsigned long), 4);

	MAX_NODE.indexNodeLeft	= MID_NODE.indexNodeRight;
	MID_NODE.indexNodeRight	= *l1;
	
	//rootNode�� �� ��带 �����Ѵ�.
	dbMemcpyFreeIndian(&MID_NODE, pMid, sizeof(unsigned long), 4);
	dbMemcpyFreeIndian(&MAX_NODE, pMax, sizeof(unsigned long), 4);	

	//max�� Ʈ������ �ٽð��
	MAX_NODE.indexNodeHeight = dbIndexGetCalculateHeight(MAX_NODE.indexNodeLeft, MAX_NODE.indexNodeRight);

	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	pMax = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	
	dbMemcpyFreeIndian(&(MAX_NODE.indexNodeHeight), &(pMax->indexNodeHeight), sizeof(unsigned long), 1);

	//root�� ���� *l2������ ����
	*l1 = *l2;
}

void dbIndexRotationRR(unsigned long *l1, 
					 unsigned long *l2, 
					 unsigned long *l3)
{
	pIndexNode			pMin, pMid;
	IndexNode			MIN_NODE, MID_NODE;

	// =====MIN_NODEó�� ==== //
	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;		//����� ���̹Ƿ� �̸� ����
	pMin=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMin, &MIN_NODE, sizeof(unsigned long), 4);

	// =====MID_NODEó�� ==== //
	dbTouch(*l2, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;
	pMid = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l2)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMid, &MID_NODE, sizeof(unsigned long), 4);

	MIN_NODE.indexNodeRight	= MID_NODE.indexNodeLeft;
	MID_NODE.indexNodeLeft	= *l1;

	dbMemcpyFreeIndian(&MID_NODE, pMid, sizeof(unsigned long), 4);	
	dbMemcpyFreeIndian(&MIN_NODE, pMin, sizeof(unsigned long), 4);	

	//root�� �� ��带 �����Ѵ�.
	//dbMemcpy(&MID_NODE, pMid, sizeof(INODE));

	//min�� Ʈ������ �ٽð��
    MIN_NODE.indexNodeHeight = dbIndexGetCalculateHeight(MIN_NODE.indexNodeLeft, MIN_NODE.indexNodeRight);

	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	pMin = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(&(MIN_NODE.indexNodeHeight), &(pMin->indexNodeHeight), sizeof(unsigned long), 1);

	//root�� ���� *l2������ ����
	*l1 = *l2;
}

void dbIndexRotationRL(unsigned long *l1,
					 unsigned long *l2, 
					 unsigned long *l3)
{
	pIndexNode			pMin, pMid, pMax;
	IndexNode			MIN_NODE, MID_NODE, MAX_NODE;

	// ==== MIN_NODE ó�� ==== //
	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;			//����� ���̹Ƿ� �̸� ����
	pMin=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMin, &MIN_NODE, sizeof(unsigned long), 4);

	// ==== MAX_NODE ó�� ==== //
	dbTouch(*l2, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;			//����� ���̹Ƿ� �̸� ����
	pMax=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l2)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMax, &MAX_NODE, sizeof(unsigned long), 4);

	// ==== MID_NODE ó�� ==== //
	dbTouch(*l3, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;			//����� ���̹Ƿ� �̸� ����
	pMid=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l3)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMid, &MID_NODE, sizeof(unsigned long), 4);

	MIN_NODE.indexNodeRight	= MID_NODE.indexNodeLeft;
	MAX_NODE.indexNodeLeft	= MID_NODE.indexNodeRight;
	MID_NODE.indexNodeLeft	= *l1;
	MID_NODE.indexNodeRight	= *l2;
	
	dbMemcpyFreeIndian(&MID_NODE, pMid, sizeof(unsigned long), 4);	
	dbMemcpyFreeIndian(&MAX_NODE, pMax, sizeof(unsigned long), 4);	
	dbMemcpyFreeIndian(&MIN_NODE, pMin, sizeof(unsigned long), 4);	

	//min�� Ʈ������ �ٽð��
	MIN_NODE.indexNodeHeight = dbIndexGetCalculateHeight(MIN_NODE.indexNodeLeft, MIN_NODE.indexNodeRight);
	MAX_NODE.indexNodeHeight = dbIndexGetCalculateHeight(MAX_NODE.indexNodeLeft, MAX_NODE.indexNodeRight);
	
	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	pMin = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(&(MIN_NODE.indexNodeHeight), &(pMin->indexNodeHeight), sizeof(unsigned long), 1);

	//max�� Ʈ������ �ٽð��
	dbTouch(*l2, DB_INDEX_FILE_LOAD);
	pMax = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l2)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(&(MAX_NODE.indexNodeHeight), &(pMax->indexNodeHeight), sizeof(unsigned long), 1);

	//root�� ���� *l3������ ����
	*l1 = *l3;
}
void dbTableRotationLL(unsigned long *source)
{
	pDictionaryHeader		pdicHeader;

	pTableNode					pMax, 
							pMid;
	
	TableNode					MAX_NODE, 
							MID_NODE;

	unsigned	long		maxOffset,
							midOffset;

	pdicHeader = dbGetDictionaryHeader();

	dbMemcpyFreeIndian(source, &maxOffset, sizeof(unsigned long), 1);
	pMax = (pTableNode)(((char*)pdicHeader) + maxOffset);
	dbMemcpyFreeIndian(pMax, &MAX_NODE, sizeof(unsigned long), 6);

	midOffset = MAX_NODE.tableNodeLeft;
	pMid = (pTableNode)(((char*)pdicHeader) + midOffset);
	dbMemcpyFreeIndian(pMid, &MID_NODE, sizeof(unsigned long), 6);

	dbMemcpyFreeIndian(&midOffset, source, sizeof(unsigned long), 1);
	
	MAX_NODE.tableNodeLeft	= MID_NODE.tableNodeRight;
	MID_NODE.tableNodeRight	= maxOffset;
	
	dbMemcpyFreeIndian(&(MAX_NODE.tableNodeLeft),  &(pMax->tableNodeLeft),  sizeof(unsigned long), 1);
	dbMemcpyFreeIndian(&(MID_NODE.tableNodeRight), &(pMid->tableNodeRight), sizeof(unsigned long), 1);

	dbTableSetHeight(pMax);
	dbTableSetHeight(pMid);
}

void dbTableRotationRR(unsigned long *source)
{
	pDictionaryHeader		pdicHeader;

	pTableNode					pMin, 
							pMid;

	TableNode					MIN_NODE, 
							MID_NODE;

	unsigned	long		minOffset,
							midOffset;

	pdicHeader = dbGetDictionaryHeader();

	dbMemcpyFreeIndian(source, &minOffset, sizeof(unsigned long), 1);
	pMin = (pTableNode)(((char*)pdicHeader) + minOffset);
	dbMemcpyFreeIndian(pMin, &MIN_NODE, sizeof(unsigned long), 6);

	midOffset = MIN_NODE.tableNodeRight;
	pMid = (pTableNode)(((char*)pdicHeader) + midOffset);
	dbMemcpyFreeIndian(pMid, &MID_NODE, sizeof(unsigned long), 6);

	dbMemcpyFreeIndian(&midOffset, source, sizeof(unsigned long), 1);
	
	MIN_NODE.tableNodeRight	= MID_NODE.tableNodeLeft;
	MID_NODE.tableNodeLeft	= minOffset;

	dbMemcpyFreeIndian(&(MIN_NODE.tableNodeRight), &(pMin->tableNodeRight), sizeof(unsigned long), 1);
	dbMemcpyFreeIndian(&(MID_NODE.tableNodeLeft),  &(pMid->tableNodeLeft),  sizeof(unsigned long), 1);

	dbTableSetHeight(pMin);
	dbTableSetHeight(pMid);
}

void dbTableRotationRL(unsigned long *source)
{
	pDictionaryHeader		pdicHeader;


	pTableNode					pMin, 
							pMid, 
							pMax;

	TableNode					MIN_NODE, 
							MID_NODE, 
							MAX_NODE;

	unsigned	long		minOffset, 
							midOffset, 
							maxOffset;

	pdicHeader = dbGetDictionaryHeader();

	dbMemcpyFreeIndian(source, &minOffset, sizeof(unsigned long), 1);
	pMin = (pTableNode)(((char*)pdicHeader) + minOffset);
	dbMemcpyFreeIndian(pMin, &MIN_NODE, sizeof(unsigned long), 6);

	maxOffset = MIN_NODE.tableNodeRight;
	pMax = (pTableNode)(((char*)pdicHeader) + maxOffset);
	dbMemcpyFreeIndian(pMax, &MAX_NODE, sizeof(unsigned long), 6);

	midOffset = MAX_NODE.tableNodeLeft;
	pMid = (pTableNode)(((char*)pdicHeader) + midOffset);
	dbMemcpyFreeIndian(pMid, &MID_NODE, sizeof(unsigned long), 6);
	
	dbMemcpyFreeIndian(&midOffset, source, sizeof(unsigned long), 1);
	
	MIN_NODE.tableNodeRight	= MID_NODE.tableNodeLeft;
	MAX_NODE.tableNodeLeft	= MID_NODE.tableNodeRight;
	MID_NODE.tableNodeLeft	= minOffset;
	MID_NODE.tableNodeRight	= maxOffset;

	dbMemcpyFreeIndian(&(MIN_NODE.tableNodeRight), &(pMin->tableNodeRight), sizeof(unsigned long), 1);
	dbMemcpyFreeIndian(&(MAX_NODE.tableNodeLeft),  &(pMax->tableNodeLeft),  sizeof(unsigned long), 1);
	dbMemcpyFreeIndian(&(MID_NODE.tableNodeRight), &(pMid->tableNodeRight), sizeof(unsigned long), 1);
	dbMemcpyFreeIndian(&(MID_NODE.tableNodeLeft),  &(pMid->tableNodeLeft),  sizeof(unsigned long), 1);

	dbTableSetHeight(pMin);
	dbTableSetHeight(pMax);
	dbTableSetHeight(pMid);
}

void dbApiCreateColumn(DB_VARCHAR	*tableName, 
						  DB_VARCHAR	*columnName, 
						  DB_COLUMN_TYPE	type)
{
	// =========================���� ���� ����(����) =========================//
	pDictionaryHeader		pdicHeader;			//Dictionary file�� �ش��� �������� ����
	pTableNode			pTable;				//Dictionary file�� �ö���� ���������� ���̺��� �������� ����
	TableNode			TABLE_NODE;
	unsigned	long			*ptableOffset,		//Dictionary file�� �ö���� �������� ���̺��� offset�� �����ϰ� ��ġ�� �������� ����
						vTableOffset,
						*pcolumnOffset;		//Dictionary file�� �ö���� ������ ���̺��� ���� �÷��� offset�� �����ϰ� ��ġ�� �������� ����
	unsigned	long			requirementSize;
	// =========================���� ���� ����(��) =========================//

	#if DB_SHOW_ERROR_CODE				//Error Check
	gDbErrorCode = DB_ERROR_NO_ERROR;		//Error Check
	#endif									//Error Check


	if (gDbStartUpDB != DB_DO_RUNNING)
	{
		#if DB_SHOW_ERROR_CODE								//Error Check
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;		//Error Check
		#endif													//Error Check
		
		return;
	}


	pdicHeader	= dbGetDictionaryHeader();						//Dictionary�� �����Ѵ�.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����
	
	//���̺��� ã�´�.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : �Է����� ���� tableName�� table�� �ý��ۿ� ���������ʴ� ���
		#if DB_SHOW_ERROR_CODE						//Error Check
		gDbErrorCode = DB_ERROR_TABLE_NOT_FOUND;		//Error Check
		#endif											//Error Check

		return;
	}
	// ===========�Ʒ����� �ش� ���̺��� ã������ ó�� ===========//

	//table�� offset�� ����Ǿ� �ִ�.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table�� �޸𸮸� �����Ѵ�.
	pTable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//���̺��� ������ TABLE_NODE�� �����Ѵ�.
	dbMemcpyFreeIndian(pTable, &TABLE_NODE, sizeof(unsigned long), 6);

	if (TABLE_NODE.tableNodeStartPage != 0)
	{
		//�����ͻ����� �־��ٸ� �� ���¿��� �÷��� ������ �� ����.
		#if DB_SHOW_ERROR_CODE							//Error Check
		gDbErrorCode = DB_ERROR_TABLE_ALLOC_PAGE_NONE;	//Error Check
		#endif												//Error Check
		
		return;
	}

	//�÷��� ���� offset�� ����Ǿ� �ִ� �ּҸ� �������Ѵ�.
	pcolumnOffset = &(pTable->tableNodeColumnOffset);

	//�÷��� ã�´�.
	if(dbTableFindColumnName(&pcolumnOffset, columnName))
	{
		//ERROR : �ش� �÷��̸��� �÷��� �̹� �����ϴ� ���
		#if DB_SHOW_ERROR_CODE								//Error Check
		gDbErrorCode = DB_ERROR_COLUMN_NAME_ALREADY_EXIST;	//Error Check
		#endif													//Error Check

		return;
	}

	//�÷��� ���ԵǱ� ���� �ʿ��� ����� ����Ѵ�.
	requirementSize = sizeof(ColumnNode) + dbStringGetSize(columnName) + DB_VARCHAR_SIZE;

	
	//�ش��̸��� �÷��� �������� �ʴ� ��� : ���� �÷��� ���� �ȴ�..^^
	{
		unsigned long	dictionarySize;

		dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderSizeOfDictionary), &dictionarySize, sizeof(unsigned long), 1);

		dbColumnSetNode(((char*)pdicHeader) + dictionarySize, columnName, type);
		dbMemcpyFreeIndian(&dictionarySize, pcolumnOffset, sizeof(unsigned long), 1);

		dictionarySize += requirementSize;
		dbMemcpyFreeIndian(&dictionarySize, &(pdicHeader->dictionaryHeaderSizeOfDictionary), sizeof(unsigned long), 1);
		
		gDbDictionaryChanged = DB_CHANGED;						//dictionary�� ������ �߻��� ���� üũ
	}
}

void dbColumnSetNode(char			*target, 
						DB_VARCHAR		*columnName, 
						DB_COLUMN_TYPE	type)
{
	ColumnNode				columnNode;

	//�÷� ��带 �����Ѵ�.
	columnNode.columnNodeDataType			= type;
	columnNode.columnNodeIndexRootOffset	= 0;
	columnNode.columnNodeNextColumnOffset	= 0;

	//�÷� ��带 ����Ѵ�.
	dbMemcpyFreeIndian(&columnNode, target, sizeof(unsigned long), 3);

	//�÷��� �̸��� ����Ѵ�.
	dbStringCopy((DB_VARCHAR*)(target + sizeof(ColumnNode)), columnName);
}

unsigned char dbTableFindColumnName(unsigned long	**source, 
										DB_VARCHAR		*columnName)
{
	pDictionaryHeader		pdicHeader;
	pColumnNode					pcolumn;
	unsigned	long		columnOffset;
	
	pdicHeader = dbGetDictionaryHeader();

	while(1)
	{
		//��ųʸ��� �ִ� �÷��� OFFSET���� �����Ѵ�.
		dbMemcpyFreeIndian(*source, &columnOffset, sizeof(unsigned long), 1);

		if (columnOffset == 0)
		{
			return 0;		//������ �÷����� �°��
		}
		else
		{
			//�ش� Column�� �����Ѵ�.
			pcolumn = (pColumnNode)(((char*)pdicHeader) + columnOffset);

			if (dbStringCompare_AREAVs_STACK((DB_VARCHAR*)(pcolumn + 1), columnName) == 0)
			{
				return 1;	//�ش� ��带 ã�� ���
			}
			else
			{
				*source = &(pcolumn->columnNodeNextColumnOffset);	//������带 ã�ư���.
			}
		}
	}
}

unsigned char dbTableFindTableName(unsigned long	**source, 
									   DB_VARCHAR		*tableName)
{
	pDictionaryHeader		pdicHeader;
	pTableNode					ptable;
	signed		char		compareResult;
	unsigned	long		tableOffset;

	pdicHeader = dbGetDictionaryHeader();

	while(1)
	{
		dbMemcpyFreeIndian(*source, &tableOffset, sizeof(unsigned long), 1);

		if (tableOffset == 0)
		{
			//���̺��� ��ã�� ���
			return 0;
		}
		else
		{
			//�ش� table�� �����Ѵ�.
			ptable = (pTableNode)(((char*)pdicHeader) + tableOffset);

			//���ڿ��� ���Ѵ�
			compareResult = dbStringCompare_AREAVs_STACK((DB_VARCHAR*)(ptable + 1), tableName);

			if (compareResult == 0)
			{
				return 1;
			}
			else if(compareResult == -1)
			{
				*source = &(ptable->tableNodeLeft);
			}
			else if(compareResult == 1)
			{
				*source = &(ptable->tableNodeRight);
			}
			else{}
		}
	}
}

void dbApiCreateIndexSingle(DB_VARCHAR	*tableName, 
								DB_VARCHAR	*columnName)
{
	// =========================���� ���� ����(����) =========================//
	pDictionaryHeader		pdicHeader;	//dictionary header�� �������� ����
	pTableNode			ptable;			//dictionary file�� table�� �������� ����
	pColumnNode			pcolumn;
	unsigned	long			*ptableOffset,		//dictionary file�� �ö���� memory������ ���̺��� offset ����κ��� ������ �� ����
						*pcolumnOffset;	//dictionary file�� �ö���� memory������ �÷��� offset ����κ��� ������ �� ����
	unsigned	long		vTableOffset,
					vColumnOffset,
					vIndexOffset;
	// =========================���� ���� ����(��) =========================//

#if DB_SHOW_ERROR_CODE
	gDbErrorCode = DB_ERROR_NO_ERROR;
#endif

//==============ERROR Code (Begin)==============//
if (gDbStartUpDB != DB_DO_RUNNING)
{
	#if DB_SHOW_ERROR_CODE
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;
	#endif
	return ;
}
//==============ERROR Code ( end )==============//

	pdicHeader		= dbGetDictionaryHeader();				//dictionary header�� �����Ѵ�.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table�� ���� offset�� �����ϴ� ���� �����Ѵ�.
	
	//�ش� ���̺��� �ִ����� �˻��Ѵ�.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		return;		//�ش� ���̺��� ���� ��� �Լ��� �����Ѵ�.
	}
	//���� if���� �ɸ��� �ʾҴٸ� �ش����̺��� �����ϴ� ����̴�
	
	//*tempTableNode���� Offset�� ����Ǿ� �ִ�.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);
	ptable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	
	{
    unsigned long vTableStartPage;
    dbMemcpyFreeIndian(&(ptable->tableNodeStartPage), &vTableStartPage, sizeof(unsigned long), 1);
    if (vTableStartPage != 0)
    {
      return;
    }
  }
	
	pcolumnOffset = &(ptable->tableNodeColumnOffset);
	
	//�ش� �÷��� �ִ����� �˻��Ѵ�.
	if (!dbTableFindColumnName(&pcolumnOffset, columnName))
	{
		//�ش� �÷��� �������� �ʴ°��
		return;
	}
	//���� if���� �ɸ��� �ʾҴٸ� �ش� �÷��� �����ϴ� ����̴�.
	
	dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);
	//�ش��÷��� �ε����� �ִ����� �˻��Ѵ�.
	
	pcolumn = (pColumnNode)( ((char*)pdicHeader) + vColumnOffset);

	dbMemcpyFreeIndian(&(pcolumn->columnNodeIndexRootOffset), &vIndexOffset, sizeof(unsigned long), 1);
	
	if (vIndexOffset != 0)
	{
		//�̹� �ش� �÷��� �ε����� �Ҵ�Ǿ� �ִ°���̴�.
		return;
	}
	//���� if���� �ɸ��� �ʾҴٸ� �ش� �÷��� �ε����� �����Ǿ� ���� ���� ����̴�.
	
	//�ش��÷��� �ε����� �������� ������� �ε����� �����Ѵ�.
	vIndexOffset = 1;
	dbMemcpyFreeIndian(&vIndexOffset, &(pcolumn->columnNodeIndexRootOffset), sizeof(unsigned long), 1);
		
	//dictionary�� ������ �߻��� ���� üũ
	gDbDictionaryChanged = DB_CHANGED;
}

unsigned long dbIndexGetCnode(DB_VARCHAR*		columnName[],
								 unsigned long		columnNumber)
{
//==================================================================================//
//	�Ʒ��� �Լ�dbIndexGetNode�� �������ڷ� ���� �����ε��� �÷��� �̸���			//
//	�Ķ���ͷ� �޾Ƽ� ��ųʸ� ���Ͽ� �߰��Ѵ�.										//
//	return����� �ش� ����� �������̴�.											//
//==================================================================================//
	pDictionaryHeader		pdicHeader;			//dictionary �ش��� ������ �� ����
	pComplexIndexNode					pcomplexIndexNode;	//�����ε����� �������� ����
	ComplexIndexNode					COMPLEX_INDEX_NODE;
	unsigned	long		dictionarySize;
	unsigned	long		requirementSize;		//�����ε����� �����Ǵµ� �ʿ��� ������ ������ ������ ����
	

	pdicHeader			 = dbGetDictionaryHeader();	//��ųʸ� �ش��� �����Ѵ�.
	
	dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderSizeOfDictionary), &dictionarySize, sizeof(unsigned long), 1);
	
	//�����ε����� ����� Dictionary AREA���� ��ġ
	pcomplexIndexNode = (pComplexIndexNode)(((char*)pdicHeader) + dictionarySize);
	
	COMPLEX_INDEX_NODE.complexIndexNodeColumnNumber		= columnNumber;
	COMPLEX_INDEX_NODE.complexIndexNodeIndexRootOffset	= 1;
	COMPLEX_INDEX_NODE.complexIndexNodeNext				= 0;
	
	//�����ε����� ������ Dictionary AREA�� �����Ѵ�.
	dbMemcpyFreeIndian(&COMPLEX_INDEX_NODE, pcomplexIndexNode, sizeof(unsigned long), 3);

	//����� ����Ѵ�.
	requirementSize = sizeof(ComplexIndexNode);

	//�����ε����� �÷��� �̸��� ������ ����Ѵ�.
	{
		unsigned long	i;
		unsigned long	unitStringSize;
		char			*target;

		target = (char*)(pcomplexIndexNode + 1);

		for(i=0; i<columnNumber; i++)
		{
			unitStringSize	=  dbStringCopy((DB_VARCHAR*)target, columnName[i]);
			requirementSize	+= unitStringSize;
			target				+= unitStringSize;
		}
	}

	return requirementSize;		//��尡 �����ϴ� ũ�⸦ �����Ѵ�.
}

void dbApiCreateIndexComplex(DB_VARCHAR*	tableName,
								 DB_VARCHAR*	columnNames[],
								 unsigned long	columnNumber)
{
	pDictionaryHeader	pdicHeader;
	pTableNode		ptable;

	unsigned	long		*ptableOffset;
	unsigned	long		tableOffset;
	unsigned	long		requirementSize;

#if DB_SHOW_ERROR_CODE
	gDbErrorCode = DB_ERROR_NO_ERROR;
#endif

//==============ERROR Code (Begin)==============//
if (gDbStartUpDB != DB_DO_RUNNING)
{
	#if DB_SHOW_ERROR_CODE
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;
	#endif
	return;
}
//==============ERROR Code ( end )==============//

	// === 1.�켱 Table�� �ִ����� ã�´�. === //
	pdicHeader		= dbGetDictionaryHeader();			//��ųʸ� �ش��� �����Ѵ�.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//���̺��� ����offset�� �������Ѵ�.

	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		return;	//�̰Ϳ� ������ ã�� ���̺��� ���� ����̴�.
	}//�� if�� ���� ������ ptableOffset�� �ش� ���̺��� ����Ų��.
	
	dbMemcpyFreeIndian(ptableOffset, &tableOffset, sizeof(unsigned long), 1);
	ptable = (pTableNode)(((char*)pdicHeader) + tableOffset);

  {
    unsigned long vTableStartPage;
    dbMemcpyFreeIndian(&(ptable->tableNodeStartPage), &vTableStartPage, sizeof(unsigned long), 1);
    if (vTableStartPage != 0)
    {
      return;
    }
  }

	// ===  2.�ش� �÷��� �����ϴ��� �˻��Ѵ�. === 
	{
		unsigned long	vColumnNumber;
		unsigned long	*pcolumnOffset;

		vColumnNumber = columnNumber;

		while(vColumnNumber)
		{
			vColumnNumber--;

			pcolumnOffset = &(ptable->tableNodeColumnOffset);
			
			if (!dbTableFindColumnName(&pcolumnOffset, columnNames[vColumnNumber]))
			{
				return;		//�ش� �÷��� ���°��
			}
		}
		//�ش� �÷��� ��� �����ϴ� ����̴�.
	}

	// ===  3. ��ųʸ� AREA�� ���ο� ��带 �߰��ϰ� �� ���̸� ���Ϲ޴´�. ===  
	requirementSize = dbIndexGetCnode(columnNames, columnNumber);	
	
	// === 4. �����ε����� �����Ű��, ��ųʸ� �ش��� �����Ѵ�. === //
	{
		// === 4.1 �����ε����� �����Ų��.
		pComplexIndexNode			pcindex;
		unsigned long	*pcindexOffset;
		unsigned long	vCindexOffset;
		unsigned long	vDictionarySize;

		pcindexOffset = &(ptable->tableNodeComplexIndexOffset);
		
		while(1)
		{
			dbMemcpyFreeIndian(pcindexOffset, &vCindexOffset, sizeof(unsigned long), 1);
			
			if (vCindexOffset == DB_NULL) break;
			
			pcindex = (pComplexIndexNode)(((char*)pdicHeader) + vCindexOffset);

			pcindexOffset = &(pcindex->complexIndexNodeNext);
		}
		
		dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderSizeOfDictionary), &vDictionarySize, sizeof(unsigned long), 1);

		dbMemcpyFreeIndian(&vDictionarySize, pcindexOffset, sizeof(unsigned long), 1);
		
		// === 4.2 ��ųʸ� �ش��� �����Ѵ�.
		vDictionarySize += requirementSize;
		dbMemcpyFreeIndian(&vDictionarySize, &(pdicHeader->dictionaryHeaderSizeOfDictionary), sizeof(unsigned long), 1);
	}
	gDbDictionaryChanged = DB_CHANGED;
}

void dbDataSetSlot(unsigned long slotNumber, 
					  unsigned long recordRowid)
{
	unsigned long*	slot;

	slot = (unsigned long*)(gDbMmuDataPage->mmuMemoryAddress + gDbPageSizeDataFilePageSize - (slotNumber*sizeof(unsigned long))); 
	recordRowid = recordRowid%gDbPageSizeDataFilePageSize;
	dbMemcpyFreeIndian(&recordRowid, slot, sizeof(unsigned long), 1);
	
	gDbMmuDataPage->mmuChanged = DB_CHANGED;
}



#if 0
unsigned long dbRecordGetSizeResultContents(unsigned long	startColumnOffset, 
							  					void*		contents[])
{
	// =====================���� ���� ����(����) =====================//
	pDictionaryHeader		pdicHeader;		//dictionary header�� ������ ����
	pColumnNode				pcolumn;			//dictionary memory�������� column����ü�� ������ �� ����
	unsigned	long		columnDataType;
	unsigned	long		recordSize;	//record�� size�� ������ ����
	unsigned	long		columnNumber;
	// =====================���� ���� ����(��) =====================//
	
	if (startColumnOffset == 0)
	{
		return 0;		//�÷��� �������� ���� ���
	}

	pdicHeader = dbGetDictionaryHeader();	//dictionary header�� �����Ѵ�.
	
	recordSize		= 0;	//record������ �ʱ������
	columnNumber	= 0;	//�÷��� ������ ������ �ӽú���
	
	while(startColumnOffset)
	{
		pcolumn = (pColumnNode)(((char*)pdicHeader) + startColumnOffset);	//�ش��÷��� �����Ѵ�.
		
		dbMemcpyFreeIndian(&(pcolumn->columnNodeDataType), &columnDataType, sizeof(unsigned long), 1);		//�޸𸮿� �ִ� �÷��� ������ ������ �����Ѵ�.

		if (columnDataType == DB_VARCHAR_TYPE)
		{
			//recordSize = recordSize + (2*(((struct mString*)(contents[columnNumber]))->NumChars)) + DB_VARCHAR_SIZE;	//060110 ����ö
		
			contents[columnNumber] = ((struct mString*)(contents[columnNumber]))->StringChars;
			
			recordSize = recordSize + dbStringGetSize(contents[columnNumber]) + DB_VARCHAR_SIZE;
		}
		else if (columnDataType == DB_UINT8_TYPE)
		{
			recordSize += DB_UINT8_SIZE;
		}
		else if (columnDataType == DB_UINT16_TYPE)
		{
			recordSize += DB_UINT16_SIZE;
		}
		else if (columnDataType == DB_UINT32_TYPE)
		{	
			recordSize += DB_UINT32_SIZE;
		}
		else
		{//���Ŀ� ������ �𸣴� DATATYPE�� 
		}

		//������° ó���� �÷��� ��ȣ�� columnNumber�� ���������ش�.
		columnNumber++;

		//���� �÷��� offset�� �����Ѵ�.
		dbMemcpyFreeIndian(&(pcolumn->columnNodeNextColumnOffset), &startColumnOffset, sizeof(unsigned long), 1);
	}
	return recordSize;
}
#endif
//####################################################################
unsigned short DB_NULL_STRING[]	= {0};		// 060112
unsigned char  DB_NULL_UINT8		= 0x00;
unsigned short DB_NULL_UINT16	= 0x0000;
unsigned long  DB_NULL_UINT32	= 0x00000000;

unsigned long dbRecordGetSize(unsigned long	startColumnOffset, 
							  void*		contents[])
{
	// =====================���� ���� ����(����) =====================//
	pDictionaryHeader		pdicHeader;		//dictionary header�� ������ ����
	pColumnNode				pcolumn;			//dictionary memory�������� column����ü�� ������ �� ����
	unsigned	long		columnDataType;
	unsigned	long		recordSize;	//record�� size�� ������ ����
	unsigned	long		columnNumber;
	// =====================���� ���� ����(��) =====================//
	
	if (startColumnOffset == 0)
	{
		return 0;		//�÷��� �������� ���� ���
	}

	pdicHeader = dbGetDictionaryHeader();	//dictionary header�� �����Ѵ�.
	
	recordSize		= 0;	//record������ �ʱ������
	columnNumber	= 0;	//�÷��� ������ ������ �ӽú���
	
	while(startColumnOffset)
	{
		pcolumn = (pColumnNode)(((char*)pdicHeader) + startColumnOffset);	//�ش��÷��� �����Ѵ�.
		
		dbMemcpyFreeIndian(&(pcolumn->columnNodeDataType), &columnDataType, sizeof(unsigned long), 1);		//�޸𸮿� �ִ� �÷��� ������ ������ �����Ѵ�.

		if (columnDataType == DB_VARCHAR_TYPE)
		{
			if (contents[columnNumber]== 0 )
			{
				contents[columnNumber] = DB_NULL_STRING;
			}
			recordSize = recordSize + dbStringGetSize(contents[columnNumber]) + DB_VARCHAR_SIZE;
		}
		else if (columnDataType == DB_UINT8_TYPE)
		{
			if (contents[columnNumber]== 0 )
			{
				contents[columnNumber] = &DB_NULL_UINT8;
			}
			recordSize += DB_UINT8_SIZE;
		}
		else if (columnDataType == DB_UINT16_TYPE)
		{
			if (contents[columnNumber]== 0 )
			{
				contents[columnNumber] = &DB_NULL_UINT16;
			}
			recordSize += DB_UINT16_SIZE;
		}
		else if (columnDataType == DB_UINT32_TYPE)
		{	
			if (contents[columnNumber]== 0 )
			{
				contents[columnNumber] = &DB_NULL_UINT32;
			}
			recordSize += DB_UINT32_SIZE;
		}
		else
		{//���Ŀ� ������ �𸣴� DATATYPE�� 
		}

		//������° ó���� �÷��� ��ȣ�� columnNumber�� ���������ش�.
		columnNumber++;

		//���� �÷��� offset�� �����Ѵ�.
		dbMemcpyFreeIndian(&(pcolumn->columnNodeNextColumnOffset), &startColumnOffset, sizeof(unsigned long), 1);
	}
	return recordSize;
}



unsigned long dbApiRecordInsertMS(DB_VARCHAR	*dbTableName,
								  void*			dbContents[],
								  unsigned char 	dbUsingInUpdate)
{
	// =====================�������� ����(����) =====================//
	pDictionaryHeader		pdictionaryHeader;	//dictionary header�� ������ �����Ͽ� ���� ����
	pTableNode			pTable;				//dictionary���� ���̺��� ������ �� ����
	TableNode			vTable;
	unsigned	long			*pTableOffset,	//dictionary file���� ���̺��� offset�� ����� ������ ������ �� ����
						*pColumnOffset;	//dictionary file���� �÷��� offset�� ����� ������ ������ �� ����
	unsigned	long			vTableOffset,	//���̺��� offset�� ������ �ӽú���
						vColumnOffset;
	unsigned	long			vRecordRowid;
	unsigned	long			vRequirementSize;//record�� ���ԵǴµ� �ʿ��� ����� ������ ����
	// =====================�������� ����(��) =====================//
	//unsigned long Time1, Time2;
	//Time1 = OSTimeGet();
	#if DB_SHOW_ERROR_CODE				//Error Check
	gDbErrorCode = DB_ERROR_NO_ERROR;		//Error Check
	#endif									//Error Check


	if (gDbStartUpDB != DB_DO_RUNNING)
	{
		#if DB_SHOW_ERROR_CODE								//Error Check
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;		//Error Check
		#endif													//Error Check

		return 0;
	}


	pdictionaryHeader	= dbGetDictionaryHeader();
	pTableOffset		= &(pdictionaryHeader->dictionaryHeaderOffsetOfRootTable);	//���̺��� ���� offset�� �����Ѵ�.
	
	
	if (!dbTableFindTableName(&pTableOffset, dbTableName))
	{
		//���̺��� �������� �ʴ� ����̴�.
		
		return 0;
	}
	//���� if���� �ɸ��� �ʾҴٴ°��� ���̺��� �����ϴ� ���̴�.
	
	//ptableOffset���� ã�� �÷��� Offset�� ����Ǿ� �ִ�.
	dbMemcpyFreeIndian(pTableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//pTable�� �ش� Table�� �������ϰ� �Ѵ�.
	pTable = (pTableNode)(((char*)pdictionaryHeader) + vTableOffset);
	dbMemcpyFreeIndian(pTable, &vTable, sizeof(unsigned long), 6);
	
	
	//�÷����� Offset�� �����ϰ� �ִ� �ּҸ� �����Ѵ�.
	pColumnOffset = &(pTable->tableNodeColumnOffset);
	dbMemcpyFreeIndian(pColumnOffset, &vColumnOffset, sizeof(unsigned long), 1);

	if (vColumnOffset == 0)	//column�� �������� �����Ƿ� ������ ���� �ʴ´�.
	{
		return 0;			//�÷��� �������� �ʽ��ϴ�.
	}
	//���� if���� �ɸ��� �ʾҴٸ� column�� �����ϹǷ� ������ �����Ѵ�.

	//Record�� size�� ����Ѵ�.
	/*
	if (dbUsingInUpdate == DB_USING_IN_UPDATE)
	{
		vRequirementSize = dbRecordGetSizeResultContents(vColumnOffset, dbContents);
	}
	else if (dbUsingInUpdate == DB_NOT_USING_IN_UPDATE)
	{
	*/
		vRequirementSize = dbRecordGetSize(vColumnOffset, dbContents);
	/*}
	else
	{
		//DbgPrintf(("\ndbApiRecordInsertMS :: �Լ� ����� ���޹��� ���� dbUsingInUpdate�� ���� ��ȿ�� ���� �ƴմϴ�. =============================\n"));
		return 0;
	}*/

	if (vRequirementSize < 8) vRequirementSize = 8;

	if ((vRequirementSize + sizeof(DataFilePageHeader) + sizeof(unsigned long)) > gDbPageSizeDataFilePageSize ) 
	{
		return 0;	//gDbPageSizeDataFilePageSize�� �������Ѿ� �ȴ�.
	}

	
	if (vTable.tableNodeStartPage == 0)		//table�� ó������ data page�� �Ҵ�� ��� ó��
	{
		unsigned long	vDataFileEndPageNumber;

		//��ųʸ��� ����Ǿ� �ִ� datafile�� ������ ��ȣ�� �о�´�.
		dbMemcpyFreeIndian(&(pdictionaryHeader->dictionaryHeaderNumberOfDataFileEndPage), 
						   &vDataFileEndPageNumber, 
						   sizeof(unsigned long), 1);
		
		//���̺��� ����Page��ȣ�� datafile�� ��ȣ�� �Ҵ��Ѵ�.
		vTable.tableNodeStartPage = vDataFileEndPageNumber + 1;
		
		dbMemcpyFreeIndian(&(vTable.tableNodeStartPage), 
						   &(pTable->tableNodeStartPage), 
						   sizeof(unsigned long), 1);

		gDbDictionaryChanged = DB_CHANGED;

		//���� ����� Page�� LRU List�� ���� �տ� ��ġ�Ѵ�.
		dbTouch((vTable.tableNodeStartPage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
			
		//record�� �����Ѵ�.
		vRecordRowid = dbDataSetRecordMS(vTable.tableNodeColumnOffset, dbContents, vRequirementSize);

		//������ page�� ���ٴ� �����Ͽ� ���� �Ҵ�� Page�� �ϳ��� page�� ������ ���̹Ƿ� page�� ��ȣ�� ������Ų��.
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
												
		//Dictionary ������ �ٲ���� �˸���.
		gDbDictionaryChanged = DB_CHANGED;	//dictionary�� ������ �߻��� ���� üũ
	}
	else	//table�� �̹� �Ҵ�� data page�� ������ ���
	{

		//table�� �Ҵ�� ���� Page�� MEMORY�� LOAD�Ѵ�.
		dbTouch((vTable.tableNodeStartPage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		
		//record�� �����Ѵ�.
		vRecordRowid = dbDataSetRecordMS(vTable.tableNodeColumnOffset, dbContents, vRequirementSize);
		
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
	}

	//�����ε����� �����Ѵ�.
	if ( vRecordRowid != 0 )	//060110..��ö.
	{
		dbIndexComplexAllInsertMS(vTableOffset, dbContents,vRecordRowid);
	}
	//Time2 = OSTimeGet();
	//DbgPrintf(("DB Insert�� �ɸ��� �ð�[%d]\n", Time2-Time1 ));
	return vRecordRowid;
}

unsigned long dbDataSetRecordMS(unsigned long	dbStartColumnOffset,
								      void*			dbContents[], 
								      unsigned long	dbRequirementSize)
{
	//=======================================================================================================================//
	// ��   ��   �� : dbDataSetRecordMS																						 //
	// ��   ��   �� : �� �� ö																								 //
	// ��        �� : �� �Լ��� ���ڷ� �޴� dbStartColumnOffset�� ���� ���� Data Page�� �ش� ���ڵ带 ������ �Ƿڸ� ó���ϴ� //
	//				 �Լ��̴�.																								 //
	//				  ������ Page�õ��ϴ� Page�� ������ ������(������ ���� �Ǵ� ���������̽�) �ش� Page�� ������ �ε����� �� //
	//				 ���ϰ�, ������ �Ұ����� ��쿡�� ������ ������ �������� ã�ư��� ������ �õ��Ѵ�.						 //
	//=======================================================================================================================//

	pDataFilePageHeader		pDataPageHeader;
	DataFilePageHeader		vDataPageHeader;
	unsigned	long				vFreeSpaceInDatapage;
	unsigned	long				vRecordRowid;
	char						*pRecord;

	//���� ������ �������� ������ �ش��� �޾ƿ´�.
	pDataPageHeader	= (pDataFilePageHeader)(gDbMmuDataPage->mmuMemoryAddress);

	dbMemcpyFreeIndian(pDataPageHeader,
					   &vDataPageHeader,
					   sizeof(unsigned long), 4);
	
	
	//������ Data page������ free space�� ���
	vFreeSpaceInDatapage = gDbPageSizeDataFilePageSize 
						 - (vDataPageHeader.dataHeaderEndOfRecords + (vDataPageHeader.dataHeaderNumberOfSlots * DB_SLOT_SIZE));

	pRecord = DB_NULL;
	
	//====������ ������ ������ �ִٸ� ������ ������ ���ڵ尡 ���Ե� ������ �ִ����� ã�´�.(begin)=====//
	if (vFreeSpaceInDatapage >= DB_SLOT_SIZE)
	{
		if (vDataPageHeader.dataHeaderDeleteRecordOffset != 0)		//DATA PAGE���� ������ ������ �����ϸ� ������ �������� ����Ҽ� �ִ� ���� �ִ����� ���캻��.

		{
			pRecord = dbDataGetPageHaveRequirementSizeInDeleteNodes(dbRequirementSize);
		}
	}
	//====������ ������ ������ �ִٸ� ������ ������ ���ڵ尡 ���Ե� ������ �ִ����� ã�´�.( end )=====//
	

	//============������ �����߿� �����Ͱ� ���Ե� �� �ִ� ��Ȳ�� ��� ó�� (begin)============//
	if (pRecord != DB_NULL)		//���������� Ȱ�밡���� ���
	{//���ڵ尡 ���ԵǴ� �ΰ��߿� ù��° ���

		vRecordRowid = (gDbMmuDataPage->mmuPageNumber - 1)*gDbPageSizeDataFilePageSize 
					+ (pRecord - gDbMmuDataPage->mmuMemoryAddress);	//Data Area�� ��ϵ� Record�� Rowid�� ���Ѵ�.


		dbDataSetRecordReal(pRecord, dbStartColumnOffset, dbContents);				//Data Area�� Record�� ����Ѵ�.


		dbDataSetSlot(vDataPageHeader.dataHeaderNumberOfSlots + 1, vRecordRowid);	//Data Page�� Record�� Slot�� ����Ѵ�.


		{//===========================�ش� page�� header�� �����Ѵ�.(begin)===========================//

			//vDataPageHeader.dataHeaderEndOfRecords += DB_SLOT_SIZE;
			vDataPageHeader.dataHeaderNumberOfSlots++ ;

			//dbMemcpyFreeIndian(&(vDataPageHeader.dataHeaderEndOfRecords),	
			//				   &(pDataPageHeader->dataHeaderEndOfRecords),
			//				   sizeof(unsigned long), 1); 
			dbMemcpyFreeIndian(&(vDataPageHeader.dataHeaderNumberOfSlots),
							   &(pDataPageHeader->dataHeaderNumberOfSlots),
							   sizeof(unsigned long), 1);

			gDbMmuDataPage->mmuChanged = DB_CHANGED;				//�ش� Data page�� �����ϴ� MMU�� ������ ����Ǿ����� ǥ���Ѵ�.
		
		}//===========================�ش� page�� header�� �����Ѵ�.( end )===========================//

//DbgPrintf(("�̱��ε��� �Է��ϴ� �����Ű��� before\n"));
		dbIndexSingleAllInsertMS(dbStartColumnOffset, dbContents, vRecordRowid);
//DbgPrintf(("�̱��ε��� �Է��ϴ� �����Ű��� after\n"));
	}
	//============������ �����߿� �����Ͱ� ���Ե� �� �ִ� ��Ȳ�� ��� ó�� ( end )============//



	//====================================������ �����߿� �����Ͱ� ���Ե� �� ���� ��Ȳ�� ��� ó�� (begin)====================================//
	//========================���� Page�� Free Space�� ũ�Ⱑ Ŀ�� ���� ���� Record�� ���԰����� ��� (begin)========================//
	else if (vFreeSpaceInDatapage >= (dbRequirementSize + DB_SLOT_SIZE))	//������ Data Page�� Record�� ������ ������ ���
	{//���ڵ尡 ���ԵǴ� �ΰ��߿� �ι�° ���

		vRecordRowid = (gDbMmuDataPage->mmuPageNumber - 1)*gDbPageSizeDataFilePageSize 
					+ vDataPageHeader.dataHeaderEndOfRecords;		//Data Area�� ��ϵ� Record�� Rowid�� ���Ѵ�.
		
		
		pRecord = gDbMmuDataPage->mmuMemoryAddress + vDataPageHeader.dataHeaderEndOfRecords;	//Data Area�� Record�� ��ϵ� �ּҰ��� ���Ѵ�.
		
		
		dbDataSetRecordReal(pRecord, dbStartColumnOffset, dbContents);							//Data Area�� Record�� ����Ѵ�.
		
		
		dbDataSetSlot(vDataPageHeader.dataHeaderNumberOfSlots + 1, vRecordRowid);				//Data Page�� Record�� Slot�� ����Ѵ�.
		

		{//===========================�ش� page�� header�� �����Ѵ�.(begin)===========================//
			vDataPageHeader.dataHeaderEndOfRecords += dbRequirementSize;
			vDataPageHeader.dataHeaderNumberOfSlots++ ;
			dbMemcpyFreeIndian(&(vDataPageHeader.dataHeaderEndOfRecords),
							&(pDataPageHeader->dataHeaderEndOfRecords),
							sizeof(unsigned long), 1); 
			dbMemcpyFreeIndian(&(vDataPageHeader.dataHeaderNumberOfSlots),
							&(pDataPageHeader->dataHeaderNumberOfSlots),
							sizeof(unsigned long), 1);

			gDbMmuDataPage->mmuChanged = DB_CHANGED;				//�ش� Data page�� �����ϴ� MMU�� ������ ����Ǿ����� ǥ���Ѵ�.
		}//===========================�ش� page�� header�� �����Ѵ�.( end )===========================//
//DbgPrintf(("�̱��ε��� �Է��ϴ� �����Ű��� before\n"));
		dbIndexSingleAllInsertMS(dbStartColumnOffset, dbContents, vRecordRowid);
//DbgPrintf(("�̱��ε��� �Է��ϴ� �����Ű��� after\n"));
	}
	//========================���� Page�� Free Space�� ũ�Ⱑ Ŀ�� ���� ���� Record�� ���԰����� ��� ( end )========================//


	//========================���� Page�� Free Space�� ũ�Ⱑ �۾Ƽ� ���� ���̺� �Ҵ�� ���ο� Page���� ������ �õ��ؾ� �� ���(begin)========================//
	//============���� Page�� ����� Page�� ���� ���(begin)=============//
	else if(vDataPageHeader.dataHeaderNextSameTablePage != 0)		//���ӵ� �ٸ� page�� �ִٸ�
	{
		//ã�� page�� ���� ������ ���� �Ѵ�.
		dbTouch((vDataPageHeader.dataHeaderNextSameTablePage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		
		//���� Table�� ���ӵ� page�� �ٽ� Record������ �Ƿ��Ѵ�.
		vRecordRowid = dbDataSetRecordMS(dbStartColumnOffset, dbContents, dbRequirementSize);
	}
	//============���� Page�� ����� Page�� ���� ���( end )=============//
	

	//============���� Page�� ������ Page�� ���(begin)=============//
	else //������ page�� ��� (dataPageHeader.dataHeaderNextSameTablePage == 0) 
	{
		pDictionaryHeader		pdictionaryHeader;
		unsigned long			vDataFileEndPageOfDictionaryHeader;

		//Dictionary header�� ���� Data file�� ������ page��ȣ�� �޾ƿ´�.
		pdictionaryHeader = dbGetDictionaryHeader();

		dbMemcpyFreeIndian(&(pdictionaryHeader->dictionaryHeaderNumberOfDataFileEndPage),
						   &vDataFileEndPageOfDictionaryHeader,
						   sizeof(unsigned long), 1);
		
		//���� page�� ���� page��ũ�� ���� ������ page�� �����Ų��.
		vDataPageHeader.dataHeaderNextSameTablePage = vDataFileEndPageOfDictionaryHeader + 1;

		dbMemcpyFreeIndian(&(vDataPageHeader.dataHeaderNextSameTablePage),
						   &(pDataPageHeader->dataHeaderNextSameTablePage),
						   sizeof(unsigned long), 1);

		gDbMmuDataPage->mmuChanged = DB_CHANGED;
		
		//ã�� page�� ���� ������ ���� �Ѵ�.
		dbTouch((vDataPageHeader.dataHeaderNextSameTablePage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
		
		//���� Table�� ���ӵ� page�� �ٽ� Record������ �Ƿ��Ѵ�.
		vRecordRowid = dbDataSetRecordMS(dbStartColumnOffset, dbContents, dbRequirementSize);
	}
	//============���� Page�� ������ Page�� ���( end )=============//
	//========================���� Page�� Free Space�� ũ�Ⱑ �۾Ƽ� ���� ���̺� �Ҵ�� ���ο� Page���� ������ �õ��ؾ� �� ���( end )========================//
	//====================================������ �����߿� �����Ͱ� ���Ե� �� ���� ��Ȳ�� ��� ó�� ( end )====================================//

	return vRecordRowid;	//���ڵ尡 ���Ե� ������ �ѱ��.*/
}

/*
void dbApiRecordInsert(DB_VARCHAR	*tableName,
					   void*			contents[])
{
	// =====================�������� ����(����) =====================//
	pDictionaryHeader		pdicHeader;		//dictionary header�� ������ �����Ͽ� ���� ����
	pTableNode					ptable;				//dictionary���� ���̺��� ������ �� ����
	TableNode					TABLE_NODE;
	unsigned	long		*ptableOffset,			//dictionary file���� ���̺��� offset�� ����� ������ ������ �� ����
							*pcolumnOffset;		//dictionary file���� �÷��� offset�� ����� ������ ������ �� ����
	unsigned	long		tableOffset,				//���̺��� offset�� ������ �ӽú���
							columnOffset;
	unsigned	long		recordRowid;
	unsigned	long		requirementSize;			//record�� ���ԵǴµ� �ʿ��� ����� ������ ����
	// =====================�������� ����(��) =====================//
	
	pdicHeader		= dbGetDictionaryHeader();
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);					//���̺��� ���� offset�� �����Ѵ�.
	
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//���̺��� �������� �ʴ� ����̴�.
		return;
	}
	//���� if���� �ɸ��� �ʾҴٴ°��� ���̺��� �����ϴ� ���̴�.
	
	//ptableOffset���� ã�� �÷��� Offset�� ����Ǿ� �ִ�.
	dbMemcpyFreeIndian(ptableOffset, &tableOffset, sizeof(unsigned long), 1);

	//pTable�� �ش� Table�� �������ϰ� �Ѵ�.
	ptable = (pTableNode)(((char*)pdicHeader) + tableOffset);
	dbMemcpyFreeIndian(ptable, &TABLE_NODE, sizeof(unsigned long), 6);

	//�÷����� Offset�� �����ϰ� �ִ� �ּҸ� �����Ѵ�.
	pcolumnOffset = &(ptable->tableNodeColumnOffset);
	dbMemcpyFreeIndian(pcolumnOffset, &columnOffset, sizeof(unsigned long), 1);

	if (columnOffset == 0)	//column�� �������� �����Ƿ� ������ ���� �ʴ´�.
	{
		return;			//�÷��� �������� �ʽ��ϴ�.
	}
	//���� if���� �ɸ��� �ʾҴٸ� column�� �����ϹǷ� ������ �����Ѵ�.

	//Record�� size�� ����Ѵ�.
	requirementSize = dbRecordGetSize(columnOffset, contents);

	if (requirementSize < 8) requirementSize = 8;

	if ( (requirementSize + sizeof(DataFilePageHeader) + sizeof(unsigned long)) > gDbPageSizeDataFilePageSize ) return;	//gDbPageSizeDataFilePageSize�� �������Ѿ� �ȴ�.
	
	if (TABLE_NODE.tableNodeStartPage == 0)		//table�� ó������ data page�� �Ҵ�� ��� ó��
	{
		unsigned long datafile_endpageNumber;

		//��ųʸ��� ����Ǿ� �ִ� datafile�� ������ ��ȣ�� �о�´�.
		dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderNumberOfDataFileEndPage), &datafile_endpageNumber, sizeof(unsigned long), 1);
		
		//���̺��� ����Page��ȣ�� datafile�� ��ȣ�� �Ҵ��Ѵ�.
		TABLE_NODE.tableNodeStartPage = datafile_endpageNumber + 1;
		dbMemcpyFreeIndian(&(TABLE_NODE.tableNodeStartPage), &(ptable->tableNodeStartPage), sizeof(unsigned long), 1);
		gDbDictionaryChanged = DB_CHANGED;

		//���� ����� Page�� LRU List�� ���� �տ� ��ġ�Ѵ�.
		dbTouch((TABLE_NODE.tableNodeStartPage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
			
		//record�� �����Ѵ�.
		recordRowid = dbDataSetRecord(TABLE_NODE.tableNodeColumnOffset, contents, requirementSize);

		//������ page�� ���ٴ� �����Ͽ� ���� �Ҵ�� Page�� �ϳ��� page�� ������ ���̹Ƿ� page�� ��ȣ�� ������Ų��.
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
												
		//Dictionary ������ �ٲ���� �˸���.
		gDbDictionaryChanged = DB_CHANGED;	//dictionary�� ������ �߻��� ���� üũ
	}
	else	//table�� �̹� �Ҵ�� data page�� ������ ���
	{
		//table�� �Ҵ�� ���� Page�� MEMORY�� LOAD�Ѵ�.
		dbTouch((TABLE_NODE.tableNodeStartPage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		
		//record�� �����Ѵ�.
		recordRowid = dbDataSetRecord(TABLE_NODE.tableNodeColumnOffset, contents, requirementSize);
		
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
	}

	//�����ε����� �����Ѵ�.
	dbIndexComplexAllInsert(tableOffset, contents,recordRowid);
}
*/


void dbDataSetRecordReal(char			*pRecord, 
						 unsigned long	dbColumnStartAddress, 
						 void*			dbContents[])
{

	pDictionaryHeader	pdictionaryHeader;
	pColumnNode			pcolumn;
	unsigned long		vColumnCount;
	unsigned long		vColumnType;

	pdictionaryHeader  = dbGetDictionaryHeader();

	vColumnCount = 0;
	
	while(dbColumnStartAddress)
	{
		//������ �÷��� ����Ʈ�Ѵ�.
		pcolumn = (pColumnNode)(((char*)pdictionaryHeader) + dbColumnStartAddress);
		
		//���� �÷��� Ÿ���� �޾ƿ´�.
		dbMemcpyFreeIndian(&(pcolumn->columnNodeDataType), &vColumnType, sizeof(unsigned long), 1);

		//������ �÷��� DATA AREA�� �����Ѵ�.
		{
			if (vColumnType == DB_VARCHAR_TYPE)
			{
//char* temp =  pRecord;
//showTwoByteString("���� ���ڿ�", dbContents[vColumnCount]);
				pRecord += dbStringCopy((DB_VARCHAR*)pRecord, (DB_VARCHAR*)dbContents[vColumnCount]);
//showTwoByteString("��ϵ� ���ڿ�", temp);
//unsigned long temp;
//temp = dbStringCopy((DB_VARCHAR*)pRecord, (DB_VARCHAR*)dbContents[vColumnCount]);
//pRecord += temp;
//DbgPrintf(("����Ҷ� ������ [%x]\n", temp));
		}
			else if (vColumnType == DB_UINT8_TYPE)
			{
				dbMemcpyFreeIndian(dbContents[vColumnCount], pRecord, DB_UINT8_SIZE, 1);
				pRecord += DB_UINT8_SIZE;
			}
			else if (vColumnType == DB_UINT16_TYPE)
			{
				dbMemcpyFreeIndian(dbContents[vColumnCount], pRecord, DB_UINT16_SIZE, 1);
				pRecord += DB_UINT16_SIZE;
			}
			else if (vColumnType == DB_UINT32_TYPE)
			{
				dbMemcpyFreeIndian(dbContents[vColumnCount], pRecord, DB_UINT32_SIZE, 1);
				pRecord += DB_UINT32_SIZE;
			}
			else{}
		}

		//���� �÷��� �����ϱ� ���� �۾��� �Ѵ�.
		dbMemcpyFreeIndian(&(pcolumn->columnNodeNextColumnOffset), &dbColumnStartAddress, sizeof(unsigned long), 1);
		vColumnCount++;
	}
}
/*
unsigned long dbDataSetRecord(unsigned long	startColumnOffset,
								 void*			contents[], 
								 unsigned long	requirementSize)
{
	pDataFilePageHeader		pDataPageHeader;
	DataFilePageHeader		dataPageHeader;
	unsigned	long		freeSpaceInDatapage;
	unsigned	long		recordRowid;
	char					*pRecord;

	//���� ������ �������� ������ �ش��� �޾ƿ´�.
	pDataPageHeader	= (pDataFilePageHeader)(gDbMmuDataPage->mmuMemoryAddress);
	dbMemcpyFreeIndian(pDataPageHeader, &dataPageHeader, sizeof(unsigned long), 4);
	
	

	//������ Data page������ free space�� ���
	freeSpaceInDatapage = gDbPageSizeDataFilePageSize 
						   - (dataPageHeader.dataHeaderEndOfRecords + (dataPageHeader.dataHeaderNumberOfSlots * DB_SLOT_SIZE));

	pRecord = DB_NULL;
	
	if (freeSpaceInDatapage >= DB_SLOT_SIZE)	// ������ ������ ������ �ִٸ�
	{
		if (dataPageHeader.dataHeaderDeleteRecordOffset != 0)		//DATA PAGE���� ������ ������ �����ϸ� ������ �������� ����Ҽ� �ִ� ���� �ִ����� ���캻��.

		{
			pRecord = dbDataGetPageHaveRequirementSizeInDeleteNodes(requirementSize);
		}
	}
	
	if (pRecord != DB_NULL)		//���������� Ȱ�밡���� ���
	{
		//recordRowid�� ���Ѵ�.
		recordRowid = (gDbMmuDataPage->mmuPageNumber - 1)*gDbPageSizeDataFilePageSize 
					 + (pRecord - gDbMmuDataPage->mmuMemoryAddress);

		//���� Data AREA�� ���ڵ带 ����Ѵ�.
		dbDataSetRecordReal(pRecord, startColumnOffset, contents);

		//�ش� slot�� ����Ѵ�.
		dbDataSetSlot(dataPageHeader.dataHeaderNumberOfSlots + 1, recordRowid);

		//�ش� page�� header�� �����Ѵ�.
		dataPageHeader.dataHeaderEndOfRecords += DB_SLOT_SIZE;
		dataPageHeader.dataHeaderNumberOfSlots++ ;
		dbMemcpyFreeIndian(&(dataPageHeader.dataHeaderEndOfRecords), &(pDataPageHeader->dataHeaderEndOfRecords), sizeof(unsigned long), 1); 
		dbMemcpyFreeIndian(&(dataPageHeader.dataHeaderNumberOfSlots), &(pDataPageHeader->dataHeaderNumberOfSlots), sizeof(unsigned long), 1);

		//�ش� Data page�� �����ϴ� MMU�� ������ ����Ǿ����� ǥ���Ѵ�.
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
		
		//���ڵ尡 ���Ե� ������ freeSpace ������ �����Ѵ�.
		dbIndexSingleAllInsert(startColumnOffset, contents, recordRowid);
	}
	else if (freeSpaceInDatapage >= (requirementSize + DB_SLOT_SIZE))	//������ Data Page�� Record�� ������ ������ ���
	{
		//����� Record�� rowid�� ����Ѵ�. 
		recordRowid = (gDbMmuDataPage->mmuPageNumber - 1)*gDbPageSizeDataFilePageSize + dataPageHeader.dataHeaderEndOfRecords;
		
		//����� Record�� memory address�� ã�´�.
		pRecord = gDbMmuDataPage->mmuMemoryAddress + dataPageHeader.dataHeaderEndOfRecords;
		
		//���� Data AREA�� ���ڵ带 ����Ѵ�.
		dbDataSetRecordReal(pRecord, startColumnOffset, contents);
		
		//�ش� slot�� ����Ѵ�.
		dbDataSetSlot(dataPageHeader.dataHeaderNumberOfSlots + 1, recordRowid);
		
		//�ش� page�� header�� �����Ѵ�.
		dataPageHeader.dataHeaderEndOfRecords += requirementSize;
		dataPageHeader.dataHeaderNumberOfSlots++ ;
		dbMemcpyFreeIndian(&(dataPageHeader.dataHeaderEndOfRecords), &(pDataPageHeader->dataHeaderEndOfRecords), sizeof(unsigned long), 1); 
		dbMemcpyFreeIndian(&(dataPageHeader.dataHeaderNumberOfSlots), &(pDataPageHeader->dataHeaderNumberOfSlots), sizeof(unsigned long), 1);

		//�ش� Data page�� �����ϴ� MMU�� ������ ����Ǿ����� ǥ���Ѵ�.
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
		
		//���ڵ尡 ���Ե� ������ freeSpace ������ �����Ѵ�.
		dbIndexSingleAllInsert(startColumnOffset, contents, recordRowid);
	}
	else if(dataPageHeader.dataHeaderNextSameTablePage != 0)		//���ӵ� �ٸ� page�� �ִٸ�
	{
		//ã�� page�� ���� ������ ���� �Ѵ�.
		dbTouch((dataPageHeader.dataHeaderNextSameTablePage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		
		//���� Table�� ���ӵ� page�� �ٽ� Record������ �Ƿ��Ѵ�.
		recordRowid = dbDataSetRecord(startColumnOffset, contents, requirementSize);
	}
	else //������ page�� ��� (dataPageHeader.dataHeaderNextSameTablePage == 0) 
	{
		pDictionaryHeader		pdicHeader;
		unsigned long			datafile_endpageOfDictionaryHeader;

		//Dictionary header�� ���� Data file�� ������ page��ȣ�� �޾ƿ´�.
		pdicHeader = dbGetDictionaryHeader();
		dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderNumberOfDataFileEndPage),
							  &datafile_endpageOfDictionaryHeader,
							  sizeof(unsigned long), 1);
		
		//���� page�� ���� page��ũ�� ���� ������ page�� �����Ų��.
		dataPageHeader.dataHeaderNextSameTablePage = datafile_endpageOfDictionaryHeader + 1;
		dbMemcpyFreeIndian(&(dataPageHeader.dataHeaderNextSameTablePage),
							  &(pDataPageHeader->dataHeaderNextSameTablePage),
							  sizeof(unsigned long), 1);
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
		
		//ã�� page�� ���� ������ ���� �Ѵ�.
		dbTouch((dataPageHeader.dataHeaderNextSameTablePage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		
		//���� Table�� ���ӵ� page�� �ٽ� Record������ �Ƿ��Ѵ�.
		recordRowid = dbDataSetRecord(startColumnOffset, contents, requirementSize);
	}
	return recordRowid;	//���ڵ尡 ���Ե� ������ �ѱ��.
}*/

char* dbIndexGetPageHaveRequirementSizeInDeleteNodes(unsigned long requirementSize)
{
	//�ε��� Page�� ��������߿��� �ʿ�������� ������尡 ���� ��� �ش� memory�ּҸ� �������ش�.
	//���� ��쿡�� DB_NULL���� �����Ѵ�.
	pIndexFilePageHeader		pIndexPageHeader;
	unsigned long				*pIndexHeaderDeleteNodeOffset;
	unsigned long				indexHeaderDeleteNodeOffset;
	
	if (gDbMmuIndexPage->mmuPageNumber == 0)
	{
		//dbTouch(0, gDbPageSizeIndexFilePageSize);
		dbTouch(1, DB_INDEX_FILE_LOAD);
		return DB_NULL;
	}
	else
	{
		pDeleteNode		pdeleteNode;
		DeleteNode		deleteNode;

		pIndexPageHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);
		pIndexHeaderDeleteNodeOffset = &(pIndexPageHeader->indexHeaderDeleteNodeOffset);
		dbMemcpyFreeIndian(pIndexHeaderDeleteNodeOffset, &indexHeaderDeleteNodeOffset, sizeof(unsigned long), 1);

		while(indexHeaderDeleteNodeOffset)
		{
			pdeleteNode = (pDeleteNode)(((char*)pIndexPageHeader) + indexHeaderDeleteNodeOffset);
			dbMemcpyFreeIndian(pdeleteNode, &deleteNode, sizeof(unsigned long), 2);
			
			if(requirementSize == deleteNode.deleteNodeSize)
			{
				dbMemcpyFreeIndian(&(deleteNode.deleteNodeNext), pIndexHeaderDeleteNodeOffset, sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;

				return (char*)pdeleteNode;
			}
			
			pIndexHeaderDeleteNodeOffset = &(pdeleteNode->deleteNodeNext);
			indexHeaderDeleteNodeOffset = deleteNode.deleteNodeNext;
		}
		return 0;
	}
}
char* dbDataGetPageHaveRequirementSizeInDeleteNodes(unsigned long requirementSize)
{
	//data Page�� ��������߿��� �ʿ�������� ������尡 ���� ��� �ش� memory�ּҸ� �������ش�.
	//���� ��쿡�� DB_NULL���� �����Ѵ�.
	pDataFilePageHeader			pDataPageHeader;
	unsigned long				*pDataHeaderDeleteNodeOffset;
	unsigned long				DataHeaderDeleteNodeOffset;
	
	//DbgPrint(("���� �����߿� �ʿ��� ������  �ִ��� ã�� ��ƾ "));
	
	if (gDbMmuDataPage->mmuPageNumber == 0)
	{
		//dbTouch(0, gDbPageSizeDataFilePageSize);
		return DB_NULL;
	}
	else
	{
		pDeleteNode		pdeleteNode;
		DeleteNode		deleteNode;

		pDataPageHeader = (pDataFilePageHeader)(gDbMmuDataPage->mmuMemoryAddress);

		pDataHeaderDeleteNodeOffset = &(pDataPageHeader->dataHeaderDeleteRecordOffset);
		
		dbMemcpyFreeIndian(pDataHeaderDeleteNodeOffset, &DataHeaderDeleteNodeOffset, sizeof(unsigned long), 1);

		while(DataHeaderDeleteNodeOffset)
		{
			pdeleteNode = (pDeleteNode)(((char*)pDataPageHeader) + DataHeaderDeleteNodeOffset);
			
			dbMemcpyFreeIndian(pdeleteNode, &deleteNode, sizeof(unsigned long), 2);
			
			if(requirementSize == deleteNode.deleteNodeSize)
			{
				
				dbMemcpyFreeIndian(&(deleteNode.deleteNodeNext), pDataHeaderDeleteNodeOffset, sizeof(unsigned long), 1);
				return (char*)pdeleteNode;
			}
			
			pDataHeaderDeleteNodeOffset = &(pdeleteNode->deleteNodeNext);
			DataHeaderDeleteNodeOffset = deleteNode.deleteNodeNext;
		}
		return 0;
	}
}

char* dbIndexGetPageHaveRequirementSizeInFreespace(unsigned long requirementSize)
{
	//�ε��� page�� ���������� �ʿ�� �ϴ� ����� �����ϴ����� �Ǵ��ؼ� �����Ұ�쿡 �ش� �ּҸ� �����Ѵ�.
	//���� ��� DB_NULL�� �����Ѵ�.
	pIndexFilePageHeader		pIndexPageHeader;
	IndexFilePageHeader			indexPageHeader;
	char*						returnLocation;

	if (gDbMmuIndexPage->mmuPageNumber == 0)
	{
		//dbTouch(1, gDbPageSizeIndexFilePageSize);
		//dbTouch(1, DB_INDEX_FILE_LOAD);
		return DB_NULL;
	}

	pIndexPageHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);
	dbMemcpyFreeIndian(pIndexPageHeader, &indexPageHeader, sizeof(unsigned long), 3);

	if ((gDbPageSizeIndexFilePageSize - indexPageHeader.indexHeaderEndOfDatas) < requirementSize)
	{	
		return DB_NULL;
	}
	//INDEX NODE�� ���� ������ �����ϴ� ���
	else
	{
		returnLocation = (gDbMmuIndexPage->mmuMemoryAddress + indexPageHeader.indexHeaderEndOfDatas);	//inode����� ����offset�� ��ġ�� backup�� ���´�.
		return (char*)returnLocation;
	}
}

pLinkNode dbIndexGetLink(unsigned long indexLinkOffset)
{
	pLinkNode	plnode;
	dbTouch(indexLinkOffset, DB_INDEX_FILE_LOAD);
	plnode = (pLinkNode)(gDbMmuIndexPage->mmuMemoryAddress + indexLinkOffset%gDbPageSizeIndexFilePageSize);
	return plnode;
}

pIndexNode dbIndexGetNode(unsigned long dbIndexOffset)
{
	pIndexNode		pindexNode;

	dbTouch(dbIndexOffset, DB_INDEX_FILE_LOAD);
	pindexNode = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + dbIndexOffset%gDbPageSizeIndexFilePageSize);
	
	return pindexNode;
}


unsigned long dbIndexSetNode(pIndexNode			pInode, 
								void*			keyValue,
								unsigned long	keyDataType)
{
	unsigned long	inodeOffset;
	IndexNode			inodeNode;
	
	//�ε��� ����� offset�� ����Ѵ�.
	inodeOffset = (unsigned long)(((char*)pInode) - gDbMmuIndexPage->mmuMemoryAddress) + (gDbPageSizeIndexFilePageSize * (gDbMmuIndexPage->mmuPageNumber - 1));

	//���ο� �ε��� ��带 �����Ѵ�.
	inodeNode.indexNodeLeft			= DB_NULL;
	inodeNode.indexNodeRight		= DB_NULL;
	inodeNode.indexNodeHeight		= 1;
	inodeNode.indexNodeLinkOffset	= DB_NULL;

	//���ο� �͵��� ��带 INDEX AREA�� �����Ѵ�.
	dbMemcpyFreeIndian(&inodeNode, pInode, sizeof(unsigned long), 4);

	//keyValue�� ����Ѵ�.
	if (keyDataType == DB_VARCHAR_TYPE)
	{
		dbStringCopy((DB_VARCHAR*)(pInode + 1), (DB_VARCHAR*)(keyValue)); 
	}
	else if (keyDataType == DB_UINT8_TYPE)
	{
		dbMemcpyFreeIndian(keyValue, pInode + 1, DB_UINT8_SIZE, 1);
	}
	else if (keyDataType == DB_UINT16_TYPE)
	{
		dbMemcpyFreeIndian(keyValue, pInode + 1, DB_UINT16_SIZE, 1);
	}
	else if (keyDataType == DB_UINT32_TYPE)
	{
		dbMemcpyFreeIndian(keyValue, pInode + 1, DB_UINT32_SIZE, 1);
	}
	else{}
	
	return inodeOffset;
}



signed char dbIndexCompareKeyAreaVsAreaMS(void*			dbAreaKeyValue1, 
										  void*			dbAreaKeyValue2, 
										  unsigned long dbKeyDataType)
{
	signed char vCompareResult;

	if (dbKeyDataType == DB_VARCHAR_TYPE)
	{
		//DbgPrintf(("dbIndexCompareKeyAreaVsAreaMS �Լ����� ��Ʈ�� �� �������� �����Ѵ�.\n"));
		vCompareResult = dbStringCompareAreaVsAreaMS((DB_VARCHAR*)dbAreaKeyValue1, (DB_VARCHAR*)dbAreaKeyValue2);

		//DbgPrintf(("dbIndexCompareKeyAreaVsAreaMS �Լ����� ��Ʈ�� �� �ϰ� ���� [%d]�� �񱳰���� �����ߴ�..\n", vCompareResult));

	}
	else if (dbKeyDataType == DB_UINT8_TYPE)
	{
		DB_UINT8	vAreaKeyValue1, vAreaKeyValue2;
		
		dbMemcpyFreeIndian(dbAreaKeyValue1, &vAreaKeyValue1, DB_UINT8_SIZE, 1);
		dbMemcpyFreeIndian(dbAreaKeyValue2, &vAreaKeyValue2, DB_UINT8_SIZE, 1);
		
		if (vAreaKeyValue1 > vAreaKeyValue2)
		{
			vCompareResult = -1;
		}
		else if  (vAreaKeyValue1 < vAreaKeyValue2)
		{
			vCompareResult = 1;
		}
		else
		{
			vCompareResult = 0;
		}
	}
	else if (dbKeyDataType == DB_UINT16_TYPE)
	{
		DB_UINT16	vAreaKeyValue1, vAreaKeyValue2;

		dbMemcpyFreeIndian(dbAreaKeyValue1, &vAreaKeyValue1, DB_UINT16_SIZE, 1);
		dbMemcpyFreeIndian(dbAreaKeyValue2, &vAreaKeyValue2, DB_UINT16_SIZE, 1);
		
		if (vAreaKeyValue1 > vAreaKeyValue2)
		{
			vCompareResult = -1;
		}
		else if (vAreaKeyValue1 < vAreaKeyValue2)
		{
			vCompareResult = 1;
		}
		else
		{
			vCompareResult = 0;
		}		
	}
	else if (dbKeyDataType == DB_UINT32_TYPE)
	{
		DB_UINT32	vAreaKeyValue1, vAreaKeyValue2;

		dbMemcpyFreeIndian(dbAreaKeyValue1, &vAreaKeyValue1, DB_UINT32_SIZE, 1);
		dbMemcpyFreeIndian(dbAreaKeyValue2, &vAreaKeyValue2, DB_UINT32_SIZE, 1);
				
		if (vAreaKeyValue1 > vAreaKeyValue2)
		{
			vCompareResult = -1;
		}
		else if (vAreaKeyValue1 < vAreaKeyValue2)
		{
			vCompareResult = 1;
		}
		else
		{
			vCompareResult = 0;
		}		
	}
	else{}

	return vCompareResult;
}

signed char dbIndexCompareKeyAreaVsStackMS(void*			dbAreaKeyValue, 
										   void*			dbStackKeyValue, 
										   unsigned long	dbKeyDataType)
{
	signed char vCompareResult;

	if (dbKeyDataType == DB_VARCHAR_TYPE)
	{
		vCompareResult = dbStringCompareAreaVsStackMS((DB_VARCHAR*)dbAreaKeyValue, (DB_VARCHAR*)dbStackKeyValue);
	}
	else if (dbKeyDataType == DB_UINT8_TYPE)
	{
		DB_UINT8	vAreaKeyValue;

		dbMemcpyFreeIndian(dbAreaKeyValue, &vAreaKeyValue, DB_UINT8_SIZE, 1);
		
		if (vAreaKeyValue > *((DB_UINT8*)dbStackKeyValue))
		{
			vCompareResult = -1;
		}
		else if (vAreaKeyValue < *((DB_UINT8*)dbStackKeyValue))
		{
			vCompareResult = 1;
		}
		else
		{
			vCompareResult = 0;
		}
	}
	else if (dbKeyDataType == DB_UINT16_TYPE)
	{
		DB_UINT16	vAreaKeyValue;
		DB_UINT16	vStackKeyValue;

		dbMemcpyFreeIndian(dbAreaKeyValue, &vAreaKeyValue, DB_UINT16_SIZE, 1);
		dbMemcpySameIndian(dbStackKeyValue, &vStackKeyValue, DB_UINT16_SIZE, 1);
		
		if (vAreaKeyValue > vStackKeyValue)
		{
			vCompareResult = -1;
		}
		else if (vAreaKeyValue < vStackKeyValue)
		{
			vCompareResult = 1;
		}
		else
		{
			vCompareResult = 0;
		}		
	}
	else if (dbKeyDataType == DB_UINT32_TYPE)
	{
		DB_UINT32	vAreaKeyValue;
		DB_UINT32	vStackKeyValue;
		
		dbMemcpyFreeIndian(dbAreaKeyValue, &vAreaKeyValue, DB_UINT32_SIZE, 1);
		dbMemcpySameIndian(dbStackKeyValue, &vStackKeyValue, DB_UINT32_SIZE, 1);
		
		if (vAreaKeyValue > vStackKeyValue)
		{
			vCompareResult = -1;
		}
		else if (vAreaKeyValue < vStackKeyValue)
		{
			vCompareResult = 1;
		}
		else
		{
			vCompareResult = 0;
		}		
	}
	else{}

	return vCompareResult;
}



signed char dbIndexCompareKey_AREAVs_STACK(void*			dbAreaKeyValue, 
										   void*			dbStackKeyValue, 
										   unsigned long	dbKeyDataType)
{
	signed char vCompareResult;

	if (dbKeyDataType == DB_VARCHAR_TYPE)
	{
		vCompareResult = dbStringCompare_AREAVs_STACK((DB_VARCHAR*)dbAreaKeyValue, (DB_VARCHAR*)dbStackKeyValue);
	}
	else if (dbKeyDataType == DB_UINT8_TYPE)
	{
		DB_UINT8	vAreaKeyValue;

		dbMemcpyFreeIndian(dbAreaKeyValue, &vAreaKeyValue, DB_UINT8_SIZE, 1);
		
		if (vAreaKeyValue > *((DB_UINT8*)dbStackKeyValue))
		{
			vCompareResult = -1;
		}
		else if (vAreaKeyValue < *((DB_UINT8*)dbStackKeyValue))
		{
			vCompareResult = 1;
		}
		else
		{
			vCompareResult = 0;
		}
	}
	else if (dbKeyDataType == DB_UINT16_TYPE)
	{
		DB_UINT16	vAreaKeyValue;
		DB_UINT16	vStackKeyValue;

		dbMemcpyFreeIndian(dbAreaKeyValue, &vAreaKeyValue, DB_UINT16_SIZE, 1);
		dbMemcpySameIndian(dbStackKeyValue, &vStackKeyValue, DB_UINT32_SIZE, 1);
		
		if (vAreaKeyValue > vStackKeyValue)
		{
			vCompareResult = -1;
		}
		else if (vAreaKeyValue < vStackKeyValue)
		{
			vCompareResult = 1;
		}
		else
		{
			vCompareResult = 0;
		}		
	}
	else if (dbKeyDataType == DB_UINT32_TYPE)
	{
		DB_UINT32	vAreaKeyValue;
		DB_UINT32	vStackKeyValue;

		dbMemcpyFreeIndian(dbAreaKeyValue, &vAreaKeyValue, DB_UINT32_SIZE, 1);
		dbMemcpySameIndian(dbStackKeyValue, &vStackKeyValue, DB_UINT32_SIZE, 1);
		
		if (vAreaKeyValue > vStackKeyValue)
		{
			vCompareResult = -1;
		}
		else if (vAreaKeyValue < vStackKeyValue)
		{
			vCompareResult = 1;
		}
		else
		{
			vCompareResult = 0;
		}		
	}
	else{}

	return vCompareResult;
}



void dbIndexLinkNodeAppend(unsigned long indexNodeOffset, unsigned long recordRowid)
{
	pIndexNode		pinode;
	pLinkNode		plink;
	LinkNode		linkNode;
	unsigned long	linkOffset;
	unsigned long	pLinkOffset;
	unsigned long	newLinkOffset;

	//�ε��� ��带 �����Ѵ�.
	
	//dbTouch(indexNodeOffset, DB_INDEX_FILE_LOAD);
	//pinode = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + indexNodeOffset%gDbPageSizeIndexFilePageSize);
	pinode	= dbIndexGetNode(indexNodeOffset);
	
	//link�� offset�� �����Ѵ�.
	dbMemcpyFreeIndian(&(pinode->indexNodeLinkOffset), &linkOffset, sizeof(unsigned long), 1);

	pLinkOffset  = indexNodeOffset + (((char*)(&(pinode->indexNodeLinkOffset))) - ((char*)pinode));
	
	while(linkOffset)
	{
		dbTouch(linkOffset, DB_INDEX_FILE_LOAD);

		plink = (pLinkNode)(gDbMmuIndexPage->mmuMemoryAddress + linkOffset%gDbPageSizeIndexFilePageSize);
		dbMemcpyFreeIndian(plink, &linkNode, sizeof(unsigned long), 2);

		if (linkNode.linkNodeRecordRowid > recordRowid)
		{
			break;
		}
		
		pLinkOffset = linkOffset + (((char*)(&(plink->linkNodeNext))) - ((char*)plink));
		linkOffset = linkNode.linkNodeNext;
	}
	//linkOffset���� next ����� offst�� �� �ְ�
	//pLinkOffset���� ���� ������ ��ũ���� �������� next�� ����� ���� ����Ǿ� �ִ�. 

	//��ũ�� �߰��� �� �ִ� page�� ã�´�
	{
		unsigned char	reuseSpace;
		plink = DB_NULL;
		//�켱 ������ index page�� ������ �õ��Ѵ�.
		if (gDbMmuIndexPage->mmuPageNumber != 0)
		{
			plink = dbIndexGetPageHaveRequirementSizeInDeleteLinks();
			
			if (plink == DB_NULL)
			{
				plink = (pLinkNode)dbIndexGetPageHaveRequirementSizeInFreespace(sizeof(LinkNode));
				
				if (plink != DB_NULL)
				{
					reuseSpace = 0;
					//������尡 �ƴ� ���������̿��̴�.
				}
			}
			else
			{
				reuseSpace = 1;
				//���� link���� �ϳ��� ���� ��
			}
		}
		
		if (plink == DB_NULL)
		{
			unsigned long			indexPageNumber = 0;

			while(1)
			{
				indexPageNumber++;
				dbTouch((indexPageNumber-1)*gDbPageSizeIndexFilePageSize, DB_INDEX_FILE_LOAD);
				
				plink = dbIndexGetPageHaveRequirementSizeInDeleteLinks();
				
				if (plink != DB_NULL)
				{
					//���� link���� �ϳ��� ���� ��.
					reuseSpace = 1;
					break;
				}
				
				plink = (pLinkNode)dbIndexGetPageHaveRequirementSizeInFreespace(sizeof(LinkNode));
				if (plink != DB_NULL)
				{
					reuseSpace = 0;
					break;
				}
			}
		}

		if (reuseSpace == 0)	//������ �Ѱ��� �ƴϱ� ������ index �ش��� ������ �ʿ��ϴ�
		{
			pIndexFilePageHeader	pIndexHeader;
			//���������� ����ؾ� �Ѵ�.
			unsigned long	indexpage_endDatas;
			//���������߿� ���������� ����Ѱ��̹Ƿ� ��ųʸ� �ش����� ���������� ���Ҹ� ���Ѿ��Ѵ�.
			pIndexHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);

			//�ε��� �ش����� ������ �������� ��ġ�� �����Ѵ�.
			dbMemcpyFreeIndian(&(pIndexHeader->indexHeaderEndOfDatas), &indexpage_endDatas, sizeof(unsigned long), 1);
			
			//�ε��� �ش����� ������ �������� ��ġ�� ������Ų��.
			indexpage_endDatas += sizeof(LinkNode);
			dbMemcpyFreeIndian(&indexpage_endDatas, &(pIndexHeader->indexHeaderEndOfDatas), sizeof(unsigned long), 1);
			
			gDbMmuIndexPage->mmuChanged = DB_CHANGED;
		}
	}

	//pInode�� link�� ������ �߰��ϸ� �ȴ�.
	{
		dbMemcpyFreeIndian(&linkOffset , &(plink->linkNodeNext), sizeof(unsigned long), 1);
		dbMemcpyFreeIndian(&recordRowid , &(plink->linkNodeRecordRowid), sizeof(unsigned long), 1);
	}

	//���� ������ link�� offset�� ���Ѵ�.
	newLinkOffset = (gDbMmuIndexPage->mmuPageNumber - 1) * gDbPageSizeIndexFilePageSize 
					+ (((char*)plink) - gDbMmuIndexPage->mmuMemoryAddress);

	//���� ������ ������ �̾��ش�.
	{
		unsigned long	*director;
		dbTouch(pLinkOffset, DB_INDEX_FILE_LOAD);
		director = (unsigned long*)(gDbMmuIndexPage->mmuMemoryAddress + pLinkOffset%gDbPageSizeIndexFilePageSize);
		dbMemcpyFreeIndian(&newLinkOffset, director, sizeof(unsigned long), 1);
		gDbMmuIndexPage->mmuChanged = DB_CHANGED;
	}
}
pLinkNode	dbIndexGetPageHaveRequirementSizeInDeleteLinks(void)
{
	//���� �ε��� page���� ������ ��ũ�� �ִ��� ���� ���� ��� �̸� �����Ѵ�.
	//������ DB_NULL�� �����Ѵ�.

	pIndexFilePageHeader	pindexHeader;
	unsigned long			linkOffset;

	pindexHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);
	dbMemcpyFreeIndian(&(pindexHeader->indexHeaderDeleteLinkOffset), &linkOffset, sizeof(unsigned long), 1);

	if (linkOffset == 0)
	{
		return DB_NULL;
	}
	else
	{
		pLinkNode			plinkNode;
		unsigned long	next;
		
		plinkNode = (pLinkNode)(gDbMmuIndexPage->mmuMemoryAddress + linkOffset);
		dbMemcpyFreeIndian(&(plinkNode->linkNodeNext) , &next, sizeof(unsigned long), 1);

		//indexHeader�� �����Ѵ�.
		dbMemcpyFreeIndian(&next, &(pindexHeader->indexHeaderDeleteLinkOffset), sizeof(unsigned long), 1);
		gDbMmuIndexPage->mmuChanged = DB_CHANGED;

		return plinkNode;
	}
}

unsigned long dbIndexSingleUnitInsertMS(unsigned long	dbIndexNodeRootOffset,
										void*			dbKeyValue,
										unsigned long	dbKeyDataType,
										unsigned long	dbRecordOffset)
{
	//���ο� ��带 �����ؾ� �� ���ó��
	if ((dbIndexNodeRootOffset == DB_NULL) || (dbIndexNodeRootOffset == 1))
	{
		pIndexNode		pInode;
		unsigned long	vInodeOffset;
		unsigned long	vRequirementSize;

		//�ε�����尡 ����Ǳ� ���� �ʿ��� ũ�⸦ ����Ѵ�.
		{
			//Index Node�� Size
			vRequirementSize = sizeof(IndexNode);

			//�ش� Key���� Size
			if (dbKeyDataType == DB_VARCHAR_TYPE)
			{
				vRequirementSize = dbStringGetSize((DB_VARCHAR*)dbKeyValue) + DB_VARCHAR_SIZE + vRequirementSize;
			}
			else if (dbKeyDataType == DB_UINT8_TYPE)
			{
				vRequirementSize += DB_UINT8_SIZE;
			}
			else if (dbKeyDataType == DB_UINT16_TYPE)
			{
				vRequirementSize += DB_UINT16_SIZE;
			}
			else if (dbKeyDataType == DB_UINT32_TYPE)
			{
				vRequirementSize += DB_UINT32_SIZE;
			}
			else{}
		}
		//vRequirementSize�� �ش� �ε��� ��尡 ����Ǳ� ���� �ʿ��� Size�� ���Ǿ� �ִ�.

		
		if ((vRequirementSize + sizeof(IndexFilePageHeader)) > gDbPageSizeIndexFilePageSize)
		{
			return 0;	//gDbPageSizeIndexFilePageSize ����� �������� ����Ѵ�.�߰��� ���Ѵ�.
		}

		//�ʿ��� ����� ���� indexPage�� loading�Ѵ�.
		{
			pIndexFilePageHeader	pIndexHeader;
			unsigned long			vIndexPageNumber = 0;

			while(1)
			{
				vIndexPageNumber++;
				dbTouch((vIndexPageNumber-1)*gDbPageSizeIndexFilePageSize, DB_INDEX_FILE_LOAD);

			#if ( AVLDB_DEBUG == 1 )
				//DbgPrintf(("dbIndexGetPageHaveRequirementSizeInDeleteNodes ����� �ƴѰ� ������...before\n"));
			#endif

				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInDeleteNodes(vRequirementSize);

			#if ( AVLDB_DEBUG == 1 )
				//DbgPrintf(("dbIndexGetPageHaveRequirementSizeInDeleteNodes ����� �ƴѰ� ������...after\n"));
			#endif

				if (pInode != DB_NULL)
				{
					//���������߿� �ϳ��� �����°��̹Ƿ� �ش��� ��ȭ�� �ʿ����.
					break;
				}
				
				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInFreespace(vRequirementSize);

				if (pInode != DB_NULL)
				{
					unsigned long	vIndexPageEndDatas;
					//���������߿� ���������� ����Ѱ��̹Ƿ� ��ųʸ� �ش����� ���������� ���Ҹ� ���Ѿ��Ѵ�.
					pIndexHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);

					//�ε��� �ش����� ������ �������� ��ġ�� �����Ѵ�.
					dbMemcpyFreeIndian(&(pIndexHeader->indexHeaderEndOfDatas), &vIndexPageEndDatas, sizeof(unsigned long), 1);
					
					//�ε��� �ش����� ������ �������� ��ġ�� ������Ų��.
					vIndexPageEndDatas += vRequirementSize;
					dbMemcpyFreeIndian(&vIndexPageEndDatas, &(pIndexHeader->indexHeaderEndOfDatas), sizeof(unsigned long), 1);
					
					gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					break;
				}
			}
		}
		//�ش� ��ġ�� �ε��� ��带 ����Ѵ�.
		//DbgPrintf(("dbIndexSetNode ����� �ƴѰ� ������...before\n"));
		vInodeOffset = dbIndexSetNode(pInode, dbKeyValue, dbKeyDataType);

		//DbgPrintf(("dbIndexSetNode ����� �ƴѰ� ������...after\n"));
		//link���� �߰��Ѵ�.
		dbIndexLinkNodeAppend(vInodeOffset, dbRecordOffset);

		return vInodeOffset;	//���� ����� inode�� offset�� �����Ѵ�.
	}
	//������ �ε��� ��尡 �����ϰ� �� �ε��� ��带 Ž���ϸ鼭 ������ �ǽ��ؾ��Ѵ�.
	else
	{
		pIndexNode		pInode;
		IndexNode		vIndexNode;
		signed char		vKeyCompareResult;
		
		pInode	= dbIndexGetNode(dbIndexNodeRootOffset);
		dbMemcpyFreeIndian(pInode, &vIndexNode, sizeof(unsigned long), 4);
		
	#if ( AVLDB_DEBUG == 1 )
//DbgPrintf(("dbIndexCompareKeyAreaVsStackMS ����� �ƴѰ� ������..����������������.before\n"));
	#endif	

		vKeyCompareResult = dbIndexCompareKeyAreaVsStackMS(pInode + 1, dbKeyValue, dbKeyDataType);

	#if ( AVLDB_DEBUG == 1 )
//DbgPrintf(("dbIndexCompareKeyAreaVsStackMS ����� �ƴѰ� ������..����������������.after �񱳰�[%d]\n", vKeyCompareResult));
	#endif	

		if (vKeyCompareResult == 0)
		{
			//link���� �߰��Ѵ�.
	#if ( AVLDB_DEBUG == 1 )
//DbgPrintf(("dbIndexLinkNodeAppend(dbIndexNodeRootOffset[%d], dbRecordOffset[%d])\n", dbIndexNodeRootOffset, dbRecordOffset));			
	#endif	
			dbIndexLinkNodeAppend(dbIndexNodeRootOffset, dbRecordOffset);
		}
		else if (vKeyCompareResult == 1)
		{
			unsigned long	vAfterRightSubOffset;
			
			//�����ʿ��� Ž���� �ؾ��ϴ� ���
			vAfterRightSubOffset = dbIndexSingleUnitInsertMS(vIndexNode.indexNodeRight, dbKeyValue, dbKeyDataType, dbRecordOffset);
			
			if (vIndexNode.indexNodeRight != vAfterRightSubOffset)
			{
				pInode						= dbIndexGetNode(dbIndexNodeRootOffset);
				vIndexNode.indexNodeRight	= vAfterRightSubOffset;
				dbMemcpyFreeIndian(&(vIndexNode.indexNodeRight), &(pInode->indexNodeRight), sizeof(unsigned long), 1);
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		else
		{
			unsigned long	vAfterLeftSubOffset;
			
			//�����ʿ��� Ž���� �ؾ��ϴ� ���
			vAfterLeftSubOffset = dbIndexSingleUnitInsertMS(vIndexNode.indexNodeLeft, dbKeyValue, dbKeyDataType, dbRecordOffset);
			
			if (vIndexNode.indexNodeLeft != vAfterLeftSubOffset)
			{
				pInode						= dbIndexGetNode(dbIndexNodeRootOffset);
				vIndexNode.indexNodeLeft	= vAfterLeftSubOffset;
				dbMemcpyFreeIndian(&(vIndexNode.indexNodeLeft), &(pInode->indexNodeLeft), sizeof(unsigned long), 1);
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}

	#if ( AVLDB_DEBUG == 1 )
//DbgPrint(("Rotation �ϸ鼭 �״°ǰ�\n"));
	#endif
		//rotation check�� �Ѵ�.
		{
			signed char		vBalanceFactor;
			IndexNode		vSubIndexNode;

			vBalanceFactor = dbIndexGetBalanceFactor(vIndexNode.indexNodeLeft, vIndexNode.indexNodeRight);
	#if ( AVLDB_DEBUG == 1 )
//DbgPrint(("vBalanceFactor�� ����� ���ϴ°� ? [%d]\n", vBalanceFactor));			
	#endif
			if (vBalanceFactor == 2)
			{
				pInode	= dbIndexGetNode(vIndexNode.indexNodeLeft);
				dbMemcpyFreeIndian(pInode, &vSubIndexNode, sizeof(unsigned long), 4);
				
				vBalanceFactor = dbIndexGetBalanceFactor(vSubIndexNode.indexNodeLeft, vSubIndexNode.indexNodeRight);
				
				if (vBalanceFactor == -1)
				{
					//LRRotation
					dbIndexRotationLR(&dbIndexNodeRootOffset, &(vIndexNode.indexNodeLeft), &(vSubIndexNode.indexNodeRight));
				}
				else
				{
					//LL rotation
					dbIndexRotationLL(&dbIndexNodeRootOffset, &(vIndexNode.indexNodeLeft), &(vSubIndexNode.indexNodeLeft));
				}
			}
			else if (vBalanceFactor == -2)
			{
				pInode	= dbIndexGetNode(vIndexNode.indexNodeRight);
				dbMemcpyFreeIndian(pInode, &vSubIndexNode, sizeof(unsigned long), 4);
				
				vBalanceFactor = dbIndexGetBalanceFactor(vSubIndexNode.indexNodeLeft, vSubIndexNode.indexNodeRight);
				
				if (vBalanceFactor == 1)
				{
					//RLRotation
					dbIndexRotationRL(&dbIndexNodeRootOffset, &(vIndexNode.indexNodeRight), &(vSubIndexNode.indexNodeLeft));
				}
				else
				{
					//RR rotation
					dbIndexRotationRR(&dbIndexNodeRootOffset, &(vIndexNode.indexNodeRight), &(vSubIndexNode.indexNodeRight));
				}

			}
			else{}
		}

		//dbIndexNodeRootOffset�� ���̸� �ٽ� ����Ѵ�.
		{
			unsigned long vInodeHeight;
			
			pInode	= dbIndexGetNode(dbIndexNodeRootOffset);
			
			dbMemcpyFreeIndian(pInode, &vIndexNode, sizeof(unsigned long), 4);

			vInodeHeight = dbIndexGetCalculateHeight(vIndexNode.indexNodeLeft, vIndexNode.indexNodeRight);
			
			if (vIndexNode.indexNodeHeight != vInodeHeight)
			{
				pInode	= dbIndexGetNode(dbIndexNodeRootOffset);

				vIndexNode.indexNodeHeight = vInodeHeight;

				dbMemcpyFreeIndian(&(vIndexNode.indexNodeHeight), &(pInode->indexNodeHeight), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}

		return dbIndexNodeRootOffset;
	}
}

unsigned long dbIndexSingleUnitInsert(unsigned long	indexNodeRootOffset,
										  void*			keyValue,
										  unsigned long	keyDataType,
										  unsigned long	recordOffset)
{
	//���ο� ��带 �����ؾ� �� ���ó��
	if ((indexNodeRootOffset == DB_NULL) || (indexNodeRootOffset == 1))
	{
		pIndexNode		pInode;
		unsigned long	inodeOffset;
		unsigned long	requirementSize;

		//�ε�����尡 ����Ǳ� ���� �ʿ��� ũ�⸦ ����Ѵ�.
		{
			requirementSize = sizeof(IndexNode);

			if (keyDataType == DB_VARCHAR_TYPE)
			{
				requirementSize = dbStringGetSize((DB_VARCHAR*)keyValue) 
								+ DB_VARCHAR_SIZE 
								+ requirementSize;
			}
			else if (keyDataType == DB_UINT8_TYPE)
			{
				requirementSize += DB_UINT8_SIZE;
			}
			else if (keyDataType == DB_UINT16_TYPE)
			{
				requirementSize += DB_UINT16_SIZE;
			}
			else if (keyDataType == DB_UINT32_TYPE)
			{
				requirementSize += DB_UINT32_SIZE;
			}
			else{}
		}

		if ((requirementSize + sizeof(IndexFilePageHeader)) > gDbPageSizeIndexFilePageSize)
		{
			return 0;	//gDbPageSizeIndexFilePageSize ����� �������� ����Ѵ�.�߰��� ���Ѵ�.
		}

		//�ʿ��� ����� ���� indexPage�� loading�Ѵ�.
		{
			pIndexFilePageHeader	pIndexHeader;
			unsigned long			indexPageNumber = 0;

			while(1)
			{
				indexPageNumber++;
				dbTouch((indexPageNumber-1)*gDbPageSizeIndexFilePageSize, DB_INDEX_FILE_LOAD);
				
				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInDeleteNodes(requirementSize);

				if (pInode != DB_NULL)
				{
					//���������߿� �ϳ��� �����°��̹Ƿ� �ش��� ��ȭ�� �ʿ����.
					break;
				}
				
				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInFreespace(requirementSize);

				if (pInode != DB_NULL)
				{
					unsigned long	datapage_endDatas;
					//���������߿� ���������� ����Ѱ��̹Ƿ� ��ųʸ� �ش����� ���������� ���Ҹ� ���Ѿ��Ѵ�.
					pIndexHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);

					//�ε��� �ش����� ������ �������� ��ġ�� �����Ѵ�.
					dbMemcpyFreeIndian(&(pIndexHeader->indexHeaderEndOfDatas), &datapage_endDatas, sizeof(unsigned long), 1);
					
					//�ε��� �ش����� ������ �������� ��ġ�� ������Ų��.
					datapage_endDatas += requirementSize;
					dbMemcpyFreeIndian(&datapage_endDatas, &(pIndexHeader->indexHeaderEndOfDatas), sizeof(unsigned long), 1);
					
					gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					break;
				}
			}
		}
		//�ش� ��ġ�� �ε��� ��带 ����Ѵ�.
		inodeOffset = dbIndexSetNode(pInode, keyValue, keyDataType);

		//link���� �߰��Ѵ�.
		dbIndexLinkNodeAppend(inodeOffset, recordOffset);

		return inodeOffset;	//���� ����� inode�� offset�� �����Ѵ�.
	}
	//������ �ε��� ��尡 �����ϰ� �� �ε��� ��带 Ž���ϸ鼭 ������ �ǽ��ؾ��Ѵ�.
	else
	{
		pIndexNode		pInode;
		IndexNode		indexNode;
		signed char		keyCompareResult;
		
		pInode	= dbIndexGetNode(indexNodeRootOffset);
		dbMemcpyFreeIndian(pInode, &indexNode, sizeof(unsigned long), 4);

		keyCompareResult = dbIndexCompareKey_AREAVs_STACK(pInode + 1, keyValue, keyDataType);

		if (keyCompareResult == 0)
		{
			//link���� �߰��Ѵ�.
			dbIndexLinkNodeAppend(indexNodeRootOffset, recordOffset);
		}
		else if (keyCompareResult == 1)
		{
			unsigned long	afterRightSubOffset;
			
			//�����ʿ��� Ž���� �ؾ��ϴ� ���
			afterRightSubOffset = dbIndexSingleUnitInsert(indexNode.indexNodeRight, keyValue, keyDataType, recordOffset);
			
			if (indexNode.indexNodeRight != afterRightSubOffset)
			{
				pInode						= dbIndexGetNode(indexNodeRootOffset);
				indexNode.indexNodeRight	= afterRightSubOffset;
				dbMemcpyFreeIndian(&(indexNode.indexNodeRight), &(pInode->indexNodeRight), sizeof(unsigned long), 1);
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		else
		{
			unsigned long	afterLeftSubOffset;
			
			//�����ʿ��� Ž���� �ؾ��ϴ� ���
			afterLeftSubOffset = dbIndexSingleUnitInsert(indexNode.indexNodeLeft, keyValue, keyDataType, recordOffset);
			
			if (indexNode.indexNodeLeft != afterLeftSubOffset)
			{
				pInode						= dbIndexGetNode(indexNodeRootOffset);
				indexNode.indexNodeLeft		= afterLeftSubOffset;
				dbMemcpyFreeIndian(&(indexNode.indexNodeLeft), &(pInode->indexNodeLeft), sizeof(unsigned long), 1);
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}

		//rotation check�� �Ѵ�.
		{
			signed char	balanceFactor;
			IndexNode		subInode;
			balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);
			
			if (balanceFactor == 2)
			{
				pInode	= dbIndexGetNode(indexNode.indexNodeLeft);
				dbMemcpyFreeIndian(pInode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == -1)
				{
					//LRRotation
					dbIndexRotationLR(&indexNodeRootOffset, &(indexNode.indexNodeLeft), &(subInode.indexNodeRight));
				}
				else
				{
					//LL rotation
					dbIndexRotationLL(&indexNodeRootOffset, &(indexNode.indexNodeLeft), &(subInode.indexNodeLeft));
				}
			}
			else if (balanceFactor==-2)
			{
				pInode	= dbIndexGetNode(indexNode.indexNodeRight);
				dbMemcpyFreeIndian(pInode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == 1)
				{
					//RLRotation
					dbIndexRotationRL(&indexNodeRootOffset, &(indexNode.indexNodeRight), &(subInode.indexNodeLeft));
				}
				else
				{
					//RR rotation
					dbIndexRotationRR(&indexNodeRootOffset, &(indexNode.indexNodeRight), &(subInode.indexNodeRight));
				}

			}
			else{}
		}

		//indexNodeRootOffset�� ���̸� �ٽ� ����Ѵ�.
		{
			unsigned long inodeHeight;
			
			pInode	= dbIndexGetNode(indexNodeRootOffset);
			
			dbMemcpyFreeIndian(pInode, &indexNode, sizeof(unsigned long), 4);

			inodeHeight = dbIndexGetCalculateHeight(indexNode.indexNodeLeft, indexNode.indexNodeRight);
			
			if (indexNode.indexNodeHeight != inodeHeight)
			{
				pInode	= dbIndexGetNode(indexNodeRootOffset);

				dbMemcpyFreeIndian(&inodeHeight, &(pInode->indexNodeHeight), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		return indexNodeRootOffset;
	}
}


signed char dbStringCompare_AREAVs_AREA(DB_VARCHAR		*areaMemory1, 
										   DB_VARCHAR		*areaMemory2)
{
	DB_VARCHAR			AREA_CHAR1, AREA_CHAR2;

	while(1)
	{
		dbMemcpyFreeIndian(areaMemory1, &AREA_CHAR1, DB_VARCHAR_SIZE, 1);
		dbMemcpyFreeIndian(areaMemory2, &AREA_CHAR2, DB_VARCHAR_SIZE, 1);
		

		if ((AREA_CHAR1 == DB_END_OF_STRING) && (AREA_CHAR2 == DB_END_OF_STRING)) return 0;

		if ( AREA_CHAR1 == AREA_CHAR2 )
		{
			areaMemory1++ ;
			areaMemory2++ ;
		}
		else if( AREA_CHAR1 > AREA_CHAR2 )
		{
			return -1;
		}
		else
		{
			return 1;
		} 
	}
}

unsigned long dbIndexComplexUnitInsertPostProcessMS(unsigned long	dbIndexRootOffset,
													void*			dbKeyValues[],
													unsigned long	dbKeyDataTypes[],
													unsigned long	dbColumnNumber,
													unsigned long	dbRecordRowid)
{

	//==================Case 1. ���ο� ��带 �����ؾ� �� ��� ó�� (begin)==================//
	if ((dbIndexRootOffset == DB_NULL) || (dbIndexRootOffset == 1))
	{
		pIndexNode		pInode;
		unsigned long	vIndexOffset;
		unsigned long	vRequirementSize;

		//==================(ó��1) �ε��� ��尡 ����Ǳ� ���� �ʿ��� ũ�⸦ ����Ѵ�.(begin)==================//
		{
			vRequirementSize = sizeof(IndexNode);

			if (dbKeyDataTypes[0] == DB_VARCHAR_TYPE)
			{
				vRequirementSize = dbStringGetSize((DB_VARCHAR*)(dbKeyValues[0])) 
								 + DB_VARCHAR_SIZE 
								 + vRequirementSize;
			}
			else if (dbKeyDataTypes[0] == DB_UINT8_TYPE)
			{
				vRequirementSize += DB_UINT8_SIZE;
			}
			else if (dbKeyDataTypes[0] == DB_UINT16_TYPE)
			{
				vRequirementSize += DB_UINT16_SIZE;
			}
			else if (dbKeyDataTypes[0] == DB_UINT32_TYPE)
			{
				vRequirementSize += DB_UINT32_SIZE;
			}
			else{}
		}
		//==================(ó��1) �ε��� ��尡 ����Ǳ� ���� �ʿ��� ũ�⸦ ����Ѵ�.( end )==================//



		//==================(ó��2) �ε��� ��尡 �ε����� ������ ũ�⺸�� �������� �����Ѵ�.(begin)==================//
		if ((vRequirementSize + sizeof(IndexFilePageHeader)) > gDbPageSizeIndexFilePageSize)
		{
			return 0 ;			//gDbPageSizeIndexFilePageSize ����� �������� ����Ѵ�.�߰��� ���Ѵ�.
		}
		//==================(ó��2) �ε��� ��尡 �ε����� ������ ũ�⺸�� �������� �����Ѵ�.( end )==================//



		//==================(ó��3) �ʿ��� ����� ���� indexPage�� loading�Ѵ�.(begin)==================//
		{
			pIndexFilePageHeader	pIndexHeader;
			unsigned long			indexPageNumber = 0;

			while(1)
			{
				indexPageNumber++;
				dbTouch((indexPageNumber - 1) * gDbPageSizeIndexFilePageSize, DB_INDEX_FILE_LOAD);
				
				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInDeleteNodes(vRequirementSize);
				if (pInode != DB_NULL)
				{
					//���������߿� �ϳ��� �����°��̹Ƿ� �ش��� ��ȭ�� �ʿ����.
					break;
				}
				
				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInFreespace(vRequirementSize);

				if (pInode != DB_NULL)
				{
					unsigned long	datapage_endDatas;
					//���������߿� ���������� ����Ѱ��̹Ƿ� ��ųʸ� �ش����� ���������� ���Ҹ� ���Ѿ��Ѵ�.
					pIndexHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);

					//�ε��� �ش����� ������ �������� ��ġ�� �����Ѵ�.
					dbMemcpyFreeIndian(&(pIndexHeader->indexHeaderEndOfDatas), &datapage_endDatas, sizeof(unsigned long), 1);
					
					//�ε��� �ش����� ������ �������� ��ġ�� ������Ų��.
					datapage_endDatas += vRequirementSize;
					dbMemcpyFreeIndian(&datapage_endDatas, &(pIndexHeader->indexHeaderEndOfDatas), sizeof(unsigned long), 1);
					
					gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					break;
				}
			}
		}
		//==================(ó��3) �ʿ��� ����� ���� indexPage�� loading�Ѵ�.( end )==================//


		//==================(ó��4) �ش� ��ġ�� �ε��� ��带 ����Ѵ�.(begin)==================//
		vIndexOffset = dbIndexSetNode(pInode, dbKeyValues[0], dbKeyDataTypes[0]);
		//==================(ó��4) �ش� ��ġ�� �ε��� ��带 ����Ѵ�.( end )==================//


		//==================(ó��5) �����ε����� ������ �÷��� ��� �ش� ��忡 ��ũ�� �߰��ϰ�, ������ ��尡 �ƴ� ��쿡�� ���� ������ Ʈ���� ������ �õ��Ѵ�.(begin)==================//
		if (dbColumnNumber == 1)
		{
			//link���� �߰��Ѵ�.
			dbIndexLinkNodeAppend(vIndexOffset, dbRecordRowid);
		}
		else
		{
			unsigned long	vNewSubRootOffset;

			vNewSubRootOffset = dbIndexComplexUnitInsertPostProcessMS(0, &dbKeyValues[1], &dbKeyDataTypes[1], dbColumnNumber - 1, dbRecordRowid);
			
			pInode = dbIndexGetNode(vIndexOffset);
			dbMemcpyFreeIndian(&vNewSubRootOffset, &(pInode->indexNodeLinkOffset), sizeof(unsigned long), 1);

			gDbMmuIndexPage->mmuChanged = DB_CHANGED;
		}
		//==================(ó��5) �����ε����� ������ �÷��� ��� �ش� ��忡 ��ũ�� �߰��ϰ�, ������ ��尡 �ƴ� ��쿡�� ���� ������ Ʈ���� ������ �õ��Ѵ�.( end )==================//

		return vIndexOffset;	//���� ����� inode�� offset�� �����Ѵ�.
	}
	//==================Case 1. ���ο� ��带 �����ؾ� �� ��� ó�� ( end )==================//



	//==================Case 2. ������ �ε��� ��尡 �����ϰ� �� �ε��� ��带 Ž���ϸ鼭 ������ �ǽ��ؾ� �ϴ� ���ó��. (begin)==================//
	else
	{
		pIndexNode		pInode;
		IndexNode		vIndexNode;
		signed char		vKeyCompareResult;
		

		pInode	= dbIndexGetNode(dbIndexRootOffset);
		dbMemcpyFreeIndian(pInode, &vIndexNode, sizeof(unsigned long), 4);

		//==================(ó��1) ���� ������ �õ��ϴ� ���� Area �ȿ� �ִ� ��带 ���Ѵ�.(begin)==================//
		vKeyCompareResult = dbIndexCompareKeyAreaVsStackMS(pInode + 1, dbKeyValues[0], dbKeyDataTypes[0]);
		//==================(ó��1) ���� ������ �õ��ϴ� ���� Area �ȿ� �ִ� ��带 ���Ѵ�.( end )==================//


		//==================(ó��2) ���� ������ �õ��ϴ� ���� Area �ȿ� �ִ� ��带 ���ϸ� ó���Ѵ�.(begin)==================//
		if (vKeyCompareResult == 0)
		{
			if (dbColumnNumber == 1)
			{
				//link���� �߰��Ѵ�.
				dbIndexLinkNodeAppend(dbIndexRootOffset, dbRecordRowid);
			}
			else
			{
				unsigned long	newSubRootOffset;

				newSubRootOffset = dbIndexComplexUnitInsertPostProcessMS(vIndexNode.indexNodeLinkOffset, &dbKeyValues[1], &dbKeyDataTypes[1], dbColumnNumber - 1, dbRecordRowid);
				
				if (vIndexNode.indexNodeLinkOffset != newSubRootOffset)
				{				
					pInode	= dbIndexGetNode(dbIndexRootOffset);
					
					vIndexNode.indexNodeLinkOffset	= newSubRootOffset;

					dbMemcpyFreeIndian(&vIndexNode.indexNodeLinkOffset, &(pInode->indexNodeLinkOffset), sizeof(unsigned long), 1);

					gDbMmuIndexPage->mmuChanged = DB_CHANGED;
				}
			}
		}
		else if (vKeyCompareResult == 1)
		{
			unsigned long	afterRightSubOffset;
			
			//�����ʿ��� Ž���� �ؾ��ϴ� ���
			afterRightSubOffset = dbIndexComplexUnitInsertPostProcessMS(vIndexNode.indexNodeRight, dbKeyValues, dbKeyDataTypes, dbColumnNumber, dbRecordRowid);

			if (vIndexNode.indexNodeRight != afterRightSubOffset)
			{
				pInode			= dbIndexGetNode(dbIndexRootOffset);

				vIndexNode.indexNodeRight = afterRightSubOffset;

				dbMemcpyFreeIndian(&(vIndexNode.indexNodeRight), &(pInode->indexNodeRight), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		else
		{
			unsigned long	afterLeftSubOffset;
			
			//�����ʿ��� Ž���� �ؾ��ϴ� ���
			afterLeftSubOffset = dbIndexComplexUnitInsertPostProcessMS(vIndexNode.indexNodeLeft, dbKeyValues, dbKeyDataTypes, dbColumnNumber, dbRecordRowid);
			
			if (vIndexNode.indexNodeLeft != afterLeftSubOffset)
			{
				pInode					= dbIndexGetNode(dbIndexRootOffset);

				vIndexNode.indexNodeLeft	= afterLeftSubOffset;

				dbMemcpyFreeIndian(&(vIndexNode.indexNodeLeft), &(pInode->indexNodeLeft), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}

		//rotation check�� �Ѵ�.
		{
			signed char	balanceFactor;

			IndexNode		subInode;
			
			balanceFactor = dbIndexGetBalanceFactor(vIndexNode.indexNodeLeft, vIndexNode.indexNodeRight);
			
			if (balanceFactor == 2)
			{
				pInode	= dbIndexGetNode(vIndexNode.indexNodeLeft);

				dbMemcpyFreeIndian(pInode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == -1)
				{
					//LRRotation
					dbIndexRotationLR(&dbIndexRootOffset, &(vIndexNode.indexNodeLeft), &(subInode.indexNodeRight));
				}
				else
				{
					//LL rotation
					dbIndexRotationLL(&dbIndexRootOffset, &(vIndexNode.indexNodeLeft), &(subInode.indexNodeLeft));
				}
			}
			else if (balanceFactor==-2)
			{
				pInode	= dbIndexGetNode(vIndexNode.indexNodeRight);

				dbMemcpyFreeIndian(pInode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == 1)
				{
					//RLRotation
					dbIndexRotationRL(&dbIndexRootOffset, &(vIndexNode.indexNodeRight), &(subInode.indexNodeLeft));
				}
				else
				{
					//RR rotation
					dbIndexRotationRR(&dbIndexRootOffset, &(vIndexNode.indexNodeRight), &(subInode.indexNodeRight));
				}

			}
			else{}
		}

		//indexNodeRootOffset�� ���̸� �ٽ� ����Ѵ�.
		{
			unsigned long inodeHeight;
			
			pInode	= dbIndexGetNode(dbIndexRootOffset);
			
			dbMemcpyFreeIndian(pInode, &vIndexNode, sizeof(unsigned long), 4);

			inodeHeight = dbIndexGetCalculateHeight(vIndexNode.indexNodeLeft, vIndexNode.indexNodeRight);
			
			if (vIndexNode.indexNodeHeight != inodeHeight)
			{
				pInode	= dbIndexGetNode(dbIndexRootOffset);

				dbMemcpyFreeIndian(&inodeHeight, &(pInode->indexNodeHeight), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		return dbIndexRootOffset;
	}
	//==================Case 2. ������ �ε��� ��尡 �����ϰ� �� �ε��� ��带 Ž���ϸ鼭 ������ �ǽ��ؾ� �ϴ� ���ó��. ( end )==================//
}

#if 0
unsigned long dbIndexComplexUnitInsertPostProcess(unsigned long	rootOffset,
														void*			keyValues[],
														unsigned long	keyDataTypes[],
														unsigned long	columnNumber,
														unsigned long	recordRowid)
{

	//���ο� ��带 �����ؾ� �� ���ó��
	if ((rootOffset == DB_NULL) || (rootOffset == 1))
	{
		pIndexNode		pInode;
		unsigned long	inodeOffset;
		unsigned long	requirementSize;

		//�ε�����尡 ����Ǳ� ���� �ʿ��� ũ�⸦ ����Ѵ�.
		{
			requirementSize = sizeof(IndexNode);

			if (keyDataTypes[0] == DB_VARCHAR_TYPE)
			{
				requirementSize = dbStringGetSize((DB_VARCHAR*)(keyValues[0])) 
								+ DB_VARCHAR_SIZE 
								+ requirementSize;
			}
			else if (keyDataTypes[0] == DB_UINT8_TYPE)
			{
				requirementSize += DB_UINT8_SIZE;
			}
			else if (keyDataTypes[0] == DB_UINT16_TYPE)
			{
				requirementSize += DB_UINT16_SIZE;
			}
			else if (keyDataTypes[0] == DB_UINT32_TYPE)
			{
				requirementSize += DB_UINT32_SIZE;
			}
			else{}
		}

		if ((requirementSize + sizeof(IndexFilePageHeader)) > gDbPageSizeIndexFilePageSize)
		{
			return 0;	//gDbPageSizeIndexFilePageSize ����� �������� ����Ѵ�.�߰��� ���Ѵ�.
		}

		//�ʿ��� ����� ���� indexPage�� loading�Ѵ�.
		{
			pIndexFilePageHeader	pIndexHeader;
			unsigned long			indexPageNumber = 0;

			while(1)
			{
				indexPageNumber++;
				dbTouch((indexPageNumber - 1) * gDbPageSizeIndexFilePageSize, DB_INDEX_FILE_LOAD);
				
				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInDeleteNodes(requirementSize);
				if (pInode != DB_NULL)
				{
					//���������߿� �ϳ��� �����°��̹Ƿ� �ش��� ��ȭ�� �ʿ����.
					break;
				}
				
				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInFreespace(requirementSize);

				if (pInode != DB_NULL)
				{
					unsigned long	datapage_endDatas;
					//���������߿� ���������� ����Ѱ��̹Ƿ� ��ųʸ� �ش����� ���������� ���Ҹ� ���Ѿ��Ѵ�.
					pIndexHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);

					//�ε��� �ش����� ������ �������� ��ġ�� �����Ѵ�.
					dbMemcpyFreeIndian(&(pIndexHeader->indexHeaderEndOfDatas), &datapage_endDatas, sizeof(unsigned long), 1);
					
					//�ε��� �ش����� ������ �������� ��ġ�� ������Ų��.
					datapage_endDatas += requirementSize;
					dbMemcpyFreeIndian(&datapage_endDatas, &(pIndexHeader->indexHeaderEndOfDatas), sizeof(unsigned long), 1);
					
					gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					break;
				}
			}
		}
		//�ش� ��ġ�� �ε��� ��带 ����Ѵ�.
		inodeOffset = dbIndexSetNode(pInode, keyValues[0], keyDataTypes[0]);

		if (columnNumber == 1)
		{
			//link���� �߰��Ѵ�.
			dbIndexLinkNodeAppend(inodeOffset, recordRowid);
		}
		else
		{
			unsigned long	newSubRootOffset;

			newSubRootOffset = dbIndexComplexUnitInsertPostProcess(0, &keyValues[1], &keyDataTypes[1], columnNumber - 1, recordRowid);
			
			dbTouch(inodeOffset, DB_INDEX_FILE_LOAD);
			
			pInode = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + inodeOffset%gDbPageSizeIndexFilePageSize);
			
			dbMemcpyFreeIndian(&newSubRootOffset, &(pInode->indexNodeLinkOffset), sizeof(unsigned long), 1);

			gDbMmuIndexPage->mmuChanged = DB_CHANGED;
		}
		return inodeOffset;	//���� ����� inode�� offset�� �����Ѵ�.
	}

	//������ �ε��� ��尡 �����ϰ� �� �ε��� ��带 Ž���ϸ鼭 ������ �ǽ��ؾ��Ѵ�.
	else
	{
		pIndexNode		pInode;
		IndexNode		indexNode;
		signed char		keyCompareResult;
		
		pInode	= dbIndexGetNode(rootOffset);

		dbMemcpyFreeIndian(pInode, &indexNode, sizeof(unsigned long), 4);

		keyCompareResult = dbIndexCompareKey_AREAVs_STACK(pInode + 1, keyValues[0], keyDataTypes[0]);

		if (keyCompareResult == 0)
		{
			if (columnNumber == 1)
			{
				//link���� �߰��Ѵ�.
				dbIndexLinkNodeAppend(rootOffset, recordRowid);
			}
			else
			{
				unsigned long	newSubRootOffset;

				newSubRootOffset = dbIndexComplexUnitInsertPostProcess(indexNode.indexNodeLinkOffset, &keyValues[1], &keyDataTypes[1], columnNumber - 1, recordRowid);
				
				if (indexNode.indexNodeLinkOffset != newSubRootOffset)
				{				
					pInode					= dbIndexGetNode(rootOffset);
					
					indexNode.indexNodeLinkOffset	= newSubRootOffset;

					dbMemcpyFreeIndian(&indexNode.indexNodeLinkOffset, &(pInode->indexNodeLinkOffset), sizeof(unsigned long), 1);

					gDbMmuIndexPage->mmuChanged = DB_CHANGED;
				}
			}
		}
		else if (keyCompareResult == 1)
		{
			unsigned long	afterRightSubOffset;
			
			//�����ʿ��� Ž���� �ؾ��ϴ� ���
			afterRightSubOffset = dbIndexComplexUnitInsertPostProcess(indexNode.indexNodeRight, keyValues, keyDataTypes, columnNumber, recordRowid);

			if (indexNode.indexNodeRight != afterRightSubOffset)
			{
				pInode			= dbIndexGetNode(rootOffset);

				indexNode.indexNodeRight = afterRightSubOffset;

				dbMemcpyFreeIndian(&(indexNode.indexNodeRight), &(pInode->indexNodeRight), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		else
		{
			unsigned long	afterLeftSubOffset;
			
			//�����ʿ��� Ž���� �ؾ��ϴ� ���
			afterLeftSubOffset = dbIndexComplexUnitInsertPostProcess(indexNode.indexNodeLeft, keyValues, keyDataTypes, columnNumber, recordRowid);
			
			if (indexNode.indexNodeLeft != afterLeftSubOffset)
			{
				pInode					= dbIndexGetNode(rootOffset);

				indexNode.indexNodeLeft	= afterLeftSubOffset;

				dbMemcpyFreeIndian(&(indexNode.indexNodeLeft), &(pInode->indexNodeLeft), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}

		//rotation check�� �Ѵ�.
		{
			signed char	balanceFactor;

			IndexNode		subInode;
			
			balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);
			
			if (balanceFactor == 2)
			{
				pInode	= dbIndexGetNode(indexNode.indexNodeLeft);

				dbMemcpyFreeIndian(pInode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == -1)
				{
					//LRRotation
					dbIndexRotationLR(&rootOffset, &(indexNode.indexNodeLeft), &(subInode.indexNodeRight));
				}
				else
				{
					//LL rotation
					dbIndexRotationLL(&rootOffset, &(indexNode.indexNodeLeft), &(subInode.indexNodeLeft));
				}
			}
			else if (balanceFactor==-2)
			{
				pInode	= dbIndexGetNode(indexNode.indexNodeRight);

				dbMemcpyFreeIndian(pInode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == 1)
				{
					//RLRotation
					dbIndexRotationRL(&rootOffset, &(indexNode.indexNodeRight), &(subInode.indexNodeLeft));
				}
				else
				{
					//RR rotation
					dbIndexRotationRR(&rootOffset, &(indexNode.indexNodeRight), &(subInode.indexNodeRight));
				}

			}
			else{}
		}

		//indexNodeRootOffset�� ���̸� �ٽ� ����Ѵ�.
		{
			unsigned long inodeHeight;
			
			pInode	= dbIndexGetNode(rootOffset);
			
			dbMemcpyFreeIndian(pInode, &indexNode, sizeof(unsigned long), 4);

			inodeHeight = dbIndexGetCalculateHeight(indexNode.indexNodeLeft, indexNode.indexNodeRight);
			
			if (indexNode.indexNodeHeight != inodeHeight)
			{
				pInode	= dbIndexGetNode(rootOffset);

				dbMemcpyFreeIndian(&inodeHeight, &(pInode->indexNodeHeight), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		return rootOffset;
	}
}
#endif

unsigned long dbFindColumnName(unsigned long		columnStartOffset,
								  pColumnNode			*columnAddress,
								  DB_VARCHAR		*columnName,
								  unsigned char		is_AREA_AREA)
{
	//�� �Լ��� ã�� �÷��� ������ �ּҴ� columnAddress�� �ְ�
	//���ϰ����δ� ���° �÷������� �����Ѵ�.
	pDictionaryHeader		pdicHeader;
	unsigned long			columnCount;

	pdicHeader		= dbGetDictionaryHeader();
	columnCount	= 0;

	while(1)
	{
		*columnAddress = (pColumnNode)( ((char*)pdicHeader) + columnStartOffset);

		if (is_AREA_AREA == DB_AREA_VS_AREA)
		{
			if (dbStringCompare_AREAVs_AREA((DB_VARCHAR*)((*columnAddress) + 1), columnName) == 0)
			{
				break;	//�ش� ��带 ã�� ���
			}
		}
		else if (is_AREA_AREA == DB_AREA_VS_STACK)
		{
			if (dbStringCompare_AREAVs_STACK((DB_VARCHAR*)((*columnAddress) + 1), columnName) == 0)
			{
				break;	//�ش� ��带 ã�� ���
			}
		}
		else{}
		
		dbMemcpyFreeIndian(&((*columnAddress)->columnNodeNextColumnOffset), &columnStartOffset, sizeof(unsigned long), 1);
		columnCount++;
	}
	return columnCount;
}


void dbIndexComplexUnitInsertPreProcessMS(pComplexIndexNode		dbComplexIndex, 
										  unsigned long			dbColumnStart, 
										  void*					dbContents[],
										  unsigned long			dbRecordRowid)
{
	unsigned long	oldRootOffset,		newRootOffset;
	unsigned long	vCurrentColumn,		vColumnNumber;
	unsigned long	vDataTypes[DB_MAX_COMPLEX_INDEX_COLUMN_NUMBER];
	void*			pIdxContents[DB_MAX_COMPLEX_INDEX_COLUMN_NUMBER];
	
	//�����ε��� root�� offset�� ���Ѵ�.
	dbMemcpyFreeIndian(&(dbComplexIndex->complexIndexNodeIndexRootOffset),	&oldRootOffset,	sizeof(unsigned long), 1);
	
	//�����ε����� �÷��� ������ ���Ѵ�.
	dbMemcpyFreeIndian(&(dbComplexIndex->complexIndexNodeColumnNumber),		&vColumnNumber,	sizeof(unsigned long), 1);

	//�����ε����� �÷��� ������ŭ �����ϸ鼭 
	//�ش��̸��� ������ �÷��� ������ Ÿ�԰� contents �߿��� ���°�� ������������ �˾Ƴ���.
	//dbDataTypes�迭�� idxContents �迭�� ���� �ִ´�.
	{
		pColumnNode			pColumn;
		DB_VARCHAR			*pcolumnName;
		unsigned long		vColumnNumberInAllColumn;
		
		//�����ε����� �̸��� �������Ѵ�.
		pcolumnName = (DB_VARCHAR*)(dbComplexIndex + 1);

		for(vCurrentColumn = 0 ; vCurrentColumn < vColumnNumber ; vCurrentColumn++)
		{
			//columnName�� ������ �÷��� ���° �ִ��� �˾Ƴ���.
			vColumnNumberInAllColumn = dbFindColumnName(dbColumnStart, &pColumn, pcolumnName, DB_AREA_VS_AREA);
			
			//�ش� �÷��� Ű���� �����Ѵ�.
			pIdxContents[vCurrentColumn] = dbContents[vColumnNumberInAllColumn];
			
			//�ش� �÷�Ű���� ������ Ÿ���� �����Ѵ�.
			dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &vDataTypes[vCurrentColumn], sizeof(unsigned long), 1);
			
			pcolumnName = pcolumnName + (dbStringGetSize(pcolumnName) + DB_VARCHAR_SIZE)/2;
		}
	}
	//���� loop���� ���� ������ �ش� �����ε����� ������ ���� ���������� ������ ��� ������.

	//������ �����Ѵ�.
	newRootOffset = dbIndexComplexUnitInsertPostProcessMS(oldRootOffset, pIdxContents, vDataTypes, vColumnNumber, dbRecordRowid);

	if(newRootOffset != oldRootOffset)
	{
		dbMemcpyFreeIndian(&newRootOffset, &(dbComplexIndex->complexIndexNodeIndexRootOffset), sizeof(unsigned long), 1);
		gDbDictionaryChanged = DB_CHANGED;
	}
}


#if 0
void dbIndexComplexUnitInsertPreProcess(pComplexIndexNode			cInode, 
											 unsigned long		columnStartOffset, 
											 void*				contents[],
											 unsigned long		recordRowid)
{
	unsigned long	oldRootOffset, newRootOffset;
	unsigned long	currentColumn, columnNumber;
	unsigned long	dataTypes[DB_MAX_COMPLEX_INDEX_COLUMN_NUMBER];
	void*			idxContents[DB_MAX_COMPLEX_INDEX_COLUMN_NUMBER];
	
	//�����ε��� root�� offset�� ���Ѵ�.
	dbMemcpyFreeIndian(&(cInode->complexIndexNodeIndexRootOffset), &oldRootOffset, sizeof(unsigned long), 1);
	
	//�����ε����� �÷��� ������ ���Ѵ�.
	dbMemcpyFreeIndian(&(cInode->complexIndexNodeColumnNumber), &columnNumber, sizeof(unsigned long), 1);

	//�����ε����� �÷��� ������ŭ �����ϸ鼭 
	//�ش��̸��� ������ �÷��� ������ Ÿ�԰� contents �߿��� ���°�� ������������ �˾Ƴ���.
	//dataTypes�迭�� idxContents �迭�� ���� �ִ´�.
	{
		pColumnNode			pColumn;
		//COLUMN			columnNode;
		DB_VARCHAR		*columnName;
		unsigned long	columnNumberInAllColumn;
		
		//�����ε����� �̸��� �������Ѵ�.
		columnName = (DB_VARCHAR*)(cInode + 1);

		for(currentColumn = 0 ; currentColumn < columnNumber ; currentColumn++)
		{
			//columnName�� ������ �÷��� ���° �ִ��� �˾Ƴ���.
			columnNumberInAllColumn = dbFindColumnName(columnStartOffset, &pColumn, columnName, DB_AREA_VS_AREA);
			
			//�ش� �÷��� Ű���� �����Ѵ�.
			idxContents[currentColumn] = contents[columnNumberInAllColumn];
			
			//�ش� �÷�Ű���� ������ Ÿ���� �����Ѵ�.
			dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &dataTypes[currentColumn], sizeof(unsigned long), 1);
			
			columnName = columnName + (dbStringGetSize(columnName) + DB_VARCHAR_SIZE)/2;
		}
	}
	//���� loop���� ���� ������ �ش� �����ε����� ������ ���� ���������� ������ ��� ������.

	//������ �����Ѵ�.
	newRootOffset = dbIndexComplexUnitInsertPostProcess(oldRootOffset, idxContents, dataTypes, columnNumber, recordRowid);

	if(newRootOffset != oldRootOffset)
	{
		dbMemcpyFreeIndian(&newRootOffset, &(cInode->complexIndexNodeIndexRootOffset), sizeof(unsigned long), 1);
		gDbDictionaryChanged = DB_CHANGED;
	}
}
#endif

void dbIndexComplexAllInsertMS(unsigned long	dbTableOffset, 
							   void*			dbContents[], 
							   unsigned long	dbRecordRowid)
{
	
	// =======================������������(����) =======================//
	pDictionaryHeader		pdictionaryHeader;	//dictionary header�� �������� ����
	pTableNode				ptable;				//�ش� ���̺��� ����ų �༮, ��ųʸ��� �����Ƿ� ��ġ�� ����� ���� ����.
	pComplexIndexNode		pcomplexIndex;		//�����ε����� ��ųʸ����� ��带 ����ų �༮�̴�.
	unsigned long			vComplexIndexOffset;
	unsigned long			vColumnStartOffset;
	// =======================������������(����) =======================//

	pdictionaryHeader	= dbGetDictionaryHeader();							//��ųʸ� �ش� ������
	ptable				= (pTableNode)(((char*)pdictionaryHeader) + dbTableOffset);	//���̺� ������
	
	//�����ε����� ���� offset�� ���Ѵ�.
	dbMemcpyFreeIndian(&(ptable->tableNodeComplexIndexOffset),	&vComplexIndexOffset,	sizeof(unsigned long), 1);
	
	//�÷��� ���� offset�� ���Ѵ�.
	dbMemcpyFreeIndian(&(ptable->tableNodeColumnOffset),		&vColumnStartOffset,	sizeof(unsigned long), 1);

	//while���� ���鼭 �ش� ���̺��� �� �����ε����� ��ȸ�ϸ鼭 �����Ѵ�.
	while (vComplexIndexOffset)
	{
		//��ųʸ��� �ִ� �����ε��� �÷��� ã�´�.
		pcomplexIndex	= (pComplexIndexNode)((char*)pdictionaryHeader + vComplexIndexOffset);
		
		//�ش� �����ε����� ó���Ѵ�.
		dbIndexComplexUnitInsertPreProcessMS(pcomplexIndex, vColumnStartOffset, dbContents, dbRecordRowid);		// 060112_heechul, "MS" �߰�. 
		
		//���� �����ε����� �ִ� �÷��� ���� ������ �����ϱ� ���Ͽ�
		//���� offset�� �����ϴ� �ּҸ� �����Ѵ�.
		dbMemcpyFreeIndian(&(pcomplexIndex->complexIndexNodeNext), &vComplexIndexOffset, sizeof(unsigned long), 1);
	}
}
/*
void dbIndexComplexAllInsert(unsigned long	tableNodeOffset, 
								 void*			contents[], 
								 unsigned long	recordRowid)
{
	// =======================������������(����) =======================//
	pDictionaryHeader		pdicHeader;	//dictionary header�� �������� ����
	pTableNode					ptable;			//�ش� ���̺��� ����ų �༮, ��ųʸ��� �����Ƿ� ��ġ�� ����� ���� ����.
	unsigned long			complexIndexOffset;
	unsigned long			columnStartOffset;
	pComplexIndexNode					cInode;		//�����ε����� ��ųʸ����� ��带 ����ų �༮�̴�.
	// =======================������������(����) =======================//

	pdicHeader = dbGetDictionaryHeader();							//��ųʸ� �ش� ������
	ptable		= (pTableNode)(((char*)pdicHeader) + tableNodeOffset);	//���̺� ������
	
	//�����ε����� ���� offset�� ���Ѵ�.
	dbMemcpyFreeIndian(&(ptable->tableNodeComplexIndexOffset), &complexIndexOffset, sizeof(unsigned long), 1);
	
	//�÷��� ���� offset�� ���Ѵ�.
	dbMemcpyFreeIndian(&(ptable->tableNodeColumnOffset), &columnStartOffset, sizeof(unsigned long), 1);

	//while���� ���鼭 �ش� ���̺��� �� �����ε����� ��ȸ�ϸ鼭 �����Ѵ�.
	while (complexIndexOffset)
	{
		//��ųʸ��� �ִ� �����ε��� �÷��� ã�´�.
		cInode			= (pComplexIndexNode)((char*)pdicHeader + complexIndexOffset);
		
		//�ش� �����ε����� ó���Ѵ�.
		dbIndexComplexUnitInsertPreProcess(cInode, columnStartOffset, contents, recordRowid);
		
		//���� �����ε����� �ִ� �÷��� ���� ������ �����ϱ� ���Ͽ�
		//���� offset�� �����ϴ� �ּҸ� �����Ѵ�.
		dbMemcpyFreeIndian(&(cInode->complexIndexNodeNext), &complexIndexOffset, sizeof(unsigned long), 1);
	}
}*/

void dbIndexSingleAllInsertMS(unsigned long	dbStartColumnOffset, 
							  void*			dbContents[], 
							  unsigned long	dbRecordRowid)
{
	pDictionaryHeader	pdictionaryHeader;		//��ųʸ� �ش��� �������� ����
	pColumnNode			pcolumn;				//dbStartColumnOffset���κ��� ������ �÷��� ������ �� ����
	ColumnNode			vColumnNode;			//�÷� ����ü �ӽ� ������ �����ϴ� �÷����� �÷��� ���� ���⿡ �����Ѵ�.
	unsigned long		vColumnCount;			//�����ϴ� �÷��� ������ ������ �����̴�. ���ڷ� ���� dbContents[]�迭�� �ش��°�� �����ϴµ� ����Ѵ�.
	unsigned long		vRootOffset;			//�ش� �÷��� �ε����� �߰��� �Ŀ� ������� ���ο� �ε��� Ʈ���� RootOffset�� �����ϴ� �����̴�.
	
	vColumnCount = 0;

	pdictionaryHeader = dbGetDictionaryHeader();	//��ųʸ� �ش��� �޾ƿ´�.


	//�� Column�� �����ϸ鼭 �� �÷����� �ε����� ������ ��쿡 ������ �õ��Ѵ�.
	while(dbStartColumnOffset)
	{
		pcolumn = (pColumnNode)(((char*)pdictionaryHeader) + dbStartColumnOffset);	//�ش� �÷��� �����Ѵ�.
		dbMemcpyFreeIndian(pcolumn, &vColumnNode, sizeof(unsigned long), 3);

		if (vColumnNode.columnNodeIndexRootOffset != 0)		//�ش� �÷��� �ε����� �����Ҷ� ó��
		{
			
			vRootOffset = dbIndexSingleUnitInsertMS(vColumnNode.columnNodeIndexRootOffset,
												   dbContents[vColumnCount],
												   vColumnNode.columnNodeDataType,
												   dbRecordRowid);
			
			if (vColumnNode.columnNodeIndexRootOffset != vRootOffset)
			{
				vColumnNode.columnNodeIndexRootOffset = vRootOffset;

				if (vColumnNode.columnNodeIndexRootOffset == 0)
				{
					vColumnNode.columnNodeIndexRootOffset = 1;
				}
				
				dbMemcpyFreeIndian(&(vColumnNode.columnNodeIndexRootOffset), 
								   &(pcolumn->columnNodeIndexRootOffset),
								   sizeof(unsigned long), 1);
				
				gDbDictionaryChanged = DB_CHANGED;
			}
		}

		dbStartColumnOffset = vColumnNode.columnNodeNextColumnOffset;	//�����÷����� �̵��Ѵ�.
		vColumnCount++;
	}
}

/*
void dbIndexSingleAllInsert(unsigned long	dbStartColumnOffset, 
							void*			dbContents[], 
							unsigned long	dbRecordRowid)
{
	pDictionaryHeader	pdictionaryHeader;
	pColumnNode			pcolumn;		//��ųʸ������� �÷��� ������ �� ����
	ColumnNode			vColumnNode; 
	unsigned long		vColumnCount;
	unsigned long		vRootOffset;
	
	vColumnCount = 0;

	pdictionaryHeader = dbGetDictionaryHeader();


	//�� Column�� �����ϸ鼭 ���÷����� ��
	while(dbStartColumnOffset)
	{
		pcolumn = (pColumnNode)(((char*)pdictionaryHeader) + dbStartColumnOffset);	//�ش� �÷��� �����Ѵ�.
		dbMemcpyFreeIndian(pcolumn, &vColumnNode, sizeof(unsigned long), 3);

		if (vColumnNode.columnNodeIndexRootOffset != 0)		//�ش� �÷��� �ε����� �����Ҷ� ó��
		{
			
			vRootOffset = dbIndexSingleUnitInsert(vColumnNode.columnNodeIndexRootOffset,
												  dbContents[vColumnCount],
												  vColumnNode.columnNodeDataType,
												  dbRecordRowid);
			
			if (vColumnNode.columnNodeIndexRootOffset != vRootOffset)
			{
				vColumnNode.columnNodeIndexRootOffset = vRootOffset;

				dbMemcpyFreeIndian(&(vColumnNode.columnNodeIndexRootOffset), 
								   &(pcolumn->columnNodeIndexRootOffset),
								   sizeof(unsigned long), 1);
			}
		}

		dbStartColumnOffset = vColumnNode.columnNodeNextColumnOffset;	//�����÷����� �̵��Ѵ�.
		vColumnCount++;
	}
}
*/

void dbIndexTreeTraverseAllAsc(unsigned long rootOffset,
									unsigned char vDepth)
{
	if ((rootOffset == 0)||(rootOffset == 1))
	{
		return;
	}
	else
	{
		pIndexNode	pInode;
		IndexNode	INDEX_NODE;

		pInode = dbIndexGetNode(rootOffset);
		dbMemcpyFreeIndian(pInode, &INDEX_NODE, sizeof(unsigned long), 4);

		dbIndexTreeTraverseAllAsc(INDEX_NODE.indexNodeLeft, vDepth);

		
		if (vDepth == 1)
		{
			pLinkNode	pLink;
			LinkNode		LINK_NODE;
			unsigned long	linkOffset;

			linkOffset = INDEX_NODE.indexNodeLinkOffset;

			
			while(linkOffset)
			{
				pLink = dbIndexGetLink(linkOffset);
				dbMemcpyFreeIndian(pLink, &LINK_NODE, sizeof(unsigned long), 2);

				*( dbSearchArray + dbSearchCount) = LINK_NODE.linkNodeRecordRowid;
				dbSearchCount++;

				linkOffset =  LINK_NODE.linkNodeNext;
			}
		}
		else
		{
			dbIndexTreeTraverseAllAsc(INDEX_NODE.indexNodeLinkOffset, vDepth-1);
		}
		dbIndexTreeTraverseAllAsc(INDEX_NODE.indexNodeRight, vDepth);
	}
}

void dbIndexTreeTraverseAllDesc(unsigned long rootOffset,
								unsigned char vDepth)
{
	if ((rootOffset == 0)||(rootOffset == 1))
	{
		return;
	}
	else
	{
		pIndexNode	pInode;
		IndexNode	INDEX_NODE;

		pInode = dbIndexGetNode(rootOffset);
		dbMemcpyFreeIndian(pInode, &INDEX_NODE, sizeof(unsigned long), 4);

		dbIndexTreeTraverseAllDesc(INDEX_NODE.indexNodeRight, vDepth);

		
		if (vDepth == 1)
		{
			pLinkNode			pLink;
			LinkNode			LINK_NODE;
			unsigned long	linkOffset;

			linkOffset = INDEX_NODE.indexNodeLinkOffset;

			
			while(linkOffset)
			{
				pLink = dbIndexGetLink(linkOffset);
				dbMemcpyFreeIndian(pLink, &LINK_NODE, sizeof(unsigned long), 2);

				*( dbSearchArray + dbSearchCount) = LINK_NODE.linkNodeRecordRowid;
				dbSearchCount++;

				linkOffset =  LINK_NODE.linkNodeNext;
			}
		}
		else
		{
			dbIndexTreeTraverseAllDesc(INDEX_NODE.indexNodeLinkOffset, vDepth-1);
		}

		dbIndexTreeTraverseAllDesc(INDEX_NODE.indexNodeLeft, vDepth);
	}
}


void dbIndexTreeTraverseAsc(unsigned long rootOffset,
								unsigned long traverseSubLink)
{
	#if 0
	#ifdef DB_SEARCH_CHANGED
	if (gDbSearchData.ResultArraySize <= gDbSearchData.dbSearchCount )
	{
		return;
	}
	#endif
	#endif
	
	if ((rootOffset == 0)||(rootOffset == 1))
	{
		return;
	}
	else
	{
		pIndexNode	pInode;
		IndexNode	INDEX_NODE;

		pInode = dbIndexGetNode(rootOffset);
		dbMemcpyFreeIndian(pInode, &INDEX_NODE, sizeof(unsigned long), 4);

		dbIndexTreeTraverseAsc(INDEX_NODE.indexNodeLeft, traverseSubLink);

		
		if (traverseSubLink == 1)
		{
			pLinkNode	pLink;
			LinkNode		LINK_NODE;
			unsigned long	linkOffset;

			linkOffset = INDEX_NODE.indexNodeLinkOffset;

			
			while(linkOffset)
			{
				pLink = dbIndexGetLink(linkOffset);
				dbMemcpyFreeIndian(pLink, &LINK_NODE, sizeof(unsigned long), 2);
				#if 0
				*( dbSearchArray + dbSearchCount) = LINK_NODE.linkNodeRecordRowid;
				dbSearchCount++;
				#endif

				if(gDbSearchData.ResultArraySize > gDbSearchData.dbSearchCount)
				{
					*(gDbSearchData.ResultArray + gDbSearchData.dbSearchCount) = LINK_NODE.linkNodeRecordRowid;
					gDbSearchData.dbSearchCount++;
				}
				else
				{
					gDbSearchData.vRemainRecordsCount++;
				}
				
				#if 0
				#ifdef DB_SEARCH_CHANGED
				if (gDbSearchData.ResultArraySize <= gDbSearchData.dbSearchCount  )
				{
					return;
				}
				#endif
				#endif
				
				linkOffset =  LINK_NODE.linkNodeNext;
			}
		}
		else
		{
			#if 0
			*( dbSearchArray + dbSearchCount) = rootOffset + sizeof(IndexNode);
			dbSearchCount++;
			#endif

			if (gDbSearchData.ResultArraySize > gDbSearchData.dbSearchCount)
			{
				*(gDbSearchData.ResultArray + gDbSearchData.dbSearchCount) = rootOffset + sizeof(IndexNode);
				gDbSearchData.dbSearchCount++;
			}
			else
			{
				gDbSearchData.vRemainRecordsCount++;
			}
		}
		
		#if 0
		#ifdef DB_SEARCH_CHANGED
		if (gDbSearchData.ResultArraySize <= gDbSearchData.dbSearchCount )
		{
			return;
		}
		#endif
		#endif
		
		dbIndexTreeTraverseAsc(INDEX_NODE.indexNodeRight, traverseSubLink);
	}
}

void dbIndexTreeTraverseDesc(unsigned long rootOffset,
								unsigned long traverseSubLink)
{
	#if 0
	#ifdef DB_SEARCH_CHANGED
	if (gDbSearchData.ResultArraySize <= gDbSearchData.dbSearchCount )
	{
		return;
	}
	#endif
	#endif
	
	if ((rootOffset == 0)||(rootOffset == 1))
	{
		return;
	}
	else
	{
		pIndexNode	pInode;
		IndexNode	INDEX_NODE;

		pInode = dbIndexGetNode(rootOffset);
		dbMemcpyFreeIndian(pInode, &INDEX_NODE, sizeof(unsigned long), 4);

		dbIndexTreeTraverseDesc(INDEX_NODE.indexNodeRight, traverseSubLink);

		
		if (traverseSubLink == 1)
		{
			pLinkNode	pLink;
			LinkNode		LINK_NODE;
			unsigned long	linkOffset;

			linkOffset = INDEX_NODE.indexNodeLinkOffset;

			
			while(linkOffset)
			{
				pLink = dbIndexGetLink(linkOffset);
				dbMemcpyFreeIndian(pLink, &LINK_NODE, sizeof(unsigned long), 2);
				
				#if 0
				*( dbSearchArray + dbSearchCount) = LINK_NODE.linkNodeRecordRowid;
				dbSearchCount++;
				#endif

				if (gDbSearchData.ResultArraySize > gDbSearchData.dbSearchCount)
				{
					*(gDbSearchData.ResultArray + gDbSearchData.dbSearchCount) = LINK_NODE.linkNodeRecordRowid;
					gDbSearchData.dbSearchCount++;
				}
				else
				{
					gDbSearchData.vRemainRecordsCount++;
				}

				#if 0
				#ifdef DB_SEARCH_CHANGED
				if (gDbSearchData.ResultArraySize <= gDbSearchData.dbSearchCount  )
				{
					return;
				}
				#endif
				#endif

				linkOffset =  LINK_NODE.linkNodeNext;
			}
		}
		else
		{
			#if 0
			*( dbSearchArray + dbSearchCount) = rootOffset + sizeof(IndexNode);
			dbSearchCount++;
			#endif

			if (gDbSearchData.ResultArraySize > gDbSearchData.dbSearchCount)
			{
				*(gDbSearchData.ResultArray + gDbSearchData.dbSearchCount) = rootOffset + sizeof(IndexNode);
				gDbSearchData.dbSearchCount++;
			}
			else
			{
				gDbSearchData.vRemainRecordsCount++;
			}

		}

		#if 0
		#ifdef DB_SEARCH_CHANGED
		if (gDbSearchData.ResultArraySize <= gDbSearchData.dbSearchCount )
		{
			return;
		}
		#endif
		#endif

		dbIndexTreeTraverseDesc(INDEX_NODE.indexNodeLeft, traverseSubLink);
	}
}

void dbIndexTreeTraverse(unsigned long rootOffset,
                            unsigned char isAsc,
                            unsigned char traverseSubLink)
{
	if (isAsc == DB_ASC)
	{
		dbIndexTreeTraverseAsc(rootOffset, traverseSubLink);
	}
	else if(isAsc == DB_DESC)
	{
		dbIndexTreeTraverseDesc(rootOffset, traverseSubLink);
	}
	else{}
}

void dbIndexTreeTraverseAll(unsigned long rootOffset, 
							unsigned char isAsc, 
							unsigned char vDepth)
{
	if (isAsc == DB_ASC)
	{
		dbIndexTreeTraverseAllAsc(rootOffset, vDepth);
	}
	else if(isAsc == DB_DESC)
	{
		dbIndexTreeTraverseAllDesc(rootOffset, vDepth);
	}
	else{}
	
}

unsigned long dbIndexSearchGetInode(unsigned long	rootOffset,
										unsigned long	dataType,
										void*			keyValue)
{
	pIndexNode				pInode;
	IndexNode				INDEX_NODE;
	signed		char		compareResult;

	while(rootOffset)
	{
		pInode = dbIndexGetNode(rootOffset);

		dbMemcpyFreeIndian(pInode, &INDEX_NODE, sizeof(unsigned long), 4);

		//��
		compareResult = dbIndexCompareKey_AREAVs_STACK(pInode + 1, keyValue, dataType);

		if (compareResult == 0)
		{
			return INDEX_NODE.indexNodeLinkOffset;
		}
		else if(compareResult == -1)
		{
			rootOffset = INDEX_NODE.indexNodeLeft;
		}
		else
		{
			rootOffset = INDEX_NODE.indexNodeRight;
		}
	}
	return 0;
}

unsigned char dbIndexCheckName(DB_VARCHAR*	areaAddress ,
								  DB_VARCHAR*	indexColumnName[],
								  unsigned long	indexColumnNumber)
{
	unsigned long	i;
	for(i = 0 ; i < indexColumnNumber ; i++)
	{
		if (dbStringCompare_AREAVs_STACK(areaAddress, indexColumnName[i])==0)
		{
		}
		else
		{
			return 0;
		}
		areaAddress = areaAddress + (dbStringGetSize(areaAddress) + DB_VARCHAR_SIZE)/2;
	}
	return 1;
}

unsigned long dbApiTableColumnNumber(DB_VARCHAR	*dbTableName)
{
	// =========================���� ���� ����(����) =========================//
	pDictionaryHeader		pdictionaryHeader;	//Dictionary file�� �ش��� �������� ����
	unsigned	long			*ptableOffset;		//Dictionary file�� �ö���� �������� ���̺��� offset�� �����ϰ� ��ġ�� �������� ����
	pTableNode			pTable;				//Dictionary file�� �ö���� �������� Table�� ��ġ�� �������� ����
	pColumnNode			pcolumnNode;		//Dictionary file�� �ö���� �������� Column�� ��ġ�� �������� ����
	TableNode			vTableNode;			//Dictionary file������  Table����� ������ ������ ����
	unsigned long			vTableOffset,		//Dictionary file�� �ö���� ������ ���̺��� ���� offset�� ������ ����
						vColumnOffset;		//Dictionary file�� �ö���� ������ ���̺��� ���� �÷��� offset�� �����ϰ� ��ġ�� �������� ����
	unsigned	long			vColumnCount;		//Table�� Column�� Total ������ ������ ����
	// =========================���� ���� ����(��) =========================//

	#if DB_SHOW_ERROR_CODE				//Error Check
	gDbErrorCode = DB_ERROR_NO_ERROR;		//Error Check
	#endif									//Error Check

	if (gDbStartUpDB != DB_DO_RUNNING)
	{
		#if DB_SHOW_ERROR_CODE								//Error Check
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;		//Error Check
		#endif													//Error Check
		
		return 0;
	}

	pdictionaryHeader	= dbGetDictionaryHeader();							//Dictionary�� �����Ѵ�.
	ptableOffset		= &(pdictionaryHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����
	
	//���̺��� ã�´�.
	if (!dbTableFindTableName(&ptableOffset, dbTableName))
	{
		//ERROR : �Է����� ���� tableName�� table�� �ý��ۿ� ���������ʴ� ���
		#if DB_SHOW_ERROR_CODE						//Error Check
		gDbErrorCode = DB_ERROR_TABLE_NOT_FOUND;		//Error Check
		#endif											//Error Check

		return 0;
	}


	//table�� offset�� ����Ǿ� �ִ�.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table�� �޸𸮸� �����Ѵ�.
	pTable = (pTableNode)(((char*)pdictionaryHeader) + vTableOffset);
	
	//���̺��� ������ TABLE_NODE�� �����Ѵ�.
	dbMemcpyFreeIndian(pTable, &vTableNode, sizeof(unsigned long), 6);

	vColumnOffset = vTableNode.tableNodeColumnOffset;
	vColumnCount  = 0;

	while(vColumnOffset)
	{
		vColumnCount++;
		pcolumnNode = (pColumnNode)(((char*)pdictionaryHeader) + vColumnOffset);
		dbMemcpyFreeIndian(&(pcolumnNode->columnNodeNextColumnOffset), &vColumnOffset, sizeof(unsigned long), 1);
	}

	return vColumnCount;
}

#if 0
unsigned long dbApiSearch(DB_VARCHAR*		tableName,
						    DB_VARCHAR*		indexColumnName[],
							unsigned long	indexColumnNumber,
							void*			conditionSet[],
							unsigned long	conditionColumnNumber,
							unsigned char	traverseOrder,
							unsigned long	resultArray[],
							unsigned char	*refMemory)
{
	pDictionaryHeader		pdicHeader;	//Dictionary file�� �ش��� �������� ����
	pTableNode					pTable;			//Dictionary file�� �ö���� ���������� ���̺��� �������� ����
	TableNode					TABLE_NODE;
	pColumnNode					pColumn;
	unsigned long			*ptableOffset, vTableOffset;
	unsigned long			*pcolumnOffset, vColumnOffset;
	unsigned long			indexOffset;

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary�� �����Ѵ�.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����
	
	//���̺��� ã�´�.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : �Է����� ���� tableName�� table�� �ý��ۿ� ���������ʴ� ���
		return 0;
	}
	// ===========�Ʒ����� �ش� ���̺��� ã������ ó�� ===========//

	//table�� offset�� ����Ǿ� �ִ�.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table�� �޸𸮸� �����Ѵ�.
	pTable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//���̺��� ������ TABLE_NODE�� �����Ѵ�.
	dbMemcpyFreeIndian(pTable, &TABLE_NODE, sizeof(unsigned long), 6);

	if (TABLE_NODE.tableNodeStartPage == 0)
	{
		//�����ͻ����� �����ٸ� ���̻� �˻��Ұ͵� ����..
		return 0;
	}

	//�ε����� ���� offset�� ��ġ�� ã�Ƴ���.
	{
		if (indexColumnNumber == 1)	//�����ε����� ���
		{
			//�÷��� ���� offset�� ����Ǿ� �ִ� �ּҸ� �������Ѵ�.
			pcolumnOffset = &(pTable->tableNodeColumnOffset);

			//�÷��� ã�´�.
			if(!dbTableFindColumnName(&pcolumnOffset, indexColumnName[0]))
			{
				//ERROR : �ش� �÷��� �������� �ʴ� ���
				return 0;
			}

			//�÷��� offset�� ����Ǿ� �ִ�.
			dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);

			pColumn =  (pColumnNode)(((char*)pdicHeader) + vColumnOffset);
			
			dbMemcpyFreeIndian(&(pColumn->columnNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);
		}
		else							//�����ε����� ���
		{
			unsigned long	complexOffset;
			pComplexIndexNode			pComplexIndex;
			ComplexIndexNode			complexIndex;
			dbMemcpyFreeIndian(&(pTable->tableNodeComplexIndexOffset), &complexOffset, sizeof(unsigned long), 1);

			//��� �����ε��� �÷��� ���������� �湮�ϸ鼭 ã�´�.
			while(complexOffset)
			{
				pComplexIndex = (pComplexIndexNode)(((char*)pdicHeader) + complexOffset);
				dbMemcpyFreeIndian(pComplexIndex, &complexIndex, sizeof(unsigned long), 3);

				if (complexIndex.complexIndexNodeColumnNumber == indexColumnNumber)
				{
					if (dbIndexCheckName((DB_VARCHAR*)(pComplexIndex + 1) , indexColumnName, indexColumnNumber) != 0)
					{
						//�ش� �����ε����� ã�����.
						break;
					}
				}

				complexOffset = complexIndex.complexIndexNodeNext;
			}

			if (complexOffset==DB_NULL) return 0;

			dbMemcpyFreeIndian(&(pComplexIndex->complexIndexNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);
		}
	}
	//indexOffset���� �ε����� ���� offset�� ���� ����Ǿ��ִ�.


	//�ε��� Ž���� �����Ѵ�.
	if (conditionColumnNumber == 0)
	{
		dbSearchArray = resultArray;
		dbSearchCount = 0;
		if (indexColumnNumber == (conditionColumnNumber + 1))
		{
			//������������ recordRowid�� ����� ������ ���
			*refMemory = DB_DATA_FILE_LOAD;
			dbIndexTreeTraverse(indexOffset, traverseOrder, 1);	//����� �����ϴ� �κ�
		}
		else
		{
			//�˻��� ����� �ε��� ������ �����̴�.
			*refMemory = DB_INDEX_FILE_LOAD;
			dbIndexTreeTraverse(indexOffset, traverseOrder, 0);	//����� �����ϴ� �κ�
		}
	}
	else
	{
		unsigned long	i;
		unsigned long	columnDataType;
		for (i = 0 ; i < conditionColumnNumber ; i++)
		{
			//�÷��� ���� offset�� ����Ǿ� �ִ� �ּҸ� �������Ѵ�.
			pcolumnOffset = &(pTable->tableNodeColumnOffset);
			
			//ã�� �÷��� offset�� ���Ѵ�.
			dbTableFindColumnName(&pcolumnOffset, indexColumnName[i]);
			dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);
						
			
			//�ش� �÷��� �������Ѵ�.
			pColumn =  (pColumnNode)(((char*)pdicHeader) + vColumnOffset);
			
			//�÷��� dataType�� �����Ѵ�.
			dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &columnDataType, sizeof(unsigned long), 1);
				
			
			//�˻��� �ǽ��Ѵ�.
			indexOffset = dbIndexSearchGetInode(indexOffset, columnDataType, conditionSet[i]);
			
			if (indexOffset == 0) return 0;
			
			//�˻��� ��� �ٸ� Ʈ���� rootOffset�� �˻��ȴ�.
			

			if (( (conditionColumnNumber - 1) == i ) && (indexColumnNumber != conditionColumnNumber))		//�������� �ѹ��� �Ͼ��. ����� �����ϴ� �κ�
			{
				dbSearchArray = resultArray;
				dbSearchCount = 0;

				if (indexColumnNumber == (conditionColumnNumber + 1))
				{
					//������������ recordRowid�� ����� ������ ���
					*refMemory = DB_DATA_FILE_LOAD;
					dbIndexTreeTraverse(indexOffset, traverseOrder, 1);
				}
				else
				{
					//�˻��� ����� �ε��� ������ �����̴�.
					*refMemory = DB_INDEX_FILE_LOAD;
					dbIndexTreeTraverse(indexOffset, traverseOrder, 0);
				}
			}
			else if(( (conditionColumnNumber - 1) == i ) && (indexColumnNumber == conditionColumnNumber))		//�������� �ѹ��� �Ͼ��. ����� �����ϴ� �κ�
			{
				dbSearchArray = resultArray;
				dbSearchCount = 0;

				//�˻��� ����� �ε��� ������ �����̴�.
				*refMemory = DB_INDEX_FILE_LOAD;

				dbIndexLinkTraverse(indexOffset);
			}
		}
	}
	return dbSearchCount;
}
#endif

unsigned char dbDataGetRecord(unsigned long columnStartOffset, 
								 unsigned long recordRowid, 
								 void* resultArray[])
{
	pDictionaryHeader		pdicHeader;	//Dictionary file�� �ش��� �������� ����
	pColumnNode					pColumn;
	ColumnNode			columnNode;
	char					*pRecord;
	unsigned long			columnCount;

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary�� �����Ѵ�.
	
	if (columnStartOffset == 0)
	{
		return 0;
	}
	else
	{
		//���� ���ڵ带 �����´�.
		dbTouch(recordRowid, DB_DATA_FILE_LOAD);
		pRecord = gDbMmuDataPage->mmuMemoryAddress + recordRowid%gDbPageSizeDataFilePageSize;
		columnCount = 0;
	}

	while(columnStartOffset)
	{
		resultArray[columnCount] = pRecord;	//record�� �ش� ��ġ�� �÷��� �����Ѵ�.
		
		//�����÷������� ����
		columnCount++;

		pColumn = (pColumnNode)(((char*)pdicHeader) + columnStartOffset);
		dbMemcpyFreeIndian(pColumn, &columnNode, sizeof(unsigned long), 3);

		if (columnNode.columnNodeDataType == DB_VARCHAR_TYPE)
		{
			pRecord = pRecord + dbStringGetSize((DB_VARCHAR*)pRecord) + DB_VARCHAR_SIZE;
		}
		else if (columnNode.columnNodeDataType == DB_UINT8_TYPE)
		{
			pRecord += DB_UINT8_SIZE;
		}
		else if (columnNode.columnNodeDataType == DB_UINT16_TYPE)
		{
			pRecord += DB_UINT16_SIZE;
		}
		else if (columnNode.columnNodeDataType == DB_UINT32_TYPE)
		{
			pRecord += DB_UINT32_SIZE;
		}

		columnStartOffset = columnNode.columnNodeNextColumnOffset;
	}
	return 1;
}

void dbDataRecordSetColumn(void*			target, 
							   void*			source, 
							   unsigned long	columnType)
{
	if (columnType == DB_VARCHAR_TYPE)
	{
		dbStringCopy(target, source);
	}
	else if (columnType == DB_UINT8_TYPE)
	{
		dbMemcpyFreeIndian(source, target, DB_UINT8_SIZE, 1);
	}
	else if (columnType == DB_UINT16_TYPE)
	{
		dbMemcpyFreeIndian(source, target, DB_UINT16_SIZE, 1);
	}
	else if (columnType == DB_UINT32_TYPE)
	{
		dbMemcpyFreeIndian(source, target, DB_UINT32_SIZE, 1);
	}
	else{}
}

#if 0
void dbApiGetRecord(DB_VARCHAR	*tableName,
					   unsigned long	recordRowid,
					   void*			columnsValue[])
{
	pDictionaryHeader		pdicHeader;		//Dictionary file�� �ش��� �������� ����
	pTableNode			ptable;			//Dictionary file�� �ö���� ���������� ���̺��� �������� ����
	TableNode			tableNode;
	unsigned long			*ptableOffset, vTableOffset;

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary�� �����Ѵ�.
	ptableOffset		= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����

#if DB_SHOW_ERROR_CODE
	gDbErrorCode = DB_ERROR_NO_ERROR;
#endif

//==============ERROR Code (Begin)==============//
if (gDbStartUpDB != DB_DO_RUNNING)
{
	#if DB_SHOW_ERROR_CODE
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;
	#endif
	return;
}
//==============ERROR Code ( end )==============//

	
	//���̺��� ã�´�.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : �Է����� ���� tableName�� table�� �ý��ۿ� ���������ʴ� ���
		return;
	}
	// ===========�Ʒ����� �ش� ���̺��� ã������ ó�� ===========//

	//table�� offset�� ����Ǿ� �ִ�.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table�� �޸𸮸� �����Ѵ�.
	ptable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//���̺��� ������ TABLE_NODE�� �����Ѵ�.
	dbMemcpyFreeIndian(ptable, &tableNode, sizeof(unsigned long), 6);

	//�ش� ���ڵ��� ���� ä���´�.
	dbDataGetRecord(tableNode.tableNodeColumnOffset, recordRowid, columnsValue);
}
#endif
#if 0
void dbApiRecordUpdateWithoutIndexKey(DB_VARCHAR	*tableName,
											unsigned long		recordRowid,
											DB_VARCHAR		*columnName,
											void				*updateColumnValue)
{
#if DB_SHOW_ERROR_CODE
	gDbErrorCode = DB_ERROR_NO_ERROR;
#endif

//==============ERROR Code (Begin)==============//
if (gDbStartUpDB != DB_DO_RUNNING)
{
	#if DB_SHOW_ERROR_CODE
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;
	#endif
	return ;
}
//==============ERROR Code ( end )==============//

	dbRecordUpdate(tableName, recordRowid, columnName, updateColumnValue, DB_UPDATE_WITHOUT_INDEX);
}
#endif
#if 0
void dbApiRecordUpdateWithSingleIndexKey(DB_VARCHAR	*tableName,
												unsigned long		recordRowid,
												DB_VARCHAR		*columnName,
												void				*updateColumnValue)
{

#if DB_SHOW_ERROR_CODE
	gDbErrorCode = DB_ERROR_NO_ERROR;
#endif

//==============ERROR Code (Begin)==============//
if (gDbStartUpDB != DB_DO_RUNNING)
{
	#if DB_SHOW_ERROR_CODE
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;
	#endif
	return ;
}
//==============ERROR Code ( end )==============//

	dbRecordUpdate(tableName, recordRowid, columnName, updateColumnValue, DB_UPDATE_WITH_SINGLE_INDEX);
}
#endif

unsigned long dbColumnTotalCount(unsigned long startColumnOffset)
{
	unsigned long			totalColumnNumber;
	pColumnNode					pColumn;
	pDictionaryHeader		pdicHeader;

	pdicHeader			= dbGetDictionaryHeader();			//Dictionary�� �����Ѵ�.
	totalColumnNumber = 0;
	
	while(startColumnOffset)
	{
		totalColumnNumber++;

		pColumn = (pColumnNode)(((char*)pdicHeader) + startColumnOffset);

		dbMemcpyFreeIndian(&(pColumn->columnNodeNextColumnOffset), &startColumnOffset, sizeof(unsigned long), 1);
	}

	return totalColumnNumber;
}

void dbDeleteNodeSortingInDataPage(char* pBaseAddress, unsigned long vDeleteOffset, unsigned long vDeleteSize)
{
	pDataFilePageHeader 	pDataHeader;
	unsigned long			vDeleteRecordOffset;
	DeleteNode			vDeleteNode;
	pDeleteNode			pDNode;
	
	pDataHeader 	= (pDataFilePageHeader)pBaseAddress;
	pDNode		= (pDeleteNode)(pBaseAddress + vDeleteOffset);

	dbMemcpyFreeIndian(&(pDataHeader->dataHeaderDeleteRecordOffset), &vDeleteRecordOffset, sizeof(unsigned long), 1);

	vDeleteNode.deleteNodeSize = vDeleteSize;
	if (vDeleteRecordOffset == 0)
	{
		//ó������ ������ ����� ��� ó��
		vDeleteNode.deleteNodeNext = 0;

		//������ Page�ش��� ��������� offset�� ���۰����� �����Ѵ�.
		dbMemcpyFreeIndian(&vDeleteRecordOffset, &(pDataHeader->dataHeaderDeleteRecordOffset), sizeof(unsigned long), 1);
	}
	else if (vDeleteRecordOffset > vDeleteOffset)
	{
		//ó������ ������ ����� ��� ó��
		vDeleteNode.deleteNodeNext = vDeleteRecordOffset;

		//������ Page�ش��� ��������� offset�� ���۰����� �����Ѵ�.
		dbMemcpyFreeIndian(&vDeleteRecordOffset, &(pDataHeader->dataHeaderDeleteRecordOffset), sizeof(unsigned long), 1);
	}
	else
	{
		unsigned long		vNextOffset;
		pDeleteNode		pCurDNode;

		while(1)
		{
			pCurDNode = (pDeleteNode)(pBaseAddress + vDeleteRecordOffset);
			dbMemcpyFreeIndian(&(pCurDNode->deleteNodeNext), &vNextOffset, sizeof(unsigned long), 1);

			if (vNextOffset == 0)
			{
				break;
			}
			if (vNextOffset > vDeleteOffset)
			{
				break;
			}

			vDeleteRecordOffset = vNextOffset;
		}
		//whlie�� Ż���Ҷ�
		//vDeleteRecordOffset���� �� ��������� offset
		//vNextOffset���� �޳���� offset�� ����ȴ�.
		dbMemcpyFreeIndian(&vDeleteOffset, &(pCurDNode->deleteNodeNext), sizeof(unsigned long), 1);
		
		vDeleteNode.deleteNodeNext = vNextOffset;
	}

	//������带 �����Ѵ�.
	dbMemcpyFreeIndian(&(vDeleteNode), pDNode, sizeof(unsigned long), 2);
}


void dbDataSetRecordDelete(unsigned long	startColumnOffset,
							   void*			columnsValue[],
							   unsigned long	recordRowid)
{
	pDictionaryHeader		pdicHeader;
	pDataFilePageHeader		pdatHeader;
	DataFilePageHeader		datHeader;
	pColumnNode					pColumn;
	//DeleteNode					deleteNode;
	unsigned long			recordSize;
	unsigned long			dataType;
	unsigned long			columnCount;
	unsigned long			recordSlotValue;

	pdicHeader = dbGetDictionaryHeader();
	dbTouch(recordRowid, DB_DATA_FILE_LOAD);								//rowid�� �����ϴ� datapage�� Loading�Ѵ�.
	pdatHeader = (pDataFilePageHeader)(gDbMmuDataPage->mmuMemoryAddress);	//�ش� Data Page�� Header�� �����Ѵ�.
	dbMemcpyFreeIndian(pdatHeader, & datHeader, sizeof(unsigned long), 4);			//�ش� Data Page�� Header�� �����Ѵ�.

	if (datHeader.dataHeaderNumberOfSlots == 1)
	{
		datHeader.dataHeaderDeleteRecordOffset = 0;
		datHeader.dataHeaderEndOfRecords		= sizeof(DataFilePageHeader);
		datHeader.dataHeaderNumberOfSlots		=0;
		
		dbMemcpyFreeIndian(&(datHeader.dataHeaderDeleteRecordOffset),	&(pdatHeader->dataHeaderDeleteRecordOffset),	sizeof(unsigned long), 1);
		dbMemcpyFreeIndian(&(datHeader.dataHeaderEndOfRecords),			&(pdatHeader->dataHeaderEndOfRecords),			sizeof(unsigned long), 1);
		dbMemcpyFreeIndian(&(datHeader.dataHeaderNumberOfSlots),		&(pdatHeader->dataHeaderNumberOfSlots),		sizeof(unsigned long), 1);
		
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
		return;
	}
	else
	{
	recordSize  = 0;
	columnCount = 0;
	
	while(startColumnOffset)
	{
		pColumn	= (pColumnNode)(((char*)pdicHeader) + startColumnOffset);
		dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &dataType, sizeof(unsigned long), 1);

		if (dataType == DB_VARCHAR_TYPE)
		{
			recordSize = recordSize + dbStringGetSize((DB_VARCHAR*)columnsValue[columnCount]) + DB_VARCHAR_SIZE;
		}
		else if (dataType == DB_UINT8_TYPE)
		{
			recordSize += DB_UINT8_SIZE;
		}
		else if (dataType == DB_UINT16_TYPE)
		{
			recordSize += DB_UINT16_SIZE;
		}
		else if (dataType == DB_UINT32_TYPE)
		{
			recordSize += DB_UINT32_SIZE;
		}
		else{}

		columnCount++;
		dbMemcpyFreeIndian(&(pColumn->columnNodeNextColumnOffset), &startColumnOffset, sizeof(unsigned long), 1);
	}
	//�ش� ���ڵ尡 �����ϴ� ũ��(recordSize)�� �˼� �ִ�.
	
	//�ش� ���ڵ��� ������ ������ ���� ���Ѵ�.
		dbTouch(recordRowid, DB_DATA_FILE_LOAD);
	recordSlotValue = recordRowid%gDbPageSizeDataFilePageSize;

	//������ ó���Ѵ�.
	{
		unsigned long	slotNumber = 0;
		unsigned long	*pSlot;
		unsigned long	vSlot;

		for(slotNumber = 0 ; slotNumber < datHeader.dataHeaderNumberOfSlots ; slotNumber++)
		{
			pSlot = (unsigned long*)(gDbMmuDataPage->mmuMemoryAddress + gDbPageSizeDataFilePageSize - (slotNumber + 1)*DB_SLOT_SIZE);
			dbMemcpyFreeIndian(pSlot, &vSlot, sizeof(unsigned long), 1);
			
			if (vSlot == recordSlotValue) break;
		}

		if ((slotNumber + 1) != datHeader.dataHeaderNumberOfSlots)
		{
			unsigned long	*pLastSlot;
			unsigned long	vLastSlot;
			pLastSlot = (unsigned long*)(gDbMmuDataPage->mmuMemoryAddress + gDbPageSizeDataFilePageSize - (datHeader.dataHeaderNumberOfSlots)*DB_SLOT_SIZE);
			dbMemcpyFreeIndian(pLastSlot, &vLastSlot, sizeof(unsigned long), 1);
			dbMemcpyFreeIndian(&vLastSlot, pSlot, sizeof(unsigned long), 1);
		}
		datHeader.dataHeaderNumberOfSlots--;
		dbMemcpyFreeIndian(&(datHeader.dataHeaderNumberOfSlots), &(pdatHeader->dataHeaderNumberOfSlots), sizeof(unsigned long), 1);
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
	}
	#if 0
	//record�� ���� ���� ó���� �Ѵ�.
	deleteNode.deleteNodeSize = recordSize;
	deleteNode.deleteNodeNext = datHeader.dataHeaderDeleteRecordOffset;

	dbMemcpyFreeIndian(&deleteNode, columnsValue[0], sizeof(unsigned long), 2);

	//��ųʸ��ش��� ���� ��忡 ���� ��ũ ����
	datHeader.dataHeaderDeleteRecordOffset = recordSlotValue;
	dbMemcpyFreeIndian(&(datHeader.dataHeaderDeleteRecordOffset), &(pdatHeader->dataHeaderDeleteRecordOffset), sizeof(unsigned long), 1);
	#else
		dbDeleteNodeSortingInDataPage(gDbMmuDataPage->mmuMemoryAddress, recordSlotValue, recordSize);
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
	#endif
}
}

#if 0
void dbIndexDeleteAllComplex(unsigned long	startColumnOffset,
								 unsigned long	startComplexIndexOffset,
								 void*			columnsValue[],
								 unsigned long	recordRowid)
{
	pDictionaryHeader	pdicHeader;
	pColumnNode				pColumn;
	pComplexIndexNode				pcomplexIndex;
	ComplexIndexNode				complexIndexNode;
	unsigned long		currentColumn, 
//						columnNumber,
						columnNumberInAllColumn;
	unsigned long		dataTypes[DB_MAX_COMPLEX_INDEX_COLUMN_NUMBER];
	void*			idxContents[DB_MAX_COMPLEX_INDEX_COLUMN_NUMBER];
	DB_VARCHAR		*columnName;

	pdicHeader = dbGetDictionaryHeader();

	while(startComplexIndexOffset)
	{
		//�ش��°�� �����ε����� ã�´�.
		pcomplexIndex = (pComplexIndexNode)(((char*)pdicHeader) + startComplexIndexOffset);

		//�ش� �����ε����� ������ �����ؿ´�.
		dbMemcpyFreeIndian(pcomplexIndex, &complexIndexNode, sizeof(unsigned long), 3);

		columnNumberInAllColumn = 0;

		columnName = (DB_VARCHAR*)(pcomplexIndex + 1);

		for(currentColumn = 0 ; currentColumn < complexIndexNode.complexIndexNodeColumnNumber ; currentColumn++)
		{
			//columnName�� ������ �÷��� ���° �ִ��� �˾Ƴ���.
			columnNumberInAllColumn = dbFindColumnName(startColumnOffset, &pColumn, columnName, DB_AREA_VS_AREA);
			
			//�ش� �÷��� Ű���� �����Ѵ�.
			idxContents[currentColumn] = columnsValue[columnNumberInAllColumn];
			
			//�ش� �÷�Ű���� ������ Ÿ���� �����Ѵ�.
			dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &dataTypes[currentColumn], sizeof(unsigned long), 1);
			
			columnName = columnName + (dbStringGetSize(columnName) + DB_VARCHAR_SIZE)/2;
		}

		//�ش� �����ε����� �����Ѵ�.
		{
			unsigned long newIndexOffset;

			newIndexOffset = dbIndexDeleteUnitComplex(complexIndexNode.complexIndexNodeIndexRootOffset, dataTypes, idxContents, recordRowid, complexIndexNode.complexIndexNodeColumnNumber);

			if(complexIndexNode.complexIndexNodeIndexRootOffset != newIndexOffset)
			{
				complexIndexNode.complexIndexNodeIndexRootOffset = newIndexOffset;

				dbMemcpyFreeIndian(&(complexIndexNode.complexIndexNodeIndexRootOffset), &(pcomplexIndex->complexIndexNodeIndexRootOffset), sizeof(unsigned long), 1);
				
				gDbDictionaryChanged = DB_CHANGED;
			}
		}
		//���� �����ε��� ������ ���Ͽ� ���� �����ε����� offset��  startComplexIndexOffset�� �����Ѵ�.
		startComplexIndexOffset = complexIndexNode.complexIndexNodeNext;
	}

}
#endif

void dbIndexDeleteAllComplexMS(unsigned long		dbStartColumnOffset,
								 	unsigned long		dbStartComplexIndexOffset,
								 	void*			dbColumnsValues[],
								 	unsigned long		dbRecordRowid)
{
	pDictionaryHeader		pdictionaryHeader;
	pColumnNode			pColumn;
	pComplexIndexNode	pcomplexIndex;
	ComplexIndexNode	vComplexIndexNode;
	unsigned long			vCurrentColumn, 
						vColumnNumberInAllColumn;
	unsigned long			vDataTypes[DB_MAX_COMPLEX_INDEX_COLUMN_NUMBER];
	void*				vIdxContents[DB_MAX_COMPLEX_INDEX_COLUMN_NUMBER];
	DB_VARCHAR			*pColumnName;

	pdictionaryHeader = dbGetDictionaryHeader();

	while(dbStartComplexIndexOffset)
	{
		//�ش��°�� �����ε����� ã�´�.
		pcomplexIndex = (pComplexIndexNode)(((char*)pdictionaryHeader) + dbStartComplexIndexOffset);

		//�ش� �����ε����� ������ �����ؿ´�.
		dbMemcpyFreeIndian(pcomplexIndex, &vComplexIndexNode, sizeof(unsigned long), 3);

		vColumnNumberInAllColumn = 0;

		pColumnName = (DB_VARCHAR*)(pcomplexIndex + 1);

		for(vCurrentColumn = 0 ; vCurrentColumn < vComplexIndexNode.complexIndexNodeColumnNumber ; vCurrentColumn++)
		{
			//columnName�� ������ �÷��� ���° �ִ��� �˾Ƴ���.
			vColumnNumberInAllColumn = dbFindColumnName(dbStartColumnOffset, &pColumn, pColumnName, DB_AREA_VS_AREA);
			
			//�ش� �÷��� Ű���� �����Ѵ�.
			vIdxContents[vCurrentColumn] = dbColumnsValues[vColumnNumberInAllColumn];
			
			//�ش� �÷�Ű���� ������ Ÿ���� �����Ѵ�.
			dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &vDataTypes[vCurrentColumn], sizeof(unsigned long), 1);
			
			pColumnName = pColumnName + (dbStringGetSize(pColumnName) + DB_VARCHAR_SIZE)/2;
		}

		//�ش� �����ε����� �����Ѵ�.
		{
			unsigned long vNewIndexOffset;

			vNewIndexOffset = dbIndexDeleteUnitComplexMS(vComplexIndexNode.complexIndexNodeIndexRootOffset, 
														 vDataTypes, 
														 vIdxContents, 
														 dbRecordRowid, 
														  vComplexIndexNode.complexIndexNodeColumnNumber);

			if(vComplexIndexNode.complexIndexNodeIndexRootOffset != vNewIndexOffset)
			{
				vComplexIndexNode.complexIndexNodeIndexRootOffset = vNewIndexOffset;

				dbMemcpyFreeIndian(&(vComplexIndexNode.complexIndexNodeIndexRootOffset), &(pcomplexIndex->complexIndexNodeIndexRootOffset), sizeof(unsigned long), 1);
				
				gDbDictionaryChanged = DB_CHANGED;
			}
		}
		//���� �����ε��� ������ ���Ͽ� ���� �����ε����� offset��  startComplexIndexOffset�� �����Ѵ�.
		dbStartComplexIndexOffset = vComplexIndexNode.complexIndexNodeNext;
	}

}

int dbHaveDataPageNumberInDataPageChain(unsigned long vDataPageStartNumber, unsigned long vCheckDataPageNumber)
{
	unsigned long			vTouchPageRowid;
	pDataFilePageHeader	pdataFilePageHeader;
	
	
	while(1)
	{
		if (vDataPageStartNumber == vCheckDataPageNumber )
		{
			return DB_FUNCTION_SUCCESS;
		}
		else
		{
			vTouchPageRowid = vDataPageStartNumber*gDbPageSizeDataFilePageSize  - (gDbPageSizeDataFilePageSize/2);
			dbTouch(vTouchPageRowid, DB_DATA_FILE_LOAD);

			pdataFilePageHeader = (pDataFilePageHeader)(gDbMmuDataPage->mmuMemoryAddress);
			dbMemcpyFreeIndian(&(pdataFilePageHeader->dataHeaderNextSameTablePage), &vDataPageStartNumber, sizeof(unsigned long), 1);

			if (vDataPageStartNumber == 0)
			{
				return DB_FUNCTION_FAIL;
			}
		}
	}
}

int dbApiRecordDeleteMS(DB_VARCHAR	*dbTableName,
					   		  unsigned long	dbRecordRowid)
{
	pDictionaryHeader		pdictionaryHeader;	//Dictionary file�� �ش��� �������� ����
	pTableNode			ptable;				//Dictionary file�� �ö���� ���������� ���̺��� �������� ����
	TableNode			vTableNode;
	unsigned long			*ptableOffset;
	unsigned long			vTableOffset;
	void*				vColumnsValues[DB_TABLE_MAX_COLUMN_NUMBER];
	//unsigned long			Time1, Time2;

	//Time1 = OSTimeGet();
	
	#if DB_SHOW_ERROR_CODE
	gDbErrorCode = DB_ERROR_NO_ERROR;
	#endif

	if (gDbStartUpDB != DB_DO_RUNNING)
	{
		#if DB_SHOW_ERROR_CODE
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;
		#endif
		return DB_FUNCTION_FAIL;
	}

	//Data File�� Page�׿� �ش� Rowid�� �����ϴ����� üũ�Ѵ�.
	if (dbDataCheckRecordValidRowID(dbRecordRowid)!=DB_VALID_ROWID)
	{
		#if DB_SHOW_ERROR_CODE					//Error check
		gDbErrorCode = DB_ERROR_INVALID_ROWID;	//Error check
		#endif										//Error check
		
		return DB_FUNCTION_FAIL;
	}

	pdictionaryHeader	= dbGetDictionaryHeader();									//Dictionary�� �����Ѵ�.
	ptableOffset		= &(pdictionaryHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����
	
	//���̺��� ã�´�.
	if (!dbTableFindTableName(&ptableOffset, dbTableName))
	{
		//ERROR : �Է����� ���� tableName�� table�� �ý��ۿ� ���������ʴ� ���
		return DB_FUNCTION_FAIL;
	}
	// ===========�Ʒ����� �ش� ���̺��� ã������ ó�� ===========//

	//table�� offset�� ����Ǿ� �ִ�.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);


	//table�� �޸𸮸� �����Ѵ�.
	ptable = (pTableNode)(((char*)pdictionaryHeader) + vTableOffset);


	//���̺��� ������ TABLE_NODE�� �����Ѵ�.
	dbMemcpyFreeIndian(ptable, &vTableNode, sizeof(unsigned long), 6);


	//������ �Ϸ��� Rowid�� �ش� ���̺� �Ҽ��� DataPage�� �ִ����� �˻��Ѵ�.
	{
		unsigned long vDeleteRecordPageNumber;
		vDeleteRecordPageNumber = (dbRecordRowid/gDbPageSizeDataFilePageSize) + 1;
		if (!dbHaveDataPageNumberInDataPageChain(vTableNode.tableNodeStartPage, vDeleteRecordPageNumber))
		{
			//ERROR : �Է����� ���� tableName�� table�� �ý��ۿ� ���������ʴ� ���
			return DB_FUNCTION_FAIL;
		}
	}


	//�ش� �÷������� �迭�� �޾ƿ´�.
	dbDataGetRecord(vTableNode.tableNodeColumnOffset, dbRecordRowid, vColumnsValues);


	//��ü �÷��� ������ �޾ƿ´�.
//	vTotalColumnNumber = dbColumnTotalCount(vTableNode.tableNodeColumnOffset);	<--�̰� Ȯ���غ�����.. �ʿ� ���°� ������..


	//��Ŭ�ε��� ���� ��ƾ�� �����Ѵ�
	dbIndexDeleteAllSingleMS(vTableNode.tableNodeColumnOffset, 
							 vColumnsValues,
							 dbRecordRowid);


	//�����ε��� ���� ��ƾ�� �����Ѵ�
	dbIndexDeleteAllComplexMS(vTableNode.tableNodeColumnOffset, 
							  vTableNode.tableNodeComplexIndexOffset, 
							  vColumnsValues, 
							  dbRecordRowid);


	//���ڵ��� ���� ��ƾ�� �����Ѵ�.
	dbDataSetRecordDelete(vTableNode.tableNodeColumnOffset, 
						  vColumnsValues,
						  dbRecordRowid);
	//Time2 = OSTimeGet();

	//DbgPrintf(("DB Delete�� �ɸ��� �ð�[%d] \n", Time2 - Time1));
	return DB_FUNCTION_SUCCESS;
}

/*
void dbApiRecordDelete(DB_VARCHAR		*tableName,
					   unsigned long	recordRowid)
{
	pDictionaryHeader		pdicHeader;	//Dictionary file�� �ش��� �������� ����
	pTableNode					ptable;			//Dictionary file�� �ö���� ���������� ���̺��� �������� ����
	TableNode					tableNode;
//	pColumnNode					pColumn;
//	unsigned long			columnType;
	unsigned long			*ptableOffset, vTableOffset;
	unsigned long			totalColumnNumber;
	void*					columnsValue[DB_TABLE_MAX_COLUMN_NUMBER];

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary�� �����Ѵ�.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����
	
	//���̺��� ã�´�.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : �Է����� ���� tableName�� table�� �ý��ۿ� ���������ʴ� ���
		return;
	}
	// ===========�Ʒ����� �ش� ���̺��� ã������ ó�� ===========//

	//table�� offset�� ����Ǿ� �ִ�.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table�� �޸𸮸� �����Ѵ�.
	ptable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//���̺��� ������ TABLE_NODE�� �����Ѵ�.
	dbMemcpyFreeIndian(ptable, &tableNode, sizeof(unsigned long), 6);

	//�ش� �÷������� �迭�� �޾ƿ´�.
	dbDataGetRecord(tableNode.tableNodeColumnOffset, recordRowid, columnsValue);

	//��ü �÷��� ������ �޾ƿ´�.
	totalColumnNumber = dbColumnTotalCount(tableNode.tableNodeColumnOffset);

	//��Ŭ�ε��� ���� ��ƾ�� �����Ѵ�
	dbIndexDeleteAllSingle(tableNode.tableNodeColumnOffset, columnsValue, recordRowid);

	//�����ε��� ���� ��ƾ�� �����Ѵ�
	dbIndexDeleteAllComplex(tableNode.tableNodeColumnOffset, tableNode.tableNodeComplexIndexOffset, columnsValue, recordRowid);

	//���ڵ��� ���� ��ƾ�� �����Ѵ�.
	dbDataSetRecordDelete(tableNode.tableNodeColumnOffset, columnsValue, recordRowid);
}
*/

signed char dbIndexCompareKey_AREAVs_AREA(void* areaKeyValue1, 
											  void* areaKeyValue2, 
											  unsigned long keyDataType)
{
	signed char compareResult;

	if (keyDataType == DB_VARCHAR_TYPE)
	{
		compareResult = dbStringCompare_AREAVs_AREA((DB_VARCHAR*)areaKeyValue1, (DB_VARCHAR*)areaKeyValue2);
	}
	else if (keyDataType == DB_UINT8_TYPE)
	{
		DB_UINT8	vAreaKeyValue1, vAreaKeyValue2;
		
		dbMemcpyFreeIndian(areaKeyValue1, &vAreaKeyValue1, DB_UINT8_SIZE, 1);
		dbMemcpyFreeIndian(areaKeyValue2, &vAreaKeyValue2, DB_UINT8_SIZE, 1);
		
		if (vAreaKeyValue1 > vAreaKeyValue2)
		{
			compareResult = -1;
		}
		else if  (vAreaKeyValue1 < vAreaKeyValue2)
		{
			compareResult = 1;
		}
		else
		{
			compareResult = 0;
		}
	}
	else if (keyDataType == DB_UINT16_TYPE)
	{
		DB_UINT16	vAreaKeyValue1, vAreaKeyValue2;

		dbMemcpyFreeIndian(areaKeyValue1, &vAreaKeyValue1, DB_UINT16_SIZE, 1);
		dbMemcpyFreeIndian(areaKeyValue2, &vAreaKeyValue2, DB_UINT16_SIZE, 1);
		
		if (vAreaKeyValue1 > vAreaKeyValue2)
		{
			compareResult = -1;
		}
		else if (vAreaKeyValue1 < vAreaKeyValue2)
		{
			compareResult = 1;
		}
		else
		{
			compareResult = 0;
		}		
	}
	else if (keyDataType == DB_UINT32_TYPE)
	{
		DB_UINT32	vAreaKeyValue1, vAreaKeyValue2;

		dbMemcpyFreeIndian(areaKeyValue1, &vAreaKeyValue1, DB_UINT32_SIZE, 1);
		dbMemcpyFreeIndian(areaKeyValue2, &vAreaKeyValue2, DB_UINT32_SIZE, 1);
				
		if (vAreaKeyValue1 > vAreaKeyValue2)
		{
			compareResult = -1;
		}
		else if (vAreaKeyValue1 < vAreaKeyValue2)
		{
			compareResult = 1;
		}
		else
		{
			compareResult = 0;
		}		
	}
	else{}

	return compareResult;
}


#if 0
unsigned long dbIndexLinkNodeDelete(unsigned long linkOffset, unsigned long recordRowid)
{

	if (linkOffset == 0 )
	{
		return 0;
	}
	else
	{
		pLinkNode	pLink;
		LinkNode	linkNode;
		
		pLink = dbIndexGetLink(linkOffset);
		dbMemcpyFreeIndian(pLink, &linkNode, sizeof(unsigned long), 2);
		
		//DbgPrintf(("linkNode.linkNodeRecordRowid = [%x]\n", linkNode.linkNodeRecordRowid));
		//DbgPrintf(("linkNode.linkNodeNext = [%x]\n", linkNode.linkNodeNext));
		
		if (linkNode.linkNodeRecordRowid == recordRowid)
		{
			//DbgPrintf(("��ũ ������ƾ���� ����\n"));
			//���� ��ũ�� ó���ϴ� ����
			{
				pIndexFilePageHeader	pidxHeader;
				unsigned long			relativeOffset;
				unsigned long			indexHeaderDeleteLinkOffset;
				
				relativeOffset = linkOffset%gDbPageSizeIndexFilePageSize;
				pidxHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);
				
				dbMemcpyFreeIndian(&(pidxHeader->indexHeaderDeleteLinkOffset), &indexHeaderDeleteLinkOffset, sizeof(unsigned long), 1);
				//DbgPrintf(("indexHeaderDeleteLinkOffset = [%x]\n", indexHeaderDeleteLinkOffset));
				//�ε��� �ش��� ����link�� ����Ѵ�.
				dbMemcpyFreeIndian(&relativeOffset, &(pidxHeader->indexHeaderDeleteLinkOffset), sizeof(unsigned long), 1);
				
				//link�� linked list�� ������Ų��.
				dbMemcpyFreeIndian(&(indexHeaderDeleteLinkOffset), &(pLink->linkNodeNext), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
			
			return linkNode.linkNodeNext;
		}
		else
		{
			unsigned long newNext;

			//DbgPrintf((" �̰� ������ ����\n"));
			newNext = dbIndexLinkNodeDelete(linkNode.linkNodeNext, recordRowid);
			
			if (newNext != linkNode.linkNodeNext)
			{
				linkNode.linkNodeNext = newNext;
				pLink = dbIndexGetLink(linkOffset);
				dbMemcpyFreeIndian(&(linkNode.linkNodeNext), &(pLink->linkNodeNext), sizeof(unsigned long), 1);
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
			return linkOffset;
		}
	}
}
#else
void DbDeleteLink(unsigned long vDeleteLinkOffset)
{
	pIndexFilePageHeader	pIndexPageHeader;
	pLinkNode			pLink;
	LinkNode				vLinkNode;
	unsigned long			vHeaderTopDeleteNodeOffset;
	unsigned long			vRelativeOffet;



	pLink = dbIndexGetLink(vDeleteLinkOffset);
	dbMemcpyFreeIndian(pLink, &vLinkNode, sizeof(unsigned long), 2);
	pIndexPageHeader = (pIndexFilePageHeader)gDbMmuIndexPage->mmuMemoryAddress;

	dbMemcpyFreeIndian(&(pIndexPageHeader->indexHeaderDeleteLinkOffset), &vHeaderTopDeleteNodeOffset, sizeof(unsigned long), 1);
	vLinkNode.linkNodeNext = vHeaderTopDeleteNodeOffset;

	dbMemcpyFreeIndian(&vLinkNode, pLink, sizeof(unsigned long), 2);
	

	vRelativeOffet = vDeleteLinkOffset%gDbPageSizeIndexFilePageSize;
	dbMemcpyFreeIndian(&vRelativeOffet, &(pIndexPageHeader->indexHeaderDeleteLinkOffset), sizeof(unsigned long), 1);
	
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;
}

unsigned long dbIndexLinkNodeDelete(unsigned long linkOffset, unsigned long recordRowid)
{
	if (linkOffset == 0 )
	{
		return 0;
	}
	else
	{
		pLinkNode	pLink;
		LinkNode		linkNode;
		
		pLink = dbIndexGetLink(linkOffset);
		dbMemcpyFreeIndian(pLink, &linkNode, sizeof(unsigned long), 2);
		
		
		if (linkNode.linkNodeRecordRowid == recordRowid)
		{
			DbDeleteLink(linkOffset);
			
			return linkNode.linkNodeNext;
		}
		else
		{
			unsigned long vNextOffset, 	vPreOffset;
			LinkNode		vNextLinkNode, 	vPreLinkNode;
			pLinkNode	pNextLinkNode,	pPreLinkNode;

			vPreOffset 						= linkOffset;
			vPreLinkNode.linkNodeNext			= linkNode.linkNodeNext;
			vPreLinkNode.linkNodeRecordRowid	= linkNode.linkNodeRecordRowid;

			while(1)
			{
				vNextOffset = vPreLinkNode.linkNodeNext;

				if (vNextOffset == 0) 
				{
					break;
				}
				else
				{
					pNextLinkNode = dbIndexGetLink(vNextOffset);
					
					dbMemcpyFreeIndian(pNextLinkNode, &vNextLinkNode, sizeof(unsigned long), 2);

					if (vNextLinkNode.linkNodeRecordRowid == recordRowid)
					{
						//�����ϰ�
						vPreLinkNode.linkNodeNext	= vNextLinkNode.linkNodeNext;
						pPreLinkNode = dbIndexGetLink(vPreOffset);
						dbMemcpyFreeIndian(&vPreLinkNode, pPreLinkNode, sizeof(unsigned long), 2);
						gDbMmuIndexPage->mmuChanged = DB_CHANGED;
						
						//�����ϱ�
						DbDeleteLink(vNextOffset);
						
						break;
					}
					else
					{
						vPreOffset						= vNextOffset;
						vPreLinkNode.linkNodeNext 			= vNextLinkNode.linkNodeNext;
						vPreLinkNode.linkNodeRecordRowid 	= vNextLinkNode.linkNodeRecordRowid;
					}
					
				}
			}
			
			return linkOffset;
		}
	}
}
#endif

unsigned long dbIndexDeleteNodeMakeTreeUpMinNode(unsigned long	rootNode, 
														 unsigned long	*minNodeOffset)
{
	//return�� �Ǵ� long ���� tree�� root���� �ȴ�.

	pIndexNode			pNode;
	IndexNode			indexNode;
//	unsigned	long		tempOffset,
//							preNodeOffset,
//							bak;

	if (rootNode == 0) return 0;

	//�˻���带 �ҷ��´�.
	pNode = dbIndexGetNode(rootNode);
	dbMemcpyFreeIndian(pNode, &indexNode, sizeof(unsigned long), 4);

	if (indexNode.indexNodeLeft == 0)
	{
		*minNodeOffset = rootNode;

		return indexNode.indexNodeRight;
	}
	else
	{
		unsigned long	newLeftSub;

		newLeftSub = dbIndexDeleteNodeMakeTreeUpMinNode(indexNode.indexNodeLeft, minNodeOffset);

		if (indexNode.indexNodeLeft != newLeftSub)
		{
			indexNode.indexNodeLeft = newLeftSub;

			pNode = dbIndexGetNode(rootNode);

			dbMemcpyFreeIndian(&(indexNode.indexNodeLeft), &(pNode->indexNodeLeft), sizeof(unsigned long), 1);

			gDbMmuIndexPage->mmuChanged = DB_CHANGED;
		}

		//balance factor�� üũ�ؼ� rotation�� �Ҽ��� �ִ�.
		{
			signed char	balanceFactor;

			IndexNode		subInode;

			balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);
			
			if (balanceFactor == 2)
			{
				pNode	= dbIndexGetNode(indexNode.indexNodeLeft);
				dbMemcpyFreeIndian(pNode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == -1)
				{
					//LRRotation
					dbIndexRotationLR(&rootNode, &(indexNode.indexNodeLeft), &(subInode.indexNodeRight));
				}
				else
				{
					//LL rotation
					dbIndexRotationLL(&rootNode, &(indexNode.indexNodeLeft), &(subInode.indexNodeLeft));
				}
			}
			else if (balanceFactor==-2)
			{
				pNode	= dbIndexGetNode(indexNode.indexNodeRight);
				dbMemcpyFreeIndian(pNode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == 1)
				{
					//RLRotation
					dbIndexRotationRL(&rootNode, &(indexNode.indexNodeRight), &(subInode.indexNodeLeft));
				}
				else
				{
					//RR rotation
					dbIndexRotationRR(&rootNode, &(indexNode.indexNodeRight), &(subInode.indexNodeRight));
				}
			}
			else{}
		}
		//�ش� rootNode�� ���̸� �ٽ� ����Ѵ�.
		{
			unsigned long inodeHeight;
			
			pNode	= dbIndexGetNode(rootNode);
			
			dbMemcpyFreeIndian(pNode, &indexNode, sizeof(unsigned long), 4);

			inodeHeight = dbIndexGetCalculateHeight(indexNode.indexNodeLeft, indexNode.indexNodeRight);
			
			if (indexNode.indexNodeHeight != inodeHeight)
			{
				pNode	= dbIndexGetNode(rootNode);

				dbMemcpyFreeIndian(&inodeHeight, &(pNode->indexNodeHeight), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}

		//�ش� Ʈ���� root�� �����Ѵ�.
		return rootNode;
	}
}

unsigned long dbIndexDeleteNodeMakeTreeUpMaxNode(unsigned long	rootNode, 
												 unsigned long	*maxNodeOffset)
{
	//return�� �Ǵ� long ���� tree�� root���� �ȴ�.

	pIndexNode					pNode;
	IndexNode					indexNode;
//	unsigned	long		tempOffset,
//							preNodeOffset;
//							bak;

	if (rootNode == 0) return 0;

	//�˻���带 �ҷ��´�.
	pNode = dbIndexGetNode(rootNode);
	dbMemcpyFreeIndian(pNode, &indexNode, sizeof(unsigned long), 4);

	if (indexNode.indexNodeRight == 0)
	{
		*maxNodeOffset = rootNode;

		return indexNode.indexNodeLeft;
	}
	else
	{
		unsigned long	newRightSub;

		newRightSub = dbIndexDeleteNodeMakeTreeUpMaxNode(indexNode.indexNodeRight, maxNodeOffset);

		if (indexNode.indexNodeRight != newRightSub)
		{
			indexNode.indexNodeRight = newRightSub;

			pNode = dbIndexGetNode(rootNode);

			dbMemcpyFreeIndian(&(indexNode.indexNodeRight), &(pNode->indexNodeRight), sizeof(unsigned long), 1);

			gDbMmuIndexPage->mmuChanged = DB_CHANGED;
		}

		//balance factor�� üũ�ؼ� rotation�� �Ҽ��� �ִ�.
		{
			signed char	balanceFactor;

			IndexNode		subInode;

			balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);
			
			if (balanceFactor == 2)
			{
				pNode	= dbIndexGetNode(indexNode.indexNodeLeft);
				dbMemcpyFreeIndian(pNode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == -1)
				{
					//LRRotation
					dbIndexRotationLR(&rootNode, &(indexNode.indexNodeLeft), &(subInode.indexNodeRight));
				}
				else
				{
					//LL rotation
					dbIndexRotationLL(&rootNode, &(indexNode.indexNodeLeft), &(subInode.indexNodeLeft));
				}
			}
			else if (balanceFactor==-2)
			{
				pNode	= dbIndexGetNode(indexNode.indexNodeRight);
				dbMemcpyFreeIndian(pNode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == 1)
				{
					//RLRotation
					dbIndexRotationRL(&rootNode, &(indexNode.indexNodeRight), &(subInode.indexNodeLeft));
				}
				else
				{
					//RR rotation
					dbIndexRotationRR(&rootNode, &(indexNode.indexNodeRight), &(subInode.indexNodeRight));
				}
			}
			else{}
		}
		//�ش� rootNode�� ���̸� �ٽ� ����Ѵ�.
		{
			unsigned long inodeHeight;
			
			pNode	= dbIndexGetNode(rootNode);
			
			dbMemcpyFreeIndian(pNode, &indexNode, sizeof(unsigned long), 4);

			inodeHeight = dbIndexGetCalculateHeight(indexNode.indexNodeLeft, indexNode.indexNodeRight);
			
			if (indexNode.indexNodeHeight != inodeHeight)
			{
				pNode	= dbIndexGetNode(rootNode);

				dbMemcpyFreeIndian(&inodeHeight, &(pNode->indexNodeHeight), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}

		//�ش� Ʈ���� root�� �����Ѵ�.
		return rootNode;
	}
}


unsigned long dbIndexDeleteUnitSingleMS(unsigned long	dbRootIndexOffset, 
										unsigned long	dbDataType, 
										void*			dbKeyValue,
										unsigned long	dbRecordRowid)
{
	signed char		vCompareResult;
	pIndexNode		pindexNode;
	IndexNode		vIndexNode;

	if (dbRootIndexOffset == 0 )
	{
		return 0;
	}

	pindexNode = dbIndexGetNode(dbRootIndexOffset);
	dbMemcpyFreeIndian(pindexNode, &vIndexNode, sizeof(unsigned long), 4);

//showTwoByteString("pindexNode + 1",pindexNode + 1);
//showTwoByteString("dbKeyValue",dbKeyValue);

	vCompareResult = dbIndexCompareKeyAreaVsAreaMS(pindexNode + 1, dbKeyValue, dbDataType);	//<--------------------��� ��� �߿�!!!!!!!!!!!

		#if ( AVLDB_DEBUG == 1 )
//DbgPrintf(("\n=============dbIndexCompareKeyAreaVsAreaMS���� ���;� �Ѵ�. [%d]\n", vCompareResult));	
		#endif

	//=======================Case 1.������ �Ϸ��µ�... �ش� ����� ��� ó�� (begin)======================//
	if (vCompareResult == 0)
	{
	
		unsigned long	vNewLinkOffset;
//DbgPrintf(("\n�ش� ��� ������ƾ ����\n"));	
//showDatMemory(512);
		vNewLinkOffset = dbIndexLinkNodeDelete(vIndexNode.indexNodeLinkOffset, dbRecordRowid);
//DbgPrintf(("\ndbIndexCompareKeyAreaVsAreaMS ���� ��ũ ������ vNewLinkOffset�� �� [%x]\n", vNewLinkOffset));
//showDatMemory(512);
		if ((vIndexNode.indexNodeLinkOffset != vNewLinkOffset) && (vNewLinkOffset != 0))
		{
			vIndexNode.indexNodeLinkOffset = vNewLinkOffset;

			pindexNode = dbIndexGetNode(dbRootIndexOffset);

			dbMemcpyFreeIndian(&(vIndexNode.indexNodeLinkOffset), &(pindexNode->indexNodeLinkOffset), sizeof(unsigned long), 1);
			
			gDbMmuIndexPage->mmuChanged = DB_CHANGED;
		}
		else if (vNewLinkOffset == 0)
		{
			unsigned long	pTempRoot = 0;
//DbgPrintf(("\n�ش� ����� ���� ��ƾ���� ������.\n"));
			if ((vIndexNode.indexNodeLeft==0)&&(vIndexNode.indexNodeRight==0))
			{
//DbgPrintf(("\n����� ������ �����ε�....\n"));

				//�ش� index��带 ���� ���� ����Ѵ�.
				dbIndexDeleteIndexNode(dbRootIndexOffset, dbDataType);
				
				dbRootIndexOffset = 0;

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
				
				return dbRootIndexOffset;
			}
			else if ((vIndexNode.indexNodeLeft==0)&&(vIndexNode.indexNodeRight!=0))
			{
				//�ش� index��带 ���� ���� ����Ѵ�.
				dbIndexDeleteIndexNode(dbRootIndexOffset, dbDataType);

				dbRootIndexOffset = vIndexNode.indexNodeRight;
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;

				return dbRootIndexOffset;
			}
			else if((vIndexNode.indexNodeLeft!=0)&&(vIndexNode.indexNodeRight==0))
			{
				//�ش� index��带 ���� ���� ����Ѵ�.
				dbIndexDeleteIndexNode(dbRootIndexOffset, dbDataType);

				dbRootIndexOffset = vIndexNode.indexNodeLeft;

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
				
				return dbRootIndexOffset;
			}
			else if(vIndexNode.indexNodeHeight==2)
			{
				//�ش� index��带 ���� ���� ����Ѵ�.
				dbIndexDeleteIndexNode(dbRootIndexOffset, dbDataType);
				
				dbRootIndexOffset = vIndexNode.indexNodeLeft;

				pindexNode = dbIndexGetNode(dbRootIndexOffset);
				
				//���̸� �����Ѵ�.
				dbMemcpyFreeIndian(&(vIndexNode.indexNodeHeight), &(pindexNode->indexNodeHeight), sizeof(unsigned long), 1);

				//������ ���� Ʈ���� offset�� �����Ѵ�.
				dbMemcpyFreeIndian(&(vIndexNode.indexNodeRight), &(pindexNode->indexNodeRight), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;

				return dbRootIndexOffset;
			}
			else
			//subNode������ Max��带 �ø����� �ƴϸ�, subNode�� minNode�� �ø����� �۾��� �־�� �Ѵ�.
			{
				signed long		balanceFactor;
				
				IndexNode		tempRoot;
				
				balanceFactor = dbIndexGetBalanceFactor(vIndexNode.indexNodeLeft, vIndexNode.indexNodeRight);

				if (balanceFactor > 0)
				{
					//������ �� ū ���
					unsigned newLeftSubRoot;
					
					newLeftSubRoot = dbIndexDeleteNodeMakeTreeUpMaxNode(vIndexNode.indexNodeLeft, &pTempRoot); 
					
					if ((newLeftSubRoot == 0) && (pTempRoot == 0))
					{
						pTempRoot = 0;
					}
					else
					{
						pindexNode = dbIndexGetNode(pTempRoot);
						
						tempRoot.indexNodeLeft	 = newLeftSubRoot;
						tempRoot.indexNodeRight	 = vIndexNode.indexNodeRight;
						dbMemcpyFreeIndian(&(pindexNode->indexNodeLinkOffset), &(tempRoot.indexNodeLinkOffset), sizeof(unsigned long), 1);
						tempRoot.indexNodeHeight = dbIndexGetCalculateHeight(tempRoot.indexNodeLeft, tempRoot.indexNodeRight);
						
						pindexNode = dbIndexGetNode(pTempRoot);
						dbMemcpyFreeIndian(&tempRoot, pindexNode, sizeof(unsigned long), 4);
						gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					}
				}
				else
				{
					//�������� �� ũ�ų� ���� ���
					unsigned long newRightSubRoot;
					
					newRightSubRoot = dbIndexDeleteNodeMakeTreeUpMinNode(vIndexNode.indexNodeRight, &pTempRoot); 
					
					if ((newRightSubRoot == 0) && (pTempRoot == 0))
					{
						pTempRoot = 0;
					}
					else
					{
						pindexNode = dbIndexGetNode(pTempRoot);
						
						tempRoot.indexNodeLeft	 = vIndexNode.indexNodeLeft;
						tempRoot.indexNodeRight	 = newRightSubRoot;
						dbMemcpyFreeIndian(&(pindexNode->indexNodeLinkOffset), &(tempRoot.indexNodeLinkOffset), sizeof(unsigned long), 1);
						tempRoot.indexNodeHeight = dbIndexGetCalculateHeight(tempRoot.indexNodeLeft, tempRoot.indexNodeRight);
						
						pindexNode = dbIndexGetNode(pTempRoot);
						dbMemcpyFreeIndian(&tempRoot, pindexNode, sizeof(unsigned long), 4);
						gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					}
				}
			}
			//�ش� index��带 ���� ���� ����Ѵ�.
			dbIndexDeleteIndexNode(dbRootIndexOffset, dbDataType);

			dbRootIndexOffset = pTempRoot;
		}
		else{}
		
		return dbRootIndexOffset;
	}
	//=======================Case 1.������ �Ϸ��µ�... �ش� ����� ��� ó�� ( end )======================//

	else
	{
		unsigned long l2;

		if (vCompareResult==-1)
		{
			l2 = dbIndexDeleteUnitSingleMS(vIndexNode.indexNodeLeft, dbDataType, dbKeyValue, dbRecordRowid);

			if (vIndexNode.indexNodeLeft != l2)
			{
				vIndexNode.indexNodeLeft = l2;

				pindexNode = dbIndexGetNode(dbRootIndexOffset);
				
				dbMemcpyFreeIndian(&vIndexNode.indexNodeLeft, &(pindexNode->indexNodeLeft), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		else if (vCompareResult==1)
		{
			l2 = dbIndexDeleteUnitSingleMS(vIndexNode.indexNodeRight, dbDataType, dbKeyValue, dbRecordRowid);

			if (vIndexNode.indexNodeRight != l2)
			{
				vIndexNode.indexNodeRight = l2;

				pindexNode = dbIndexGetNode(dbRootIndexOffset);
				
				dbMemcpyFreeIndian(&vIndexNode.indexNodeRight, &(pindexNode->indexNodeRight), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		else{}

		//rotation check�� �Ѵ�.
		{
			signed char		balanceFactor;
			IndexNode		subInode;

			balanceFactor = dbIndexGetBalanceFactor(vIndexNode.indexNodeLeft, vIndexNode.indexNodeRight);

			if (balanceFactor == 2)
			{
				pindexNode	= dbIndexGetNode(vIndexNode.indexNodeLeft);

				dbMemcpyFreeIndian(pindexNode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == -1)
				{
					//LRRotation	
					dbIndexRotationLR(&dbRootIndexOffset, &(vIndexNode.indexNodeLeft), &(subInode.indexNodeRight));
				}
				else
				{
					//LL rotation
					dbIndexRotationLL(&dbRootIndexOffset, &(vIndexNode.indexNodeLeft), &(subInode.indexNodeLeft));
				}
			}
			else if (balanceFactor == -2)
			{
				pindexNode	= dbIndexGetNode(vIndexNode.indexNodeRight);

				dbMemcpyFreeIndian(pindexNode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == 1)
				{
					//RLRotation
					dbIndexRotationRL(&dbRootIndexOffset, &(vIndexNode.indexNodeRight), &(subInode.indexNodeLeft));
				}
				else
				{
					//RR rotation
					dbIndexRotationRR(&dbRootIndexOffset, &(vIndexNode.indexNodeRight), &(subInode.indexNodeRight));
				}

			}
			else{}
		}

		//indexNodeRootOffset�� ���̸� �ٽ� ����Ѵ�.
		{
			unsigned long inodeHeight;
			
			pindexNode	= dbIndexGetNode(dbRootIndexOffset);
			
			dbMemcpyFreeIndian(pindexNode, &vIndexNode, sizeof(unsigned long), 4);

			inodeHeight = dbIndexGetCalculateHeight(vIndexNode.indexNodeLeft, vIndexNode.indexNodeRight);
			
			if (vIndexNode.indexNodeHeight != inodeHeight)
			{
				pindexNode	= dbIndexGetNode(dbRootIndexOffset);

				dbMemcpyFreeIndian(&inodeHeight, &(pindexNode->indexNodeHeight), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
	}
	return dbRootIndexOffset;
}

#if 0
unsigned long dbIndexDeleteUnitSingle(unsigned long	rootIndexOffset, 
										  unsigned long	dataType, 
										  void*			keyValue,
										  unsigned long recordRowid)
{
	signed char	compareResult;
	pIndexNode		pIndexNode;
	IndexNode		indexNode;

	if (rootIndexOffset == 0 )
	{
		return 0;
	}

	pIndexNode = dbIndexGetNode(rootIndexOffset);
	dbMemcpyFreeIndian(pIndexNode, &indexNode, sizeof(unsigned long), 4);

	compareResult = dbIndexCompareKey_AREAVs_AREA( pIndexNode + 1, keyValue, dataType);
	
	if (compareResult==0)
	{
		unsigned long	newLinkOffset;
		
		newLinkOffset = dbIndexLinkNodeDelete(indexNode.indexNodeLinkOffset, recordRowid);

		if ((indexNode.indexNodeLinkOffset != newLinkOffset) && (newLinkOffset != 0))
		{
			indexNode.indexNodeLinkOffset = newLinkOffset;

			pIndexNode = dbIndexGetNode(rootIndexOffset);

			dbMemcpyFreeIndian(&(indexNode.indexNodeLinkOffset), &(pIndexNode->indexNodeLinkOffset), sizeof(unsigned long), 1);
			
			gDbMmuIndexPage->mmuChanged = DB_CHANGED;
		}
		else if (newLinkOffset == 0)
		{
			unsigned long	pTempRoot = 0;

			//subNode������ Max��带 �ø����� �ƴϸ�, subNode�� minNode�� �ø����� �۾��� �־�� �Ѵ�.
			{
				signed long		balanceFactor;
				
				IndexNode			tempRoot;
				
				balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);

				if (balanceFactor > 0)
				{
					//������ �� ū ���
					unsigned newLeftSubRoot;
					
					newLeftSubRoot = dbIndexDeleteNodeMakeTreeUpMaxNode(indexNode.indexNodeLeft, &pTempRoot); 
					
					if ((newLeftSubRoot == 0) && (pTempRoot == 0))
					{
						pTempRoot = 0;
					}
					else
					{
						pIndexNode = dbIndexGetNode(pTempRoot);
						
						tempRoot.indexNodeLeft	 = newLeftSubRoot;
						tempRoot.indexNodeRight	 = indexNode.indexNodeRight;
						dbMemcpyFreeIndian(&(pIndexNode->indexNodeLinkOffset), &(tempRoot.indexNodeLinkOffset), sizeof(unsigned long), 1);
						tempRoot.indexNodeHeight = dbIndexGetCalculateHeight(tempRoot.indexNodeLeft, tempRoot.indexNodeRight);
						
						pIndexNode = dbIndexGetNode(pTempRoot);
						dbMemcpyFreeIndian(&tempRoot, pIndexNode, sizeof(unsigned long), 4);
						gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					}
				}
				else
				{
					//�������� �� ũ�ų� ���� ���
					unsigned newRightSubRoot;
					
					newRightSubRoot = dbIndexDeleteNodeMakeTreeUpMinNode(indexNode.indexNodeRight, &pTempRoot); 
					
					if ((newRightSubRoot == 0) && (pTempRoot == 0))
					{
						pTempRoot = 0;
					}
					else
					{
						pIndexNode = dbIndexGetNode(pTempRoot);
						
						tempRoot.indexNodeLeft	 = indexNode.indexNodeLeft;
						tempRoot.indexNodeRight	 = newRightSubRoot;
						dbMemcpyFreeIndian(&(pIndexNode->indexNodeLinkOffset), &(tempRoot.indexNodeLinkOffset), sizeof(unsigned long), 1);
						tempRoot.indexNodeHeight = dbIndexGetCalculateHeight(tempRoot.indexNodeLeft, tempRoot.indexNodeRight);
						
						pIndexNode = dbIndexGetNode(pTempRoot);
						dbMemcpyFreeIndian(&tempRoot, pIndexNode, sizeof(unsigned long), 4);
						gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					}
				}
			}
			//�ش� index��带 ���� ���� ����Ѵ�.
			dbIndexDeleteIndexNode(rootIndexOffset, dataType);

			rootIndexOffset = pTempRoot;
		}
		else{}
		return rootIndexOffset;
	}
	else
	{
		unsigned long l2;

		if (compareResult==-1)
		{
			l2 = dbIndexDeleteUnitSingle(indexNode.indexNodeLeft, dataType, keyValue, recordRowid);

			if (indexNode.indexNodeLeft != l2)
			{
				indexNode.indexNodeLeft = l2;

				pIndexNode = dbIndexGetNode(rootIndexOffset);
				
				dbMemcpyFreeIndian(&indexNode.indexNodeLeft, &(pIndexNode->indexNodeLeft), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		else if (compareResult==1)
		{
			l2 = dbIndexDeleteUnitSingle(indexNode.indexNodeRight, dataType, keyValue, recordRowid);

			if (indexNode.indexNodeRight != l2)
			{
				indexNode.indexNodeRight = l2;

				pIndexNode = dbIndexGetNode(rootIndexOffset);
				
				dbMemcpyFreeIndian(&indexNode.indexNodeRight, &(pIndexNode->indexNodeRight), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		else{}

		//rotation check�� �Ѵ�.
		{
			signed char balanceFactor;
			IndexNode		subInode;

			balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);

			if (balanceFactor == 2)
			{
				pIndexNode	= dbIndexGetNode(indexNode.indexNodeLeft);

				dbMemcpyFreeIndian(pIndexNode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == -1)
				{
					//LRRotation
					dbIndexRotationLR(&rootIndexOffset, &(indexNode.indexNodeLeft), &(subInode.indexNodeRight));
				}
				else
				{
					//LL rotation
					dbIndexRotationLL(&rootIndexOffset, &(indexNode.indexNodeLeft), &(subInode.indexNodeLeft));
				}
			}
			else if (balanceFactor == -2)
			{
				pIndexNode	= dbIndexGetNode(indexNode.indexNodeRight);

				dbMemcpyFreeIndian(pIndexNode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == 1)
				{
					//RLRotation
					dbIndexRotationRL(&rootIndexOffset, &(indexNode.indexNodeRight), &(subInode.indexNodeLeft));
				}
				else
				{
					//RR rotation
					dbIndexRotationRR(&rootIndexOffset, &(indexNode.indexNodeRight), &(subInode.indexNodeRight));
				}

			}
			else{}
		}

		//indexNodeRootOffset�� ���̸� �ٽ� ����Ѵ�.
		{
			unsigned long inodeHeight;
			
			pIndexNode	= dbIndexGetNode(rootIndexOffset);
			
			dbMemcpyFreeIndian(pIndexNode, &indexNode, sizeof(unsigned long), 4);

			inodeHeight = dbIndexGetCalculateHeight(indexNode.indexNodeLeft, indexNode.indexNodeRight);
			
			if (indexNode.indexNodeHeight != inodeHeight)
			{
				pIndexNode	= dbIndexGetNode(rootIndexOffset);

				dbMemcpyFreeIndian(&inodeHeight, &(pIndexNode->indexNodeHeight), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
	}
	return rootIndexOffset;
}
#endif

unsigned long dbIndexDeleteUnitComplex(unsigned long	rootIndexOffset, 
										   unsigned long	dataTypes[], 
										   void*			keyValues[],
										   unsigned long	recordRowid,
										   unsigned long	indexLevel)
{
	signed char	compareResult;
	pIndexNode		pIndexNode;
	IndexNode		indexNode;

	if (rootIndexOffset == 0 )
	{
		//��� �̷����±����� ���� �������̴�. �׳� ���� ó���Ҷ� ���� �κ��ϻ�
		return 0;
	}

	pIndexNode = dbIndexGetNode(rootIndexOffset);
	dbMemcpyFreeIndian(pIndexNode, &indexNode, sizeof(unsigned long), 4);

	compareResult = dbIndexCompareKey_AREAVs_AREA( pIndexNode + 1, keyValues[0], dataTypes[0]);
	
	if (compareResult==0)
	{
		if (indexLevel == 1)
		{
			unsigned long	newLinkOffset;
			
			newLinkOffset = dbIndexLinkNodeDelete(indexNode.indexNodeLinkOffset, recordRowid);

			if (indexNode.indexNodeLinkOffset != newLinkOffset)
			{
				indexNode.indexNodeLinkOffset = newLinkOffset;

				pIndexNode = dbIndexGetNode(rootIndexOffset);

				dbMemcpyFreeIndian(&(indexNode.indexNodeLinkOffset), &(pIndexNode->indexNodeLinkOffset), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
			
		}
		else
		{
			unsigned long	newSubLevelRoot;

			newSubLevelRoot = dbIndexDeleteUnitComplex(indexNode.indexNodeLinkOffset, &dataTypes[1], &keyValues[1], recordRowid, indexLevel - 1);
			
			if (indexNode.indexNodeLinkOffset != newSubLevelRoot)
			{
				indexNode.indexNodeLinkOffset = newSubLevelRoot;

				pIndexNode = dbIndexGetNode(rootIndexOffset);

				dbMemcpyFreeIndian(&(indexNode.indexNodeLinkOffset), &(pIndexNode->indexNodeLinkOffset), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
//####################
		if (indexNode.indexNodeLinkOffset == 0)
		{
			unsigned long	pTempRoot = 0;

			//subNode������ Max��带 �ø����� �ƴϸ�, subNode�� minNode�� �ø����� �۾��� �־�� �Ѵ�.
			{
				signed long		balanceFactor;
				
				IndexNode			tempRoot;
				
				balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);

				if (balanceFactor > 0)
				{
					//������ �� ū ���
					unsigned newLeftSubRoot;
					
					newLeftSubRoot = dbIndexDeleteNodeMakeTreeUpMaxNode(indexNode.indexNodeLeft, &pTempRoot); 
					
					if ((newLeftSubRoot == 0) && (pTempRoot == 0))
					{
						pTempRoot = 0;
					}
					else
					{
						pIndexNode = dbIndexGetNode(pTempRoot);
						
						tempRoot.indexNodeLeft	 = newLeftSubRoot;
						tempRoot.indexNodeRight	 = indexNode.indexNodeRight;
						dbMemcpyFreeIndian(&(pIndexNode->indexNodeLinkOffset), &(tempRoot.indexNodeLinkOffset), sizeof(unsigned long), 1);
						tempRoot.indexNodeHeight = dbIndexGetCalculateHeight(tempRoot.indexNodeLeft, tempRoot.indexNodeRight);
						
						pIndexNode = dbIndexGetNode(pTempRoot);
						dbMemcpyFreeIndian(&tempRoot, pIndexNode, sizeof(unsigned long), 4);
						gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					}
				}
				else
				{
					//�������� �� ũ�ų� ���� ���
					unsigned newRightSubRoot;
					
					newRightSubRoot = dbIndexDeleteNodeMakeTreeUpMinNode(indexNode.indexNodeRight, &pTempRoot); 
					
					if ((newRightSubRoot == 0) && (pTempRoot == 0))
					{
						pTempRoot = 0;
					}
					else
					{
						pIndexNode = dbIndexGetNode(pTempRoot);
						
						tempRoot.indexNodeLeft	 = indexNode.indexNodeLeft;
						tempRoot.indexNodeRight	 = newRightSubRoot;
						dbMemcpyFreeIndian(&(pIndexNode->indexNodeLinkOffset), &(tempRoot.indexNodeLinkOffset), sizeof(unsigned long), 1);
						tempRoot.indexNodeHeight = dbIndexGetCalculateHeight(tempRoot.indexNodeLeft, tempRoot.indexNodeRight);
						
						pIndexNode = dbIndexGetNode(pTempRoot);
						dbMemcpyFreeIndian(&tempRoot, pIndexNode, sizeof(unsigned long), 4);
						gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					}
				}
			}
			//�ش� index��带 ���� ���� ����Ѵ�.
			dbIndexDeleteIndexNode(rootIndexOffset, dataTypes[0]);

			rootIndexOffset = pTempRoot;
		}
//###################

		return rootIndexOffset;
	}
	else
	{
		unsigned long l2;

		if (compareResult==-1)
		{
			l2 = dbIndexDeleteUnitComplex(indexNode.indexNodeLeft, dataTypes, keyValues, recordRowid, indexLevel);

			if (indexNode.indexNodeLeft != l2)
			{
				indexNode.indexNodeLeft = l2;

				pIndexNode = dbIndexGetNode(rootIndexOffset);
				
				dbMemcpyFreeIndian(&indexNode.indexNodeLeft, &(pIndexNode->indexNodeLeft), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		else if (compareResult==1)
		{
			l2 = dbIndexDeleteUnitComplex(indexNode.indexNodeRight, dataTypes, keyValues, recordRowid, indexLevel);

			if (indexNode.indexNodeRight != l2)
			{
				indexNode.indexNodeRight = l2;

				pIndexNode = dbIndexGetNode(rootIndexOffset);
				
				dbMemcpyFreeIndian(&indexNode.indexNodeRight, &(pIndexNode->indexNodeRight), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		else{}

		//rotation check�� �Ѵ�.
		{
			signed char balanceFactor;
			IndexNode		subInode;

			balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);

			if (balanceFactor == 2)
			{
				pIndexNode	= dbIndexGetNode(indexNode.indexNodeLeft);

				dbMemcpyFreeIndian(pIndexNode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == -1)
				{
					//LRRotation
					dbIndexRotationLR(&rootIndexOffset, &(indexNode.indexNodeLeft), &(subInode.indexNodeRight));
				}
				else
				{
					//LL rotation
					dbIndexRotationLL(&rootIndexOffset, &(indexNode.indexNodeLeft), &(subInode.indexNodeLeft));
				}
			}
			else if (balanceFactor == -2)
			{
				pIndexNode	= dbIndexGetNode(indexNode.indexNodeRight);

				dbMemcpyFreeIndian(pIndexNode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == 1)
				{
					//RLRotation
					dbIndexRotationRL(&rootIndexOffset, &(indexNode.indexNodeRight), &(subInode.indexNodeLeft));
				}
				else
				{
					//RR rotation
					dbIndexRotationRR(&rootIndexOffset, &(indexNode.indexNodeRight), &(subInode.indexNodeRight));
				}

			}
			else{}
		}

		//indexNodeRootOffset�� ���̸� �ٽ� ����Ѵ�.
		{
			unsigned long inodeHeight;
			
			pIndexNode	= dbIndexGetNode(rootIndexOffset);
			
			dbMemcpyFreeIndian(pIndexNode, &indexNode, sizeof(unsigned long), 4);

			inodeHeight = dbIndexGetCalculateHeight(indexNode.indexNodeLeft, indexNode.indexNodeRight);
			
			if (indexNode.indexNodeHeight != inodeHeight)
			{
				pIndexNode	= dbIndexGetNode(rootIndexOffset);

				dbMemcpyFreeIndian(&inodeHeight, &(pIndexNode->indexNodeHeight), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
	}
	return rootIndexOffset;
}


unsigned long dbIndexDeleteUnitComplexMS(unsigned long	dbRootIndexOffset, 
										 unsigned long	dbDataTypes[], 
										 void*			dbKeyValues[],
										 unsigned long	dbRecordRowid,
										 unsigned long	dbIndexLevel)
{
	signed char		vCompareResult;
	pIndexNode		pIndexNode;
	IndexNode		indexNode;

	if (dbRootIndexOffset == 0 )
	{
		//��� �̷����±����� ���� �������̴�. �׳� ���� ó���Ҷ� ���� �κ��ϻ�
		return 0;
	}

	pIndexNode = dbIndexGetNode(dbRootIndexOffset);
	dbMemcpyFreeIndian(pIndexNode, &indexNode, sizeof(unsigned long), 4);

	vCompareResult = dbIndexCompareKeyAreaVsAreaMS( pIndexNode + 1, dbKeyValues[0], dbDataTypes[0]);//<-----����� ����߿�!!!
	
	if (vCompareResult==0)
	{
		if (dbIndexLevel == 1)
		{
			unsigned long	newLinkOffset;
			
			newLinkOffset = dbIndexLinkNodeDelete(indexNode.indexNodeLinkOffset, dbRecordRowid);

			if (indexNode.indexNodeLinkOffset != newLinkOffset)
			{
				indexNode.indexNodeLinkOffset = newLinkOffset;

				pIndexNode = dbIndexGetNode(dbRootIndexOffset);

				dbMemcpyFreeIndian(&(indexNode.indexNodeLinkOffset), &(pIndexNode->indexNodeLinkOffset), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
			
		}
		else
		{
			unsigned long	newSubLevelRoot;

			newSubLevelRoot = dbIndexDeleteUnitComplexMS(indexNode.indexNodeLinkOffset, &dbDataTypes[1], &dbKeyValues[1], dbRecordRowid, dbIndexLevel - 1);
			
			if (indexNode.indexNodeLinkOffset != newSubLevelRoot)
			{
				indexNode.indexNodeLinkOffset = newSubLevelRoot;

				pIndexNode = dbIndexGetNode(dbRootIndexOffset);

				dbMemcpyFreeIndian(&(indexNode.indexNodeLinkOffset), &(pIndexNode->indexNodeLinkOffset), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
//####################
		if (indexNode.indexNodeLinkOffset == 0)
		{
			unsigned long	pTempRoot = 0;

			//subNode������ Max��带 �ø����� �ƴϸ�, subNode�� minNode�� �ø����� �۾��� �־�� �Ѵ�.
			{
				signed long		balanceFactor;
				
				IndexNode			tempRoot;
				
				balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);

				if (balanceFactor > 0)
				{
					//������ �� ū ���
					unsigned newLeftSubRoot;
					
					newLeftSubRoot = dbIndexDeleteNodeMakeTreeUpMaxNode(indexNode.indexNodeLeft, &pTempRoot); 
					
					if ((newLeftSubRoot == 0) && (pTempRoot == 0))
					{
						pTempRoot = 0;
					}
					else
					{
						pIndexNode = dbIndexGetNode(pTempRoot);
						
						tempRoot.indexNodeLeft	 = newLeftSubRoot;
						tempRoot.indexNodeRight	 = indexNode.indexNodeRight;
						dbMemcpyFreeIndian(&(pIndexNode->indexNodeLinkOffset), &(tempRoot.indexNodeLinkOffset), sizeof(unsigned long), 1);
						tempRoot.indexNodeHeight = dbIndexGetCalculateHeight(tempRoot.indexNodeLeft, tempRoot.indexNodeRight);
						
						pIndexNode = dbIndexGetNode(pTempRoot);
						dbMemcpyFreeIndian(&tempRoot, pIndexNode, sizeof(unsigned long), 4);
						gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					}
				}
				else
				{
					//�������� �� ũ�ų� ���� ���
					unsigned newRightSubRoot;
					
					newRightSubRoot = dbIndexDeleteNodeMakeTreeUpMinNode(indexNode.indexNodeRight, &pTempRoot); 
					
					if ((newRightSubRoot == 0) && (pTempRoot == 0))
					{
						pTempRoot = 0;
					}
					else
					{
						pIndexNode = dbIndexGetNode(pTempRoot);
						
						tempRoot.indexNodeLeft	 = indexNode.indexNodeLeft;
						tempRoot.indexNodeRight	 = newRightSubRoot;
						dbMemcpyFreeIndian(&(pIndexNode->indexNodeLinkOffset), &(tempRoot.indexNodeLinkOffset), sizeof(unsigned long), 1);
						tempRoot.indexNodeHeight = dbIndexGetCalculateHeight(tempRoot.indexNodeLeft, tempRoot.indexNodeRight);
						
						pIndexNode = dbIndexGetNode(pTempRoot);
						dbMemcpyFreeIndian(&tempRoot, pIndexNode, sizeof(unsigned long), 4);
						gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					}
				}
			}
			//�ش� index��带 ���� ���� ����Ѵ�.
			dbIndexDeleteIndexNode(dbRootIndexOffset, dbDataTypes[0]);

			dbRootIndexOffset = pTempRoot;
		}
//###################

		return dbRootIndexOffset;
	}
	else
	{
		unsigned long l2;

		if (vCompareResult==-1)
		{
			l2 = dbIndexDeleteUnitComplexMS(indexNode.indexNodeLeft, dbDataTypes, dbKeyValues, dbRecordRowid, dbIndexLevel);

			if (indexNode.indexNodeLeft != l2)
			{
				indexNode.indexNodeLeft = l2;

				pIndexNode = dbIndexGetNode(dbRootIndexOffset);
				
				dbMemcpyFreeIndian(&indexNode.indexNodeLeft, &(pIndexNode->indexNodeLeft), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		else if (vCompareResult==1)
		{
			l2 = dbIndexDeleteUnitComplexMS(indexNode.indexNodeRight, dbDataTypes, dbKeyValues, dbRecordRowid, dbIndexLevel);

			if (indexNode.indexNodeRight != l2)
			{
				indexNode.indexNodeRight = l2;

				pIndexNode = dbIndexGetNode(dbRootIndexOffset);
				
				dbMemcpyFreeIndian(&indexNode.indexNodeRight, &(pIndexNode->indexNodeRight), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
		else{}

		//rotation check�� �Ѵ�.
		{
			signed char balanceFactor;
			IndexNode		subInode;

			balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);

			if (balanceFactor == 2)
			{
				pIndexNode	= dbIndexGetNode(indexNode.indexNodeLeft);

				dbMemcpyFreeIndian(pIndexNode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == -1)
				{
					//LRRotation
					dbIndexRotationLR(&dbRootIndexOffset, &(indexNode.indexNodeLeft), &(subInode.indexNodeRight));
				}
				else
				{
					//LL rotation
					dbIndexRotationLL(&dbRootIndexOffset, &(indexNode.indexNodeLeft), &(subInode.indexNodeLeft));
				}
			}
			else if (balanceFactor == -2)
			{
				pIndexNode	= dbIndexGetNode(indexNode.indexNodeRight);

				dbMemcpyFreeIndian(pIndexNode, &subInode, sizeof(unsigned long), 4);
				
				balanceFactor = dbIndexGetBalanceFactor(subInode.indexNodeLeft, subInode.indexNodeRight);
				
				if (balanceFactor == 1)
				{
					//RLRotation
					dbIndexRotationRL(&dbRootIndexOffset, &(indexNode.indexNodeRight), &(subInode.indexNodeLeft));
				}
				else
				{
					//RR rotation
					dbIndexRotationRR(&dbRootIndexOffset, &(indexNode.indexNodeRight), &(subInode.indexNodeRight));
				}

			}
			else{}
		}

		//indexNodeRootOffset�� ���̸� �ٽ� ����Ѵ�.
		{
			unsigned long inodeHeight;
			
			pIndexNode	= dbIndexGetNode(dbRootIndexOffset);
			
			dbMemcpyFreeIndian(pIndexNode, &indexNode, sizeof(unsigned long), 4);

			inodeHeight = dbIndexGetCalculateHeight(indexNode.indexNodeLeft, indexNode.indexNodeRight);
			
			if (indexNode.indexNodeHeight != inodeHeight)
			{
				pIndexNode	= dbIndexGetNode(dbRootIndexOffset);

				dbMemcpyFreeIndian(&inodeHeight, &(pIndexNode->indexNodeHeight), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}
	}
	return dbRootIndexOffset;
}


void dbIndexDeleteIndexNode(unsigned long deleteIndexOffset, 
								unsigned long dataType)
{
	DeleteNode					deleteNode;
	pIndexNode				pIndexNode;
	pIndexFilePageHeader	pidxHeader;
	unsigned long			relationOffset;
	
	deleteNode.deleteNodeSize = sizeof(IndexNode);

	pIndexNode = dbIndexGetNode(deleteIndexOffset);

	if (dataType == DB_VARCHAR_TYPE)
	{
		deleteNode.deleteNodeSize += dbStringGetSize_AREA((DB_VARCHAR*)(pIndexNode+1));
		deleteNode.deleteNodeSize += DB_VARCHAR_SIZE;
	}
	else if (dataType == DB_UINT8_TYPE)
	{
		deleteNode.deleteNodeSize += DB_UINT8_SIZE;
	}
	else if (dataType == DB_UINT16_TYPE)
	{
		deleteNode.deleteNodeSize += DB_UINT16_SIZE;
	}
	else if (dataType == DB_UINT32_TYPE)
	{
		deleteNode.deleteNodeSize += DB_UINT32_SIZE;
	}
	else{}
	
	pidxHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);

	dbMemcpyFreeIndian(&(pidxHeader->indexHeaderDeleteNodeOffset), &(deleteNode.deleteNodeNext), sizeof(unsigned long), 1);
	
	//�ε��� �ش��� �����Ѵ�.
	relationOffset = deleteIndexOffset%gDbPageSizeIndexFilePageSize;
	dbMemcpyFreeIndian(&relationOffset, &(pidxHeader->indexHeaderDeleteNodeOffset), sizeof(unsigned long), 1);

	pIndexNode = dbIndexGetNode(deleteIndexOffset);
	//���� ��带 ����Ѵ�.
	dbMemcpyFreeIndian(&deleteNode, pIndexNode, sizeof(unsigned long), 2);

	gDbMmuIndexPage->mmuChanged = DB_CHANGED;
}

#if 0
void dbIndexDeleteAllSingle(unsigned long	columnStartOffset, 
								void*			columnsValue[],
								unsigned long	recordRowid)
{
	pDictionaryHeader	pdicHeader;	//Dictionary file�� �ش��� �������� ����
	pColumnNode				pColumn;
	ColumnNode			columnNode;
	unsigned long		columnCount;

	pdicHeader = dbGetDictionaryHeader();

	columnCount = 0;

	while(columnStartOffset != 0)
	{
		pColumn = (pColumnNode)(((char*)pdicHeader) + columnStartOffset);

		dbMemcpyFreeIndian(pColumn, &columnNode, sizeof(unsigned long), 3);

		if (!(columnNode.columnNodeIndexRootOffset == 0 || columnNode.columnNodeIndexRootOffset == 1))
		{
			unsigned long newRoot;

			newRoot = dbIndexDeleteUnitSingle(columnNode.columnNodeIndexRootOffset, 
												   columnNode.columnNodeDataType, 
												   columnsValue[columnCount],
												   recordRowid);
			if (newRoot != columnNode.columnNodeIndexRootOffset)
			{
				if (newRoot == 0) newRoot = 1;

				dbMemcpyFreeIndian(&newRoot, &(pColumn->columnNodeIndexRootOffset), sizeof(unsigned long), 1);
				gDbDictionaryChanged = DB_CHANGED;
			}
		}

		columnCount++;
		columnStartOffset = columnNode.columnNodeNextColumnOffset;
	}
}
#endif
void dbIndexDeleteAllSingleMS(unsigned long	dbColumnStartOffset, 
							  void*			dbColumnsValues[],
							  unsigned long	dbRecordRowid)
{
	pDictionaryHeader	pdictionaryHeader;	//Dictionary file�� �ش��� �������� ����
	pColumnNode		pCurrentColumn;
	ColumnNode		vColumnNode;
	unsigned long		vColumnCount;

	pdictionaryHeader = dbGetDictionaryHeader();

	vColumnCount = 0;

	while(dbColumnStartOffset != 0)
	{
		pCurrentColumn = (pColumnNode)(((char*)pdictionaryHeader) + dbColumnStartOffset);

		dbMemcpyFreeIndian(pCurrentColumn, &vColumnNode, sizeof(unsigned long), 3);

		if (!(vColumnNode.columnNodeIndexRootOffset == 0 || vColumnNode.columnNodeIndexRootOffset == 1))
		{
			unsigned long vNewRoot;
			
		#if ( AVLDB_DEBUG == 1 )
			//DbgPrintf(("===================���� �ε����� ���Ͽ� �ε��� ������ �õ��Ѵ�..====================\n"));
		#endif
	
			vNewRoot = dbIndexDeleteUnitSingleMS(vColumnNode.columnNodeIndexRootOffset, 
												vColumnNode.columnNodeDataType, 
												dbColumnsValues[vColumnCount],
												dbRecordRowid);

		#if ( AVLDB_DEBUG == 1 )
			//DbgPrintf(("==================������ �ε����� ���� Offset�� �� vNewRoot = [%x]=====================\n", vNewRoot));
		#endif
		
			if (vNewRoot != vColumnNode.columnNodeIndexRootOffset)
			{
				if (vNewRoot == 0) vNewRoot = 1;

				dbMemcpyFreeIndian(&vNewRoot, &(pCurrentColumn->columnNodeIndexRootOffset), sizeof(unsigned long), 1);
				gDbDictionaryChanged = DB_CHANGED;
			}

		#if ( AVLDB_DEBUG == 1 )
			//DbgPrintf(("==================������ �ε����� ���� ������ Offset�� �� vNewRoot = [%x]=====================\n", vNewRoot));
		#endif	
		}

		vColumnCount++;
		dbColumnStartOffset = vColumnNode.columnNodeNextColumnOffset;
	}
}

char* dbApiAllocKey(unsigned long dbKeyValueOffset, 
					   unsigned char dbFileType)
{
//�� �Լ��� Search����� ���� Rowid�� �ش� ���� �����ϴ� DB�� Area���� Address�� �ѱ�� �Լ��̴�.
	char*	pKeyValueAddress;		//DB�� Area���� Address�� Pointing�� ����
	
	#if DB_SHOW_ERROR_CODE				//Error Check
	gDbErrorCode = DB_ERROR_NO_ERROR;		//Error Check
	#endif									//Error Check

	if (gDbStartUpDB != DB_DO_RUNNING)
	{
		#if DB_SHOW_ERROR_CODE								//Error Check
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;		//Error Check
		#endif													//Error Check
		
		pKeyValueAddress = DB_NULL;
	}
	else
	{
		if (dbFileType == DB_DATA_FILE_LOAD)
		{
			dbTouch(dbKeyValueOffset, DB_DATA_FILE_LOAD);

			pKeyValueAddress = (char*)(gDbMmuDataPage->mmuMemoryAddress + dbKeyValueOffset%gDbPageSizeDataFilePageSize);
		}
		else if (dbFileType == DB_INDEX_FILE_LOAD)
		{
			dbTouch(dbKeyValueOffset, DB_INDEX_FILE_LOAD);
			
			pKeyValueAddress = (char*)(gDbMmuIndexPage->mmuMemoryAddress + dbKeyValueOffset%gDbPageSizeIndexFilePageSize);
		}
		else
		{
			#if DB_SHOW_ERROR_CODE																//Error Check
			gDbErrorCode = DB_ERROR_DBAPIALLOCKEY_FUNCTION_DBFILETYPE_PARAMETER_INCORRECT;		//Error Check
			#endif																					//Error Check
			
			pKeyValueAddress = DB_NULL;
		}
	}
	
	return pKeyValueAddress;
}

//#####################################################################################
#if 0
void dbRecordUpdate(DB_VARCHAR		*tableName,
										   unsigned long	recordRowid,
										   DB_VARCHAR		*columnName,
										   void				*updateColumnValue,
										   unsigned char	state)
{
	pDictionaryHeader		pdicHeader;	//Dictionary file�� �ش��� �������� ����
	pTableNode					ptable;			//Dictionary file�� �ö���� ���������� ���̺��� �������� ����
	TableNode					tableNode;
	pColumnNode					pColumn;
	//unsigned long			columnType;
	unsigned long			*ptableOffset, vTableOffset;
	unsigned long			columnCount;
	void*					columnsValue[DB_TABLE_MAX_COLUMN_NUMBER];

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary�� �����Ѵ�.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����
	
	//���̺��� ã�´�.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : �Է����� ���� tableName�� table�� �ý��ۿ� ���������ʴ� ���
		return;
	}
	// ===========�Ʒ����� �ش� ���̺��� ã������ ó�� ===========//

	//table�� offset�� ����Ǿ� �ִ�.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table�� �޸𸮸� �����Ѵ�.
	ptable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//���̺��� ������ TABLE_NODE�� �����Ѵ�.
	dbMemcpyFreeIndian(ptable, &tableNode, sizeof(unsigned long), 6);


	//�ش� record�� �� �÷��� ������ ä���´�.
	if (dbDataGetRecord(tableNode.tableNodeColumnOffset, recordRowid, columnsValue) == 0)
	{
		//�ش� �����Ͱ� ���� ���
		return;
	}
	// ===========�Ʒ� ���� �ش� ���̺�� �ش� ���� ã������̴�.======//
	
	{
		ColumnNode					columnNode;
		unsigned long			newRootOffset;

		//���° �÷����� ã�´�.
		columnCount = dbFindColumnName(tableNode.tableNodeColumnOffset, &pColumn, columnName, DB_AREA_VS_STACK);
		
		//�ش� �÷��� �޾ƿ´�.
		dbMemcpyFreeIndian(pColumn, &columnNode, sizeof(unsigned long), 3);

		if (state == DB_UPDATE_WITH_COMPLEX_INDEX)
		{
			//�����ε����� �����Ѵ�.
		}

		if ((state == DB_UPDATE_WITH_SINGLE_INDEX)||(state == DB_UPDATE_WITH_COMPLEX_INDEX))
		{
			//�̱��ε��� ���Ÿ� �Ѵ�.
			{//������ �̱� �ε����� �����ϴ� ��ƾ
				newRootOffset = dbIndexDeleteUnitSingle(columnNode.columnNodeIndexRootOffset, 
															  columnNode.columnNodeDataType, 
															  columnsValue[columnCount],
															  recordRowid);
				if (columnNode.columnNodeIndexRootOffset != newRootOffset)
				{
					columnNode.columnNodeIndexRootOffset = newRootOffset;

					//�ش� �÷��� dataType�� ���´�
					dbMemcpyFreeIndian(&(columnNode.columnNodeIndexRootOffset), &(pColumn->columnNodeIndexRootOffset), sizeof(unsigned long), 1);
					gDbDictionaryChanged = DB_CHANGED;
				}
			}

			{//�ش� ���� ������ �̱��ε����� �߰��ϴ� ��ƾ
				newRootOffset = dbIndexSingleUnitInsert(columnNode.columnNodeIndexRootOffset,
															updateColumnValue,
															columnNode.columnNodeDataType,
															recordRowid);

				if (columnNode.columnNodeIndexRootOffset != newRootOffset)
				{
					columnNode.columnNodeIndexRootOffset = newRootOffset;

					//�ش� �÷��� dataType�� ���´�
					dbMemcpyFreeIndian(&(columnNode.columnNodeIndexRootOffset), &(pColumn->columnNodeIndexRootOffset), sizeof(unsigned long), 1);
					gDbDictionaryChanged = DB_CHANGED;
				}
			}
		}
		
		if ((state == DB_UPDATE_WITHOUT_INDEX)||(state == DB_UPDATE_WITH_SINGLE_INDEX)||(state == DB_UPDATE_WITH_COMPLEX_INDEX))
		{
			//���ڵ��� �ش� �÷��� ��ġ�� ���� �����Ѵ�.
			dbDataRecordSetColumn( columnsValue[columnCount], updateColumnValue, columnNode.columnNodeDataType);
		}
	}
}
#endif
void dbIndexLinkTraverse(unsigned long dbLinkOffset)
{
	pLinkNode	pLink;		//Memory Area���� Link�� ��ġ�� ������ �� ����
	LinkNode		vLinkNode;	//Memory Area���� Link�� ������ ������  Stack����

	
	while(dbLinkOffset)
	{
		//Memory Area���� Link�� ��ġ�� �����Ѵ�.
		pLink = dbIndexGetLink(dbLinkOffset);

		//Stack������ �ش� ��ũ�� �����Ѵ�.
		dbMemcpyFreeIndian(pLink, &vLinkNode, sizeof(unsigned long), 2);

		//===================================================//
		//1. �˻������ ������ Arrray�� �ش� Record�� Rowid�� �����Ѵ�.
		#if 0
		*( dbSearchArray + dbSearchCount) = vLinkNode.linkNodeRecordRowid;
		//2. �˻��� ������ ������Ų��.
		dbSearchCount++;
		#endif
		
		if (gDbSearchData.ResultArraySize > gDbSearchData.dbSearchCount)
		{
			*(gDbSearchData.ResultArray + gDbSearchData.dbSearchCount) = vLinkNode.linkNodeRecordRowid;
			gDbSearchData.dbSearchCount++;
		}
		else
		{
			gDbSearchData.vRemainRecordsCount++;
		}

		#if 0
		#ifdef DB_SEARCH_CHANGED
		if (gDbSearchData.dbSearchCount>=gDbSearchData.ResultArraySize)
		{
			return;
		}
		#endif
		#endif
		//===================================================//
		
		//���� ��ũ�� Offset�� �����Ѵ�.
		dbLinkOffset =  vLinkNode.linkNodeNext;
	}
}


unsigned long dbApiSearchWithout(DB_VARCHAR*		tableName,
									DB_VARCHAR*	indexColumnName,
									void*			columnValues[],
									unsigned long		columnCount,
									unsigned char		traverseOrder,
									unsigned long		resultArray[],
									unsigned char		*refMemory)
{
	pDictionaryHeader		pdicHeader;	//Dictionary file�� �ش��� �������� ����
	pTableNode					pTable;			//Dictionary file�� �ö���� ���������� ���̺��� �������� ����
	TableNode					TABLE_NODE;
	pColumnNode					pColumn;
	ColumnNode				columnNode;
	unsigned long			*ptableOffset, vTableOffset;
	unsigned long			*pcolumnOffset, vColumnOffset;
//	unsigned long			indexOffset;

#if DB_SHOW_ERROR_CODE
	gDbErrorCode = DB_ERROR_NO_ERROR;
#endif

//==============ERROR Code (Begin)==============//
if (gDbStartUpDB != DB_DO_RUNNING)
{
	#if DB_SHOW_ERROR_CODE
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;
	#endif
	return 0;
}
//==============ERROR Code ( end )==============//


	pdicHeader		= dbGetDictionaryHeader();			//Dictionary�� �����Ѵ�.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����
	
	//���̺��� ã�´�.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : �Է����� ���� tableName�� table�� �ý��ۿ� ���������ʴ� ���
		return 0;
	}
	// ===========�Ʒ����� �ش� ���̺��� ã������ ó�� ===========//

	//table�� offset�� ����Ǿ� �ִ�.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table�� �޸𸮸� �����Ѵ�.
	pTable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//���̺��� ������ TABLE_NODE�� �����Ѵ�.
	dbMemcpyFreeIndian(pTable, &TABLE_NODE, sizeof(unsigned long), 6);

	if (TABLE_NODE.tableNodeStartPage == 0)
	{
		//�����ͻ����� �����ٸ� ���̻� �˻��Ұ͵� ����..
		return 0;
	}

	//�ε����� ���� offset�� ��ġ�� ã�Ƴ���.
	{
		//�÷��� ���� offset�� ����Ǿ� �ִ� �ּҸ� �������Ѵ�.
		pcolumnOffset = &(pTable->tableNodeColumnOffset);

		//�÷��� ã�´�.
		if(!dbTableFindColumnName(&pcolumnOffset, indexColumnName))
		{
			//ERROR : �ش� �÷��� �������� �ʴ� ���
			return 0;
		}

		//�÷��� offset�� ����Ǿ� �ִ�.
		dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);

		pColumn =  (pColumnNode)(((char*)pdicHeader) + vColumnOffset);
		dbMemcpyFreeIndian(pColumn, &columnNode, sizeof(unsigned long), 3);
	}
	//indexOffset���� �ε����� ���� offset�� ���� ����Ǿ��ִ�.


	//�ε��� Ž���� �����Ѵ�.
	dbSearchArray = resultArray;
	dbSearchCount = 0;
	
	//������������ recordRowid�� ����� ������ ���
	*refMemory = DB_DATA_FILE_LOAD;
	
	dbIndexTreeTraverseConditonWithout(columnNode.columnNodeIndexRootOffset, columnNode.columnNodeDataType, columnValues, columnCount, traverseOrder);	//����� �����ϴ� �κ�
	
	return dbSearchCount;
}

void dbIndexTreeTraverseConditonWithout(unsigned long	rootOffset, 
											 unsigned long	dataType,
											 void*			keyValues[],
											 unsigned long	keyNumber,
											 unsigned char	isAsc)
{
	if (isAsc == DB_ASC)
	{
		dbIndexTreeTraverseAscConditionWithout(rootOffset, dataType, keyValues, keyNumber);
	}
	else if(isAsc == DB_DESC)
	{
		dbIndexTreeTraverseDescConditionWithout(rootOffset, dataType, keyValues, keyNumber);
	}
	else{}
}

void dbIndexTreeTraverseAscConditionWithout(unsigned long rootOffset,
												  unsigned long dataType,
												  void*			withoutKeys[],
												  unsigned long keyNumber)
{
	if (rootOffset == 0)
	{
		return;
	}
	else
	{
		pIndexNode			pInode;
		IndexNode			INDEX_NODE;
		pLinkNode			pLink;
		LinkNode			LINK_NODE;
		unsigned long	linkOffset;
		unsigned long	i;
		signed char		compareResult;

		pInode = dbIndexGetNode(rootOffset);
		dbMemcpyFreeIndian(pInode, &INDEX_NODE, sizeof(unsigned long), 4);
		
		
		dbIndexTreeTraverseAscConditionWithout(INDEX_NODE.indexNodeLeft, dataType, withoutKeys, keyNumber);
		
		pInode = dbIndexGetNode(rootOffset);
		
		for(i = 0 ; i < keyNumber ; i++)
		{
			compareResult = dbIndexCompareKey_AREAVs_STACK(pInode + 1, withoutKeys[i], dataType);
			if (compareResult==0) break;
		}
		if (compareResult == 0)
		{
		}
		else
		{
			linkOffset = INDEX_NODE.indexNodeLinkOffset;

				
			while(linkOffset)
			{
				pLink = dbIndexGetLink(linkOffset);
				dbMemcpyFreeIndian(pLink, &LINK_NODE, sizeof(unsigned long), 2);

				*( dbSearchArray + dbSearchCount) = LINK_NODE.linkNodeRecordRowid;
				dbSearchCount++;

				linkOffset =  LINK_NODE.linkNodeNext;
			}
		}

		dbIndexTreeTraverseAscConditionWithout(INDEX_NODE.indexNodeRight, dataType, withoutKeys, keyNumber);
	}
}
void dbIndexTreeTraverseDescConditionWithout(unsigned long rootOffset,
												  unsigned long dataType,
												  void*			withoutKeys[],
												  unsigned long keyNumber)
{
	if (rootOffset == 0)
	{
		return;
	}
	else
	{
		pIndexNode			pInode;
		IndexNode			INDEX_NODE;
		pLinkNode			pLink;
		LinkNode			LINK_NODE;
		unsigned long	linkOffset;
		unsigned long	i;
		signed char		compareResult;
		
		pInode = dbIndexGetNode(rootOffset);
		dbMemcpyFreeIndian(pInode, &INDEX_NODE, sizeof(unsigned long), 4);

		dbIndexTreeTraverseDescConditionWithout(INDEX_NODE.indexNodeRight, dataType, withoutKeys, keyNumber);

		pInode = dbIndexGetNode(rootOffset);
		
		for(i = 0 ; i < keyNumber ; i++)
		{
			compareResult = dbIndexCompareKey_AREAVs_STACK(pInode + 1, withoutKeys[i], dataType);
			
			if (compareResult == 0) break;
		}

		if (compareResult == 0)
		{}
		else
		{
			linkOffset = INDEX_NODE.indexNodeLinkOffset;

			while(linkOffset)
			{
				pLink = dbIndexGetLink(linkOffset);
				dbMemcpyFreeIndian(pLink, &LINK_NODE, sizeof(unsigned long), 2);

				*( dbSearchArray + dbSearchCount) = LINK_NODE.linkNodeRecordRowid;
				dbSearchCount++;

				linkOffset =  LINK_NODE.linkNodeNext;
			}
		}

		dbIndexTreeTraverseDescConditionWithout(INDEX_NODE.indexNodeLeft, dataType, withoutKeys, keyNumber);
	}
}


//====================================================================================//
//================= ������ ������ PC���� �ʿ�� �ϴ� �κ���(begin)====================//
//====================================================================================//

int dbApiGetTotalRecordNum(DB_VARCHAR	*tableName, unsigned long *pTotal)
{
	pDictionaryHeader		pdicHeader;	//Dictionary file�� �ش��� �������� ����
	pTableNode			ptable;			//Dictionary file�� �ö���� ���������� ���̺��� �������� ����
	unsigned long			tableStartPage;
	unsigned long			*ptableOffset, vTableOffset;
	unsigned long			recordTotalCount;
	pDataFilePageHeader	pdataFileHeader;
	unsigned long			dataFileRecordNum;

#if DB_SHOW_ERROR_CODE					//error check
	gDbErrorCode = DB_ERROR_NO_ERROR;		//error check
#endif										//error check

//==============ERROR Code (Begin)==============//
if (gDbStartUpDB != DB_DO_RUNNING)
{
	#if DB_SHOW_ERROR_CODE									//error check
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;		//error check
	#endif														//error check

	return DB_FUNCTION_FAIL;
}
//==============ERROR Code ( end )==============//

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary�� �����Ѵ�.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����
	
	//���̺��� ã�´�.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : �Է����� ���� tableName�� table�� �ý��ۿ� ���������ʴ� ���
		#if DB_SHOW_ERROR_CODE							//error check
			gDbErrorCode = DB_ERROR_TABLE_NOT_FOUND;		//error check
		#endif												//error check

		return DB_FUNCTION_FAIL;
	}
	// ===========�Ʒ����� �ش� ���̺��� ã������ ó�� ===========//

	//table�� offset�� ����Ǿ� �ִ�.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table�� �޸𸮸� �����Ѵ�.
	ptable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//���̺��� ������ TABLE_NODE�� �����Ѵ�.
	dbMemcpyFreeIndian(&(ptable->tableNodeStartPage), &tableStartPage, sizeof(unsigned long), 1);

	recordTotalCount = 0;

	//���̺� �Ҽ��� ��� page�� ���鼭 ���ڵ��� ������ ��ģ��.
	while(tableStartPage)
	{
		dbTouch((tableStartPage-1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		
		pdataFileHeader = (pDataFilePageHeader)(gDbMmuDataPage->mmuMemoryAddress);

		dbMemcpyFreeIndian(&(pdataFileHeader->dataHeaderNumberOfSlots), &dataFileRecordNum, sizeof(unsigned long), 1);
		
		recordTotalCount += dataFileRecordNum;
		
		dbMemcpyFreeIndian(&(pdataFileHeader->dataHeaderNextSameTablePage), &tableStartPage, sizeof(unsigned long), 1);
	}

	*pTotal = recordTotalCount;
	
	return DB_FUNCTION_SUCCESS;
}

void dbApiAllSave(void)
{

	DbgPrintf(("\ndbApiAllSave ����"));
	#if DB_SHOW_ERROR_CODE					//Error check
	gDbErrorCode = DB_ERROR_NO_ERROR;			//Error check
	#endif										//Error check

	if (gDbStartUpDB != DB_DO_RUNNING)
	{
		#if DB_SHOW_ERROR_CODE								//Error check
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;		//Error check
		#endif													//Error check
		
		return;
	}

	DbgPrintf(("\ndbApiAllSave : 1. Dictionary File Save ����"));
	//��ųʸ��� ��������� ������� ��ųʸ� ������ �����Ѵ�.
	if (gDbDictionaryChanged == DB_CHANGED)
	{
		dbStoreDictionaryfile();
	}
	//DbgPrintf(("\ndbApiAllSave : 1. Dictionary File Save Ż��"));
	//DbgPrintf(("\ndbApiAllSave : 2. Data File Save ����"));
	
	//������ ������ �ε��� �Ǿ��ٸ� �Ǵ��Ͽ� ������ �κ��� �����Ѵ�.
	if (gDbMmuDataPage->mmuPageNumber != DB_NULL)
	{
		dbStoreDatafile();
	}
	DbgPrintf(("\ndbApiAllSave : 2. Data File Save Ż��"));
	DbgPrintf(("\ndbApiAllSave : 3. Index File Save ����"));
	//�ε��� ������ �ε��� �Ǿ��ٸ� �Ǵ��Ͽ� ������ �κ��� �����Ѵ�.
	if (gDbMmuIndexPage->mmuPageNumber != DB_NULL)
	{
		dbStoreIndexfile();
	}
	DbgPrintf(("\ndbApiAllSave : 3. Index File Save Ż��"));
	DbgPrintf(("\ndbApiAllSave Ż��"));
}
unsigned char dbApiDataCheckRecordValidRowID(unsigned long dbRecordRowid)
{
	unsigned char result;
	#if DB_SHOW_ERROR_CODE				//Error Check
	gDbErrorCode = DB_ERROR_NO_ERROR;		//Error Check
	#endif									//Error Check

	result = dbDataCheckRecordValidRowID(dbRecordRowid);

	return result;
}
unsigned char dbDataCheckRecordValidRowID(unsigned long dbRecordRowid)
{
	pDictionaryHeader		pdicHeader;
	unsigned long			vDataPageNumber;
	unsigned long			vEndDataPageInDictionary;
	unsigned long*		pSlotPointer;
	unsigned long			vSlotValue;
	pDataFilePageHeader	pdataFilePageHeader;
	unsigned long			vPageTotalRecordNumbers;
	unsigned long			vRecordOffsetInPage;
	
	if (dbRecordRowid == 0)
	{
		//�����Ϸ��� Rowid�� ���� �ʹ� ū ���
		#if DB_SHOW_ERROR_CODE						//Error Check
		gDbErrorCode = DB_ERROR_INVALID_ROWID_ZERO;	//Error Check
		#endif											//Error Check
		
		return DB_INVALID_ROWID_ZERO;
	}

	//Dictionary�� �����Ѵ�.
	pdicHeader	= dbGetDictionaryHeader();

	//��ųʸ��ش��� ������ DataPage�� ������ Page��ȣ���� �����´�.
	dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderNumberOfDataFileEndPage), &vEndDataPageInDictionary, sizeof(unsigned long), 1);

	//Datafile������ page ���� �����Ѵ�.
	vDataPageNumber = (dbRecordRowid/gDbPageSizeDataFilePageSize)+1;

	if (vEndDataPageInDictionary<vDataPageNumber)
	{
		//�����Ϸ��� Rowid�� ���� �ʹ� ū ���
		#if DB_SHOW_ERROR_CODE									//Error Check
		gDbErrorCode = DB_ERROR_INVALID_ROWID_TOO_VERY_LARGE	;	//Error Check
		#endif														//Error Check
		
		return DB_INVALID_ROWID_TOO_VERY_LARGE;
	}
	//Datafile������ ��� Offset ���� �����Ѵ�.
	vRecordOffsetInPage = dbRecordRowid%gDbPageSizeDataFilePageSize;

	//�ش� dbRecordRowid�� ���� ������ Record�� Page�� Load�Ѵ�.
	dbTouch(dbRecordRowid, DB_DATA_FILE_LOAD);

	//Data Page�� �ش��� Pointing�Ѵ�.
	pdataFilePageHeader = (pDataFilePageHeader)(gDbMmuDataPage->mmuMemoryAddress);

	//Data Page���� ���ڵ��� ������ �����Ѵ�.
	dbMemcpyFreeIndian(&(pdataFilePageHeader->dataHeaderNumberOfSlots), &vPageTotalRecordNumbers, sizeof(unsigned long), 1);

	while(vPageTotalRecordNumbers)
	{
		pSlotPointer = (unsigned long*)(gDbMmuDataPage->mmuMemoryAddress + gDbPageSizeDataFilePageSize - (vPageTotalRecordNumbers*sizeof(unsigned long)));
		dbMemcpyFreeIndian(pSlotPointer, &vSlotValue, sizeof(unsigned long), 1);

		if (vSlotValue == vRecordOffsetInPage)
		{
			return DB_VALID_ROWID;
		}

		vPageTotalRecordNumbers--;
	}

	#if DB_SHOW_ERROR_CODE					//Error Check
	gDbErrorCode = DB_ERROR_INVALID_ROWID;	//Error Check
	#endif										//Error Check
	
	return DB_INVALID_ROWID;
}



unsigned char dbApiGetRecordCopy(DB_VARCHAR	*tableName,
									 unsigned long		recordRowid,
									 void*			columnsValue[])
{
	pDictionaryHeader		pdicHeader;		//Dictionary file�� �ش��� �������� ����
	pTableNode			ptable;			//Dictionary file�� �ö���� ���������� ���̺��� �������� ����
	TableNode			vTableNode;
	unsigned long			*ptableOffset, vTableOffset;

	#if DB_SHOW_ERROR_CODE				//Error Check
	gDbErrorCode = DB_ERROR_NO_ERROR;		//Error Check
	#endif									//Error Check


	if (gDbStartUpDB != DB_DO_RUNNING)
	{
		#if DB_SHOW_ERROR_CODE								//Error Check
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;		//Error Check
		#endif													//Error Check

		return DB_FUNCTION_FAIL;
	}

	//Record Copy�� �����ϱ� ���� ���� recordRowid�� ���� ������ �������� �Ǵ��Ѵ�.
	if (dbDataCheckRecordValidRowID(recordRowid) != DB_VALID_ROWID)
	{
		#if DB_SHOW_ERROR_CODE					//Error check
		gDbErrorCode = DB_ERROR_INVALID_ROWID;	//Error check
		#endif										//Error check
		
		return DB_FUNCTION_FAIL;
	}

	{
	}
	
	pdicHeader	= dbGetDictionaryHeader();						//Dictionary�� �����Ѵ�.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����
	
	//���̺��� ã�´�.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : �Է����� ���� tableName�� table�� �ý��ۿ� ���������ʴ� ���
		#if DB_SHOW_ERROR_CODE					//Error check
		gDbErrorCode = DB_ERROR_TABLE_NOT_FOUND;	//Error check
		#endif										//Error check
		
		return DB_FUNCTION_FAIL;
	}
	//���� if���� ����ϸ� ptableOffset�� Dictionary File���� Table Offset�� ����Ǿ��ִ� ���� �������Ѵ�.


	//vTableOffset�� Table�� Offset�� �����Ѵ�.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);


	//ptable�� Dictionary�� Table�� Pointing�Ѵ�.
	ptable = (pTableNode)(((char*)pdicHeader) + vTableOffset);

	
	//���̺��� ������ TABLE_NODE�� �����Ѵ�.
	dbMemcpyFreeIndian(ptable, &vTableNode, sizeof(unsigned long), 6);


	//���縦 �Ϸ��� Rowid�� �ش� ���̺� �Ҽ��� DataPage�� �ִ����� �˻��Ѵ�.
	{
		unsigned long vCopyRecordPageNumber;
		vCopyRecordPageNumber = (recordRowid/gDbPageSizeDataFilePageSize) + 1;
		if (!dbHaveDataPageNumberInDataPageChain(vTableNode.tableNodeStartPage, vCopyRecordPageNumber))
		{
			//ERROR : �Է����� ���� tableName�� table�� �ý��ۿ� ���������ʴ� ���
			return DB_FUNCTION_FAIL;
		}
	}


	//�ش� ���ڵ��� ���� ä���´�.
	if (dbDataGetRecordCopy(vTableNode.tableNodeColumnOffset, recordRowid, columnsValue) != 1)
	{
		#if DB_SHOW_ERROR_CODE						//Error check
		gDbErrorCode = DB_ERROR_RECORD_COPY_ERROR;	//Error check
		#endif											//Error check
		
		return DB_FUNCTION_FAIL;
	}

	return DB_FUNCTION_SUCCESS;
}


/*
struct  mstring{ 
	unsigned char		NumChars;
	unsigned short	StringChars[255];
};
*/
/*
unsigned char dbDataGetRecordCopy(unsigned long columnStartOffset, 
									  unsigned long recordRowid, 
								      void* resultArray[])
{
	pDictionaryHeader		pdicHeader;	//Dictionary file�� �ش��� �������� ����
	pColumnNode			pColumn;
	ColumnNode			columnNode;
	char					*pRecord;
	char					*pBuffer;
	unsigned long			columnCount;

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary�� �����Ѵ�.
	
	if (columnStartOffset == 0)
	{
		return 0;
	}
	else
	{
		//���� ���ڵ带 �����´�.
		dbTouch(recordRowid, DB_DATA_FILE_LOAD);
		pRecord = gDbMmuDataPage->mmuMemoryAddress + recordRowid%gDbPageSizeDataFilePageSize;
		columnCount = 0;
	}

	while(columnStartOffset)
	{
		pBuffer = resultArray[columnCount];	//record�� ��ϵ� ������ �ִ´�.
		
		//�����÷������� ����
		columnCount++;

		pColumn = (pColumnNode)(((char*)pdicHeader) + columnStartOffset);
		dbMemcpyFreeIndian(pColumn, &columnNode, sizeof(unsigned long), 3);

		if (columnNode.columnNodeDataType == DB_VARCHAR_TYPE)
		{
			struct mString	*pMtpString;
			unsigned long		vStringSize;

			pMtpString = (struct mString*)pBuffer;

			vStringSize = dbStringCopy(pMtpString->StringChars, (DB_VARCHAR*)pRecord);
			
			pRecord = pRecord + vStringSize;
			
			pMtpString->NumChars = ((vStringSize)/2) -1;
		}
		else if (columnNode.columnNodeDataType == DB_UINT8_TYPE)
		{
			dbMemcpyFreeIndian(pRecord, pBuffer, DB_UINT8_SIZE, 1);
			pRecord += DB_UINT8_SIZE;
		}
		else if (columnNode.columnNodeDataType == DB_UINT16_TYPE)
		{
			dbMemcpyFreeIndian(pRecord, pBuffer, DB_UINT16_SIZE, 1);
			pRecord += DB_UINT16_SIZE;
		}
		else if (columnNode.columnNodeDataType == DB_UINT32_TYPE)
		{
			dbMemcpyFreeIndian(pRecord, pBuffer, DB_UINT32_SIZE, 1);
			pRecord += DB_UINT32_SIZE;
		}

		columnStartOffset = columnNode.columnNodeNextColumnOffset;
	}
	return 1;
}*/
unsigned char dbDataGetRecordCopy(unsigned long columnStartOffset, 
									  unsigned long recordRowid, 
								      void* resultArray[])
{
	pDictionaryHeader		pdicHeader;	//Dictionary file�� �ش��� �������� ����
	pColumnNode			pColumn;
	ColumnNode			columnNode;
	char					*pRecord;
	char					*pBuffer;
	unsigned long			columnCount;
//DbgPrintf(("===================================\n"));
	pdicHeader		= dbGetDictionaryHeader();			//Dictionary�� �����Ѵ�.
	
	if (columnStartOffset == 0)
	{
		return 0;
	}
	else
	{
		//���� ���ڵ带 �����´�.
		dbTouch(recordRowid, DB_DATA_FILE_LOAD);
		pRecord = gDbMmuDataPage->mmuMemoryAddress + recordRowid%gDbPageSizeDataFilePageSize;
		columnCount = 0;
	}

	while(columnStartOffset)
	{
		pBuffer = resultArray[columnCount];	//record�� ��ϵ� ������ �ִ´�.
		
		//�����÷������� ����
		columnCount++;

		pColumn = (pColumnNode)(((char*)pdicHeader) + columnStartOffset);
		dbMemcpyFreeIndian(pColumn, &columnNode, sizeof(unsigned long), 3);

		if (columnNode.columnNodeDataType == DB_VARCHAR_TYPE)
		{
//			struct mString	*pMtpString;
			unsigned long		vStringSize;

//DbgPrintf(("Dest[%x], �ޱ����  ", pBuffer));
#if 0	//060110 15:02 ��ö
			if (pBuffer!=0)
			{
				pMtpString = (struct mString*)pBuffer;

				vStringSize = dbStringCopy(pMtpString->StringChars, (DB_VARCHAR*)pRecord);
				
				pRecord = pRecord + vStringSize;
				
				pMtpString->NumChars = ((vStringSize)/2) -1;
//DbgPrintf(("[1] Size [%x]\n", vStringSize));
			}
#else
			if (pBuffer!=0)
			{
				pRecord = pRecord + dbStringCopy( (DB_VARCHAR*)pBuffer, (DB_VARCHAR*)pRecord );		// 060110
			}
#endif
			else
			{
				vStringSize = dbStringGetSize((DB_VARCHAR*)pRecord) + DB_VARCHAR_SIZE;
				pRecord = pRecord + vStringSize;
			}
			
		}
		else if (columnNode.columnNodeDataType == DB_UINT8_TYPE)
		{
			if (pBuffer!=0)
			{
				dbMemcpyFreeIndian(pRecord, pBuffer, DB_UINT8_SIZE, 1);
			}
			pRecord += DB_UINT8_SIZE;
		}
		else if (columnNode.columnNodeDataType == DB_UINT16_TYPE)
		{
			if (pBuffer!=0)
			{
				dbMemcpyFreeIndian(pRecord, pBuffer, DB_UINT16_SIZE, 1);
			}
			pRecord += DB_UINT16_SIZE;
		}
		else if (columnNode.columnNodeDataType == DB_UINT32_TYPE)
		{
			if (pBuffer!=0)
			{
				dbMemcpyFreeIndian(pRecord, pBuffer, DB_UINT32_SIZE, 1);
			}
			pRecord += DB_UINT32_SIZE;
		}
		else{}

		columnStartOffset = columnNode.columnNodeNextColumnOffset;
	}
	return 1;
}
//====================================================================================//
//================= 			������ ������ PC���� �ʿ�� �ϴ� �κ���(e n d)			====================//
//====================================================================================//


//====================================================================================//
//================= 			������ ������ ������� Ȯ�� �κ���(begin)			 =======================//
//====================================================================================//
/*unsigned long dbApiSearchResultSort(DB_VARCHAR		*tableName,
										DB_VARCHAR		*sortColumnName,
										unsigned char	isAsc,
										unsigned long	resultSet[],
										unsigned long	resultSetNum,
										unsigned long	sortedSet[])
{
	unsigned long recordNumber;
}*/
//====================================================================================//
//================= ������ ������ ������� Ȯ�� �κ���(e n d)=======================//
//====================================================================================//
unsigned long dbFindRowidIndexInResultArray(unsigned long *pArrayStart, unsigned long vStartIndex, unsigned long vArraySize, unsigned long vFindRowid)
{
	unsigned long * pArrayCurrent;// = pArrayStart + vStartIndex;

//	DbgPrintf(("[0x%x] �� �񱳸� �����Ѵ�. : ", vFindRowid));
	
	while(1)
	{
		pArrayCurrent = pArrayStart + vStartIndex;


//		DbgPrintf(("[0x%x] ", *pArrayCurrent));

		if ((*pArrayCurrent) == vFindRowid)
		{
			return vStartIndex;
		}

		if ((vStartIndex+1) >= vArraySize)
		{
			return 0xffffffff;
		}

		
		else
		{
			vStartIndex++;
			pArrayCurrent++;
		}
		
	}
//	DbgPrintf(("\n"));

}

void dbIndexTreeAscSortTraverse(unsigned long vNodeOffset,  
									unsigned long* pResultArray, 
									unsigned long vResultArrayTotal,
									unsigned long *pSortedIndex)
{
	if (vNodeOffset==0)
	{
		return;
	}
	else
	{
		pIndexNode	pInode;
		IndexNode	INDEX_NODE;

		pInode = dbIndexGetNode(vNodeOffset);
		dbMemcpyFreeIndian(pInode, &INDEX_NODE, sizeof(unsigned long), 4);

		dbIndexTreeAscSortTraverse(INDEX_NODE.indexNodeLeft, pResultArray, vResultArrayTotal, pSortedIndex);

		{
			unsigned long	vSearchIndex;
			pLinkNode	pLink;
			LinkNode		LINK_NODE;
			unsigned long	linkOffset;

			linkOffset = INDEX_NODE.indexNodeLinkOffset;

//			DbgPrintf(("Name�� Sorting Rowid : "));

			while(linkOffset)
			{
				pLink = dbIndexGetLink(linkOffset);
				dbMemcpyFreeIndian(pLink, &LINK_NODE, sizeof(unsigned long), 2);
//				DbgPrintf((" 0x%x,  ",  LINK_NODE.linkNodeRecordRowid));

				if (((*pSortedIndex)+1)>=vResultArrayTotal) return;

				vSearchIndex = dbFindRowidIndexInResultArray(pResultArray, 
															*(pSortedIndex), 
															vResultArrayTotal,
															LINK_NODE.linkNodeRecordRowid);
				if (vSearchIndex != 0xffffffff)
				{
					unsigned long vBackupRowid;

//					DbgPrintf(("\n ã�Ҵ�!! \n"));
//					DbgPrintf(("%d��°�� %d��° ������ ��ġ�� �ٲ۴�.\n", *pSortedIndex, vSearchIndex));
					
					vBackupRowid 					= *(pResultArray+(*pSortedIndex));
					*(pResultArray+(*pSortedIndex))	= *(pResultArray+vSearchIndex);
					*(pResultArray+vSearchIndex)		= vBackupRowid;
					
					(*pSortedIndex)++;
				}
				
				linkOffset =  LINK_NODE.linkNodeNext;
			}
//			DbgPrintf(("\n"));
		}

		dbIndexTreeAscSortTraverse(INDEX_NODE.indexNodeRight, pResultArray, vResultArrayTotal, pSortedIndex);
	}

}

void dbIndexTreeDescSortTraverse(unsigned long vNodeOffset,  
									unsigned long* pResultArray, 
									unsigned long vResultArrayTotal,
									unsigned long *pSortedIndex)
{
	if (vNodeOffset==0)
	{
		return;
	}
	else
	{
		pIndexNode	pInode;
		IndexNode	INDEX_NODE;

		pInode = dbIndexGetNode(vNodeOffset);
		dbMemcpyFreeIndian(pInode, &INDEX_NODE, sizeof(unsigned long), 4);

		dbIndexTreeDescSortTraverse(INDEX_NODE.indexNodeRight, pResultArray, vResultArrayTotal, pSortedIndex);

		{
			unsigned long	vSearchIndex;
			pLinkNode	pLink;
			LinkNode		LINK_NODE;
			unsigned long	linkOffset;

			linkOffset = INDEX_NODE.indexNodeLinkOffset;

			
			while(linkOffset)
			{
				pLink = dbIndexGetLink(linkOffset);
				dbMemcpyFreeIndian(pLink, &LINK_NODE, sizeof(unsigned long), 2);

				if (((*pSortedIndex)+1)>=vResultArrayTotal) return;

				vSearchIndex = dbFindRowidIndexInResultArray(pResultArray, 
															(*pSortedIndex), 
															vResultArrayTotal,
															LINK_NODE.linkNodeRecordRowid);
				if (vSearchIndex != 0xffffffff)
				{
					unsigned long vBackupRowid;
					
					vBackupRowid 					= *(pResultArray+(*pSortedIndex));
					*(pResultArray+(*pSortedIndex))	= *(pResultArray+vSearchIndex);
					*(pResultArray+vSearchIndex)		= vBackupRowid;
					
					(*pSortedIndex)++;
				}
				
				linkOffset =  LINK_NODE.linkNodeNext;
			}
		}

		dbIndexTreeDescSortTraverse(INDEX_NODE.indexNodeLeft, pResultArray, vResultArrayTotal, pSortedIndex);
	}

}

int dbApiResultArraySorting(	unsigned short	*pTableName, 
								unsigned short	*pColumnName, 
								int				vAscOrDesc,
								unsigned long		*pResultArray, 
								unsigned long		vResultArryTotal)
{
	if (gDbStartUpDB == DB_DO_RUNNING)
	{
		pDictionaryHeader		pdictionaryHeader;	//Dictionary file�� �ش��� �������� ����
		pTableNode			pTable;				//Dictionary file�� �ö���� ���������� ���̺��� �������� ����
		TableNode			TABLE_NODE;
		pColumnNode			pColumn;
		unsigned long			*ptableOffset, vTableOffset;
		unsigned long			*pcolumnOffset, vColumnOffset;
		unsigned long			indexOffset;
		
		//���̺��� ã�´�.
		pdictionaryHeader	= dbGetDictionaryHeader();							//Dictionary�� �����Ѵ�.
		ptableOffset		= &(pdictionaryHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����
		
		if (!dbTableFindTableName(&ptableOffset, pTableName))
		{
			//ERROR : �Է����� ���� dbTableName�� table�� �ý��ۿ� ���������ʴ� ���
			#if DB_SHOW_ERROR_CODE												//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_TABLE_NOT_FOUND;		//Error Check
			#endif																	//Error Check
			
			return DB_FUNCTION_FAIL;
		}
		// ===========�Ʒ����� �ش� ���̺��� ã������ ó�� ===========//

		//table�� offset�� ����Ǿ� �ִ�.
		dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

		//table�� �޸𸮸� �����Ѵ�.
		pTable = (pTableNode)(((char*)pdictionaryHeader) + vTableOffset);
		//���̺��� ������ TABLE_NODE�� �����Ѵ�.
		dbMemcpyFreeIndian(pTable, &TABLE_NODE, sizeof(unsigned long), 6);

		if (TABLE_NODE.tableNodeStartPage == 0)
		{
			//�����ͻ����� �����ٸ� ���̻� �˻��Ұ͵� ����..
			#if DB_SHOW_ERROR_CODE							//Error Check
			gDbErrorCode = DB_ERROR_TABLE_ALLOC_PAGE_NONE;	//Error Check
			#endif												//Error Check
			
			return DB_FUNCTION_FAIL;
		}


		//�÷��� ���� offset�� ����Ǿ� �ִ� �ּҸ� �������Ѵ�.
		pcolumnOffset = &(pTable->tableNodeColumnOffset);

		//�÷��� ã�´�.
		if(!dbTableFindColumnName(&pcolumnOffset, pColumnName))
		{
			//ERROR : �ش� �÷��� �������� �ʴ� ���
			#if DB_SHOW_ERROR_CODE												//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_COLUMN_NOT_FOUND;	//Error Check
			#endif																	//Error Check
			
			return 0;
		}

		//�÷��� offset�� ����Ǿ� �ִ�.
		dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);

		pColumn =  (pColumnNode)(((char*)pdictionaryHeader) + vColumnOffset);
		dbMemcpyFreeIndian(&(pColumn->columnNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);	//indexOffset���� �ε����� ���� offset�� ���� �����Ѵ�.

		//��ȸ �ϸ鼭 Sorting�� �ǽ��Ѵ�.
		{
			unsigned long vSortedIndex = 0;
			if (vAscOrDesc==DB_ASC)
			{
				dbIndexTreeAscSortTraverse(indexOffset, pResultArray, vResultArryTotal, &vSortedIndex);
			}
			else
			{
				dbIndexTreeDescSortTraverse(indexOffset, pResultArray, vResultArryTotal, &vSortedIndex);	
			}
		}
		return DB_FUNCTION_SUCCESS;
	}
	else
	{
		return DB_FUNCTION_FAIL;
	}
}

unsigned long dbApiSearchMSAllRecords(DB_VARCHAR*	dbTableName,
										DB_VARCHAR*	indexColumnName[],
										unsigned long		indexColumnNumber,
										void*			conditionSet[],
										unsigned long		conditionColumnNumber,
										unsigned char		traverseOrder,
										unsigned long		resultArray[])
{
	pDictionaryHeader		pdictionaryHeader;	//Dictionary file�� �ش��� �������� ����
	pTableNode			pTable;				//Dictionary file�� �ö���� ���������� ���̺��� �������� ����
	TableNode			TABLE_NODE;
	pColumnNode			pColumn;
	unsigned long			*ptableOffset, vTableOffset;
	unsigned long			*pcolumnOffset, vColumnOffset;
	unsigned long			indexOffset;
	unsigned char 		vDepth;

	#if DB_SHOW_ERROR_CODE				//Error Check
	gDbErrorCode = DB_ERROR_NO_ERROR;		//Error Check
	#endif									//Error Check
	
	//Database�� ������������ üũ�Ѵ�.
	if (gDbStartUpDB != DB_DO_RUNNING)
	{
		#if DB_SHOW_ERROR_CODE								//Error Check
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;		//Error Check
		#endif													//Error Check
		
		return 0;
	}
	else
	{
		//���� Parameter�� ������������ üũ�Ѵ�.
		if (indexColumnNumber <conditionColumnNumber)
		{
			#if DB_SHOW_ERROR_CODE															//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_CONDITION_PARAMETER_INCORRECT;	//Error Check
			#endif																				//Error Check

			return 0;
		}

		//������������ �������� ������������ �������� ���޹��� ���ڰ� �������� �Ǵ��Ѵ�.
		if ((traverseOrder != DB_ASC) && (traverseOrder != DB_DESC))
		{
			#if DB_SHOW_ERROR_CODE											//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_ORDER_INVALID;	//Error Check
			#endif																//Error Check

			traverseOrder = DB_ASC;
		}

		

		//���̺��� ã�´�.
		pdictionaryHeader	= dbGetDictionaryHeader();							//Dictionary�� �����Ѵ�.
		ptableOffset		= &(pdictionaryHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����
		
		if (!dbTableFindTableName(&ptableOffset, dbTableName))
		{
			//ERROR : �Է����� ���� dbTableName�� table�� �ý��ۿ� ���������ʴ� ���
			#if DB_SHOW_ERROR_CODE												//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_TABLE_NOT_FOUND;		//Error Check
			#endif																	//Error Check
			
			return 0;
		}
		// ===========�Ʒ����� �ش� ���̺��� ã������ ó�� ===========//

		//table�� offset�� ����Ǿ� �ִ�.
		dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

		//table�� �޸𸮸� �����Ѵ�.
		pTable = (pTableNode)(((char*)pdictionaryHeader) + vTableOffset);
		//���̺��� ������ TABLE_NODE�� �����Ѵ�.
		dbMemcpyFreeIndian(pTable, &TABLE_NODE, sizeof(unsigned long), 6);

		if (TABLE_NODE.tableNodeStartPage == 0)
		{
			//�����ͻ����� �����ٸ� ���̻� �˻��Ұ͵� ����..
			#if DB_SHOW_ERROR_CODE							//Error Check
			gDbErrorCode = DB_ERROR_TABLE_ALLOC_PAGE_NONE;	//Error Check
			#endif												//Error Check
			
			return 0;
		}

		//�ε����� ���� offset�� ��ġ�� ã�Ƴ���.
		{
			//�����ε����� ��� �ε����� ���� offset�� ��ġ�� ã�Ƴ���.
			if (indexColumnNumber == 1)	
			{
				//�÷��� ���� offset�� ����Ǿ� �ִ� �ּҸ� �������Ѵ�.
				pcolumnOffset = &(pTable->tableNodeColumnOffset);

				//�÷��� ã�´�.
				if(!dbTableFindColumnName(&pcolumnOffset, indexColumnName[0]))
				{
					//ERROR : �ش� �÷��� �������� �ʴ� ���
					#if DB_SHOW_ERROR_CODE												//Error Check
					gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_COLUMN_NOT_FOUND;	//Error Check
					#endif																	//Error Check
					
					return 0;
				}

				//�÷��� offset�� ����Ǿ� �ִ�.
				dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);

				pColumn =  (pColumnNode)(((char*)pdictionaryHeader) + vColumnOffset);

				dbMemcpyFreeIndian(&(pColumn->columnNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);	//indexOffset���� �ε����� ���� offset�� ���� �����Ѵ�.
			}
			//�����ε����� ��� �ε����� ���� offset�� ��ġ�� ã�Ƴ���.
			else
			{
				unsigned long			complexOffset;
				pComplexIndexNode	pComplexIndex;
				ComplexIndexNode	complexIndex;

				//complexOffset�� �����ε����� ���� offset�� ���� �����Ѵ�.
				dbMemcpyFreeIndian(&(pTable->tableNodeComplexIndexOffset), &complexOffset, sizeof(unsigned long), 1);

				//��� �����ε��� �÷��� ���������� �湮�ϸ鼭 ã�´�.
				while(complexOffset)
				{
					pComplexIndex = (pComplexIndexNode)(((char*)pdictionaryHeader) + complexOffset);
					dbMemcpyFreeIndian(pComplexIndex, &complexIndex, sizeof(unsigned long), 3);

					if (complexIndex.complexIndexNodeColumnNumber == indexColumnNumber)
					{
						if (dbIndexCheckName((DB_VARCHAR*)(pComplexIndex + 1) , indexColumnName, indexColumnNumber) != 0)
						{
							//�ش� �����ε����� ã�����.
							break;
						}
					}

					complexOffset = complexIndex.complexIndexNodeNext;
				}

				if (complexOffset == DB_NULL)
				{
					#if DB_SHOW_ERROR_CODE											//Error Check
					gDbErrorCode = DB_ERROR_DBAPISEARCHMS_COMPLEXINDEX_NOT_FOUND;	//Error Check
					#endif																//Error Check

					return 0;
				}

				dbMemcpyFreeIndian(&(pComplexIndex->complexIndexNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);	//indexOffset���� �ε����� ���� offset�� ���� �����Ѵ�.
			}
		}
		//indexOffset���� �ε����� ���� offset�� ���� ����Ǿ��ִ�.


		vDepth = indexColumnNumber - conditionColumnNumber;

		//�ε��� Ž���� �����Ѵ�.
		if (conditionColumnNumber == 0)
		{
			dbSearchArray = resultArray;
			dbSearchCount = 0;

			dbIndexTreeTraverseAll(indexOffset, traverseOrder, vDepth);	//����� �����ϴ� �κ�
		}
		else
		{
			unsigned long	i;
			unsigned long	columnDataType;
			for (i = 0 ; i < conditionColumnNumber ; i++)
			{
				//�÷��� ���� offset�� ����Ǿ� �ִ� �ּҸ� �������Ѵ�.
				pcolumnOffset = &(pTable->tableNodeColumnOffset);
				
				//ã�� �÷��� offset�� ���Ѵ�.
				dbTableFindColumnName(&pcolumnOffset, indexColumnName[i]);
				dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);
							
				
				//�ش� �÷��� �������Ѵ�.
				pColumn =  (pColumnNode)(((char*)pdictionaryHeader) + vColumnOffset);
				
				//�÷��� dataType�� �����Ѵ�.
				dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &columnDataType, sizeof(unsigned long), 1);
					
				
				//�˻��� �ǽ��Ѵ�.
				indexOffset = dbIndexSearchGetInodeMS(indexOffset, columnDataType, conditionSet[i]);
				
				if (indexOffset == 0) return 0;
				
				//�˻��� ��� �ٸ� Ʈ���� rootOffset�� �˻��ȴ�.
				

				if (( (conditionColumnNumber - 1) == i ))		//�������� �ѹ��� �Ͼ��. ����� �����ϴ� �κ�
				{
					dbSearchArray = resultArray;
					dbSearchCount = 0;

					dbIndexTreeTraverseAll(indexOffset, traverseOrder, vDepth);
				}
			}
		}
		return dbSearchCount;
	}

}

int dbApiSearchMSRemainRecords(unsigned long* pRemainRecordsNumber)
{
	if (gDbStartUpDB != DB_DO_RUNNING)
	{
		#if DB_SHOW_ERROR_CODE								//Error Check
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;		//Error Check
		#endif													//Error Check
		
		return DB_FUNCTION_FAIL;
	}
	else
	{
		*pRemainRecordsNumber = gDbSearchData.vRemainRecordsCount;
		return DB_FUNCTION_SUCCESS;
	}
	
}
unsigned long dbApiSearchMS(DB_VARCHAR*	dbTableName,
						    	    DB_VARCHAR*	indexColumnName[],
							    unsigned long	indexColumnNumber,
							    void*			conditionSet[],
							    unsigned long	conditionColumnNumber,
							    unsigned char	traverseOrder,
							    unsigned long	resultArray[],
							    #ifdef DB_SEARCH_CHANGED
							    unsigned long	resultArraySize,
							    #endif
							    unsigned char	*refMemory)
{
	pDictionaryHeader		pdictionaryHeader;	//Dictionary file�� �ش��� �������� ����
	pTableNode			pTable;				//Dictionary file�� �ö���� ���������� ���̺��� �������� ����
	TableNode			TABLE_NODE;
	pColumnNode			pColumn;
	unsigned long			*ptableOffset, vTableOffset;
	unsigned long			*pcolumnOffset, vColumnOffset;
	unsigned long			indexOffset;

	#if DB_SHOW_ERROR_CODE				//Error Check
	gDbErrorCode = DB_ERROR_NO_ERROR;		//Error Check
	#endif									//Error Check
	
	//Database�� ������������ üũ�Ѵ�.
	if (gDbStartUpDB != DB_DO_RUNNING)
	{
		#if DB_SHOW_ERROR_CODE								//Error Check
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;		//Error Check
		#endif													//Error Check
		
		return 0;
	}
	else
	{
		gDbSearchData.dbSearchCount			= 0;
		gDbSearchData.vRemainRecordsCount 	= 0;
		gDbSearchData.ResultArray			= resultArray;
		gDbSearchData.ResultArraySize			= resultArraySize;

		//���� Parameter�� ������������ üũ�Ѵ�.
		if (indexColumnNumber <conditionColumnNumber)
		{
			#if DB_SHOW_ERROR_CODE															//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_CONDITION_PARAMETER_INCORRECT;	//Error Check
			#endif																				//Error Check

			return 0;
		}

		//������������ �������� ������������ �������� ���޹��� ���ڰ� �������� �Ǵ��Ѵ�.
		if ((traverseOrder != DB_ASC) && (traverseOrder != DB_DESC))
		{
			#if DB_SHOW_ERROR_CODE											//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_ORDER_INVALID;	//Error Check
			#endif																//Error Check

			traverseOrder = DB_ASC;
		}

		

		//���̺��� ã�´�.
		pdictionaryHeader	= dbGetDictionaryHeader();							//Dictionary�� �����Ѵ�.
		ptableOffset		= &(pdictionaryHeader->dictionaryHeaderOffsetOfRootTable);	//table�� �����ϴ� Ʈ���� ���� offset�� �����ϴ� ��ġ�� ����
		
		if (!dbTableFindTableName(&ptableOffset, dbTableName))
		{
			//ERROR : �Է����� ���� dbTableName�� table�� �ý��ۿ� ���������ʴ� ���
			#if DB_SHOW_ERROR_CODE												//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_TABLE_NOT_FOUND;		//Error Check
			#endif																	//Error Check
			
			return 0;
		}
		// ===========�Ʒ����� �ش� ���̺��� ã������ ó�� ===========//

		//table�� offset�� ����Ǿ� �ִ�.
		dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

		//table�� �޸𸮸� �����Ѵ�.
		pTable = (pTableNode)(((char*)pdictionaryHeader) + vTableOffset);
		//���̺��� ������ TABLE_NODE�� �����Ѵ�.
		dbMemcpyFreeIndian(pTable, &TABLE_NODE, sizeof(unsigned long), 6);

		if (TABLE_NODE.tableNodeStartPage == 0)
		{
			//�����ͻ����� �����ٸ� ���̻� �˻��Ұ͵� ����..
			#if DB_SHOW_ERROR_CODE							//Error Check
			gDbErrorCode = DB_ERROR_TABLE_ALLOC_PAGE_NONE;	//Error Check
			#endif												//Error Check
			
			return 0;
		}

		//�ε����� ���� offset�� ��ġ�� ã�Ƴ���.
		{
			//�����ε����� ��� �ε����� ���� offset�� ��ġ�� ã�Ƴ���.
			if (indexColumnNumber == 1)	
			{
				//�÷��� ���� offset�� ����Ǿ� �ִ� �ּҸ� �������Ѵ�.
				pcolumnOffset = &(pTable->tableNodeColumnOffset);

				//�÷��� ã�´�.
				if(!dbTableFindColumnName(&pcolumnOffset, indexColumnName[0]))
				{
					//ERROR : �ش� �÷��� �������� �ʴ� ���
					#if DB_SHOW_ERROR_CODE												//Error Check
					gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_COLUMN_NOT_FOUND;	//Error Check
					#endif																	//Error Check
					
					return 0;
				}

				//�÷��� offset�� ����Ǿ� �ִ�.
				dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);

				pColumn =  (pColumnNode)(((char*)pdictionaryHeader) + vColumnOffset);
				
				dbMemcpyFreeIndian(&(pColumn->columnNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);	//indexOffset���� �ε����� ���� offset�� ���� �����Ѵ�.
			}
			//�����ε����� ��� �ε����� ���� offset�� ��ġ�� ã�Ƴ���.
			else
			{
				unsigned long			complexOffset;
				pComplexIndexNode	pComplexIndex;
				ComplexIndexNode	complexIndex;

				//complexOffset�� �����ε����� ���� offset�� ���� �����Ѵ�.
				dbMemcpyFreeIndian(&(pTable->tableNodeComplexIndexOffset), &complexOffset, sizeof(unsigned long), 1);

				//��� �����ε��� �÷��� ���������� �湮�ϸ鼭 ã�´�.
				while(complexOffset)
				{
					pComplexIndex = (pComplexIndexNode)(((char*)pdictionaryHeader) + complexOffset);
					dbMemcpyFreeIndian(pComplexIndex, &complexIndex, sizeof(unsigned long), 3);

					if (complexIndex.complexIndexNodeColumnNumber == indexColumnNumber)
					{
						if (dbIndexCheckName((DB_VARCHAR*)(pComplexIndex + 1) , indexColumnName, indexColumnNumber) != 0)
						{
							//�ش� �����ε����� ã�����.
							break;
						}
					}

					complexOffset = complexIndex.complexIndexNodeNext;
				}

				if (complexOffset == DB_NULL)
				{
					#if DB_SHOW_ERROR_CODE											//Error Check
					gDbErrorCode = DB_ERROR_DBAPISEARCHMS_COMPLEXINDEX_NOT_FOUND;	//Error Check
					#endif																//Error Check

					return 0;
				}

				dbMemcpyFreeIndian(&(pComplexIndex->complexIndexNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);	//indexOffset���� �ε����� ���� offset�� ���� �����Ѵ�.
			}
		}
		//indexOffset���� �ε����� ���� offset�� ���� ����Ǿ��ִ�.


		//�ε��� Ž���� �����Ѵ�.
		if (conditionColumnNumber == 0)
		{
			#if 0
			dbSearchArray = resultArray;
			dbSearchCount = 0;
			#endif
			
			if (indexColumnNumber == (conditionColumnNumber + 1))
			{
				if ((indexOffset != 0)&&(indexOffset != 1))
				{
					//������������ recordRowid�� ����� ������ ���
					*refMemory = DB_DATA_FILE_LOAD;
					gDbSearchData.refMemory = DB_DATA_FILE_LOAD;
					dbIndexTreeTraverse(indexOffset, traverseOrder, 1);	//����� �����ϴ� �κ�
				}
			}
			else
			{
				if ((indexOffset != 0)&&(indexOffset != 1))
				{
					//�˻��� ����� �ε��� ������ �����̴�.
					*refMemory = DB_INDEX_FILE_LOAD;
					gDbSearchData.refMemory = DB_INDEX_FILE_LOAD;
					dbIndexTreeTraverse(indexOffset, traverseOrder, 0);	//����� �����ϴ� �κ�
				}
			}
		}
		else
		{
			unsigned long	i;
			unsigned long	columnDataType;
			for (i = 0 ; i < conditionColumnNumber ; i++)
			{
				//�÷��� ���� offset�� ����Ǿ� �ִ� �ּҸ� �������Ѵ�.
				pcolumnOffset = &(pTable->tableNodeColumnOffset);
				
				//ã�� �÷��� offset�� ���Ѵ�.
				dbTableFindColumnName(&pcolumnOffset, indexColumnName[i]);
				dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);
							
				
				//�ش� �÷��� �������Ѵ�.
				pColumn =  (pColumnNode)(((char*)pdictionaryHeader) + vColumnOffset);
				
				//�÷��� dataType�� �����Ѵ�.
				dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &columnDataType, sizeof(unsigned long), 1);
					
				if ((indexOffset == 0)||(indexOffset == 1)) return 0;
				
				//�˻��� �ǽ��Ѵ�.
				indexOffset = dbIndexSearchGetInodeMS(indexOffset, columnDataType, conditionSet[i]);
				
				if ((indexOffset == 0)||(indexOffset == 1)) return 0;
				
				//�˻��� ��� �ٸ� Ʈ���� rootOffset�� �˻��ȴ�.
				

				if (( (conditionColumnNumber - 1) == i ) && (indexColumnNumber != conditionColumnNumber))		//�������� �ѹ��� �Ͼ��. ����� �����ϴ� �κ�
				{
					#if 0
					dbSearchArray = resultArray;
					dbSearchCount = 0;
					#endif
					
					if (indexColumnNumber == (conditionColumnNumber + 1))
					{
						//������������ recordRowid�� ����� ������ ���
						*refMemory = DB_DATA_FILE_LOAD;
						gDbSearchData.refMemory = DB_DATA_FILE_LOAD;
						dbIndexTreeTraverse(indexOffset, traverseOrder, 1);
					}
					else
					{
						//�˻��� ����� �ε��� ������ �����̴�.
						*refMemory = DB_INDEX_FILE_LOAD;
						gDbSearchData.refMemory = DB_INDEX_FILE_LOAD;
						dbIndexTreeTraverse(indexOffset, traverseOrder, 0);
					}
				}
				else if(( (conditionColumnNumber - 1) == i ) && (indexColumnNumber == conditionColumnNumber))		//�������� �ѹ��� �Ͼ��. ����� �����ϴ� �κ�
				{
					#if 0
					dbSearchArray = resultArray;
					dbSearchCount = 0;
					#endif
					//�˻��� ����� �ε��� ������ �����̴�.
					*refMemory = DB_INDEX_FILE_LOAD;
					gDbSearchData.refMemory = DB_INDEX_FILE_LOAD;

					if ((indexOffset != 0)&&(indexOffset != 1))
					{
						dbIndexLinkTraverse(indexOffset);
					}
				}
			}
		}
		return gDbSearchData.dbSearchCount;
	}
}


unsigned long dbIndexSearchGetInodeMS(unsigned long	rootOffset,
										unsigned long	dataType,
										void*			keyValue)
{
	pIndexNode				pInode;
	IndexNode				INDEX_NODE;
	signed		char		compareResult;

	while(rootOffset)
	{
		pInode = dbIndexGetNode(rootOffset);

		dbMemcpyFreeIndian(pInode, &INDEX_NODE, sizeof(unsigned long), 4);

		//��
		compareResult = dbIndexCompareKeyAreaVsStackMS(pInode + 1, keyValue, dataType);

		if (compareResult == 0)
		{
			return INDEX_NODE.indexNodeLinkOffset;
		}
		else if(compareResult == -1)
		{
			rootOffset = INDEX_NODE.indexNodeLeft;
		}
		else
		{
			rootOffset = INDEX_NODE.indexNodeRight;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//																								//
//									File ���� ó�� �κ�(Being)									//
//																								//
//////////////////////////////////////////////////////////////////////////////////////////////////
#if ANSI_BASE_STAND_IO

#include	"stdio.h"

FILE			*gDbFileDictionaryFile = 0, 
			*gDbFileDataFile = 0, 
			*gDbFileIndexFile = 0;


void dbSetFileLoadMemorypage(unsigned long	filePageNumber,
							 	     char			*targetMemoryAddress, 
								     unsigned long	fileType)
{
	pDictionaryHeader	pDbDictionaryHeader;
	DictionaryHeader	dbDictionaryHeader;

	pDbDictionaryHeader = dbGetDictionaryHeader();
	dbMemcpyFreeIndian(pDbDictionaryHeader, &dbDictionaryHeader, sizeof(unsigned long), 6);
	

	if (fileType == DB_DATA_FILE_LOAD)
	{//Data File Load�� �õ��� ��� ó��

		if (filePageNumber <= dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage)
		{//�� ��� Data File�� �����ϴ� Page�� �䱸�� ��� ó��

			unsigned long vReadingSize = 0;

			//============ 1. file�� ����. (Begin) ============//
			{
				if (gDbFileDataFile)
				{					
					fclose(gDbFileDataFile);
					gDbFileDataFile = 0;
				}

				gDbFileDataFile = fopen(gDbFileNameDataFileName, "r+b");

				if (gDbFileDataFile==0)
				{
					#if DB_SHOW_ERROR_CODE															//Error Check
					gDbErrorCode = DB_ERROR_DATA_FILE_OPEN_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION;	//Error Check
					#endif

					fclose(gDbFileDataFile);
					gDbFileDataFile = 0;
					return;
				}
			}
			//============ 1. file�� ����. (E n d) ============//


			//============ 2. file�� �д´�. (Begin) ============//
			{
				fseek(gDbFileDataFile, (filePageNumber - 1)*gDbPageSizeDataFilePageSize, SEEK_SET);
				vReadingSize = fread(targetMemoryAddress, gDbPageSizeDataFilePageSize, 1, gDbFileDataFile);
			}
			//============ 2. file�� �д´�. (E n d) ============//


			//============ 3. file�� �ݴ´�. (Begin) ============//
			{
				fclose(gDbFileDataFile);
				gDbFileDataFile = 0;
			}
			//============ 3. file�� �ݴ´�. (E n d) ============//
				
			if (vReadingSize != 1)
			{//�� ��� ����� �� ���� ��� ó��
				#if DB_SHOW_ERROR_CODE															//Error Check
				gDbErrorCode = DB_ERROR_DATA_FILE_READ_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION;	//Error Check
				#endif

				//============ 1. file�� ����. (Begin) ============//
				{

					if (gDbFileDataFile)
					{
						fclose(gDbFileDataFile);
						gDbFileDataFile = 0;
					}
				
					gDbFileDataFile = fopen(gDbFileNameDataFileName, "r+b");
					
					if (gDbFileDataFile==0)
					{
						#if DB_SHOW_ERROR_CODE															//Error Check
						gDbErrorCode = DB_ERROR_DATA_FILE_OPEN_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION;	//Error Check
						#endif

						fclose(gDbFileDataFile);
						gDbFileDataFile = 0;
						return;
					}
				}
				//============ 1. file�� ����. (E n d) ============//
				
				//============ 2. file�� �д´�. (Begin) ============//
				{
					fseek(gDbFileDataFile, (filePageNumber - 1)*gDbPageSizeDataFilePageSize, SEEK_SET);
					vReadingSize = fread(targetMemoryAddress, gDbPageSizeDataFilePageSize, 1, gDbFileDataFile);
				}
				//============ 2. file�� �д´�. (E n d) ============//
				
				//============ 3. file�� �ݴ´�. (Begin) ============//
				{
					fclose(gDbFileDataFile);
					gDbFileDataFile = 0;
				}
				//============ 3. file�� �ݴ´�. (E n d) ============//
			}

			
			if (vReadingSize != 1)
			{//File�� �ٽ� �б⸦ �õ������� ���и� �� ���
				#if DB_SHOW_ERROR_CODE															//Error Check
				gDbErrorCode = DB_ERROR_DATA_FILE_READ_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION;	//Error Check
				#endif
				printf("File Read Error");
			}
			
			gDbMmuDataPage->mmuChanged = DB_UNCHANGED;
		}
		else
		{//�� ��� Data File�� �������� �ʴ� Page�� �䱸�� ��� ó��.

			pDataFilePageHeader pDH = (pDataFilePageHeader)targetMemoryAddress;
			
			DataFilePageHeader	DH;
			
			//============= 1. �ε��� DataPage�� Memory�� �ʱ�Header���� �����Ѵ�. (Begin) =============//
			{
				DH.dataHeaderDeleteRecordOffset	= 0;
				DH.dataHeaderEndOfRecords		= sizeof(DataFilePageHeader);
				DH.dataHeaderNextSameTablePage	= 0;
				DH.dataHeaderNumberOfSlots		= 0;

				dbMemcpyFreeIndian(&DH, pDH, sizeof(unsigned long), 4);
			}
			//============= 1. �ε��� DataPage�� Memory�� �ʱ�Header���� �����Ѵ�. (E n d) =============//


			//============= 2. ���� �߰��� Data Page �̹Ƿ� Dictionary Header�� �����Ѵ�. (Being) ===========//
			{
				if (dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage < filePageNumber)
				{
					//dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage = filePageNumber;
					dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage++;

					dbMemcpyFreeIndian(&dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage, 
									  &(pDbDictionaryHeader->dictionaryHeaderNumberOfDataFileEndPage), sizeof(unsigned long), 1);
				
					gDbDictionaryChanged = DB_CHANGED;
				}
				gDbMmuDataPage->mmuChanged = DB_CHANGED;
			}		
			//============= 2. ���� �߰��� Data Page �̹Ƿ� Dictionary Header�� �����Ѵ�. (E n d) ===========//
		}		
	}
	else if(fileType == DB_INDEX_FILE_LOAD)
	{//IndexFile Load�� �õ��� ��� ó��
		
		if (filePageNumber <= dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage)
		{//�� ��� Index File�� �����ϴ� Page�� �䱸�� ��� ó��

			unsigned long vReadingSize = 0;

			//============ 1. file�� ����. (Begin) ============//
			{
				if (gDbFileIndexFile)
				{
					fclose(gDbFileIndexFile);
					gDbFileIndexFile = 0;
				}
				
				gDbFileIndexFile = fopen(gDbFileNameIndexFileName, "r+b");

				if (gDbFileIndexFile == 0)
				{
					#if DB_SHOW_ERROR_CODE															//Error Check
					gDbErrorCode = DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION;	//Error Check
					#endif
					fclose(gDbFileIndexFile);
					gDbFileIndexFile = 0;
					return ;
				}
			}
			//============ 1. file�� ����. (E n d) ============//
			

			//============ 2. file�� �д´�. (Begin) ============//
			{
				fseek(gDbFileIndexFile, (filePageNumber - 1)*gDbPageSizeIndexFilePageSize, SEEK_SET);
				vReadingSize = fread(targetMemoryAddress, gDbPageSizeIndexFilePageSize, 1, gDbFileIndexFile);
			}
			//============ 2. file�� �д´�. (E n d) ============//


			//============ 3. file�� �ݴ´�. (Begin) ============//
			{
				fclose(gDbFileIndexFile);
				gDbFileIndexFile = 0;					
			}
			//============ 3. file�� �ݴ´�. (E n d) ============//
				
			if (vReadingSize != 1)
			{//�� ��� ����� �� ���� ��� ó��
				#if DB_SHOW_ERROR_CODE															//Error Check
				gDbErrorCode = DB_ERROR_INDEX_FILE_READ_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION;	//Error Check
				#endif																				//Error Check

				//============ 1. file�� ����. (Begin) ============//
				{
					if (gDbFileIndexFile)
					{
						fclose(gDbFileIndexFile);
						gDbFileIndexFile = 0;					
					}
					gDbFileIndexFile = fopen(gDbFileNameIndexFileName, "r+b");

					if (gDbFileIndexFile == 0)
					{
						#if DB_SHOW_ERROR_CODE															//Error Check
						gDbErrorCode = DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION;	//Error Check
						#endif																				//Error Check

						fclose(gDbFileIndexFile);
						gDbFileIndexFile = 0;
						return ;
					}					
				}
				//============ 1. file�� ����. (E n d) ============//
				
				//============ 2. file�� �д´�. (Begin) ============//
				{
					fseek(gDbFileIndexFile, (filePageNumber - 1)*gDbPageSizeIndexFilePageSize, SEEK_SET);
					vReadingSize = fread(targetMemoryAddress, gDbPageSizeIndexFilePageSize, 1, gDbFileIndexFile);
				}
				//============ 2. file�� �д´�. (E n d) ============//

				//============ 3. file�� �ݴ´�. (Begin) ============//
				{
					fclose(gDbFileIndexFile);
					gDbFileIndexFile = 0;
				}
				//============ 3. file�� �ݴ´�. (E n d) ============//

				if (vReadingSize != 1)
				{//File�� �ٽ� �б⸦ �õ������� ���и� �� ���
					#if DB_SHOW_ERROR_CODE															//Error Check
					gDbErrorCode = DB_ERROR_INDEX_FILE_READ_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION;	//Error Check
					#endif																				//Error Check
					printf("File Read Error");
				}
			}
			
			gDbMmuIndexPage->mmuChanged = DB_UNCHANGED;
		}
		else
		{//�� ��� Index File�� �����ϴ� Page�� �䱸�� ��� ó��

			pIndexFilePageHeader pDH = (pIndexFilePageHeader)targetMemoryAddress;
			
			IndexFilePageHeader	 DH;
			
			//============= 1. �ε��� IndexPage�� Memory�� �ʱ�Header���� �����Ѵ�. (Begin) =============//
			{
				DH.indexHeaderDeleteNodeOffset	= 0;
				DH.indexHeaderDeleteLinkOffset	= 0;
				DH.indexHeaderEndOfDatas			= sizeof(IndexFilePageHeader);
			
				dbMemcpyFreeIndian(&DH, pDH, sizeof(unsigned long), 3);
			}
			//============= 1. �ε��� IndexPage�� Memory�� �ʱ�Header���� �����Ѵ�. (E n d) =============//

			//============= 2. ���� �߰��� Index Page �̹Ƿ� Dictionary Header�� �����Ѵ�. (Being) ===========//
			{
				if (dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage < filePageNumber)
				{
					//dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage = filePageNumber;
					dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage++;

					dbMemcpyFreeIndian(&(dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage), 
									  &(pDbDictionaryHeader->dictionaryHeaderNumberOfIndexFileEndPage), sizeof(unsigned long), 1);
				
					gDbDictionaryChanged = DB_CHANGED;
				}
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
			//============= 2. ���� �߰��� Data Page �̹Ƿ� Dictionary Header�� �����Ѵ�. (Being) ===========//
		}
	}
	else{}
}

void dbSetFileStoreMemorypage(unsigned long	filePageNumber, 
							   char				*targetMemoryAddress, 
							   unsigned long	filetype)
{
	if (filetype == DB_DATA_FILE_LOAD)
	{
		unsigned long vWriteCount;

		//============= 1. File�� Open�Ѵ�. (Begin) =============//
		{
			if (gDbFileDataFile)
			{
				fclose(gDbFileDataFile);
				gDbFileDataFile = 0;
			}		
			gDbFileDataFile = fopen(gDbFileNameDataFileName, "r+b");
			
			if (gDbFileDataFile == 0)
			{
				#if DB_SHOW_ERROR_CODE															//Error Check
				gDbErrorCode = DB_ERROR_DATA_FILE_OPEN_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION;	//Error Check
				#endif																				//Error Check

				fclose(gDbFileDataFile);
				gDbFileDataFile = 0;
				return;
			}
		}
		//============= 1. File�� Open�Ѵ�. (E n d) =============//


		//============= 2. File�� Write�Ѵ�. (Begin) =============//
		{
			fseek(gDbFileDataFile, (filePageNumber-1)*gDbPageSizeDataFilePageSize, SEEK_SET);
			vWriteCount = fwrite(targetMemoryAddress, gDbPageSizeDataFilePageSize, 1, gDbFileDataFile);
		}
		//============= 2. File�� Write�Ѵ�. (E n d) =============//

		//============= 3. File�� Close�Ѵ�. (Begin) =============//
		{
			fclose(gDbFileDataFile);
			gDbFileDataFile = 0;
		}
		//============= 3. File�� Close�Ѵ�. (E n d) =============//

		if (vWriteCount != 1 )
		{//��Ͽ� �������� ���Ѱ�� 

			#if DB_SHOW_ERROR_CODE																//Error Check
			gDbErrorCode = DB_ERROR_DATA_FILE_WRITE_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION;	//Error Check
			#endif																					//Error Check
			
			//============= 1. File�� Open�Ѵ�. (Begin) =============//
			{
				if (gDbFileDataFile) 
				{
					fclose(gDbFileDataFile);
					gDbFileDataFile = 0;				
				}			
				gDbFileDataFile = fopen(gDbFileNameDataFileName, "r+b");

				if (gDbFileDataFile == 0)
				{
					#if DB_SHOW_ERROR_CODE															//Error Check
					gDbErrorCode = DB_ERROR_DATA_FILE_OPEN_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION;	//Error Check
					#endif																				//Error Check

					fclose(gDbFileDataFile);
					gDbFileDataFile = 0;
					return;
				}
			}
			//============= 1. File�� Open�Ѵ�. (E n d) =============//
			
			fseek(gDbFileDataFile, (filePageNumber-1)*gDbPageSizeDataFilePageSize, SEEK_SET);

			vWriteCount = fwrite(targetMemoryAddress, gDbPageSizeDataFilePageSize, 1, gDbFileDataFile);

			fclose(gDbFileDataFile);
			gDbFileDataFile = 0;

			if (vWriteCount != 1 )
			{
				#if DB_SHOW_ERROR_CODE																//Error Check
				gDbErrorCode = DB_ERROR_DATA_FILE_WRITE_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION;	//Error Check
				#endif																					//Error Check
				printf("file write error");
			}
		}
		
	}
	else if (filetype == DB_INDEX_FILE_LOAD)
	{
		unsigned long vWriteCount;

		if (gDbFileIndexFile) 
		{
			fclose(gDbFileIndexFile);
			gDbFileIndexFile = 0;
		}
		
		gDbFileIndexFile = fopen(gDbFileNameIndexFileName, "r+b");

		if (gDbFileIndexFile)
		{
			#if DB_SHOW_ERROR_CODE															//Error Check
			gDbErrorCode = DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION;	//Error Check
			#endif																				//Error Check
			fclose(gDbFileIndexFile);
			gDbFileIndexFile = 0;
			return;
		}
		fseek(gDbFileIndexFile, (filePageNumber-1)*gDbPageSizeIndexFilePageSize, SEEK_SET);

		vWriteCount = fwrite(targetMemoryAddress, gDbPageSizeIndexFilePageSize, 1, gDbFileIndexFile);

		fclose(gDbFileIndexFile);
		gDbFileIndexFile = 0;

		if (vWriteCount == 1 )
		{
			//Error���� ��Ͽ� ������ ���
		}
		else
		{
			#if DB_SHOW_ERROR_CODE																//Error Check
			gDbErrorCode = DB_ERROR_INDEX_FILE_WRITE_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION;	//Error Check
			#endif																					//Error Check
			
			//��Ͽ� �������� ���Ѱ�� 
			if (gDbFileIndexFile) 
			{
				fclose(gDbFileIndexFile);
				gDbFileIndexFile = 0;
			}
			
			gDbFileIndexFile = fopen(gDbFileNameIndexFileName, "r+b");

			if (gDbFileIndexFile)
			{
				#if DB_SHOW_ERROR_CODE															//Error Check
				gDbErrorCode = DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION;	//Error Check
				#endif																				//Error Check
				fclose(gDbFileIndexFile);
				gDbFileIndexFile = 0;
				return;
			}
			
			fseek(gDbFileIndexFile, (filePageNumber-1)*gDbPageSizeIndexFilePageSize, SEEK_SET);

			vWriteCount = fwrite(targetMemoryAddress, gDbPageSizeIndexFilePageSize, 1, gDbFileIndexFile);
			if (vWriteCount  != 1)
			{
				#if DB_SHOW_ERROR_CODE																//Error Check
				gDbErrorCode = DB_ERROR_INDEX_FILE_WRITE_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION;	//Error Check
				#endif																					//Error Check
			}
			fclose(gDbFileIndexFile);
			gDbFileIndexFile = 0;
		}
	}
	else
	{
		//error invalid parameter
	}
}

int dbLoadDatabaseFiles(char *dbFileNameDictionaryFileName,
						 char *dbFileNameDataFileName,
						 char *dbFileNameIndexFileName,
						 unsigned long vDictionaryFileSize)
{

	//�� �Լ��� database�� start�ϸ鼭 db���� ����ϴ� ������ �������� �Լ��̴�.
	pDictionaryHeader		pDbDictionaryHeader;
	DictionaryHeader		dbDictionaryHeader;


	//=============================Dictionary File Loading (Begin)=============================//
	if (vDictionaryFileSize == 0)	//����ڰ� ������ ����� ������ ���� �ʾ��� ��� ó��
	{
		// Action 1. ��ųʸ��� �ش��� ������ �ö�� ���� ���� �ʱ��� ��ųʸ� ������ �ø���.
		{
			pDbDictionaryHeader = dbGetDictionaryHeader();

			dbDictionaryHeader.dictionaryHeaderDeleteNode				= 0;
			dbDictionaryHeader.dictionaryHeaderDeleteStartPage				= 0;
			dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage	= 0;
			dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage	= 0;
			dbDictionaryHeader.dictionaryHeaderOffsetOfRootTable			= 0;
			dbDictionaryHeader.dictionaryHeaderSizeOfDictionary			= sizeof(DictionaryHeader);
			

			//��ųʸ��ش��� ���������� ���� ��ųʸ� �ش��� ������ �����Ѵ�.
			dbMemcpyFreeIndian(&dbDictionaryHeader, pDbDictionaryHeader, sizeof(unsigned long), 6);
		}

		// Action 2. ��ųʸ� ������ �����ϸ� �� ������ �ø���.
		{
			unsigned long vReadingSize = 0;

			// Action 2.1 ��ųʸ� ������ Open�Ѵ�.
			{
				if (gDbFileDictionaryFile)
				{
					fclose(gDbFileDictionaryFile);
					gDbFileDictionaryFile = 0;
				}
				
				gDbFileDictionaryFile = fopen(gDbFileNameDictionaryFileName, "a+b");		//��ųʸ� ������ Open�Ѵ�.

				//Error Dictionary File�� Open�Ͽ������� �Ǵ��Ѵ�.
				if (gDbFileDictionaryFile == 0)
				{
					#if DB_SHOW_ERROR_CODE																	//Error Check
					gDbErrorCode = DB_ERROR_DICTIONARY_FILE_OPEN_FAIL_AT_DBLOADDATABASEFILES_FUNTION;		//Error Check
					#endif																						//Error Check

					return DB_FUNCTION_FAIL;
				}
			}
			
			// Action 2.2. ��ųʸ� ������ �ش��� �ش� ��ġ�� �����Ѵ�.
			{
				vReadingSize = fread(pDbDictionaryHeader, sizeof(DictionaryHeader), 1, gDbFileDictionaryFile);

				fclose(gDbFileDictionaryFile);
				gDbFileDictionaryFile = 0;

				if (vReadingSize != 1)
				{
					#if DB_SHOW_ERROR_CODE																	//Error Check
					gDbErrorCode = DB_ERROR_DICTIONARY_FILE_READING_FAIL_AT_DBLOADDATABASEFILES_FUNTION;		//Error Check
					#endif																						//Error Check
					printf("Dictionary File Header Reading Error");
					return DB_FUNCTION_FAIL;
				}
			}

			// Action 2.3. ��ųʸ� ������ �ش��� �ش� ����ü�� �����Ѵ�.
			dbMemcpyFreeIndian(pDbDictionaryHeader, &dbDictionaryHeader, sizeof(unsigned long), 6);

			// Action 2.3. ��ųʸ� ������ Body�� �ش� ��ġ�� �����Ѵ�.
			{
				vReadingSize = 0;
				
				// Action 2.3.1. ��ųʸ� ������ Open�Ѵ�..
				{
					if (gDbFileDictionaryFile)
					{
						fclose(gDbFileDictionaryFile);
						gDbFileDictionaryFile = 0;
					}
					
					gDbFileDictionaryFile = fopen(gDbFileNameDictionaryFileName, "a+b");		//��ųʸ� ������ Open�Ѵ�.

					//Error Dictionary File�� Open�Ͽ������� �Ǵ��Ѵ�.
					if (gDbFileDictionaryFile == 0)
					{
						#if DB_SHOW_ERROR_CODE																	//Error Check
						gDbErrorCode = DB_ERROR_DICTIONARY_FILE_OPEN_FAIL_AT_DBLOADDATABASEFILES_FUNTION;		//Error Check
						#endif																						//Error Check

						return DB_FUNCTION_FAIL;
					}
				}

				// Action 2.3.2. ��ųʸ� ������ Body�� Read�Ѵ�..
				{
					fseek(gDbFileDictionaryFile, sizeof(dbDictionaryHeader), SEEK_SET);
						
					vReadingSize = fread((char*)(pDbDictionaryHeader + 1), sizeof(char), (dbDictionaryHeader.dictionaryHeaderSizeOfDictionary - sizeof(DictionaryHeader)), gDbFileDictionaryFile);

					fclose(gDbFileDictionaryFile);
					gDbFileDictionaryFile = 0;
					
					if (vReadingSize != (dbDictionaryHeader.dictionaryHeaderSizeOfDictionary - sizeof(DictionaryHeader)))
					{
						#if DB_SHOW_ERROR_CODE																	//Error Check
						gDbErrorCode = DB_ERROR_DICTIONARY_FILE_READING_FAIL_AT_DBLOADDATABASEFILES_FUNTION;		//Error Check
						#endif																						//Error Check
						printf("Dictionary File Body Reading Error");
						return DB_FUNCTION_FAIL;
					}
				}
			}
		}
	}
	else		//����ڰ� ������ ����� ������ �־��� ��� ó��
	{
		unsigned long vReadingSize = 0;

		// Action 1. ��ųʸ� ������ ������ �ö�� ���� �ּҸ� ã�´�.
		pDbDictionaryHeader = dbGetDictionaryHeader();

		// Action 2. ��ųʸ� ������ Open�Ѵ�.
		{
			if (gDbFileDictionaryFile)
			{
				fclose(gDbFileDictionaryFile);
				gDbFileDictionaryFile = 0;
			}

			gDbFileDictionaryFile = fopen(gDbFileNameDictionaryFileName, "a+b");		//��ųʸ� ������ Open�Ѵ�.

			//Error Dictionary File�� Open�Ͽ������� �Ǵ��Ѵ�.
			if (gDbFileDictionaryFile == 0)
			{
				#if DB_SHOW_ERROR_CODE																	//Error Check
				gDbErrorCode = DB_ERROR_DICTIONARY_FILE_OPEN_FAIL_AT_DBLOADDATABASEFILES_FUNTION;		//Error Check
				#endif																						//Error Check

				return DB_FUNCTION_FAIL;
			}
		}

		// Action 3. ��ųʸ� ������ Memory�� Load�Ѵ�.
		{
			vReadingSize = fread((char*)(pDbDictionaryHeader + 1), sizeof(char), vDictionaryFileSize, gDbFileDictionaryFile);

			fclose(gDbFileDictionaryFile);
			gDbFileDictionaryFile = 0;
			
			if (vReadingSize != vDictionaryFileSize)
			{
				#if DB_SHOW_ERROR_CODE																	//Error Check
				gDbErrorCode = DB_ERROR_DICTIONARY_FILE_READING_FAIL_AT_DBLOADDATABASEFILES_FUNTION;		//Error Check
				#endif																						//Error Check
				printf("Dictionary File Reading Error");
				return DB_FUNCTION_FAIL;
			}
		}
	}
	//=============================Dictionary File Loading (E n d)=============================//

	//===============================Data File Loading (Begin)===============================//
	{
		if (gDbFileDataFile)
		{
			fclose(gDbFileDataFile);
			gDbFileDataFile = 0;
		}
		gDbFileDataFile = fopen(gDbFileNameDataFileName, "a+b");

		fclose(gDbFileDataFile);
		gDbFileDataFile = 0;
		
		gDbFileDataFile = fopen(gDbFileNameDataFileName, "r+b");

		fclose(gDbFileDataFile);
		gDbFileDataFile = 0;
	}
	//===============================Data File Loading (E n d)===============================//

	//===============================Index File Loading (Begin)===============================//
	{
		if (gDbFileIndexFile)
		{
			fclose(gDbFileIndexFile);
			gDbFileIndexFile = 0;
		}
		
		gDbFileIndexFile = fopen(gDbFileNameIndexFileName, "a+b");
		fclose(gDbFileIndexFile);
		gDbFileIndexFile = 0;
		gDbFileIndexFile = fopen(gDbFileNameIndexFileName, "r+b");
		
		fclose(gDbFileIndexFile);
		gDbFileIndexFile = 0;
	}
	//===============================Index File Loading (E n d)===============================//
	return DB_FUNCTION_SUCCESS;
}

void dbStoreDictionaryfile(void)
{
	pDictionaryHeader	pdicHeader;
	unsigned long		vDictionarySize;

	pdicHeader = dbGetDictionaryHeader();
	dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderSizeOfDictionary) , &vDictionarySize, sizeof(unsigned long), 1);

	if (gDbFileDictionaryFile)
	{
		fclose(gDbFileDictionaryFile);
		gDbFileDictionaryFile = 0;
	}

	gDbFileDictionaryFile = fopen(gDbFileNameDictionaryFileName, "r+b");

	if (gDbFileDictionaryFile == 0)
	{
		#if DB_SHOW_ERROR_CODE																	//Error Check
		gDbErrorCode = DB_ERROR_Dictionary_FILE_OPEN_FAIL_AT_DBSTOREDICTIONARYFILE_FUNTION;		//Error Check
		#endif																						//Error Check

		fclose(gDbFileDictionaryFile);
		gDbFileDictionaryFile = 0;
		
		return;
	}
	
	fseek(gDbFileDictionaryFile, 0, SEEK_SET);

	if (fwrite(pdicHeader, sizeof(char), vDictionarySize, gDbFileDictionaryFile) != vDictionarySize)
	{
		#if DB_SHOW_ERROR_CODE																	//Error Check
		gDbErrorCode = DB_ERROR_Dictionary_FILE_WRITE_FAIL_AT_DBSTOREDICTIONARYFILE_FUNTION;		//Error Check
		#endif																						//Error Check
	}
	
	fclose(gDbFileDictionaryFile);
	gDbFileDictionaryFile = 0;	
}

void dbStoreDatafile(void)
{
	pMMNODE		temp;
	unsigned long 		vWriteCount;
	
	temp = gDbMmuDataPage;
	
	while(temp != DB_NULL)
	{
		//������ �ִ� page���� �����Ѵ�.
		if (temp->mmuChanged == DB_CHANGED)
		{
			vWriteCount = 0;
			
			//�� �Լ��� �޸𸮿� �ε�Ǿ� �ִ� ��� datapage�� datafile�� �����Ѵ�.
			if (gDbFileDataFile)
			{
				fclose(gDbFileDataFile);
				gDbFileDataFile = 0;
			}

			gDbFileDataFile = fopen(gDbFileNameDataFileName, "r+b");

			if (gDbFileDataFile == 0)
			{
				#if DB_SHOW_ERROR_CODE														//Error Check
				gDbErrorCode = DB_ERROR_DATA_FILE_OPEN_FAIL_AT_DBSTOREDATAFILE_FUNTION;		//Error Check
				#endif																			//Error Check
			
				fclose(gDbFileDataFile);
				gDbFileDataFile = 0;
				return;
			}

			fseek(gDbFileDataFile, (temp->mmuPageNumber - 1)*gDbPageSizeDataFilePageSize, SEEK_SET);

			vWriteCount = fwrite(temp->mmuMemoryAddress, gDbPageSizeDataFilePageSize, 1, gDbFileDataFile);

			fclose(gDbFileDataFile);
			gDbFileDataFile = 0;
			
			if  (vWriteCount==1)
			{
				//��Ͽ� ������ ��� 	
			}
			else
			{

				#if DB_SHOW_ERROR_CODE														//Error Check
				gDbErrorCode = DB_ERROR_DATA_FILE_WRITE_FAIL_AT_DBSTOREDATAFILE_FUNTION;		//Error Check
				#endif																			//Error Check

				//��Ͽ� �������� ���� ��� 
				if (gDbFileDataFile) 
				{
					fclose(gDbFileDataFile);
					gDbFileDataFile = 0;
				}

				gDbFileDataFile = fopen(gDbFileNameDataFileName, "r+b");


				if (gDbFileDataFile == 0)
				{
					#if DB_SHOW_ERROR_CODE														//Error Check
					gDbErrorCode = DB_ERROR_DATA_FILE_OPEN_FAIL_AT_DBSTOREDATAFILE_FUNTION;		//Error Check
					#endif																			//Error Check
				
					fclose(gDbFileDataFile);
					gDbFileDataFile = 0;
					return;
				}
				
				fseek(gDbFileDataFile, (temp->mmuPageNumber - 1)*gDbPageSizeDataFilePageSize, SEEK_SET);

				vWriteCount = fwrite(temp->mmuMemoryAddress, gDbPageSizeDataFilePageSize, 1, gDbFileDataFile);

				fclose(gDbFileDataFile);
				gDbFileDataFile = 0;
			}

			if (vWriteCount==1)
			{
				temp->mmuChanged = DB_UNCHANGED;
			}
			else
			{
				#if DB_SHOW_ERROR_CODE														//Error Check
				gDbErrorCode = DB_ERROR_DATA_FILE_WRITE_FAIL_AT_DBSTOREDATAFILE_FUNTION;		//Error Check
				#endif		
				printf("Data File Write Error");
			}
			
		}
		temp = temp->mmuNext;
	}
}

void dbStoreIndexfile(void)
{
	pMMNODE		temp;
	unsigned long 		vWriteCount;
	
	temp = gDbMmuIndexPage;

	while(temp != DB_NULL)
	{
		//������ �ִ� page���� �����Ѵ�.
		if (temp->mmuChanged == DB_CHANGED)
		{
			vWriteCount = 0;
			//�� �Լ��� �޸𸮿� �ε�Ǿ� �ִ� ��� datapage�� datafile�� �����Ѵ�.
			if (gDbFileIndexFile)
			{
				fclose(gDbFileIndexFile);
				gDbFileIndexFile = 0;
			}
		
			gDbFileIndexFile = fopen(gDbFileNameIndexFileName, "r+b");

			if (gDbFileIndexFile == 0)
			{
				#if DB_SHOW_ERROR_CODE														//Error Check
				gDbErrorCode = DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSTOREINDEXFILE_FUNTION;		//Error Check
				#endif		

				fclose(gDbFileIndexFile);
				gDbFileIndexFile = 0;
				return;
			}
			
			
			fseek(gDbFileIndexFile, (temp->mmuPageNumber - 1)*gDbPageSizeIndexFilePageSize, SEEK_SET);

			vWriteCount = fwrite(temp->mmuMemoryAddress, gDbPageSizeIndexFilePageSize, 1, gDbFileIndexFile);

			fclose(gDbFileIndexFile);
			gDbFileIndexFile = 0;

			if  (vWriteCount==1)
			{
				//��Ͽ� ������ ��� 
			}
			else
			{
				#if DB_SHOW_ERROR_CODE														//Error Check
				gDbErrorCode = DB_ERROR_INDEX_FILE_WRITE_FAIL_AT_DBSTOREINDEXFILE_FUNTION;		//Error Check
				#endif						
				
				//��Ͽ� �������� ���� ��� 
				if (gDbFileIndexFile) 
				{
					fclose(gDbFileIndexFile);
					gDbFileIndexFile = 0;
				}
				
				gDbFileIndexFile = fopen(gDbFileNameIndexFileName, "r+b");

				if (gDbFileIndexFile == 0)
				{
					#if DB_SHOW_ERROR_CODE														//Error Check
					gDbErrorCode = DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSTOREINDEXFILE_FUNTION;		//Error Check
					#endif		

					fclose(gDbFileIndexFile);
					gDbFileIndexFile = 0;
					return;
				}
				
				fseek(gDbFileIndexFile, (temp->mmuPageNumber - 1)*gDbPageSizeIndexFilePageSize, SEEK_SET);

				vWriteCount = fwrite(temp->mmuMemoryAddress, gDbPageSizeIndexFilePageSize, 1, gDbFileIndexFile);

				fclose(gDbFileIndexFile);
				gDbFileIndexFile = 0;
			}

			if  (vWriteCount==1)
			{
				temp->mmuChanged = DB_UNCHANGED;
			}
			else
			{
				#if DB_SHOW_ERROR_CODE														//Error Check
				gDbErrorCode = DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSTOREINDEXFILE_FUNTION;		//Error Check
				#endif
				printf("Index File Write Error");
			}
		}

		temp = temp->mmuNext;
	}
}

void dbApiShutDown(void)
{

#if DB_SHOW_ERROR_CODE
	gDbErrorCode = DB_ERROR_NO_ERROR;
#endif

//==============ERROR Code (Begin)==============//
if (gDbStartUpDB != DB_DO_RUNNING)
{
	#if DB_SHOW_ERROR_CODE
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;
	#endif
	return;
}
//==============ERROR Code ( end )==============//

	if (gDbStartUpDB == DB_DO_RUNNING)
	{
		if (gDbDictionaryChanged == DB_CHANGED)	dbStoreDictionaryfile();

		if (gDbMmuDataPage->mmuPageNumber != 0)		dbStoreDatafile();

		if (gDbMmuIndexPage->mmuPageNumber != 0)	dbStoreIndexfile();

		gDbStartUpDB = DB_DO_NOT_RUNNING;
	}
}
#endif



#if ( DB_FILE_DEBUG == 1 )

void MemoryToFile4Debug(unsigned short *Pathname, unsigned char * startMem, unsigned long size )
{
	static unsigned char test = 0;	
	
		// DICTIONARYFILE_START_ADDRESS
	tFile sFile;

	DbgPrintf(("\n\n\nWrite to File, Path:%c%c%c%c%c%c, start:0x%x, size:0x%x", 
					Pathname[0], Pathname[1], Pathname[2], Pathname[3], Pathname[4], Pathname[5],
					(unsigned long)startMem, size));

	//str2ustr( Pathname, "/a/DBTest.dat" );

	//FSOpen( &sFile, 0, Pathname, O_RDWR|O_TFS4_DIRECTIO );
	FSCreate( &sFile, 0, Pathname, 0, ATTR_ARCHIEVE );

	FSWrite( &sFile, startMem, size );

	FSTruncate( &sFile, size );

	FSClose( &sFile );

	DbgPrintf(("\nWrite to File OK!"));

}

#endif


#if TFS4_BASE_STAND_IO
//���������� ���Ⱑ TFS4

#include	"..\..\fs\tfs4_1.0\api\tfs4_api.h"

typedef int	DB_FILE;

DB_FILE			gDbFileDictionaryFile = -1, 
				gDbFileDataFile = -1, 
				gDbFileIndexFile = -1;

#ifndef CONFIG_HDD
//hjnam..20050106..
extern unsigned char g_MBuf[65536];
#endif

void dbCloseDatabaseFiles(void)
{
	if (gDbFileDictionaryFile != -1)
	{
		tfs4_close(gDbFileDictionaryFile);
		gDbFileDictionaryFile = -1;
	}
	if (gDbFileDataFile != -1)
	{
		tfs4_close(gDbFileDataFile);
		gDbFileDataFile = -1;
	}
	if (gDbFileIndexFile != -1)
	{
		tfs4_close(gDbFileIndexFile);
		gDbFileIndexFile = -1;
	}
}


static void dbFileRead(unsigned long lDbFileHandle, unsigned char *pReadBuf, signed long lTotalReadSize)
{
//	memset(g_MBuf,0,2048);
	
	unsigned long lReadSize;
//DbgPrintf(("dbFileRead:: lTotalReadSize = %d\n", lTotalReadSize));	
#ifdef CONFIG_HDD  //jaihong.kim noncache ������ (E10)

	lReadSize=tfs4_read(lDbFileHandle, pReadBuf, lTotalReadSize);
	if(lTotalReadSize != lReadSize)
	{
		DbgPrintf(("dbFileRead:: READ ERROR \n", lTotalReadSize));	

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



//hjnam..20050106..
static int dbFileWrite(unsigned long lDbFileHandle, unsigned char *pWriteBuf, signed long lTotalWriteSize)
{
	unsigned long lFileSize, lOldPos, lWriteSize;
	unsigned long vWriteCount = 0;
	//
	//	1st stage, truncate file
	//	
	lOldPos = tfs4_ftell(lDbFileHandle);	

	tfs4_lseek(lDbFileHandle,0, SEEK_END);
	
	lFileSize = tfs4_ftell(lDbFileHandle);

	if ( (lOldPos + lTotalWriteSize) > lFileSize )
	{	
		if (tfs4_ftruncate(lDbFileHandle, (lOldPos + lTotalWriteSize)) < 0)
		{
			DbgPrintf(("\ndbLoadDatabaseFiles : data file truncation error!!!"));
		}
	}

	tfs4_lseek(lDbFileHandle, lOldPos, SEEK_SET);

	//
	//	2nd stage, write file
	//	
#ifdef CONFIG_HDD  //jaihong.kim noncache ������ (E10)

	if(tfs4_write(lDbFileHandle, pWriteBuf, lTotalWriteSize)<0)
	{
		debug("DB Write File Error\n");
		return DB_FUNCTION_FAIL;
	}
	
	return DB_FUNCTION_SUCCESS;
	
#else
	
	while (lTotalWriteSize > 0)
	{
		if (lTotalWriteSize > sizeof(g_MBuf))
			lWriteSize = sizeof(g_MBuf);
		else
			lWriteSize = lTotalWriteSize;	

		memcpy(g_MBuf, pWriteBuf, lWriteSize);

		
		if(tfs4_write(lDbFileHandle, g_MBuf, lWriteSize)<0){
			vWriteCount++;
			debug("DB Write File Error\n");
			print_ERRMSG();
			
			if (vWriteCount>20)
			{
				//�������� ��쿡�� 2�� ���� ��ϵǾ�� �Ѵ�.
				return DB_FUNCTION_FAIL;
			}
		}

		pWriteBuf += lWriteSize;
		lTotalWriteSize -= lWriteSize;
	}		
	
	return DB_FUNCTION_SUCCESS;
#endif
}	

//tfs4
#if 1
int dbLoadDatabaseFiles(char *dbFileNameDictionaryFileName,
						 char *dbFileNameDataFileName,
						 char *dbFileNameIndexFileName,
						 unsigned long vDictionaryFileSize)
{

	//�� �Լ��� database�� start�ϸ鼭 db���� ����ϴ� ������ �������� �Լ��̴�.
	pDictionaryHeader		pDbDictionaryHeader;
	DictionaryHeader		dbDictionaryHeader;


	//==========================Dictionary File Load (Being)==========================//
	if(vDictionaryFileSize == 0)
	{
		// Action 1. ��ųʸ��� �ش��� ������ �ö�� ���� ���� �ʱ��� ��ųʸ� ������ �ø���.
		{
			pDbDictionaryHeader = dbGetDictionaryHeader();

			//��ųʸ��� ����ü�� �ʱ� ������ ���� �����Ѵ�.
			dbDictionaryHeader.dictionaryHeaderDeleteNode				= 0;
			dbDictionaryHeader.dictionaryHeaderDeleteStartPage				= 0;
			dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage	= 0;
			dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage	= 0;
			dbDictionaryHeader.dictionaryHeaderOffsetOfRootTable			= 0;
			dbDictionaryHeader.dictionaryHeaderSizeOfDictionary			= sizeof(DictionaryHeader);
			
			//��ųʸ��ش��� ���������� ���� ��ųʸ� �ش��� ������ �����Ѵ�.
			dbMemcpyFreeIndian(&dbDictionaryHeader, pDbDictionaryHeader, sizeof(unsigned long), 6);
		}

		// Action 2. ��ųʸ� ������ �����ϸ� �� ������ �ø���.
		{
			//hjnam..20060105
			if (gDbFileDictionaryFile!=-1)
			{
				tfs4_close(gDbFileDictionaryFile);
				gDbFileDictionaryFile = -1;
			}
			
			gDbFileDictionaryFile = tfs4_open((t_char*)gDbFileNameDictionaryFileName, O_RDWR|O_TFS4_DIRECTIO);

			if (gDbFileDictionaryFile < 0)
			{
				//������ ���� �� ���� ���
				unsigned long ulFlags;
			
				ulFlags = /*O_WRONLY*/O_RDWR|O_CREAT|O_TRUNC|O_TFS4_NO_WRITE_UPDATE|O_TFS4_DIRECTIO;	 	

				gDbFileDictionaryFile = tfs4_open((t_char*)gDbFileNameDictionaryFileName, (t_int32)ulFlags);

				if (gDbFileDictionaryFile >= 0)
				{
					if (tfs4_ftruncate(gDbFileDictionaryFile, gDbAreaSizeDictionaryAreaSize) < 0)
					{
						#if DB_SHOW_ERROR_CODE									//Error Check
						gDbErrorCode = DB_ERROR_DICTIONARY_FILE_TRUNCATE_FAILED;	//Error Check
						#endif														//Error Check
					}
				}
				else
				{
					#if DB_SHOW_ERROR_CODE									//Error Check
					gDbErrorCode = DB_ERROR_DICTIONARY_FILE_CREATE_FAILED;		//Error Check
					#endif														//Error Check
				}
			}
			else
			{
			//����� ������  �����ϴ� ���
				// Action 2.2. ��ųʸ� ������ �ش��� �ش� ����ü�� �����Ѵ�.
				dbFileRead((unsigned long)gDbFileDictionaryFile, (unsigned char *)(&dbDictionaryHeader), sizeof(DictionaryHeader) );

				if (dbDictionaryHeader.dictionaryHeaderSizeOfDictionary >=sizeof(DictionaryHeader))
				{
					dbMemcpyFreeIndian(&dbDictionaryHeader, pDbDictionaryHeader, sizeof(unsigned long), 6);
					
					tfs4_lseek(gDbFileDictionaryFile, sizeof(DictionaryHeader), SEEK_SET);
					
					dbFileRead((unsigned long)gDbFileDictionaryFile, (unsigned char *)(pDbDictionaryHeader + 1),
								 (dbDictionaryHeader.dictionaryHeaderSizeOfDictionary - sizeof(DictionaryHeader)));
				}
				else
				{
					#if DB_SHOW_ERROR_CODE																//Error Check
					gDbErrorCode = DB_ERROR_DICTIONARY_FILE_CRASH_DICTIONARY_HEADER_SIZE_VERY_SMALL;	//Error Check
					#endif																					//Error Check				
				}
			}
			tfs4_close(gDbFileDictionaryFile);
			gDbFileDictionaryFile = -1;
		}
	}
	else
	{
		pDbDictionaryHeader = dbGetDictionaryHeader();

		// ��ųʸ� ������ Open�Ѵ�.
		{
			if (gDbFileDictionaryFile!=-1)
			{
				tfs4_close(gDbFileDictionaryFile);
				gDbFileDictionaryFile = -1;
			}

			gDbFileDictionaryFile = tfs4_open((t_char*)gDbFileNameDictionaryFileName, O_RDWR|O_TFS4_DIRECTIO);

			if (gDbFileDictionaryFile==-1)
			{
				#if DB_SHOW_ERROR_CODE																//Error Check
				gDbErrorCode = DB_ERROR_DICTIONARY_FILE_OPEN_FAIL_AT_DBLOADDATABASEFILES_FUNTION;	//Error Check
				#endif
				
				tfs4_close(gDbFileDictionaryFile);
				gDbFileDictionaryFile = -1;
						
				return 	DB_FUNCTION_FAIL;
			}
		}
		//��ųʸ� ������ Read�Ѵ�.
		dbFileRead((unsigned long)gDbFileDictionaryFile, (unsigned char*)pDbDictionaryHeader, vDictionaryFileSize);

		//��ųʸ� ������ Close�Ѵ�.
		{
			tfs4_close(gDbFileDictionaryFile);
			gDbFileDictionaryFile = -1;
		}

	}
	//==========================Dictionary File Load (E n d)==========================//

	//============================Data File Load (Being)============================//
	{
		if (gDbFileDataFile!=-1)
		{
			tfs4_close(gDbFileDataFile);
			gDbFileDataFile = -1;
		}
		
		gDbFileDataFile = tfs4_open((t_char*)gDbFileNameDataFileName, O_RDWR|O_TFS4_DIRECTIO);

		if (gDbFileDataFile < 0)
		{
			unsigned long ulFlags;
		
			ulFlags = /*O_WRONLY*/O_RDWR|O_CREAT|O_TRUNC|O_TFS4_NO_WRITE_UPDATE|O_TFS4_DIRECTIO;	 	
			gDbFileDataFile = tfs4_open((t_char*)gDbFileNameDataFileName, (t_int32)ulFlags);

			if (gDbFileDataFile >= 0)
			{
				if (tfs4_ftruncate(gDbFileDataFile, gDbAreaSizeDataAreaSize) < 0)
				{
					#if DB_SHOW_ERROR_CODE								//Error Check
					gDbErrorCode = DB_ERROR_DATA_FILE_TRUNCATE_FAILED;		//Error Check
					#endif													//Error Check
				}
			}
			else
			{
				#if DB_SHOW_ERROR_CODE								//Error Check
				gDbErrorCode = DB_ERROR_DATA_FILE_CREATE_FAILED;		//Error Check
				#endif													//Error Check
			}
		}
		tfs4_close(gDbFileDataFile);		//���� �׻� �ܾ� �д�.
		gDbFileDataFile = -1;
	}
	//============================Data File Load (E n d)============================//

	//============================Index File Load (Being)============================//
	{
		if (gDbFileIndexFile != -1)
		{
			tfs4_close(gDbFileIndexFile);
			gDbFileIndexFile = -1;
		}
		
		gDbFileIndexFile = tfs4_open((t_char*)gDbFileNameIndexFileName, O_RDWR|O_TFS4_DIRECTIO);
		
		if (gDbFileIndexFile < 0)
		{
			unsigned long ulFlags;
		
			ulFlags = /*O_WRONLY*/O_RDWR|O_CREAT|O_TRUNC|O_TFS4_NO_WRITE_UPDATE|O_TFS4_DIRECTIO;	 	
			gDbFileIndexFile = tfs4_open((t_char*)gDbFileNameIndexFileName, (t_int32)ulFlags);

			if (gDbFileIndexFile >= 0)
			{
				if (tfs4_ftruncate(gDbFileIndexFile, gDbAreaSizeIndexAreaSize) < 0)
				{
					print_ERRMSG();
					#if DB_SHOW_ERROR_CODE								//Error Check
					gDbErrorCode = DB_ERROR_INDEX_FILE_TRUNCATE_FAILED;	//Error Check
					#endif													//Error Check
				}
			}
			else
			{
				#if DB_SHOW_ERROR_CODE								//Error Check
				gDbErrorCode = DB_ERROR_INDEX_FILE_CREATE_FAILED;		//Error Check
				#endif													//Error Check
			}		
		}
		tfs4_close(gDbFileIndexFile);	//���� �׻� �ܾ� �д�.
		gDbFileIndexFile = -1;
	}
	//============================Index File Load (E n d)============================//
	return DB_FUNCTION_SUCCESS;
}

#else
void dbLoadDatabaseFiles(char *dbFileNameDictionaryFileName,
							 char *dbFileNameDataFileName,
							 char *dbFileNameIndexFileName)
{

	//�� �Լ��� database�� start�ϸ鼭 db���� ����ϴ� ������ �������� �Լ��̴�.
	pDictionaryHeader		pDbDictionaryHeader;
	DictionaryHeader		dbDictionaryHeader;

//DbgPrintf(("dbLoadDatabaseFiles++\n"));
	//dictionaryFile load
	{
		// Action 1. ��ųʸ��� �ش��� ������ �ö�� ���� ���� �ʱ��� ��ųʸ� ������ �ø���.
		{
			pDbDictionaryHeader = dbGetDictionaryHeader();

			//��ųʸ��� ����ü�� �ʱ� ������ ���� �����Ѵ�.
			dbDictionaryHeader.dictionaryHeaderDeleteNode				= 0;
			dbDictionaryHeader.dictionaryHeaderDeleteStartPage				= 0;
			dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage	= 0;
			dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage	= 0;
			dbDictionaryHeader.dictionaryHeaderOffsetOfRootTable			= 0;
			dbDictionaryHeader.dictionaryHeaderSizeOfDictionary			= sizeof(DictionaryHeader);
			
			//��ųʸ��ش��� ���������� ���� ��ųʸ� �ش��� ������ �����Ѵ�.
			dbMemcpyFreeIndian(&dbDictionaryHeader, pDbDictionaryHeader, sizeof(unsigned long), 6);
		}

		// Action 2. ��ųʸ� ������ �����ϸ� �� ������ �ø���.
		{
			//hjnam..20060105
			if (gDbFileDictionaryFile!=-1)
			{
				tfs4_close(gDbFileDictionaryFile);
				gDbFileDictionaryFile = -1;
			}
			
			gDbFileDictionaryFile = tfs4_open((t_char*)gDbFileNameDictionaryFileName, O_RDWR|O_TFS4_DIRECTIO);

			if (gDbFileDictionaryFile < 0)
			{
				//������ ���� �� ���� ���
				unsigned long ulFlags;
			
				ulFlags = /*O_WRONLY*/O_RDWR|O_CREAT|O_TRUNC|O_TFS4_NO_WRITE_UPDATE|O_TFS4_DIRECTIO;	 	

				gDbFileDictionaryFile = tfs4_open((t_char*)gDbFileNameDictionaryFileName, (t_int32)ulFlags);

				if (gDbFileDictionaryFile >= 0)
				{
					if (tfs4_ftruncate(gDbFileDictionaryFile, gDbAreaSizeDictionaryAreaSize) < 0)
					{
						#if DB_SHOW_ERROR_CODE									//Error Check
						gDbErrorCode = DB_ERROR_DICTIONARY_FILE_TRUNCATE_FAILED;	//Error Check
						#endif														//Error Check
					}
				}
				else
				{
					#if DB_SHOW_ERROR_CODE									//Error Check
					gDbErrorCode = DB_ERROR_DICTIONARY_FILE_CREATE_FAILED;		//Error Check
					#endif														//Error Check
				}
			}
			else
			{
			//����� ������  �����ϴ� ���
				// Action 2.2. ��ųʸ� ������ �ش��� �ش� ����ü�� �����Ѵ�.
				dbFileRead((unsigned long)gDbFileDictionaryFile, (unsigned char *)(&dbDictionaryHeader), sizeof(DictionaryHeader) );

				if (dbDictionaryHeader.dictionaryHeaderSizeOfDictionary >=sizeof(DictionaryHeader))
				{
					dbMemcpyFreeIndian(&dbDictionaryHeader, pDbDictionaryHeader, sizeof(unsigned long), 6);
					
					tfs4_lseek(gDbFileDictionaryFile, sizeof(DictionaryHeader), SEEK_SET);
					
					dbFileRead((unsigned long)gDbFileDictionaryFile, (unsigned char *)(pDbDictionaryHeader + 1),
								 (dbDictionaryHeader.dictionaryHeaderSizeOfDictionary - sizeof(DictionaryHeader)));
				}
				else
				{
					#if DB_SHOW_ERROR_CODE																//Error Check
					gDbErrorCode = DB_ERROR_DICTIONARY_FILE_CRASH_DICTIONARY_HEADER_SIZE_VERY_SMALL;	//Error Check
					#endif																					//Error Check				
				}
			}
			tfs4_close(gDbFileDictionaryFile);
			gDbFileDictionaryFile = -1;
		}
	}

	//dataFileLoad
	{
		if (gDbFileDataFile!=-1)
		{
			tfs4_close(gDbFileDataFile);
			gDbFileDataFile = -1;
		}
		
		gDbFileDataFile = tfs4_open((t_char*)gDbFileNameDataFileName, O_RDWR|O_TFS4_DIRECTIO);

		if (gDbFileDataFile < 0)
		{
			unsigned long ulFlags;
		
			ulFlags = /*O_WRONLY*/O_RDWR|O_CREAT|O_TRUNC|O_TFS4_NO_WRITE_UPDATE|O_TFS4_DIRECTIO;	 	
			gDbFileDataFile = tfs4_open((t_char*)gDbFileNameDataFileName, (t_int32)ulFlags);

			if (gDbFileDataFile >= 0)
			{
				if (tfs4_ftruncate(gDbFileDataFile, gDbAreaSizeDataAreaSize) < 0)
				{
					#if DB_SHOW_ERROR_CODE								//Error Check
					gDbErrorCode = DB_ERROR_DATA_FILE_TRUNCATE_FAILED;		//Error Check
					#endif													//Error Check
				}
			}
			else
			{
				#if DB_SHOW_ERROR_CODE								//Error Check
				gDbErrorCode = DB_ERROR_DATA_FILE_CREATE_FAILED;		//Error Check
				#endif													//Error Check
			}
		}
		tfs4_close(gDbFileDataFile);		//���� �׻� �ܾ� �д�.
		gDbFileDataFile = -1;
	}

	//indexFileLoad
	{
		if (gDbFileIndexFile != -1)
		{
			tfs4_close(gDbFileIndexFile);
			gDbFileIndexFile = -1;
		}
		
		gDbFileIndexFile = tfs4_open((t_char*)gDbFileNameIndexFileName, O_RDWR|O_TFS4_DIRECTIO);
		
		if (gDbFileIndexFile < 0)
		{
			unsigned long ulFlags;
		
			ulFlags = /*O_WRONLY*/O_RDWR|O_CREAT|O_TRUNC|O_TFS4_NO_WRITE_UPDATE|O_TFS4_DIRECTIO;	 	
			gDbFileIndexFile = tfs4_open((t_char*)gDbFileNameIndexFileName, (t_int32)ulFlags);

			if (gDbFileIndexFile >= 0)
			{
				if (tfs4_ftruncate(gDbFileIndexFile, gDbAreaSizeIndexAreaSize) < 0)
				{
					print_ERRMSG();
					#if DB_SHOW_ERROR_CODE								//Error Check
					gDbErrorCode = DB_ERROR_INDEX_FILE_TRUNCATE_FAILED;	//Error Check
					#endif													//Error Check
				}
			}
			else
			{
				#if DB_SHOW_ERROR_CODE								//Error Check
				gDbErrorCode = DB_ERROR_INDEX_FILE_CREATE_FAILED;		//Error Check
				#endif													//Error Check
			}		
		}
		tfs4_close(gDbFileIndexFile);	//���� �׻� �ܾ� �д�.
		gDbFileIndexFile = -1;
	}
//DbgPrintf(("dbLoadDatabaseFiles--\n"));		
}
#endif

//tfs4
void dbApiShutDown(void)
{

#if DB_SHOW_ERROR_CODE
	gDbErrorCode = DB_ERROR_NO_ERROR;
#endif

//==============ERROR Code (Begin)==============//
if (gDbStartUpDB != DB_DO_RUNNING)
{
	#if DB_SHOW_ERROR_CODE
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;
	#endif
	return;
}
//==============ERROR Code ( end )==============//


//DbgPrintf(("dbApiShutDown:: �Լ�����\n"));		

	if (gDbStartUpDB == DB_DO_RUNNING)
	{
		//DbgPrintf(("\n===dbApiShutDown!==="));		
		
		if (gDbDictionaryChanged == DB_CHANGED)
		{
			dbStoreDictionaryfile();
		}
		//fclose(gDbFileDictionaryFile);
		//tfs4_close(gDbFileDictionaryFile);

		if (gDbMmuDataPage->mmuPageNumber != 0)
		{
			dbStoreDatafile();
		}
		//fclose(gDbFileDataFile);	//datafile�� close�Ѵ�
		//tfs4_close(gDbFileDataFile);			

		if (gDbMmuIndexPage->mmuPageNumber != 0)
		{
			dbStoreIndexfile();
		}
		//fclose(gDbFileIndexFile);	//indexfile�� close�Ѵ�
		//tfs4_close(gDbFileIndexFile);				

		gDbStartUpDB = DB_DO_NOT_RUNNING;
	}
}


//tfs4
void dbStoreDictionaryfile(void)
{

	pDictionaryHeader	pdicHeader;
	unsigned long		vDictionarySize;

	//DbgPrintf(("\n\n\nDictionary file write"));

	pdicHeader = dbGetDictionaryHeader();
	dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderSizeOfDictionary) , &vDictionarySize, sizeof(unsigned long), 1);

	if (gDbFileDictionaryFile !=-1)
	{
		tfs4_close(gDbFileDictionaryFile);
		gDbFileDictionaryFile = -1;
	}
	
	gDbFileDictionaryFile = tfs4_open((t_char*)gDbFileNameDictionaryFileName, O_RDWR|O_TFS4_DIRECTIO);
	
	if (gDbFileDictionaryFile < 0)
	{
		DbgPrintf(("dbStoreDictionaryfile:: ��ųʸ� ������ ���� ���߽��ϴ�.\n"));
	}
	else
	{
		DbgPrintf(("=================[dbStoreDictionaryfile:: ��༭�� ���ϱ���� �õ��մϴ�.]=================\n"));

		DbgPrintf(("dbStoreDictionaryfile:: ��༭�� ���ϱ���� �õ��մϴ�.\n"));

		tfs4_lseek(gDbFileDictionaryFile, 0, SEEK_SET);

		if (dbFileWrite((unsigned long)gDbFileDictionaryFile, (unsigned char *)pdicHeader, vDictionarySize) == DB_FUNCTION_SUCCESS)
		{
			gDbDictionaryChanged = DB_UNCHANGED;
		}
		else
		{
			gDbDictionaryChanged = DB_CHANGED;
		}

		tfs4_close(gDbFileDictionaryFile);
		gDbFileDictionaryFile = -1;
		
	}


#if ( DB_FILE_DEBUG == 1 )
//	tfs4_close(gDbFileDictionaryFile);
//	gDbFileDictionaryFile = -1;
//	{
//		unsigned short Pathname[12] = {'/','a','/','t','e','s','t','.','d','i','c',0};
//		MemoryToFile4Debug(Pathname, (unsigned char *)pdicHeader, vDictionarySize );
//	}
#endif

}


//tfs4
void dbSetFileLoadMemorypage(unsigned long	filePageNumber,
							 	    char			*targetMemoryAddress, 
							 	    unsigned long	fileType)
{
	pDictionaryHeader	pDbDictionaryHeader;
	DictionaryHeader	dbDictionaryHeader;

	pDbDictionaryHeader = dbGetDictionaryHeader();
	dbMemcpyFreeIndian(pDbDictionaryHeader, &dbDictionaryHeader, sizeof(unsigned long), 6);
	

	if (fileType == DB_DATA_FILE_LOAD)
	{

		if (filePageNumber <= dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage)	//�� ��� datafile���� �о�´�
		{
			if (gDbFileDataFile != -1)
			{
				tfs4_close(gDbFileDataFile);
				gDbFileDataFile = -1;
			}
			
			gDbFileDataFile = tfs4_open((t_char*)gDbFileNameDataFileName, O_RDWR|O_TFS4_DIRECTIO);

			tfs4_lseek(gDbFileDataFile, (filePageNumber - 1)*gDbPageSizeDataFilePageSize, SEEK_SET);

			dbFileRead((unsigned long)gDbFileDataFile, (unsigned char *)targetMemoryAddress,gDbPageSizeDataFilePageSize);

			tfs4_close(gDbFileDataFile);
			gDbFileDataFile = -1;
			
			gDbMmuDataPage->mmuChanged = DB_UNCHANGED;
		}
		else	//�̰�� datafile���� �������� �����Ƿ� ���ø��� �Ѵ�.
		{
			pDataFilePageHeader pDH = (pDataFilePageHeader)targetMemoryAddress;
			
			DataFilePageHeader	DH;
			
			DH.dataHeaderDeleteRecordOffset	= 0;
			DH.dataHeaderEndOfRecords		= sizeof(DataFilePageHeader);
			DH.dataHeaderNextSameTablePage	= 0;
			DH.dataHeaderNumberOfSlots		= 0;

			dbMemcpyFreeIndian(&DH, pDH, sizeof(unsigned long), 4);

			if (dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage < filePageNumber)
			{
				//dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage = filePageNumber;
				dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage++;
				
				dbMemcpyFreeIndian(&dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage, 
									  &(pDbDictionaryHeader->dictionaryHeaderNumberOfDataFileEndPage), sizeof(unsigned long), 1);

				gDbMmuDataPage->mmuPageNumber = dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage;
				gDbDictionaryChanged = DB_CHANGED;
			}
			gDbMmuDataPage->mmuChanged = DB_CHANGED;
		}		
	}
	else if(fileType == DB_INDEX_FILE_LOAD)
	{

		if (filePageNumber <= dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage)	//�� ��� indexfile���� �о�´�
		{
			if (gDbFileIndexFile != -1)
			{
				tfs4_close(gDbFileIndexFile);
				gDbFileIndexFile = -1;
			}
			
			gDbFileIndexFile = tfs4_open((t_char*)gDbFileNameIndexFileName, O_RDWR|O_TFS4_DIRECTIO);
		
			tfs4_lseek(gDbFileIndexFile, (filePageNumber - 1)*gDbPageSizeIndexFilePageSize, SEEK_SET);

			dbFileRead((unsigned long)gDbFileIndexFile, (unsigned char *)targetMemoryAddress,gDbPageSizeIndexFilePageSize);

			tfs4_close(gDbFileIndexFile);
			gDbFileIndexFile = -1;
			
			gDbMmuIndexPage->mmuChanged = DB_UNCHANGED;
		}
		else	//�̰�� indexfile���� �������� �����Ƿ� ���ø��� �Ѵ�.
		{
			pIndexFilePageHeader pDH = (pIndexFilePageHeader)targetMemoryAddress;
			IndexFilePageHeader	 DH;
			
			DH.indexHeaderDeleteNodeOffset	= 0;
			DH.indexHeaderDeleteLinkOffset	= 0;
			DH.indexHeaderEndOfDatas			= sizeof(IndexFilePageHeader);
			
			dbMemcpyFreeIndian(&DH, pDH, sizeof(unsigned long), 3);

			if (dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage < filePageNumber)
			{
				//dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage = filePageNumber;
				dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage++;

				dbMemcpyFreeIndian(&(dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage), 
									  &(pDbDictionaryHeader->dictionaryHeaderNumberOfIndexFileEndPage), sizeof(unsigned long), 1);
				
				gDbMmuIndexPage->mmuPageNumber = dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage;
				gDbDictionaryChanged = DB_CHANGED;
			}
			gDbMmuIndexPage->mmuChanged = DB_CHANGED;
		}
	}
	else{}
}

//tfs4
int dbSetFileStoreMemorypage(unsigned long	filePageNumber, 
							   char				*targetMemoryAddress, 
							   unsigned long	filetype)
{
	if (filetype == DB_DATA_FILE_LOAD)
	{
		if (gDbFileDataFile != -1)
		{
			tfs4_close(gDbFileDataFile);
			gDbFileDataFile = -1;
		}
		
		gDbFileDataFile = tfs4_open((t_char*)gDbFileNameDataFileName, O_RDWR|O_TFS4_DIRECTIO);

		tfs4_lseek(gDbFileDataFile, (filePageNumber-1)*gDbPageSizeDataFilePageSize, SEEK_SET);

		if (dbFileWrite((unsigned long)gDbFileDataFile, (unsigned char *)targetMemoryAddress, gDbPageSizeDataFilePageSize)==DB_FUNCTION_SUCCESS)
		{
			tfs4_close(gDbFileDataFile);
			gDbFileDataFile = -1;
			return DB_FUNCTION_SUCCESS;
		}
		else
		{
			tfs4_close(gDbFileDataFile);
			gDbFileDataFile = -1;
			return DB_FUNCTION_FAIL;
		}
	}
	else if (filetype == DB_INDEX_FILE_LOAD)
	{
		if (gDbFileIndexFile !=-1)
		{
			tfs4_close(gDbFileIndexFile);
			gDbFileIndexFile = -1;
		}
		
		gDbFileIndexFile = tfs4_open((t_char*)gDbFileNameIndexFileName, O_RDWR|O_TFS4_DIRECTIO);

		tfs4_lseek(gDbFileIndexFile, (filePageNumber-1)*gDbPageSizeIndexFilePageSize, SEEK_SET);

		if (dbFileWrite((unsigned long)gDbFileIndexFile, (unsigned char *)targetMemoryAddress, gDbPageSizeIndexFilePageSize)==DB_FUNCTION_SUCCESS)
		{
			tfs4_close(gDbFileIndexFile);
			gDbFileIndexFile = -1;
			return DB_FUNCTION_FAIL;
		}
		else
		{
			tfs4_close(gDbFileIndexFile);
			gDbFileIndexFile = -1;
			return DB_FUNCTION_FAIL;
		}
	}
	else
	{
		return DB_FUNCTION_FAIL;
	}
}


//tfs4
void dbStoreDatafile(void)
{
	pMMNODE		temp;

#if ( DB_FILE_DEBUG == 1 )
	unsigned char * buf = (unsigned char *)malloc(1024*1024);
	unsigned long pos = 0;
#endif

	temp = gDbMmuDataPage;

//DbgPrintf(("=================[dbStoreDatafile:: ������ ���ϱ���� �õ��մϴ�.]=================\n"));

	//DbgPrintf(("\n\n\nData file store"));

	while(temp != DB_NULL)
	{
		//������ �ִ� page���� �����Ѵ�.
		if (temp->mmuPageNumber == 0) return;
		if (temp->mmuChanged == DB_CHANGED)
		{

//DbgPrintf(("[dbStoreDatafile:: %d Page ����� �õ��մϴ�.]\n", temp->mmuPageNumber));
//DbgPrintf(("[dbStoreDatafile:: 0x%x �ּҿ���  0x%x Size��ŭ ����ϴ°��� �õ��մϴ�.]\n", temp->mmuMemoryAddress, gDbPageSizeDataFilePageSize));

			if (gDbFileDataFile != -1)
			{
				tfs4_close(gDbFileDataFile);
				gDbFileDataFile = -1;
			}
			
			gDbFileDataFile = tfs4_open((t_char*)gDbFileNameDataFileName, O_RDWR|O_TFS4_DIRECTIO);

			//�� �Լ��� �޸𸮿� �ε�Ǿ� �ִ� ��� datapage�� datafile�� �����Ѵ�.
			tfs4_lseek(gDbFileDataFile, (temp->mmuPageNumber - 1)*gDbPageSizeDataFilePageSize, SEEK_SET);
			
			if (dbFileWrite((unsigned long)gDbFileDataFile, (unsigned char *)temp->mmuMemoryAddress, gDbPageSizeDataFilePageSize) == DB_FUNCTION_SUCCESS)
			{
				tfs4_close(gDbFileDataFile);
				gDbFileDataFile = -1;
				temp->mmuChanged = DB_UNCHANGED;
			}
			else
			{
				tfs4_close(gDbFileDataFile);
				gDbFileDataFile = -1;
			}

			#if ( DB_FILE_DEBUG == 1 )
			{
				DbgPrintf(("\n     Write 2 Dram, pos:0x%x, size:0x%x", pos, gDbPageSizeDataFilePageSize));
				if( (pos + gDbPageSizeDataFilePageSize) < (1024*1024) )
				{
					memcpy( buf + pos, (unsigned char *)temp->mmuMemoryAddress, gDbPageSizeDataFilePageSize);
					pos = pos + gDbPageSizeDataFilePageSize;
				} 
				else 
				{
					DbgPrintf(("\n     Malloc Over!"));
				}	
			}				
			#endif

		}
		temp = temp->mmuNext;
	}
	
#if ( DB_FILE_DEBUG == 1 )
//	tfs4_close(gDbFileDataFile);
//	{
//		unsigned short Pathname[12] = {'/','a','/','t','e','s','t','.','d','a','t',0};
//	
//		MemoryToFile4Debug(Pathname, buf, pos );
//		free(buf);
//	}
#endif
	
}

//tfs4
void dbStoreIndexfile(void)
{
	pMMNODE		temp;

#if ( DB_FILE_DEBUG == 1 )
	unsigned char * buf = (unsigned char *)malloc(1024*1024);
	unsigned long pos = 0;
#endif

	temp = gDbMmuIndexPage;
	
//DbgPrintf(("=================[dbStoreIndexfile:: �ε��� ���ϱ���� �õ��մϴ�.]=================\n"));

	
	//DbgPrintf(("\n\n\nIndex file write"));
	

	while(temp != DB_NULL)
	{
		if (temp->mmuPageNumber == 0) return;
		//������ �ִ� page���� �����Ѵ�.
		if (temp->mmuChanged == DB_CHANGED)
		{
//DbgPrintf(("[dbStoreIndexfile:: %d Page ����� �õ��մϴ�.]\n", temp->mmuPageNumber));
//DbgPrintf(("[dbStoreIndexfile:: 0x%x �ּҿ���  0x%x Size��ŭ ����ϴ°��� �õ��մϴ�.]\n", temp->mmuMemoryAddress, gDbPageSizeIndexFilePageSize));
			if (gDbFileIndexFile != -1)
			{
				tfs4_close(gDbFileIndexFile);
				gDbFileIndexFile = -1;
			}
			
			gDbFileIndexFile = tfs4_open((t_char*)gDbFileNameIndexFileName, O_RDWR|O_TFS4_DIRECTIO);

			//DbgPrintf(("\n DB index Changed!, dbStoreIndexfile() Page:%d", temp->mmuPageNumber));

			//�� �Լ��� �޸𸮿� �ε�Ǿ� �ִ� ��� datapage�� datafile�� �����Ѵ�.
			tfs4_lseek(gDbFileIndexFile, (temp->mmuPageNumber - 1)*gDbPageSizeIndexFilePageSize, SEEK_SET);

			if (dbFileWrite((unsigned long)gDbFileIndexFile, (unsigned char *)temp->mmuMemoryAddress, gDbPageSizeIndexFilePageSize)==DB_FUNCTION_SUCCESS)
			{
				tfs4_close(gDbFileIndexFile);
				gDbFileIndexFile = -1;
			
				temp->mmuChanged = DB_UNCHANGED;
			}
			else
			{
				tfs4_close(gDbFileIndexFile);
				gDbFileIndexFile = -1;
			}
			
			#if ( DB_FILE_DEBUG == 1 )
			{
				DbgPrintf(("\n     Write 2 Dram, pos:0x%x, size:0x%x", pos, gDbPageSizeIndexFilePageSize));

				if( (pos + gDbPageSizeDataFilePageSize) < (1024*1024) )
				{
					memcpy( buf + pos, (unsigned char *)temp->mmuMemoryAddress, gDbPageSizeIndexFilePageSize);
					pos = pos + gDbPageSizeIndexFilePageSize;
				} 
				else 
				{
					DbgPrintf(("\n     Malloc Over!"));
				}	
			}
			#endif				
		}

		temp = temp->mmuNext;
	}
	
	
#if ( DB_FILE_DEBUG == 1 )
//	tfs4_close(gDbFileIndexFile);	
//	{
//		unsigned short Pathname[12] = {'/','a','/','t','e','s','t','.','i','d','x',0};
//	
//		MemoryToFile4Debug(Pathname, buf, pos );
//		free(buf);
//	}
#endif	
	
}


#endif
//////////////////////////////////////////////////////////////////////////////////////////////////
//																								//
//									File ���� ó�� �κ�( End )									//
//																								//
//////////////////////////////////////////////////////////////////////////////////////////////////

