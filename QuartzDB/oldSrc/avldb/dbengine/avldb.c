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


//########## 전역변수 선언(begin) ##########//
char			*gDbFileNameDictionaryFileName;	//디비에서 사용하는 딕셔너리 파일의 이름
char			*gDbFileNameDataFileName;		//디비에서 사용하는 데이터 파일의 이름
char			*gDbFileNameIndexFileName;		//디비에서 사용하는 인덱스 파일의 이름


unsigned long	gDbAreaSizeDictionaryAreaSize;	//디비의 Area중에 딕셔너리가 사용하는 메모리의 크기
unsigned long	gDbAreaSizeDataAreaSize;		//디비의 Area중에 데이터가 사용하는 메모리의 크기
unsigned long	gDbAreaSizeIndexAreaSize;		//디비의 Area중에 인덱스가 사용하는 메모리의 크기

unsigned long	gDbPageSizeDataFilePageSize;	//디비에서 사용하는 데이터 파일의 Page Size
unsigned long	gDbPageSizeIndexFilePageSize;	//디비에서 사용하는 인덱스 파일의 Page Size


char			*gDbAreaDictionaryAreaStart;	//디비에서 사용하는 Area중 Dictionary Area의 Start Address
char			*gDbAreaDataAreaStart;			//디비에서 사용하는 Area중 Data Area의 Start Address
char			*gDbAreaIndexAreaStart;			//디비에서 사용하는 Area중 Index Area의 Start Address
char			*gDbMmuAreaDataMmuStart;		//디비에서 사용하는 Area중 Data MMU Area의 Start Address
char			*gDbMmuAreaIndexMmuStart;		//디비에서 사용하는 Area중 Index MMU Area의 Start Address

unsigned char	gDbSameIndian;					//디비가 동작할 시스템의 인디언 체계
pMMNODE	gDbMmuDataPage;					//디비에서 사용하는 메모리 관리 유닛중 데이터 Page를 관리하는 MMU의 시작 포인터
pMMNODE	gDbMmuIndexPage;				//디비에서 사용하는 메모리 관리 유닛중 인덱스 Page를 관리하는 MMU의 시작 포인터
unsigned char	gDbStartUpDB = DB_DO_NOT_RUNNING;	//디비가 현재 구동중인지 아닌지를 저장하는 변수

unsigned long	dbSearchCount;				//검색된 결과를 저장할 변수
unsigned long	*dbSearchArray;
unsigned char	gDbDictionaryChanged;		//딕셔너리의 변경여부 저장하는 상태변수

void 		(*gFpMemcpy)(void*, void*, unsigned long, unsigned long);

struct DB_SEARCH_DATA gDbSearchData;
//########## 전역변수 선언( end ) ##########//


#define  	AVLDB_DEBUG  		0 
#define     DB_FILE_DEBUG		0			// 060123woody
//#define 	DB_DEBUG	1

#ifdef	DB_DEBUG
void showDicMemory(unsigned long userDefine)
{
	char* baseAddress;
	unsigned long i;
	
	baseAddress = gDbAreaDictionaryAreaStart;

	DbgPrintf(("=========== 딕셔너리 메모리의 값========== "));
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

	DbgPrintf(("===========데이타 메모리의 값========== "));
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
		//DbgPrintf(("내용이 비어 있습니다."));
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
	gDbFileNameDictionaryFileName	= DB_NULL;	//디비에서 사용하는 딕셔너리 파일의 이름
	gDbFileNameDataFileName		= DB_NULL;	//디비에서 사용하는 데이터 파일의 이름
	gDbFileNameIndexFileName		= DB_NULL;	//디비에서 사용하는 인덱스 파일의 이름


	gDbAreaSizeDictionaryAreaSize	= DB_NULL;	//디비의 Area중에 딕셔너리가 사용하는 메모리의 크기
	gDbAreaSizeDataAreaSize		= DB_NULL;	//디비의 Area중에 데이터가 사용하는 메모리의 크기
	gDbAreaSizeIndexAreaSize		= DB_NULL;	//디비의 Area중에 인덱스가 사용하는 메모리의 크기

	gDbPageSizeDataFilePageSize	= DB_NULL;	//디비에서 사용하는 데이터 파일의 Page Size
	gDbPageSizeIndexFilePageSize	= DB_NULL;	//디비에서 사용하는 인덱스 파일의 Page Size


	gDbAreaDictionaryAreaStart		= DB_NULL;	//디비에서 사용하는 Area중 Dictionary Area의 Start Address
	gDbAreaDataAreaStart			= DB_NULL;	//디비에서 사용하는 Area중 Data Area의 Start Address
	gDbAreaIndexAreaStart			= DB_NULL;	//디비에서 사용하는 Area중 Index Area의 Start Address
	gDbMmuAreaDataMmuStart		= DB_NULL;	//디비에서 사용하는 Area중 Data MMU Area의 Start Address
	gDbMmuAreaIndexMmuStart		= DB_NULL;	//디비에서 사용하는 Area중 Index MMU Area의 Start Address

	gDbSameIndian					= DB_NULL;				//디비가 동작할 시스템의 인디언 체계
	gDbMmuDataPage				= DB_NULL;				//디비에서 사용하는 메모리 관리 유닛중 데이터 Page를 관리하는 MMU의 시작 포인터
	gDbMmuIndexPage				= DB_NULL;				//디비에서 사용하는 메모리 관리 유닛중 인덱스 Page를 관리하는 MMU의 시작 포인터
	gDbStartUpDB					= DB_DO_NOT_RUNNING;	//디비가 현재 구동중인지 아닌지를 저장하는 변수

	dbSearchCount					= DB_NULL;				//검색된 결과를 저장할 변수
	dbSearchArray 					= DB_NULL;
	gDbDictionaryChanged			= DB_UNCHANGED;		//딕셔너리의 변경여부 저장하는 상태변수
}


int dbApiGetDictionaryFileSize(unsigned long *pFileSize)
{
	if (gDbStartUpDB == DB_DO_RUNNING)
	{
		//DB가 Startup이 된 상황에서만 동작한다.
		pDictionaryHeader		pdicHeader;
		DictionaryHeader		vDicHeader;

		//딕셔너리 해더를 받아온다.
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
		//DB가 Startup이 된 상황에서만 동작한다.
		pDictionaryHeader		pdicHeader;
		DictionaryHeader		vDicHeader;

		//딕셔너리 해더를 받아온다.
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
		//DB가 Startup이 된 상황에서만 동작한다.
		pDictionaryHeader		pdicHeader;
		DictionaryHeader		vDicHeader;

		//딕셔너리 해더를 받아온다.
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

	//Dictionary file의 Size를 받아온다.
	if (dbApiGetDictionaryFileSize(&vTempSize) ==DB_FUNCTION_FAIL)
	{
		return DB_FUNCTION_FAIL;
	}
	*pFileSize = vTempSize;

	//Data file의 Size를 받아온다.	
	if (dbApiGetDataFileSize(&vTempSize) ==DB_FUNCTION_FAIL)
	{
		return DB_FUNCTION_FAIL;
	}
	*pFileSize += vTempSize;
	
	//Index file의 Size를 받아온다.
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
		//Database가 사용할 Indian 체계에 맞춰 함수를 Mapping한다.(being)
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
		//Database가 사용할 Indian 체계에 맞춰 함수를 Mapping한다.( end )
		
		//데이터 베이스가 사용할 화일을 지정한다.
		gDbFileNameDictionaryFileName	= dbDictionaryFileName;
		gDbFileNameDataFileName		= dbDataFileName;
		gDbFileNameIndexFileName		= dbIndexFileName;

		//데이터 베이스의 Area의 시작 Address지정
		gDbAreaDictionaryAreaStart		= dbAreaDictionaryAreaStart;
		gDbAreaDataAreaStart			= dbAreaDataAreaStart;
		gDbAreaIndexAreaStart			= dbAreaIndexAreaStart;
		gDbMmuAreaDataMmuStart		= dbMmuAreaDataMmuStart;
		gDbMmuAreaIndexMmuStart		= dbMmuAreaIndexMmuStart;


		//디비의 Area의 크기를 셋팅한다.
		gDbAreaSizeDictionaryAreaSize	= dbAreaSizeDictionaryAreaSize;
		gDbAreaSizeDataAreaSize		= dbAreaSizeDataAreaSize;
		gDbAreaSizeIndexAreaSize		= dbAreaSizeIndexAreaSize;


		//데이터 베이스에서 사용하는 데이터 파일과 인덱스 파일의 Page Size를 지정한다.
		gDbPageSizeDataFilePageSize	= dbPageSizeDataFilePageSize;
		gDbPageSizeIndexFilePageSize	= dbPageSizeIndexFilePageSize;


		//데이터 베이스가 사용하는 메모리에 관한 설정을 한다.
		//dbInitMemorySetting(&gDbAreaDictionaryAreaStart,
		//					&gDbAreaDataAreaStart,
		//					&gDbAreaIndexAreaStart, 
		//					&gDbMmuAreaDataMmuStart,
		//					&gDbMmuAreaIndexMmuStart);
		//메모리 관리 유닛을 생성하고 설정한다.
		dbMmuCreate(&gDbMmuDataPage,			&gDbMmuIndexPage, 
					gDbMmuAreaDataMmuStart,	gDbMmuAreaIndexMmuStart,
					gDbAreaDataAreaStart,		gDbAreaIndexAreaStart);

		//데이터 베이스 파일을 Load한다.
		if (dbLoadDatabaseFiles(gDbFileNameDictionaryFileName, gDbFileNameDataFileName,  gDbFileNameIndexFileName, vDictionaryLogicalSize) == DB_FUNCTION_FAIL)
		{
			//DBFile Loading에 실패를 한 경우
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

	//Memory의 시작포인터를 할당한다.
//	*dbAreaDictionaryAreaStart	= (char*)malloc(gDbAreaSizeDictionaryAreaSize);
	
//	*dbAreaDataAreaStart		= (char*)malloc(gDbAreaSizeDataAreaSize);
	
//	*dbAreaIndexAreaStart		= (char*)malloc(gDbAreaSizeIndexAreaSize);
	
//	*dbMmuAreaDataMmuStart		= (char*)malloc(sizeof(MMNODE)*(gDbAreaSizeDataAreaSize/gDbPageSizeDataFilePageSize));
	
//	*dbMmuAreaIndexMmuStart		= (char*)malloc(sizeof(MMNODE)*(gDbAreaSizeIndexAreaSize/gDbPageSizeIndexFilePageSize));
/*
	// porting 가이드//
	*dictionaryAreaStart	= (char*)DB_DICTIONARY_AREA_START;
	
	*dataAreaStart		= (char*)DB_DATA_AREA_SATAT;
	
	*indexAreaStart		= (char*)DB_INDEX_AREA_START;
	
	*mmuDatStart			= (char*)DB_GLOBAL_AREA_DATA_MMU_START;
	
	*mmuIdxStart			= (char*)DB_GLOBAL_AREA_INDEX_MMU_START;
*/
}

 
void dbMmuCreate(pMMNODE	*dbMmuDataPage,				//data area의 MMU의 시작을 포인팅할 변수
				 pMMNODE	*dbMmuIndexPage,			//index area의 MMU의 시작을 포인팅할 변수
				 char		*dbMmuAreaDataMmuStart,		//data area의 MMU가 저장되기 위한 공간의 시작번지
				 char		*dbMmuAreaIndexMmuStart,	//index area의 MMU가 저장되기 위한 공간의 시작번지
				 char		*dbAreaDataAreaStart,		//mmu에 의해 관리될 data area의 시작번지
				 char		*dbAreaIndexAreaStart)		//mmu에 의해 관리될 index area의 시작번지
{
	//이 함수는 메모리를 관리하기 위한 MMU블럭을 생성하는 모듈이다.
	unsigned long	i;					//counter로 사용될 임시변수
	unsigned long	dbMmuDataMmuNum;	//DATA AREA의 메모리 관리 블럭의 갯수를 저장할 변수
	unsigned long	dbMmuIndexMmuNum;	//INDEX AREA의 메모리 관리 블럭의 갯수를 저장할 변수

	//Memory를 page를 관리하기 위하여 Index와 Data page가 몇개 올라올수 있는지 계산한다.
	dbMmuDataMmuNum	= (gDbAreaSizeDataAreaSize)/(gDbPageSizeDataFilePageSize);
	dbMmuIndexMmuNum	= (gDbAreaSizeIndexAreaSize)/(gDbPageSizeIndexFilePageSize);

	*dbMmuDataPage  = (pMMNODE)dbMmuAreaDataMmuStart;
	*dbMmuIndexPage = (pMMNODE)dbMmuAreaIndexMmuStart;
	
	//Data Mmu를 생성한다.
	for (i = 0; i < dbMmuDataMmuNum; i++ )
	{
		((*dbMmuDataPage) + i)->mmuPageNumber		= 0;
		((*dbMmuDataPage) + i)->mmuChanged			= DB_UNCHANGED;
		((*dbMmuDataPage) + i)->mmuMemoryAddress	= dbAreaDataAreaStart + (i*(gDbPageSizeDataFilePageSize));
		((*dbMmuDataPage) + i)->mmuNext				= (i == (dbMmuDataMmuNum - 1)) ? 0 : ((*dbMmuDataPage) + i + 1);
	}

	//Index Mmu를 생성한다.
	for (i = 0; i < dbMmuIndexMmuNum; i++ )
	{
		((*dbMmuIndexPage) + i)->mmuPageNumber		= 0;
		((*dbMmuIndexPage) + i)->mmuChanged			= DB_UNCHANGED;
		((*dbMmuIndexPage) + i)->mmuMemoryAddress	= dbAreaIndexAreaStart + (i*(gDbPageSizeIndexFilePageSize));
		((*dbMmuIndexPage) + i)->mmuNext			= (i == (dbMmuIndexMmuNum - 1)) ? 0 : ((*dbMmuIndexPage) + i + 1);
	}
}



//################### 새로운 코딩룰 대로 변환하면서 작업하는 부분 ( end )########################//

unsigned char dbGetLruListPage(pMMNODE	**start, 
								unsigned long	pageNumber)
{
	//MMU의 List에서 해당 페이지를 찾으면 1 못찾으면 0을 리턴한다.
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
	//MMU의 리스트에서 lastAccessPage를 리스트의 가장 앞에 배치한다.
	pMMNODE firstpage	= *lastAccessPage;
	*lastAccessPage	= (*lastAccessPage)->mmuNext;
	firstpage->mmuNext	= *start;
	*start				= firstpage;
}


void dbTouch(unsigned long offset, 
		      unsigned long filetype)
{
//==================================================================================================//
//	dbTouch 함수는 해당 offset이 저장되어 있는 data page 또는 index page를 메모리에 load시켜주고		//
//	각각의 메모리 관리 링크드 리스트의 가장앞에 위치 시켜줌으로써, datapage와 indexpage라는 두변수로//
//	자신의 메모리보다 터 큰 index file과 data file사용을 가능하게 해준다.							//
//==================================================================================================//
	pMMNODE		*temp,
					*target;
	unsigned	long		pageNumber;	//현재 찾고자 하는 offset이 저장되어 있는 page번호를 저장할 변수


	if (filetype == DB_DATA_FILE_LOAD)		//datafile
	{
		pageNumber	= (offset/gDbPageSizeDataFilePageSize) + 1;
		temp		= &gDbMmuDataPage;			//datapage에는 항상 찾는 memory offset의 기준값이 들어간다.
		target		= &gDbMmuDataPage;
	}
	else if (filetype == DB_INDEX_FILE_LOAD)	//indexfile
	{
		pageNumber = (offset/gDbPageSizeIndexFilePageSize) + 1;
		temp		= &gDbMmuIndexPage;			//datapage에는 항상 찾는 memory offset의 기준값이 들어간다.
		target		= &gDbMmuIndexPage;
	}
	else{}


	if (dbGetLruListPage(&temp,pageNumber))
	{	//*temp에는 찾는 Node가 들어간다.
		//memory page내에 찾는 Page가 있을경우 : 존재하기 때문에 LRU List만 수정하면 끝
		dbMaintainLruList(target, temp);
	}
	else
	{
		//memory page내에 찾는 Page가 없을경우 : LRU List를 수정하고 , page교체를 해야한다.
		dbMaintainLruList(target, temp);
		
		if ((*target)->mmuChanged == DB_UNCHANGED)	//교체를 하려는 Page가 변경된 사항이 없는 경우
		{
			(*target)->mmuPageNumber = pageNumber;
			dbSetFileLoadMemorypage(pageNumber, (*target)->mmuMemoryAddress, filetype);
			(*target)->mmuChanged= DB_UNCHANGED;
		}
		else	//교체를 하려는 Page가 변경된 사항이 있는 경우
		{
			//미리 올라온 저장하고
			if (dbSetFileStoreMemorypage((*target)->mmuPageNumber, (*target)->mmuMemoryAddress, filetype)==DB_FUNCTION_SUCCESS)
			{
				//page를 load한다.
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

	//딕셔너리 해더를 받아온다.
	pdicHeader = dbGetDictionaryHeader();

	dbMemcpyFreeIndian(pdicHeader, &vDicHeader, sizeof(unsigned long), 6);

	//Memory에 Load될 수 있는 Page의 수와 실제로 구성되어 있는 Page의 수중에 작은 값을 취한다.
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

	//Data file과 Index file의 load한다
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


	//딕셔너리 해더를 받아온다.
	pdicHeader = dbGetDictionaryHeader();
	
	//이것이 끝나고 나면 노드를 생성한후에 들어갈 값은 ptable에 저장하면된다.
	dbTableSetCreateTab(&(pdicHeader->dictionaryHeaderOffsetOfRootTable), tablename);

	//dictionary의 변경이 발생한 것을 체크
	gDbDictionaryChanged = DB_CHANGED;
}

void dbTableSetNode(char				*target, 
					   DB_VARCHAR		*tablename)
{
	TableNode				TABLE_NODE;

	//테이블 노드를 셋팅한다.
	TABLE_NODE.tableNodeComplexIndexOffset	= 0;
	TABLE_NODE.tableNodeLeft				= 0;
	TABLE_NODE.tableNodeRight				= 0;
	TABLE_NODE.tableNodeColumnOffset		= 0;
	TABLE_NODE.tableNodeStartPage			= 0;
	TABLE_NODE.tableNodeHeight				= 1;

	//테이블 노드를 기록한다.
	dbMemcpyFreeIndian(&TABLE_NODE, target, sizeof(unsigned long), 6);

	//테이블의 이름을 기록한다.
	dbStringCopy((DB_VARCHAR*)(target + sizeof(TableNode)), tablename);
}

unsigned long dbStringCopy(DB_VARCHAR* target, 
							 DB_VARCHAR* string)
{
	unsigned long		vStringSize = 0;
	unsigned short	vChar;
	
	//이 함수의 전달인자 String의 포인터는 aline이 맞다는 가정이 전제이다.
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

	//이 함수의 전달인자 String의 포인터는 aline이 맞다는 가정이 전제이다.
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

	//이 함수의 전달인자 String의 포인터는 aline이 맞다는 가정이 전제이다.
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
	//이 함수는 크게 두가지 일을 한다.
	//첫번째일은 삽입될 곳을 찾아 가고 삽입을 하는 일이고,
	//나머지는 역으로 삽입하기 위해 찾아간길을 돌아오면서 트리를 재구성하는 일이다.
	
	pDictionaryHeader		pdicHeader;	
	unsigned	long		vOffset;
	signed		long		vBalanceFactor;

	pdicHeader		= dbGetDictionaryHeader();
	
	dbMemcpyFreeIndian(tableOffset, &vOffset, sizeof(unsigned long), 1);	//현재 노드를 가르키는 offset의 값

	if (vOffset == 0)		//삽입이 일어나야 하는 지점에 왔을때 처리
	{
		unsigned	long		needsize;
		unsigned	long		vSizeOfDictionary;
		
		//딕셔너리의 현재 크기를 읽어온후.
		dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderSizeOfDictionary), &vSizeOfDictionary, sizeof(unsigned long), 1);
		
		//테이블이 삽입되기 위하여 필요한 사이즈를 계산한다.
		needsize	= sizeof(TableNode) + dbStringGetSize(tableName) + DB_VARCHAR_SIZE;
		
		
		//테이블의 노드를 생성한다.
		dbTableSetNode(((char*)pdicHeader) + vSizeOfDictionary, tableName);
		
		
		
		//포인팅 값을 설정하고.
		dbMemcpyFreeIndian(&vSizeOfDictionary, tableOffset, sizeof(unsigned long), 1);

		//딕셔너리의 현재 크기를 증가시킨다..
		vSizeOfDictionary += needsize;
		dbMemcpyFreeIndian(&vSizeOfDictionary, &(pdicHeader->dictionaryHeaderSizeOfDictionary), sizeof(unsigned long), 1);
			
		//딕셔너리의 내용이 바뀌었음을 알린다.
		gDbDictionaryChanged = DB_CHANGED;
	}
	else	//삽입이 일어나기 위해 찾아가는 과정과 찾은후 처리
	{
		pTableNode					ptable;		//딕셔너리 상에서 현재 비교를 하려고 하는 노드 
		signed		char		result;		//현재 포인팅하고 있는 노드와 생성하려는 테이블 이름을 비교결과를 저장할 변수
		unsigned	long		*tableOffsetBackup;

		//dictionary 상의 테이블 노드를 포인팅 한다.
		ptable = (pTableNode)(((char*)pdicHeader) + vOffset);
		
		result = dbStringCompare_AREAVs_STACK( (DB_VARCHAR*)(ptable + 1), tableName);
		
		tableOffsetBackup = tableOffset;
		
		if (result == 0)	//해당 문자열이 일치하는 경우
		{
			return;		//동일한 이름의 Table이 Database에 존재한다.
		}
		else if (result == -1)
		{
			tableOffset = &(ptable->tableNodeLeft);
		}
		else
		{
			tableOffset = &(ptable->tableNodeRight);
		}

		//########### recursive수행부분(시작) ##########//
		dbTableSetCreateTab(tableOffset, tableName);
		//########### recursive수행부분 (끝)  ##########//


		//해당 노드의 balanceFactor를 계산한다.
		vBalanceFactor = dbTableGetBalanceFactor(vOffset);
		
		if (vBalanceFactor == 2)		//해당 노드의 balanceFactor가 깨진경우
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
		else if (vBalanceFactor == -2)	//해당 노드의 balanceFactor가 깨진경우
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
		else			// 해당 노드의 balanceFactor가 안깨진경우
		{
			//할일이 없다.
		}	

		//해당 노드의 높이를 다시 계산한다.
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
		balanceFactor = 0;		//이러한 경우 table이 존재하지 않으므로 balance factor가 존재하지 않는다.
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
				//할일 없다.
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
	//이 함수는 디비 Area에 있는 스트링값에서 관사를 지난 최초의 스트링값을 포인팅하도록 한다.
	unsigned short	vLowerCaseMask = 0x20;
	unsigned short	vChar1, vChar2, vChar3, vChar4;

	dbAreaMemoryString = dbStringRTrimArea(dbAreaMemoryString); //공백문자를 건너뛴다..
	
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
		//관사 a가 있는 경우
		//dbAreaMemoryString = dbStringRTrimStack(dbAreaMemoryString + 2);
		dbAreaMemoryString = dbStringRTrimArea(dbAreaMemoryString + 2);
	}
	else if ((vChar1 == 'a') && (vChar2 == 'n') && (vChar3 == ' '))
	{
		//관사 an이 있는 경우
		//dbAreaMemoryString = dbStringRTrimStack(dbAreaMemoryString + 3);
		dbAreaMemoryString = dbStringRTrimArea(dbAreaMemoryString + 3);
	}
	else if ((vChar1 == 't') && (vChar2 == 'h') && (vChar3 == 'e') && (vChar4 == ' '))
	{
		//관사 the이 있는 경우
		//dbAreaMemoryString = dbStringRTrimStack(dbAreaMemoryString + 4);
		dbAreaMemoryString = dbStringRTrimArea(dbAreaMemoryString + 4);
	}
	else
	{}

	return dbAreaMemoryString;
}


unsigned short* dbStringWithoutArticleStack(DB_VARCHAR	*dbStackMemoryString)
{
	//이 함수는 디비 Area에 있는 스트링값에서 관사를 지난 최초의 스트링값을 포인팅하도록 한다.
	unsigned short vLowerCaseMask = 0x20;

	dbStackMemoryString = dbStringRTrimStack(dbStackMemoryString); //공백문자를 건너뛴다..

	if ((((*dbStackMemoryString)|vLowerCaseMask) == 'a') 
		&& (((*(dbStackMemoryString + 1))|vLowerCaseMask) == ' ')
		&&((*(dbStackMemoryString + 1))!=0))
	{
		//관사 a가 있는 경우
		dbStackMemoryString = dbStringRTrimStack(dbStackMemoryString + 2);
	}
	else if ((((*dbStackMemoryString)|vLowerCaseMask) == 'a') 
		&& (((*(dbStackMemoryString+1))|vLowerCaseMask) == 'n') 
		&& (((*(dbStackMemoryString+2))|vLowerCaseMask) == ' ')
		&&((*(dbStackMemoryString + 2))!=0))
	{
		//관사 an이 있는 경우
		dbStackMemoryString = dbStringRTrimStack(dbStackMemoryString + 3);
	}
	else if ((((*dbStackMemoryString)|vLowerCaseMask) == 't') 
		&& (((*(dbStackMemoryString+1))|vLowerCaseMask) == 'h') 
		&& (((*(dbStackMemoryString+2))|vLowerCaseMask) == 'e') 
		&& (((*(dbStackMemoryString+3))|vLowerCaseMask) == ' ')
		&&((*(dbStackMemoryString + 3))!=0))
	{
		//관사 the이 있는 경우
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

//DbgPrintf(("관사 제거하기 전에 두개의 문자열의 시작 Address{%x, %x}\n", dbAreaMemory1Bak, dbAreaMemory2Bak));
	dbAreaMemory1	= dbStringWithoutArticleArea(dbAreaMemory1);
	dbAreaMemory2	= dbStringWithoutArticleArea(dbAreaMemory2);
//DbgPrintf(("관사 제거한 후의 두개의 문자열의 시작 Address{%x, %x}\n", dbAreaMemory1Bak, dbAreaMemory2Bak));
//showTwoByteString("관사를 제거한 dbAreaMemory1", dbAreaMemory1);
//showTwoByteString("관사를 제거한 dbAreaMemory2", dbAreaMemory2);
	while(1)
	{
		dbMemcpyFreeIndian(dbAreaMemory1, &AREA_CHAR1, DB_VARCHAR_SIZE, 1);
		dbMemcpyFreeIndian(dbAreaMemory2, &AREA_CHAR2, DB_VARCHAR_SIZE, 1);
		

		{//영문자 범위에 드는 문자라면 소문자로 만든다.
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
//DbgPrintf(("두문자열이 일치하여 다시 대소문자구분하여 비교하는 루틴으로 진입한다."));
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

		{//영문자 범위에 드는 문자라면 소문자로 만든다.
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
	
	// ==== MAX_NODE 처리 ==== //
	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;		//변경될 것이므로 미리 변경
	pMax=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMax, &MAX_NODE, sizeof(unsigned long), 4);
	
	// ==== MIN_NODE 처리 ==== //
	dbTouch(*l2, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;		//변경될 것이므로 미리 변경
	pMin=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l2)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMin, &MIN_NODE, sizeof(unsigned long), 4);
	
	// ==== MID_NODE 처리 ==== //
	dbTouch(*l3, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;		//변경될 것이므로 미리 변경
	pMid=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l3)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMid, &MID_NODE, sizeof(unsigned long), 4);

	MIN_NODE.indexNodeRight	= MID_NODE.indexNodeLeft;
	MAX_NODE.indexNodeLeft	= MID_NODE.indexNodeRight;
	MID_NODE.indexNodeRight	= *l1;
	MID_NODE.indexNodeLeft	= *l2;

	dbMemcpyFreeIndian(&MID_NODE, pMid, sizeof(unsigned long), 4);	
	dbMemcpyFreeIndian(&MAX_NODE, pMax, sizeof(unsigned long), 4);	
	dbMemcpyFreeIndian(&MIN_NODE, pMin, sizeof(unsigned long), 4);	

	
	//min의 트리높에 다시계산
	MIN_NODE.indexNodeHeight = dbIndexGetCalculateHeight(MIN_NODE.indexNodeLeft, MIN_NODE.indexNodeRight);
	
	dbTouch(*l2, DB_INDEX_FILE_LOAD);
	pMin = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l2)%gDbPageSizeIndexFilePageSize);
	
	dbMemcpyFreeIndian(&(MIN_NODE.indexNodeHeight), &(pMin->indexNodeHeight), sizeof(unsigned long), 1);
	
	//max의 트리 높이 다시계산
	MAX_NODE.indexNodeHeight = dbIndexGetCalculateHeight(MAX_NODE.indexNodeLeft, MAX_NODE.indexNodeRight);
	
	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	pMax = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	
	dbMemcpyFreeIndian(&(MAX_NODE.indexNodeHeight), &(pMax->indexNodeHeight), sizeof(unsigned long), 1);

	//root인 *l1값 변경
	*l1 = *l3;
}

void dbIndexRotationLL(unsigned long *l1, 
					 unsigned long *l2, 
					 unsigned long *l3)
{
	pIndexNode			pMax, pMid;
	IndexNode			MAX_NODE, MID_NODE;

	// ==== MAX_NODE 처리 ==== //
	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;		//변경될 것이므로 미리 변경
	pMax = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMax, &MAX_NODE, sizeof(unsigned long), 4);
	
	// ==== MID_NODE 처리 ==== //
	dbTouch(*l2, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;		//변경될 것이므로 미리 변경
	pMid = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l2)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMid, &MID_NODE, sizeof(unsigned long), 4);

	MAX_NODE.indexNodeLeft	= MID_NODE.indexNodeRight;
	MID_NODE.indexNodeRight	= *l1;
	
	//rootNode가 될 노드를 저장한다.
	dbMemcpyFreeIndian(&MID_NODE, pMid, sizeof(unsigned long), 4);
	dbMemcpyFreeIndian(&MAX_NODE, pMax, sizeof(unsigned long), 4);	

	//max의 트리높에 다시계산
	MAX_NODE.indexNodeHeight = dbIndexGetCalculateHeight(MAX_NODE.indexNodeLeft, MAX_NODE.indexNodeRight);

	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	pMax = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	
	dbMemcpyFreeIndian(&(MAX_NODE.indexNodeHeight), &(pMax->indexNodeHeight), sizeof(unsigned long), 1);

	//root의 값을 *l2값으로 변경
	*l1 = *l2;
}

void dbIndexRotationRR(unsigned long *l1, 
					 unsigned long *l2, 
					 unsigned long *l3)
{
	pIndexNode			pMin, pMid;
	IndexNode			MIN_NODE, MID_NODE;

	// =====MIN_NODE처리 ==== //
	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;		//변경될 것이므로 미리 변경
	pMin=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMin, &MIN_NODE, sizeof(unsigned long), 4);

	// =====MID_NODE처리 ==== //
	dbTouch(*l2, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;
	pMid = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l2)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMid, &MID_NODE, sizeof(unsigned long), 4);

	MIN_NODE.indexNodeRight	= MID_NODE.indexNodeLeft;
	MID_NODE.indexNodeLeft	= *l1;

	dbMemcpyFreeIndian(&MID_NODE, pMid, sizeof(unsigned long), 4);	
	dbMemcpyFreeIndian(&MIN_NODE, pMin, sizeof(unsigned long), 4);	

	//root가 될 노드를 저장한다.
	//dbMemcpy(&MID_NODE, pMid, sizeof(INODE));

	//min의 트리높에 다시계산
    MIN_NODE.indexNodeHeight = dbIndexGetCalculateHeight(MIN_NODE.indexNodeLeft, MIN_NODE.indexNodeRight);

	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	pMin = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(&(MIN_NODE.indexNodeHeight), &(pMin->indexNodeHeight), sizeof(unsigned long), 1);

	//root의 값을 *l2값으로 변경
	*l1 = *l2;
}

void dbIndexRotationRL(unsigned long *l1,
					 unsigned long *l2, 
					 unsigned long *l3)
{
	pIndexNode			pMin, pMid, pMax;
	IndexNode			MIN_NODE, MID_NODE, MAX_NODE;

	// ==== MIN_NODE 처리 ==== //
	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;			//변경될 것이므로 미리 변경
	pMin=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMin, &MIN_NODE, sizeof(unsigned long), 4);

	// ==== MAX_NODE 처리 ==== //
	dbTouch(*l2, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;			//변경될 것이므로 미리 변경
	pMax=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l2)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMax, &MAX_NODE, sizeof(unsigned long), 4);

	// ==== MID_NODE 처리 ==== //
	dbTouch(*l3, DB_INDEX_FILE_LOAD);
	gDbMmuIndexPage->mmuChanged = DB_CHANGED;			//변경될 것이므로 미리 변경
	pMid=(pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l3)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(pMid, &MID_NODE, sizeof(unsigned long), 4);

	MIN_NODE.indexNodeRight	= MID_NODE.indexNodeLeft;
	MAX_NODE.indexNodeLeft	= MID_NODE.indexNodeRight;
	MID_NODE.indexNodeLeft	= *l1;
	MID_NODE.indexNodeRight	= *l2;
	
	dbMemcpyFreeIndian(&MID_NODE, pMid, sizeof(unsigned long), 4);	
	dbMemcpyFreeIndian(&MAX_NODE, pMax, sizeof(unsigned long), 4);	
	dbMemcpyFreeIndian(&MIN_NODE, pMin, sizeof(unsigned long), 4);	

	//min의 트리높이 다시계산
	MIN_NODE.indexNodeHeight = dbIndexGetCalculateHeight(MIN_NODE.indexNodeLeft, MIN_NODE.indexNodeRight);
	MAX_NODE.indexNodeHeight = dbIndexGetCalculateHeight(MAX_NODE.indexNodeLeft, MAX_NODE.indexNodeRight);
	
	dbTouch(*l1, DB_INDEX_FILE_LOAD);
	pMin = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l1)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(&(MIN_NODE.indexNodeHeight), &(pMin->indexNodeHeight), sizeof(unsigned long), 1);

	//max의 트리높이 다시계산
	dbTouch(*l2, DB_INDEX_FILE_LOAD);
	pMax = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + (*l2)%gDbPageSizeIndexFilePageSize);
	dbMemcpyFreeIndian(&(MAX_NODE.indexNodeHeight), &(pMax->indexNodeHeight), sizeof(unsigned long), 1);

	//root의 값을 *l3값으로 변경
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
	// =========================지역 변수 선언(시작) =========================//
	pDictionaryHeader		pdicHeader;			//Dictionary file의 해더를 포인팅할 변수
	pTableNode			pTable;				//Dictionary file이 올라오는 영역에서의 테이블을 포인팅할 변수
	TableNode			TABLE_NODE;
	unsigned	long			*ptableOffset,		//Dictionary file이 올라오는 영역에서 테이블의 offset을 저장하고 위치를 포인팅할 변수
						vTableOffset,
						*pcolumnOffset;		//Dictionary file이 올라오는 영역의 테이블에서 시작 컬럼의 offset을 저장하고 위치를 포인팅할 변수
	unsigned	long			requirementSize;
	// =========================지역 변수 선언(끝) =========================//

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


	pdicHeader	= dbGetDictionaryHeader();						//Dictionary를 지정한다.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정
	
	//테이블을 찾는다.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : 입력으로 들어온 tableName의 table이 시스템에 존재하지않는 경우
		#if DB_SHOW_ERROR_CODE						//Error Check
		gDbErrorCode = DB_ERROR_TABLE_NOT_FOUND;		//Error Check
		#endif											//Error Check

		return;
	}
	// ===========아래부터 해당 테이블을 찾았을때 처리 ===========//

	//table의 offset이 저장되어 있다.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table의 메모리를 지정한다.
	pTable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//테이블의 내용을 TABLE_NODE에 복사한다.
	dbMemcpyFreeIndian(pTable, &TABLE_NODE, sizeof(unsigned long), 6);

	if (TABLE_NODE.tableNodeStartPage != 0)
	{
		//테이터삽입이 있었다면 현 상태에서 컬럼을 생성할 수 없다.
		#if DB_SHOW_ERROR_CODE							//Error Check
		gDbErrorCode = DB_ERROR_TABLE_ALLOC_PAGE_NONE;	//Error Check
		#endif												//Error Check
		
		return;
	}

	//컬럼의 시작 offset이 저장되어 있는 주소를 포인팅한다.
	pcolumnOffset = &(pTable->tableNodeColumnOffset);

	//컬럼을 찾는다.
	if(dbTableFindColumnName(&pcolumnOffset, columnName))
	{
		//ERROR : 해당 컬럼이름의 컬럼이 이미 존재하는 경우
		#if DB_SHOW_ERROR_CODE								//Error Check
		gDbErrorCode = DB_ERROR_COLUMN_NAME_ALREADY_EXIST;	//Error Check
		#endif													//Error Check

		return;
	}

	//컬럼이 삽입되기 위해 필요한 사이즈를 계산한다.
	requirementSize = sizeof(ColumnNode) + dbStringGetSize(columnName) + DB_VARCHAR_SIZE;

	
	//해당이름의 컬럼이 존재하지 않는 경우 : 이제 컬럼을 만들어도 된다..^^
	{
		unsigned long	dictionarySize;

		dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderSizeOfDictionary), &dictionarySize, sizeof(unsigned long), 1);

		dbColumnSetNode(((char*)pdicHeader) + dictionarySize, columnName, type);
		dbMemcpyFreeIndian(&dictionarySize, pcolumnOffset, sizeof(unsigned long), 1);

		dictionarySize += requirementSize;
		dbMemcpyFreeIndian(&dictionarySize, &(pdicHeader->dictionaryHeaderSizeOfDictionary), sizeof(unsigned long), 1);
		
		gDbDictionaryChanged = DB_CHANGED;						//dictionary의 변경이 발생한 것을 체크
	}
}

void dbColumnSetNode(char			*target, 
						DB_VARCHAR		*columnName, 
						DB_COLUMN_TYPE	type)
{
	ColumnNode				columnNode;

	//컬럼 노드를 셋팅한다.
	columnNode.columnNodeDataType			= type;
	columnNode.columnNodeIndexRootOffset	= 0;
	columnNode.columnNodeNextColumnOffset	= 0;

	//컬럼 노드를 기록한다.
	dbMemcpyFreeIndian(&columnNode, target, sizeof(unsigned long), 3);

	//컬럼의 이름을 기록한다.
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
		//딕셔너리에 있는 컬럼의 OFFSET값을 복사한다.
		dbMemcpyFreeIndian(*source, &columnOffset, sizeof(unsigned long), 1);

		if (columnOffset == 0)
		{
			return 0;		//마지막 컬럼까지 온경우
		}
		else
		{
			//해당 Column을 지정한다.
			pcolumn = (pColumnNode)(((char*)pdicHeader) + columnOffset);

			if (dbStringCompare_AREAVs_STACK((DB_VARCHAR*)(pcolumn + 1), columnName) == 0)
			{
				return 1;	//해당 노드를 찾은 경우
			}
			else
			{
				*source = &(pcolumn->columnNodeNextColumnOffset);	//다음노드를 찾아간다.
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
			//테이블을 못찾은 경우
			return 0;
		}
		else
		{
			//해당 table을 지정한다.
			ptable = (pTableNode)(((char*)pdicHeader) + tableOffset);

			//문자열을 비교한다
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
	// =========================지역 변수 선언(시작) =========================//
	pDictionaryHeader		pdicHeader;	//dictionary header를 포인팅할 변수
	pTableNode			ptable;			//dictionary file의 table을 포인팅할 변수
	pColumnNode			pcolumn;
	unsigned	long			*ptableOffset,		//dictionary file이 올라오는 memory영역의 테이블의 offset 저장부분을 포인팅 할 변수
						*pcolumnOffset;	//dictionary file이 올라오는 memory영역의 컬럼의 offset 저장부분을 포인팅 할 변수
	unsigned	long		vTableOffset,
					vColumnOffset,
					vIndexOffset;
	// =========================지역 변수 선언(끝) =========================//

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

	pdicHeader		= dbGetDictionaryHeader();				//dictionary header를 지정한다.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table의 시작 offset을 저장하는 곳을 지정한다.
	
	//해당 테이블이 있는지를 검사한다.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		return;		//해당 테이블이 없는 경우 함수를 종료한다.
	}
	//위의 if문에 걸리지 않았다면 해당테이블이 존재하는 경우이다
	
	//*tempTableNode에는 Offset이 저장되어 있다.
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
	
	//해당 컬럼이 있는지를 검사한다.
	if (!dbTableFindColumnName(&pcolumnOffset, columnName))
	{
		//해당 컬럼이 존재하지 않는경우
		return;
	}
	//위의 if문에 걸리지 않았다면 해당 컬럼이 존재하는 경우이다.
	
	dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);
	//해당컬럼에 인덱스가 있는지를 검사한다.
	
	pcolumn = (pColumnNode)( ((char*)pdicHeader) + vColumnOffset);

	dbMemcpyFreeIndian(&(pcolumn->columnNodeIndexRootOffset), &vIndexOffset, sizeof(unsigned long), 1);
	
	if (vIndexOffset != 0)
	{
		//이미 해당 컬럼에 인덱스가 할당되어 있는경우이다.
		return;
	}
	//위의 if문에 걸리지 않았다면 해당 컬럼에 인덱스가 설정되어 있지 않은 경우이다.
	
	//해당컬럼에 인덱스가 설정되지 않은경우 인덱스를 설정한다.
	vIndexOffset = 1;
	dbMemcpyFreeIndian(&vIndexOffset, &(pcolumn->columnNodeIndexRootOffset), sizeof(unsigned long), 1);
		
	//dictionary의 변경이 발생한 것을 체크
	gDbDictionaryChanged = DB_CHANGED;
}

unsigned long dbIndexGetCnode(DB_VARCHAR*		columnName[],
								 unsigned long		columnNumber)
{
//==================================================================================//
//	아래의 함수dbIndexGetNode는 전달인자로 받은 복합인덱스 컬럼의 이름을			//
//	파라메터로 받아서 딕셔너리 파일에 추가한다.										//
//	return결과는 해당 노드의 사이즈이다.											//
//==================================================================================//
	pDictionaryHeader		pdicHeader;			//dictionary 해더를 포인팅 할 변수
	pComplexIndexNode					pcomplexIndexNode;	//복합인덱스를 포인팅할 변수
	ComplexIndexNode					COMPLEX_INDEX_NODE;
	unsigned	long		dictionarySize;
	unsigned	long		requirementSize;		//복합인덱스가 생성되는데 필요한 공간의 갯수를 저장할 변수
	

	pdicHeader			 = dbGetDictionaryHeader();	//딕셔너리 해더를 지정한다.
	
	dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderSizeOfDictionary), &dictionarySize, sizeof(unsigned long), 1);
	
	//복합인덱스를 기록할 Dictionary AREA상의 위치
	pcomplexIndexNode = (pComplexIndexNode)(((char*)pdicHeader) + dictionarySize);
	
	COMPLEX_INDEX_NODE.complexIndexNodeColumnNumber		= columnNumber;
	COMPLEX_INDEX_NODE.complexIndexNodeIndexRootOffset	= 1;
	COMPLEX_INDEX_NODE.complexIndexNodeNext				= 0;
	
	//복합인덱스의 구조를 Dictionary AREA에 복사한다.
	dbMemcpyFreeIndian(&COMPLEX_INDEX_NODE, pcomplexIndexNode, sizeof(unsigned long), 3);

	//사이즈를 계산한다.
	requirementSize = sizeof(ComplexIndexNode);

	//복합인덱스의 컬럼의 이름을 차래로 기록한다.
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

	return requirementSize;		//노드가 차지하는 크기를 리턴한다.
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

	// === 1.우선 Table이 있는지를 찾는다. === //
	pdicHeader		= dbGetDictionaryHeader();			//딕셔너리 해더를 지정한다.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//테이블의 시작offset을 포인팅한다.

	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		return;	//이것에 들어오면 찾는 테이블이 없는 경우이다.
	}//이 if를 빠져 나오면 ptableOffset는 해당 테이블을 가르킨다.
	
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

	// ===  2.해당 컬럼이 존재하는지 검사한다. === 
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
				return;		//해당 컬럼이 없는경우
			}
		}
		//해당 컬럼도 모두 존재하는 경우이다.
	}

	// ===  3. 딕셔너리 AREA에 새로운 노드를 추가하고 그 길이를 리턴받는다. ===  
	requirementSize = dbIndexGetCnode(columnNames, columnNumber);	
	
	// === 4. 복합인덱스를 연결시키고, 딕셔너리 해더를 갱신한다. === //
	{
		// === 4.1 복합인덱스를 연결시킨다.
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
		
		// === 4.2 딕셔너리 해더를 갱신한다.
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
	// =====================지역 변수 선언(시작) =====================//
	pDictionaryHeader		pdicHeader;		//dictionary header를 지정할 변수
	pColumnNode				pcolumn;			//dictionary memory영역에서 column구조체를 포인팅 할 변수
	unsigned	long		columnDataType;
	unsigned	long		recordSize;	//record의 size를 저장할 변수
	unsigned	long		columnNumber;
	// =====================지역 변수 선언(끝) =====================//
	
	if (startColumnOffset == 0)
	{
		return 0;		//컬럼이 존재하지 않을 경우
	}

	pdicHeader = dbGetDictionaryHeader();	//dictionary header를 지정한다.
	
	recordSize		= 0;	//record길이의 초기사이즈
	columnNumber	= 0;	//컬럼의 순서를 저장할 임시변수
	
	while(startColumnOffset)
	{
		pcolumn = (pColumnNode)(((char*)pdicHeader) + startColumnOffset);	//해당컬럼을 지정한다.
		
		dbMemcpyFreeIndian(&(pcolumn->columnNodeDataType), &columnDataType, sizeof(unsigned long), 1);		//메모리에 있는 컬럼의 내용을 변수에 삽입한다.

		if (columnDataType == DB_VARCHAR_TYPE)
		{
			//recordSize = recordSize + (2*(((struct mString*)(contents[columnNumber]))->NumChars)) + DB_VARCHAR_SIZE;	//060110 김희철
		
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
		{//추후에 생실지 모르는 DATATYPE들 
		}

		//다음번째 처리할 컬럼의 번호로 columnNumber를 증가시켜준다.
		columnNumber++;

		//다음 컬럼의 offset을 지정한다.
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
	// =====================지역 변수 선언(시작) =====================//
	pDictionaryHeader		pdicHeader;		//dictionary header를 지정할 변수
	pColumnNode				pcolumn;			//dictionary memory영역에서 column구조체를 포인팅 할 변수
	unsigned	long		columnDataType;
	unsigned	long		recordSize;	//record의 size를 저장할 변수
	unsigned	long		columnNumber;
	// =====================지역 변수 선언(끝) =====================//
	
	if (startColumnOffset == 0)
	{
		return 0;		//컬럼이 존재하지 않을 경우
	}

	pdicHeader = dbGetDictionaryHeader();	//dictionary header를 지정한다.
	
	recordSize		= 0;	//record길이의 초기사이즈
	columnNumber	= 0;	//컬럼의 순서를 저장할 임시변수
	
	while(startColumnOffset)
	{
		pcolumn = (pColumnNode)(((char*)pdicHeader) + startColumnOffset);	//해당컬럼을 지정한다.
		
		dbMemcpyFreeIndian(&(pcolumn->columnNodeDataType), &columnDataType, sizeof(unsigned long), 1);		//메모리에 있는 컬럼의 내용을 변수에 삽입한다.

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
		{//추후에 생실지 모르는 DATATYPE들 
		}

		//다음번째 처리할 컬럼의 번호로 columnNumber를 증가시켜준다.
		columnNumber++;

		//다음 컬럼의 offset을 지정한다.
		dbMemcpyFreeIndian(&(pcolumn->columnNodeNextColumnOffset), &startColumnOffset, sizeof(unsigned long), 1);
	}
	return recordSize;
}



unsigned long dbApiRecordInsertMS(DB_VARCHAR	*dbTableName,
								  void*			dbContents[],
								  unsigned char 	dbUsingInUpdate)
{
	// =====================지역변수 선언(시작) =====================//
	pDictionaryHeader		pdictionaryHeader;	//dictionary header의 내용을 복사하여 가질 변수
	pTableNode			pTable;				//dictionary에서 테이블을 포인팅 할 변수
	TableNode			vTable;
	unsigned	long			*pTableOffset,	//dictionary file에서 테이블의 offset이 저장된 지점을 포인팅 할 변수
						*pColumnOffset;	//dictionary file에서 컬럼의 offset이 저장된 지점을 포인팅 할 변수
	unsigned	long			vTableOffset,	//테이블의 offset을 저장할 임시변수
						vColumnOffset;
	unsigned	long			vRecordRowid;
	unsigned	long			vRequirementSize;//record가 삽입되는데 필요한 사이즈를 저장할 변수
	// =====================지역변수 선언(끝) =====================//
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
	pTableOffset		= &(pdictionaryHeader->dictionaryHeaderOffsetOfRootTable);	//테이블의 시작 offset을 지정한다.
	
	
	if (!dbTableFindTableName(&pTableOffset, dbTableName))
	{
		//테이블이 존재하지 않는 경우이다.
		
		return 0;
	}
	//위의 if문에 걸리지 않았다는것은 테이블이 존재하는 것이다.
	
	//ptableOffset에는 찾은 컬럼의 Offset이 저장되어 있다.
	dbMemcpyFreeIndian(pTableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//pTable에 해당 Table을 포인팅하게 한다.
	pTable = (pTableNode)(((char*)pdictionaryHeader) + vTableOffset);
	dbMemcpyFreeIndian(pTable, &vTable, sizeof(unsigned long), 6);
	
	
	//컬럼시작 Offset을 저장하고 있는 주소를 지정한다.
	pColumnOffset = &(pTable->tableNodeColumnOffset);
	dbMemcpyFreeIndian(pColumnOffset, &vColumnOffset, sizeof(unsigned long), 1);

	if (vColumnOffset == 0)	//column이 존재하지 않으므로 삽입을 하지 않는다.
	{
		return 0;			//컬럼이 존재하지 않습니다.
	}
	//위의 if문에 걸리지 않았다면 column이 존재하므로 삽입을 시작한다.

	//Record의 size를 계산한다.
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
		//DbgPrintf(("\ndbApiRecordInsertMS :: 함수 실행시 전달받은 인자 dbUsingInUpdate의 값이 유효한 값이 아닙니다. =============================\n"));
		return 0;
	}*/

	if (vRequirementSize < 8) vRequirementSize = 8;

	if ((vRequirementSize + sizeof(DataFilePageHeader) + sizeof(unsigned long)) > gDbPageSizeDataFilePageSize ) 
	{
		return 0;	//gDbPageSizeDataFilePageSize를 증가시켜야 된다.
	}

	
	if (vTable.tableNodeStartPage == 0)		//table에 처음으로 data page가 할당될 경우 처리
	{
		unsigned long	vDataFileEndPageNumber;

		//딕셔너리에 저장되어 있는 datafile의 마지막 번호를 읽어온다.
		dbMemcpyFreeIndian(&(pdictionaryHeader->dictionaryHeaderNumberOfDataFileEndPage), 
						   &vDataFileEndPageNumber, 
						   sizeof(unsigned long), 1);
		
		//테이블의 시작Page번호에 datafile의 번호를 할당한다.
		vTable.tableNodeStartPage = vDataFileEndPageNumber + 1;
		
		dbMemcpyFreeIndian(&(vTable.tableNodeStartPage), 
						   &(pTable->tableNodeStartPage), 
						   sizeof(unsigned long), 1);

		gDbDictionaryChanged = DB_CHANGED;

		//지금 사용할 Page를 LRU List의 가장 앞에 배치한다.
		dbTouch((vTable.tableNodeStartPage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
			
		//record를 삽입한다.
		vRecordRowid = dbDataSetRecordMS(vTable.tableNodeColumnOffset, dbContents, vRequirementSize);

		//삭제된 page가 없다는 가정하에 새로 할당된 Page는 하나의 page가 생성된 것이므로 page의 번호를 증가시킨다.
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
												
		//Dictionary 정보가 바뀐것을 알린다.
		gDbDictionaryChanged = DB_CHANGED;	//dictionary의 변경이 발생한 것을 체크
	}
	else	//table에 이미 할당된 data page가 존재할 경우
	{

		//table에 할당된 시작 Page를 MEMORY에 LOAD한다.
		dbTouch((vTable.tableNodeStartPage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		
		//record를 삽입한다.
		vRecordRowid = dbDataSetRecordMS(vTable.tableNodeColumnOffset, dbContents, vRequirementSize);
		
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
	}

	//복합인덱스를 삽입한다.
	if ( vRecordRowid != 0 )	//060110..희철.
	{
		dbIndexComplexAllInsertMS(vTableOffset, dbContents,vRecordRowid);
	}
	//Time2 = OSTimeGet();
	//DbgPrintf(("DB Insert에 걸리는 시간[%d]\n", Time2-Time1 ));
	return vRecordRowid;
}

unsigned long dbDataSetRecordMS(unsigned long	dbStartColumnOffset,
								      void*			dbContents[], 
								      unsigned long	dbRequirementSize)
{
	//=======================================================================================================================//
	// 함   수   명 : dbDataSetRecordMS																						 //
	// 작   성   자 : 김 희 철																								 //
	// 설        명 : 이 함수는 인자로 받는 dbStartColumnOffset의 값을 가진 Data Page에 해당 레코드를 삽입의 의뢰를 처리하는 //
	//				 함수이다.																								 //
	//				  삽입을 Page시도하는 Page에 공간이 있으면(삭제된 공간 또는 프리스페이스) 해당 Page에 삽입후 인덱스를 삽 //
	//				 입하고, 삽입이 불가능한 경우에는 삽입이 가능한 페이지를 찾아가며 삽입을 시도한다.						 //
	//=======================================================================================================================//

	pDataFilePageHeader		pDataPageHeader;
	DataFilePageHeader		vDataPageHeader;
	unsigned	long				vFreeSpaceInDatapage;
	unsigned	long				vRecordRowid;
	char						*pRecord;

	//현재 데이터 페이지의 데이터 해더를 받아온다.
	pDataPageHeader	= (pDataFilePageHeader)(gDbMmuDataPage->mmuMemoryAddress);

	dbMemcpyFreeIndian(pDataPageHeader,
					   &vDataPageHeader,
					   sizeof(unsigned long), 4);
	
	
	//현재의 Data page에서의 free space를 계산
	vFreeSpaceInDatapage = gDbPageSizeDataFilePageSize 
						 - (vDataPageHeader.dataHeaderEndOfRecords + (vDataPageHeader.dataHeaderNumberOfSlots * DB_SLOT_SIZE));

	pRecord = DB_NULL;
	
	//====슬랏이 생성될 공간이 있다면 삭제된 공간중 레코드가 삽입될 공간이 있는지를 찾는다.(begin)=====//
	if (vFreeSpaceInDatapage >= DB_SLOT_SIZE)
	{
		if (vDataPageHeader.dataHeaderDeleteRecordOffset != 0)		//DATA PAGE내에 삭제된 공간이 존재하면 삭제된 공간에서 사용할수 있는 곳이 있는지를 살펴본다.

		{
			pRecord = dbDataGetPageHaveRequirementSizeInDeleteNodes(dbRequirementSize);
		}
	}
	//====슬랏이 생성될 공간이 있다면 삭제된 공간중 레코드가 삽입될 공간이 있는지를 찾는다.( end )=====//
	

	//============삭제된 공간중에 데이터가 삽입될 수 있는 상황일 경우 처리 (begin)============//
	if (pRecord != DB_NULL)		//삭제공간이 활용가능할 경우
	{//레코드가 삽입되는 두개중에 첫번째 경우

		vRecordRowid = (gDbMmuDataPage->mmuPageNumber - 1)*gDbPageSizeDataFilePageSize 
					+ (pRecord - gDbMmuDataPage->mmuMemoryAddress);	//Data Area에 기록될 Record의 Rowid를 구한다.


		dbDataSetRecordReal(pRecord, dbStartColumnOffset, dbContents);				//Data Area에 Record를 기록한다.


		dbDataSetSlot(vDataPageHeader.dataHeaderNumberOfSlots + 1, vRecordRowid);	//Data Page에 Record의 Slot을 기록한다.


		{//===========================해당 page의 header를 갱신한다.(begin)===========================//

			//vDataPageHeader.dataHeaderEndOfRecords += DB_SLOT_SIZE;
			vDataPageHeader.dataHeaderNumberOfSlots++ ;

			//dbMemcpyFreeIndian(&(vDataPageHeader.dataHeaderEndOfRecords),	
			//				   &(pDataPageHeader->dataHeaderEndOfRecords),
			//				   sizeof(unsigned long), 1); 
			dbMemcpyFreeIndian(&(vDataPageHeader.dataHeaderNumberOfSlots),
							   &(pDataPageHeader->dataHeaderNumberOfSlots),
							   sizeof(unsigned long), 1);

			gDbMmuDataPage->mmuChanged = DB_CHANGED;				//해당 Data page를 관리하는 MMU에 내용이 변경되었음을 표시한다.
		
		}//===========================해당 page의 header를 갱신한다.( end )===========================//

//DbgPrintf(("싱글인덱스 입력하다 죽은거겠지 before\n"));
		dbIndexSingleAllInsertMS(dbStartColumnOffset, dbContents, vRecordRowid);
//DbgPrintf(("싱글인덱스 입력하다 죽은거겠지 after\n"));
	}
	//============삭제된 공간중에 데이터가 삽입될 수 있는 상황일 경우 처리 ( end )============//



	//====================================삭제된 공간중에 데이터가 삽입될 수 없는 상황일 경우 처리 (begin)====================================//
	//========================현재 Page의 Free Space의 크기가 커서 새로 생길 Record가 삽입가능한 경우 (begin)========================//
	else if (vFreeSpaceInDatapage >= (dbRequirementSize + DB_SLOT_SIZE))	//현재의 Data Page에 Record의 삽입이 가능한 경우
	{//레코드가 삽입되는 두개중에 두번째 경우

		vRecordRowid = (gDbMmuDataPage->mmuPageNumber - 1)*gDbPageSizeDataFilePageSize 
					+ vDataPageHeader.dataHeaderEndOfRecords;		//Data Area에 기록될 Record의 Rowid를 구한다.
		
		
		pRecord = gDbMmuDataPage->mmuMemoryAddress + vDataPageHeader.dataHeaderEndOfRecords;	//Data Area에 Record가 기록될 주소값을 구한다.
		
		
		dbDataSetRecordReal(pRecord, dbStartColumnOffset, dbContents);							//Data Area에 Record를 기록한다.
		
		
		dbDataSetSlot(vDataPageHeader.dataHeaderNumberOfSlots + 1, vRecordRowid);				//Data Page에 Record의 Slot을 기록한다.
		

		{//===========================해당 page의 header를 갱신한다.(begin)===========================//
			vDataPageHeader.dataHeaderEndOfRecords += dbRequirementSize;
			vDataPageHeader.dataHeaderNumberOfSlots++ ;
			dbMemcpyFreeIndian(&(vDataPageHeader.dataHeaderEndOfRecords),
							&(pDataPageHeader->dataHeaderEndOfRecords),
							sizeof(unsigned long), 1); 
			dbMemcpyFreeIndian(&(vDataPageHeader.dataHeaderNumberOfSlots),
							&(pDataPageHeader->dataHeaderNumberOfSlots),
							sizeof(unsigned long), 1);

			gDbMmuDataPage->mmuChanged = DB_CHANGED;				//해당 Data page를 관리하는 MMU에 내용이 변경되었음을 표시한다.
		}//===========================해당 page의 header를 갱신한다.( end )===========================//
//DbgPrintf(("싱글인덱스 입력하다 죽은거겠지 before\n"));
		dbIndexSingleAllInsertMS(dbStartColumnOffset, dbContents, vRecordRowid);
//DbgPrintf(("싱글인덱스 입력하다 죽은거겠지 after\n"));
	}
	//========================현재 Page의 Free Space의 크기가 커서 새로 생길 Record가 삽입가능한 경우 ( end )========================//


	//========================현재 Page의 Free Space의 크기가 작아서 같은 테이블에 할당된 새로운 Page에서 삽입을 시도해야 할 경우(begin)========================//
	//============현재 Page의 연결된 Page가 있을 경우(begin)=============//
	else if(vDataPageHeader.dataHeaderNextSameTablePage != 0)		//연속된 다른 page가 있다면
	{
		//찾는 page를 가장 앞으로 오게 한다.
		dbTouch((vDataPageHeader.dataHeaderNextSameTablePage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		
		//같은 Table의 연속된 page에 다시 Record삽입을 의뢰한다.
		vRecordRowid = dbDataSetRecordMS(dbStartColumnOffset, dbContents, dbRequirementSize);
	}
	//============현재 Page의 연결된 Page가 있을 경우( end )=============//
	

	//============현재 Page가 마지막 Page일 경우(begin)=============//
	else //마지막 page인 경우 (dataPageHeader.dataHeaderNextSameTablePage == 0) 
	{
		pDictionaryHeader		pdictionaryHeader;
		unsigned long			vDataFileEndPageOfDictionaryHeader;

		//Dictionary header로 부터 Data file의 마지막 page번호를 받아온다.
		pdictionaryHeader = dbGetDictionaryHeader();

		dbMemcpyFreeIndian(&(pdictionaryHeader->dictionaryHeaderNumberOfDataFileEndPage),
						   &vDataFileEndPageOfDictionaryHeader,
						   sizeof(unsigned long), 1);
		
		//현재 page의 다음 page링크에 지금 생성할 page를 연결시킨다.
		vDataPageHeader.dataHeaderNextSameTablePage = vDataFileEndPageOfDictionaryHeader + 1;

		dbMemcpyFreeIndian(&(vDataPageHeader.dataHeaderNextSameTablePage),
						   &(pDataPageHeader->dataHeaderNextSameTablePage),
						   sizeof(unsigned long), 1);

		gDbMmuDataPage->mmuChanged = DB_CHANGED;
		
		//찾는 page를 가장 앞으로 오게 한다.
		dbTouch((vDataPageHeader.dataHeaderNextSameTablePage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
		
		//같은 Table의 연속된 page에 다시 Record삽입을 의뢰한다.
		vRecordRowid = dbDataSetRecordMS(dbStartColumnOffset, dbContents, dbRequirementSize);
	}
	//============현재 Page가 마지막 Page일 경우( end )=============//
	//========================현재 Page의 Free Space의 크기가 작아서 같은 테이블에 할당된 새로운 Page에서 삽입을 시도해야 할 경우( end )========================//
	//====================================삭제된 공간중에 데이터가 삽입될 수 없는 상황일 경우 처리 ( end )====================================//

	return vRecordRowid;	//레코드가 삽입된 지정을 넘긴다.*/
}

/*
void dbApiRecordInsert(DB_VARCHAR	*tableName,
					   void*			contents[])
{
	// =====================지역변수 선언(시작) =====================//
	pDictionaryHeader		pdicHeader;		//dictionary header의 내용을 복사하여 가질 변수
	pTableNode					ptable;				//dictionary에서 테이블을 포인팅 할 변수
	TableNode					TABLE_NODE;
	unsigned	long		*ptableOffset,			//dictionary file에서 테이블의 offset이 저장된 지점을 포인팅 할 변수
							*pcolumnOffset;		//dictionary file에서 컬럼의 offset이 저장된 지점을 포인팅 할 변수
	unsigned	long		tableOffset,				//테이블의 offset을 저장할 임시변수
							columnOffset;
	unsigned	long		recordRowid;
	unsigned	long		requirementSize;			//record가 삽입되는데 필요한 사이즈를 저장할 변수
	// =====================지역변수 선언(끝) =====================//
	
	pdicHeader		= dbGetDictionaryHeader();
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);					//테이블의 시작 offset을 지정한다.
	
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//테이블이 존재하지 않는 경우이다.
		return;
	}
	//위의 if문에 걸리지 않았다는것은 테이블이 존재하는 것이다.
	
	//ptableOffset에는 찾은 컬럼의 Offset이 저장되어 있다.
	dbMemcpyFreeIndian(ptableOffset, &tableOffset, sizeof(unsigned long), 1);

	//pTable에 해당 Table을 포인팅하게 한다.
	ptable = (pTableNode)(((char*)pdicHeader) + tableOffset);
	dbMemcpyFreeIndian(ptable, &TABLE_NODE, sizeof(unsigned long), 6);

	//컬럼시작 Offset을 저장하고 있는 주소를 지정한다.
	pcolumnOffset = &(ptable->tableNodeColumnOffset);
	dbMemcpyFreeIndian(pcolumnOffset, &columnOffset, sizeof(unsigned long), 1);

	if (columnOffset == 0)	//column이 존재하지 않으므로 삽입을 하지 않는다.
	{
		return;			//컬럼이 존재하지 않습니다.
	}
	//위의 if문에 걸리지 않았다면 column이 존재하므로 삽입을 시작한다.

	//Record의 size를 계산한다.
	requirementSize = dbRecordGetSize(columnOffset, contents);

	if (requirementSize < 8) requirementSize = 8;

	if ( (requirementSize + sizeof(DataFilePageHeader) + sizeof(unsigned long)) > gDbPageSizeDataFilePageSize ) return;	//gDbPageSizeDataFilePageSize를 증가시켜야 된다.
	
	if (TABLE_NODE.tableNodeStartPage == 0)		//table에 처음으로 data page가 할당될 경우 처리
	{
		unsigned long datafile_endpageNumber;

		//딕셔너리에 저장되어 있는 datafile의 마지막 번호를 읽어온다.
		dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderNumberOfDataFileEndPage), &datafile_endpageNumber, sizeof(unsigned long), 1);
		
		//테이블의 시작Page번호에 datafile의 번호를 할당한다.
		TABLE_NODE.tableNodeStartPage = datafile_endpageNumber + 1;
		dbMemcpyFreeIndian(&(TABLE_NODE.tableNodeStartPage), &(ptable->tableNodeStartPage), sizeof(unsigned long), 1);
		gDbDictionaryChanged = DB_CHANGED;

		//지금 사용할 Page를 LRU List의 가장 앞에 배치한다.
		dbTouch((TABLE_NODE.tableNodeStartPage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
			
		//record를 삽입한다.
		recordRowid = dbDataSetRecord(TABLE_NODE.tableNodeColumnOffset, contents, requirementSize);

		//삭제된 page가 없다는 가정하에 새로 할당된 Page는 하나의 page가 생성된 것이므로 page의 번호를 증가시킨다.
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
												
		//Dictionary 정보가 바뀐것을 알린다.
		gDbDictionaryChanged = DB_CHANGED;	//dictionary의 변경이 발생한 것을 체크
	}
	else	//table에 이미 할당된 data page가 존재할 경우
	{
		//table에 할당된 시작 Page를 MEMORY에 LOAD한다.
		dbTouch((TABLE_NODE.tableNodeStartPage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		
		//record를 삽입한다.
		recordRowid = dbDataSetRecord(TABLE_NODE.tableNodeColumnOffset, contents, requirementSize);
		
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
	}

	//복합인덱스를 삽입한다.
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
		//현재의 컬럼을 포인트한다.
		pcolumn = (pColumnNode)(((char*)pdictionaryHeader) + dbColumnStartAddress);
		
		//현재 컬럼의 타입을 받아온다.
		dbMemcpyFreeIndian(&(pcolumn->columnNodeDataType), &vColumnType, sizeof(unsigned long), 1);

		//현재의 컬럼을 DATA AREA에 복사한다.
		{
			if (vColumnType == DB_VARCHAR_TYPE)
			{
//char* temp =  pRecord;
//showTwoByteString("받은 문자열", dbContents[vColumnCount]);
				pRecord += dbStringCopy((DB_VARCHAR*)pRecord, (DB_VARCHAR*)dbContents[vColumnCount]);
//showTwoByteString("기록된 문자열", temp);
//unsigned long temp;
//temp = dbStringCopy((DB_VARCHAR*)pRecord, (DB_VARCHAR*)dbContents[vColumnCount]);
//pRecord += temp;
//DbgPrintf(("기록할때 싸이즈 [%x]\n", temp));
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

		//다음 컬럼을 복사하기 위한 작업을 한다.
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

	//현재 데이터 페이지의 데이터 해더를 받아온다.
	pDataPageHeader	= (pDataFilePageHeader)(gDbMmuDataPage->mmuMemoryAddress);
	dbMemcpyFreeIndian(pDataPageHeader, &dataPageHeader, sizeof(unsigned long), 4);
	
	

	//현재의 Data page에서의 free space를 계산
	freeSpaceInDatapage = gDbPageSizeDataFilePageSize 
						   - (dataPageHeader.dataHeaderEndOfRecords + (dataPageHeader.dataHeaderNumberOfSlots * DB_SLOT_SIZE));

	pRecord = DB_NULL;
	
	if (freeSpaceInDatapage >= DB_SLOT_SIZE)	// 슬랏이 생성될 공간이 있다면
	{
		if (dataPageHeader.dataHeaderDeleteRecordOffset != 0)		//DATA PAGE내에 삭제된 공간이 존재하면 삭제된 공간에서 사용할수 있는 곳이 있는지를 살펴본다.

		{
			pRecord = dbDataGetPageHaveRequirementSizeInDeleteNodes(requirementSize);
		}
	}
	
	if (pRecord != DB_NULL)		//삭제공간이 활용가능할 경우
	{
		//recordRowid를 구한다.
		recordRowid = (gDbMmuDataPage->mmuPageNumber - 1)*gDbPageSizeDataFilePageSize 
					 + (pRecord - gDbMmuDataPage->mmuMemoryAddress);

		//실제 Data AREA에 레코드를 기록한다.
		dbDataSetRecordReal(pRecord, startColumnOffset, contents);

		//해당 slot을 기록한다.
		dbDataSetSlot(dataPageHeader.dataHeaderNumberOfSlots + 1, recordRowid);

		//해당 page의 header를 갱신한다.
		dataPageHeader.dataHeaderEndOfRecords += DB_SLOT_SIZE;
		dataPageHeader.dataHeaderNumberOfSlots++ ;
		dbMemcpyFreeIndian(&(dataPageHeader.dataHeaderEndOfRecords), &(pDataPageHeader->dataHeaderEndOfRecords), sizeof(unsigned long), 1); 
		dbMemcpyFreeIndian(&(dataPageHeader.dataHeaderNumberOfSlots), &(pDataPageHeader->dataHeaderNumberOfSlots), sizeof(unsigned long), 1);

		//해당 Data page를 관리하는 MMU에 내용이 변경되었음을 표시한다.
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
		
		//레코드가 삽입된 지정을 freeSpace 변수에 저장한다.
		dbIndexSingleAllInsert(startColumnOffset, contents, recordRowid);
	}
	else if (freeSpaceInDatapage >= (requirementSize + DB_SLOT_SIZE))	//현재의 Data Page에 Record의 삽입이 가능한 경우
	{
		//기록할 Record의 rowid를 계산한다. 
		recordRowid = (gDbMmuDataPage->mmuPageNumber - 1)*gDbPageSizeDataFilePageSize + dataPageHeader.dataHeaderEndOfRecords;
		
		//기록할 Record의 memory address를 찾는다.
		pRecord = gDbMmuDataPage->mmuMemoryAddress + dataPageHeader.dataHeaderEndOfRecords;
		
		//실제 Data AREA에 레코드를 기록한다.
		dbDataSetRecordReal(pRecord, startColumnOffset, contents);
		
		//해당 slot을 기록한다.
		dbDataSetSlot(dataPageHeader.dataHeaderNumberOfSlots + 1, recordRowid);
		
		//해당 page의 header를 갱신한다.
		dataPageHeader.dataHeaderEndOfRecords += requirementSize;
		dataPageHeader.dataHeaderNumberOfSlots++ ;
		dbMemcpyFreeIndian(&(dataPageHeader.dataHeaderEndOfRecords), &(pDataPageHeader->dataHeaderEndOfRecords), sizeof(unsigned long), 1); 
		dbMemcpyFreeIndian(&(dataPageHeader.dataHeaderNumberOfSlots), &(pDataPageHeader->dataHeaderNumberOfSlots), sizeof(unsigned long), 1);

		//해당 Data page를 관리하는 MMU에 내용이 변경되었음을 표시한다.
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
		
		//레코드가 삽입된 지정을 freeSpace 변수에 저장한다.
		dbIndexSingleAllInsert(startColumnOffset, contents, recordRowid);
	}
	else if(dataPageHeader.dataHeaderNextSameTablePage != 0)		//연속된 다른 page가 있다면
	{
		//찾는 page를 가장 앞으로 오게 한다.
		dbTouch((dataPageHeader.dataHeaderNextSameTablePage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		
		//같은 Table의 연속된 page에 다시 Record삽입을 의뢰한다.
		recordRowid = dbDataSetRecord(startColumnOffset, contents, requirementSize);
	}
	else //마지막 page인 경우 (dataPageHeader.dataHeaderNextSameTablePage == 0) 
	{
		pDictionaryHeader		pdicHeader;
		unsigned long			datafile_endpageOfDictionaryHeader;

		//Dictionary header로 부터 Data file의 마지막 page번호를 받아온다.
		pdicHeader = dbGetDictionaryHeader();
		dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderNumberOfDataFileEndPage),
							  &datafile_endpageOfDictionaryHeader,
							  sizeof(unsigned long), 1);
		
		//현재 page의 다음 page링크에 지금 생성할 page를 연결시킨다.
		dataPageHeader.dataHeaderNextSameTablePage = datafile_endpageOfDictionaryHeader + 1;
		dbMemcpyFreeIndian(&(dataPageHeader.dataHeaderNextSameTablePage),
							  &(pDataPageHeader->dataHeaderNextSameTablePage),
							  sizeof(unsigned long), 1);
		gDbMmuDataPage->mmuChanged = DB_CHANGED;
		
		//찾는 page를 가장 앞으로 오게 한다.
		dbTouch((dataPageHeader.dataHeaderNextSameTablePage - 1)*gDbPageSizeDataFilePageSize, DB_DATA_FILE_LOAD);
		
		//같은 Table의 연속된 page에 다시 Record삽입을 의뢰한다.
		recordRowid = dbDataSetRecord(startColumnOffset, contents, requirementSize);
	}
	return recordRowid;	//레코드가 삽입된 지정을 넘긴다.
}*/

char* dbIndexGetPageHaveRequirementSizeInDeleteNodes(unsigned long requirementSize)
{
	//인덱스 Page의 삭제노드중에서 필요사이즈의 삭제노드가 있을 경우 해당 memory주소를 리턴해준다.
	//없을 경우에는 DB_NULL값을 리턴한다.
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
	//data Page의 삭제노드중에서 필요사이즈의 삭제노드가 있을 경우 해당 memory주소를 리턴해준다.
	//없을 경우에는 DB_NULL값을 리턴한다.
	pDataFilePageHeader			pDataPageHeader;
	unsigned long				*pDataHeaderDeleteNodeOffset;
	unsigned long				DataHeaderDeleteNodeOffset;
	
	//DbgPrint(("삭제 공간중에 필요한 공간이  있는지 찾는 루틴 "));
	
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
	//인덱스 page의 프리공간상에 필요로 하는 사이즈가 존재하는지를 판단해서 존재할경우에 해당 주소를 리턴한다.
	//없을 경우 DB_NULL을 리턴한다.
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
	//INDEX NODE가 생길 공간이 존재하는 경우
	else
	{
		returnLocation = (gDbMmuIndexPage->mmuMemoryAddress + indexPageHeader.indexHeaderEndOfDatas);	//inode기록의 시작offset의 위치를 backup해 놓는다.
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
	
	//인덱스 노드의 offset을 계산한다.
	inodeOffset = (unsigned long)(((char*)pInode) - gDbMmuIndexPage->mmuMemoryAddress) + (gDbPageSizeIndexFilePageSize * (gDbMmuIndexPage->mmuPageNumber - 1));

	//새로운 인덱스 노드를 생성한다.
	inodeNode.indexNodeLeft			= DB_NULL;
	inodeNode.indexNodeRight		= DB_NULL;
	inodeNode.indexNodeHeight		= 1;
	inodeNode.indexNodeLinkOffset	= DB_NULL;

	//새로운 익덱스 노드를 INDEX AREA에 복사한다.
	dbMemcpyFreeIndian(&inodeNode, pInode, sizeof(unsigned long), 4);

	//keyValue를 기록한다.
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
		//DbgPrintf(("dbIndexCompareKeyAreaVsAreaMS 함수에서 스트링 비교 구문으로 진입한다.\n"));
		vCompareResult = dbStringCompareAreaVsAreaMS((DB_VARCHAR*)dbAreaKeyValue1, (DB_VARCHAR*)dbAreaKeyValue2);

		//DbgPrintf(("dbIndexCompareKeyAreaVsAreaMS 함수에서 스트링 비교 하고 난후 [%d]를 비교결과로 리턴했다..\n", vCompareResult));

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

	//인덱스 노드를 지정한다.
	
	//dbTouch(indexNodeOffset, DB_INDEX_FILE_LOAD);
	//pinode = (pIndexNode)(gDbMmuIndexPage->mmuMemoryAddress + indexNodeOffset%gDbPageSizeIndexFilePageSize);
	pinode	= dbIndexGetNode(indexNodeOffset);
	
	//link의 offset을 저장한다.
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
	//linkOffset에는 next 노드의 offst이 들어가 있고
	//pLinkOffset에는 지금 생성할 링크값을 포인팅할 next에 저장될 값이 저장되어 있다. 

	//링크가 추가될 수 있는 page를 찾는다
	{
		unsigned char	reuseSpace;
		plink = DB_NULL;
		//우선 현재의 index page에 삽입을 시도한다.
		if (gDbMmuIndexPage->mmuPageNumber != 0)
		{
			plink = dbIndexGetPageHaveRequirementSizeInDeleteLinks();
			
			if (plink == DB_NULL)
			{
				plink = (pLinkNode)dbIndexGetPageHaveRequirementSizeInFreespace(sizeof(LinkNode));
				
				if (plink != DB_NULL)
				{
					reuseSpace = 0;
					//삭제노드가 아닌 프리공간이용이다.
				}
			}
			else
			{
				reuseSpace = 1;
				//삭제 link에서 하나를 꺼네 씀
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
					//삭제 link에서 하나를 꺼네 씀.
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

		if (reuseSpace == 0)	//재사용을 한것이 아니기 때문에 index 해더의 갱신이 필요하다
		{
			pIndexFilePageHeader	pIndexHeader;
			//프리영역을 사용해야 한다.
			unsigned long	indexpage_endDatas;
			//자유공간중에 일정영역을 사용한것이므로 딕셔너리 해더에서 자유영역의 감소를 시켜야한다.
			pIndexHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);

			//인덱스 해더에서 마지막 데이터의 위치를 저장한다.
			dbMemcpyFreeIndian(&(pIndexHeader->indexHeaderEndOfDatas), &indexpage_endDatas, sizeof(unsigned long), 1);
			
			//인덱스 해더에서 마지막 데이터의 위치를 증가시킨다.
			indexpage_endDatas += sizeof(LinkNode);
			dbMemcpyFreeIndian(&indexpage_endDatas, &(pIndexHeader->indexHeaderEndOfDatas), sizeof(unsigned long), 1);
			
			gDbMmuIndexPage->mmuChanged = DB_CHANGED;
		}
	}

	//pInode에 link의 내용을 추가하면 된다.
	{
		dbMemcpyFreeIndian(&linkOffset , &(plink->linkNodeNext), sizeof(unsigned long), 1);
		dbMemcpyFreeIndian(&recordRowid , &(plink->linkNodeRecordRowid), sizeof(unsigned long), 1);
	}

	//새로 생성한 link의 offset을 구한다.
	newLinkOffset = (gDbMmuIndexPage->mmuPageNumber - 1) * gDbPageSizeIndexFilePageSize 
					+ (((char*)plink) - gDbMmuIndexPage->mmuMemoryAddress);

	//이전 노드와의 연결을 이어준다.
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
	//현재 인덱스 page에서 삭제된 링크가 있는지 보고 있을 경우 이를 리턴한다.
	//없으면 DB_NULL을 리턴한다.

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

		//indexHeader를 갱신한다.
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
	//새로운 노드를 생성해야 할 경우처리
	if ((dbIndexNodeRootOffset == DB_NULL) || (dbIndexNodeRootOffset == 1))
	{
		pIndexNode		pInode;
		unsigned long	vInodeOffset;
		unsigned long	vRequirementSize;

		//인덱스노드가 저장되기 위해 필요한 크기를 계산한다.
		{
			//Index Node의 Size
			vRequirementSize = sizeof(IndexNode);

			//해당 Key값의 Size
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
		//vRequirementSize에 해당 인덱스 노드가 저장되기 위해 필요한 Size가 계산되어 있다.

		
		if ((vRequirementSize + sizeof(IndexFilePageHeader)) > gDbPageSizeIndexFilePageSize)
		{
			return 0;	//gDbPageSizeIndexFilePageSize 사이즈를 증가시켜 줘야한다.추가를 못한다.
		}

		//필요한 사이즈를 가진 indexPage를 loading한다.
		{
			pIndexFilePageHeader	pIndexHeader;
			unsigned long			vIndexPageNumber = 0;

			while(1)
			{
				vIndexPageNumber++;
				dbTouch((vIndexPageNumber-1)*gDbPageSizeIndexFilePageSize, DB_INDEX_FILE_LOAD);

			#if ( AVLDB_DEBUG == 1 )
				//DbgPrintf(("dbIndexGetPageHaveRequirementSizeInDeleteNodes 여기는 아닌거 같은데...before\n"));
			#endif

				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInDeleteNodes(vRequirementSize);

			#if ( AVLDB_DEBUG == 1 )
				//DbgPrintf(("dbIndexGetPageHaveRequirementSizeInDeleteNodes 여기는 아닌거 같은데...after\n"));
			#endif

				if (pInode != DB_NULL)
				{
					//삭제공간중에 하나를 가져온것이므로 해더에 변화가 필요없다.
					break;
				}
				
				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInFreespace(vRequirementSize);

				if (pInode != DB_NULL)
				{
					unsigned long	vIndexPageEndDatas;
					//자유공간중에 일정영역을 사용한것이므로 딕셔너리 해더에서 자유영역의 감소를 시켜야한다.
					pIndexHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);

					//인덱스 해더에서 마지막 데이터의 위치를 저장한다.
					dbMemcpyFreeIndian(&(pIndexHeader->indexHeaderEndOfDatas), &vIndexPageEndDatas, sizeof(unsigned long), 1);
					
					//인덱스 해더에서 마지막 데이터의 위치를 증가시킨다.
					vIndexPageEndDatas += vRequirementSize;
					dbMemcpyFreeIndian(&vIndexPageEndDatas, &(pIndexHeader->indexHeaderEndOfDatas), sizeof(unsigned long), 1);
					
					gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					break;
				}
			}
		}
		//해당 위치에 인덱스 노드를 기록한다.
		//DbgPrintf(("dbIndexSetNode 여기는 아닌거 같은데...before\n"));
		vInodeOffset = dbIndexSetNode(pInode, dbKeyValue, dbKeyDataType);

		//DbgPrintf(("dbIndexSetNode 여기는 아닌거 같은데...after\n"));
		//link만을 추가한다.
		dbIndexLinkNodeAppend(vInodeOffset, dbRecordOffset);

		return vInodeOffset;	//지금 기록한 inode의 offset을 리턴한다.
	}
	//기존에 인덱스 노드가 존재하고 그 인덱스 노드를 탐색하면서 삽입을 실시해야한다.
	else
	{
		pIndexNode		pInode;
		IndexNode		vIndexNode;
		signed char		vKeyCompareResult;
		
		pInode	= dbIndexGetNode(dbIndexNodeRootOffset);
		dbMemcpyFreeIndian(pInode, &vIndexNode, sizeof(unsigned long), 4);
		
	#if ( AVLDB_DEBUG == 1 )
//DbgPrintf(("dbIndexCompareKeyAreaVsStackMS 여기는 아닌거 같은데..설마아직까지버그.before\n"));
	#endif	

		vKeyCompareResult = dbIndexCompareKeyAreaVsStackMS(pInode + 1, dbKeyValue, dbKeyDataType);

	#if ( AVLDB_DEBUG == 1 )
//DbgPrintf(("dbIndexCompareKeyAreaVsStackMS 여기는 아닌거 같은데..설마아직까지버그.after 비교값[%d]\n", vKeyCompareResult));
	#endif	

		if (vKeyCompareResult == 0)
		{
			//link만을 추가한다.
	#if ( AVLDB_DEBUG == 1 )
//DbgPrintf(("dbIndexLinkNodeAppend(dbIndexNodeRootOffset[%d], dbRecordOffset[%d])\n", dbIndexNodeRootOffset, dbRecordOffset));			
	#endif	
			dbIndexLinkNodeAppend(dbIndexNodeRootOffset, dbRecordOffset);
		}
		else if (vKeyCompareResult == 1)
		{
			unsigned long	vAfterRightSubOffset;
			
			//오른쪽에서 탐색을 해야하는 경우
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
			
			//오른쪽에서 탐색을 해야하는 경우
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
//DbgPrint(("Rotation 하면서 죽는건가\n"));
	#endif
		//rotation check를 한다.
		{
			signed char		vBalanceFactor;
			IndexNode		vSubIndexNode;

			vBalanceFactor = dbIndexGetBalanceFactor(vIndexNode.indexNodeLeft, vIndexNode.indexNodeRight);
	#if ( AVLDB_DEBUG == 1 )
//DbgPrint(("vBalanceFactor는 제대로 구하는가 ? [%d]\n", vBalanceFactor));			
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

		//dbIndexNodeRootOffset의 높이를 다시 계산한다.
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
	//새로운 노드를 생성해야 할 경우처리
	if ((indexNodeRootOffset == DB_NULL) || (indexNodeRootOffset == 1))
	{
		pIndexNode		pInode;
		unsigned long	inodeOffset;
		unsigned long	requirementSize;

		//인덱스노드가 저장되기 위해 필요한 크기를 계산한다.
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
			return 0;	//gDbPageSizeIndexFilePageSize 사이즈를 증가시켜 줘야한다.추가를 못한다.
		}

		//필요한 사이즈를 가진 indexPage를 loading한다.
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
					//삭제공간중에 하나를 가져온것이므로 해더에 변화가 필요없다.
					break;
				}
				
				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInFreespace(requirementSize);

				if (pInode != DB_NULL)
				{
					unsigned long	datapage_endDatas;
					//자유공간중에 일정영역을 사용한것이므로 딕셔너리 해더에서 자유영역의 감소를 시켜야한다.
					pIndexHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);

					//인덱스 해더에서 마지막 데이터의 위치를 저장한다.
					dbMemcpyFreeIndian(&(pIndexHeader->indexHeaderEndOfDatas), &datapage_endDatas, sizeof(unsigned long), 1);
					
					//인덱스 해더에서 마지막 데이터의 위치를 증가시킨다.
					datapage_endDatas += requirementSize;
					dbMemcpyFreeIndian(&datapage_endDatas, &(pIndexHeader->indexHeaderEndOfDatas), sizeof(unsigned long), 1);
					
					gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					break;
				}
			}
		}
		//해당 위치에 인덱스 노드를 기록한다.
		inodeOffset = dbIndexSetNode(pInode, keyValue, keyDataType);

		//link만을 추가한다.
		dbIndexLinkNodeAppend(inodeOffset, recordOffset);

		return inodeOffset;	//지금 기록한 inode의 offset을 리턴한다.
	}
	//기존에 인덱스 노드가 존재하고 그 인덱스 노드를 탐색하면서 삽입을 실시해야한다.
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
			//link만을 추가한다.
			dbIndexLinkNodeAppend(indexNodeRootOffset, recordOffset);
		}
		else if (keyCompareResult == 1)
		{
			unsigned long	afterRightSubOffset;
			
			//오른쪽에서 탐색을 해야하는 경우
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
			
			//오른쪽에서 탐색을 해야하는 경우
			afterLeftSubOffset = dbIndexSingleUnitInsert(indexNode.indexNodeLeft, keyValue, keyDataType, recordOffset);
			
			if (indexNode.indexNodeLeft != afterLeftSubOffset)
			{
				pInode						= dbIndexGetNode(indexNodeRootOffset);
				indexNode.indexNodeLeft		= afterLeftSubOffset;
				dbMemcpyFreeIndian(&(indexNode.indexNodeLeft), &(pInode->indexNodeLeft), sizeof(unsigned long), 1);
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}

		//rotation check를 한다.
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

		//indexNodeRootOffset의 높이를 다시 계산한다.
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

	//==================Case 1. 새로운 노드를 생성해야 할 경우 처리 (begin)==================//
	if ((dbIndexRootOffset == DB_NULL) || (dbIndexRootOffset == 1))
	{
		pIndexNode		pInode;
		unsigned long	vIndexOffset;
		unsigned long	vRequirementSize;

		//==================(처리1) 인덱스 노드가 저장되기 위해 필요한 크기를 계산한다.(begin)==================//
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
		//==================(처리1) 인덱스 노드가 저장되기 위해 필요한 크기를 계산한다.( end )==================//



		//==================(처리2) 인덱스 노드가 인덱스의 페이지 크기보다 작은지를 점검한다.(begin)==================//
		if ((vRequirementSize + sizeof(IndexFilePageHeader)) > gDbPageSizeIndexFilePageSize)
		{
			return 0 ;			//gDbPageSizeIndexFilePageSize 사이즈를 증가시켜 줘야한다.추가를 못한다.
		}
		//==================(처리2) 인덱스 노드가 인덱스의 페이지 크기보다 작은지를 점검한다.( end )==================//



		//==================(처리3) 필요한 사이즈를 가진 indexPage를 loading한다.(begin)==================//
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
					//삭제공간중에 하나를 가져온것이므로 해더에 변화가 필요없다.
					break;
				}
				
				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInFreespace(vRequirementSize);

				if (pInode != DB_NULL)
				{
					unsigned long	datapage_endDatas;
					//자유공간중에 일정영역을 사용한것이므로 딕셔너리 해더에서 자유영역의 감소를 시켜야한다.
					pIndexHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);

					//인덱스 해더에서 마지막 데이터의 위치를 저장한다.
					dbMemcpyFreeIndian(&(pIndexHeader->indexHeaderEndOfDatas), &datapage_endDatas, sizeof(unsigned long), 1);
					
					//인덱스 해더에서 마지막 데이터의 위치를 증가시킨다.
					datapage_endDatas += vRequirementSize;
					dbMemcpyFreeIndian(&datapage_endDatas, &(pIndexHeader->indexHeaderEndOfDatas), sizeof(unsigned long), 1);
					
					gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					break;
				}
			}
		}
		//==================(처리3) 필요한 사이즈를 가진 indexPage를 loading한다.( end )==================//


		//==================(처리4) 해당 위치에 인덱스 노드를 기록한다.(begin)==================//
		vIndexOffset = dbIndexSetNode(pInode, dbKeyValues[0], dbKeyDataTypes[0]);
		//==================(처리4) 해당 위치에 인덱스 노드를 기록한다.( end )==================//


		//==================(처리5) 복합인덱스의 마지막 컬럼일 경우 해당 노드에 링크를 추가하고, 마지막 노드가 아닌 경우에는 다음 차원의 트리에 삽입을 시도한다.(begin)==================//
		if (dbColumnNumber == 1)
		{
			//link만을 추가한다.
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
		//==================(처리5) 복합인덱스의 마지막 컬럼일 경우 해당 노드에 링크를 추가하고, 마지막 노드가 아닌 경우에는 다음 차원의 트리에 삽입을 시도한다.( end )==================//

		return vIndexOffset;	//지금 기록한 inode의 offset을 리턴한다.
	}
	//==================Case 1. 새로운 노드를 생성해야 할 경우 처리 ( end )==================//



	//==================Case 2. 기존에 인덱스 노드가 존재하고 그 인덱스 노드를 탐색하면서 삽입을 실시해야 하는 경우처리. (begin)==================//
	else
	{
		pIndexNode		pInode;
		IndexNode		vIndexNode;
		signed char		vKeyCompareResult;
		

		pInode	= dbIndexGetNode(dbIndexRootOffset);
		dbMemcpyFreeIndian(pInode, &vIndexNode, sizeof(unsigned long), 4);

		//==================(처리1) 지금 삽입을 시도하는 노드와 Area 안에 있는 노드를 비교한다.(begin)==================//
		vKeyCompareResult = dbIndexCompareKeyAreaVsStackMS(pInode + 1, dbKeyValues[0], dbKeyDataTypes[0]);
		//==================(처리1) 지금 삽입을 시도하는 노드와 Area 안에 있는 노드를 비교한다.( end )==================//


		//==================(처리2) 지금 삽입을 시도하는 노드와 Area 안에 있는 노드를 비교하며 처리한다.(begin)==================//
		if (vKeyCompareResult == 0)
		{
			if (dbColumnNumber == 1)
			{
				//link만을 추가한다.
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
			
			//오른쪽에서 탐색을 해야하는 경우
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
			
			//오른쪽에서 탐색을 해야하는 경우
			afterLeftSubOffset = dbIndexComplexUnitInsertPostProcessMS(vIndexNode.indexNodeLeft, dbKeyValues, dbKeyDataTypes, dbColumnNumber, dbRecordRowid);
			
			if (vIndexNode.indexNodeLeft != afterLeftSubOffset)
			{
				pInode					= dbIndexGetNode(dbIndexRootOffset);

				vIndexNode.indexNodeLeft	= afterLeftSubOffset;

				dbMemcpyFreeIndian(&(vIndexNode.indexNodeLeft), &(pInode->indexNodeLeft), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}

		//rotation check를 한다.
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

		//indexNodeRootOffset의 높이를 다시 계산한다.
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
	//==================Case 2. 기존에 인덱스 노드가 존재하고 그 인덱스 노드를 탐색하면서 삽입을 실시해야 하는 경우처리. ( end )==================//
}

#if 0
unsigned long dbIndexComplexUnitInsertPostProcess(unsigned long	rootOffset,
														void*			keyValues[],
														unsigned long	keyDataTypes[],
														unsigned long	columnNumber,
														unsigned long	recordRowid)
{

	//새로운 노드를 생성해야 할 경우처리
	if ((rootOffset == DB_NULL) || (rootOffset == 1))
	{
		pIndexNode		pInode;
		unsigned long	inodeOffset;
		unsigned long	requirementSize;

		//인덱스노드가 저장되기 위해 필요한 크기를 계산한다.
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
			return 0;	//gDbPageSizeIndexFilePageSize 사이즈를 증가시켜 줘야한다.추가를 못한다.
		}

		//필요한 사이즈를 가진 indexPage를 loading한다.
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
					//삭제공간중에 하나를 가져온것이므로 해더에 변화가 필요없다.
					break;
				}
				
				pInode = (pIndexNode)dbIndexGetPageHaveRequirementSizeInFreespace(requirementSize);

				if (pInode != DB_NULL)
				{
					unsigned long	datapage_endDatas;
					//자유공간중에 일정영역을 사용한것이므로 딕셔너리 해더에서 자유영역의 감소를 시켜야한다.
					pIndexHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);

					//인덱스 해더에서 마지막 데이터의 위치를 저장한다.
					dbMemcpyFreeIndian(&(pIndexHeader->indexHeaderEndOfDatas), &datapage_endDatas, sizeof(unsigned long), 1);
					
					//인덱스 해더에서 마지막 데이터의 위치를 증가시킨다.
					datapage_endDatas += requirementSize;
					dbMemcpyFreeIndian(&datapage_endDatas, &(pIndexHeader->indexHeaderEndOfDatas), sizeof(unsigned long), 1);
					
					gDbMmuIndexPage->mmuChanged = DB_CHANGED;
					break;
				}
			}
		}
		//해당 위치에 인덱스 노드를 기록한다.
		inodeOffset = dbIndexSetNode(pInode, keyValues[0], keyDataTypes[0]);

		if (columnNumber == 1)
		{
			//link만을 추가한다.
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
		return inodeOffset;	//지금 기록한 inode의 offset을 리턴한다.
	}

	//기존에 인덱스 노드가 존재하고 그 인덱스 노드를 탐색하면서 삽입을 실시해야한다.
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
				//link만을 추가한다.
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
			
			//오른쪽에서 탐색을 해야하는 경우
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
			
			//오른쪽에서 탐색을 해야하는 경우
			afterLeftSubOffset = dbIndexComplexUnitInsertPostProcess(indexNode.indexNodeLeft, keyValues, keyDataTypes, columnNumber, recordRowid);
			
			if (indexNode.indexNodeLeft != afterLeftSubOffset)
			{
				pInode					= dbIndexGetNode(rootOffset);

				indexNode.indexNodeLeft	= afterLeftSubOffset;

				dbMemcpyFreeIndian(&(indexNode.indexNodeLeft), &(pInode->indexNodeLeft), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
		}

		//rotation check를 한다.
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

		//indexNodeRootOffset의 높이를 다시 계산한다.
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
	//이 함수는 찾은 컬럼의 포인터 주소는 columnAddress에 넣고
	//리턴값으로는 몇번째 컬럼인지를 리턴한다.
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
				break;	//해당 노드를 찾은 경우
			}
		}
		else if (is_AREA_AREA == DB_AREA_VS_STACK)
		{
			if (dbStringCompare_AREAVs_STACK((DB_VARCHAR*)((*columnAddress) + 1), columnName) == 0)
			{
				break;	//해당 노드를 찾은 경우
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
	
	//복합인덱스 root의 offset을 구한다.
	dbMemcpyFreeIndian(&(dbComplexIndex->complexIndexNodeIndexRootOffset),	&oldRootOffset,	sizeof(unsigned long), 1);
	
	//복합인덱스의 컬럼의 갯수를 구한다.
	dbMemcpyFreeIndian(&(dbComplexIndex->complexIndexNodeColumnNumber),		&vColumnNumber,	sizeof(unsigned long), 1);

	//복합인덱스의 컬럼의 갯수만큼 접근하면서 
	//해당이름을 가지는 컬럼의 데이터 타입과 contents 중에서 몇번째의 컨텐츠인지를 알아내어.
	//dbDataTypes배열과 idxContents 배열에 집에 넣는다.
	{
		pColumnNode			pColumn;
		DB_VARCHAR			*pcolumnName;
		unsigned long		vColumnNumberInAllColumn;
		
		//복합인덱스의 이름을 포인팅한다.
		pcolumnName = (DB_VARCHAR*)(dbComplexIndex + 1);

		for(vCurrentColumn = 0 ; vCurrentColumn < vColumnNumber ; vCurrentColumn++)
		{
			//columnName을 가지는 컬럼이 몇번째 있는지 알아낸다.
			vColumnNumberInAllColumn = dbFindColumnName(dbColumnStart, &pColumn, pcolumnName, DB_AREA_VS_AREA);
			
			//해당 컬럼의 키값을 저장한다.
			pIdxContents[vCurrentColumn] = dbContents[vColumnNumberInAllColumn];
			
			//해당 컬럼키값의 데이터 타입을 저장한다.
			dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &vDataTypes[vCurrentColumn], sizeof(unsigned long), 1);
			
			pcolumnName = pcolumnName + (dbStringGetSize(pcolumnName) + DB_VARCHAR_SIZE)/2;
		}
	}
	//위의 loop문을 빠져 나오면 해당 복합인덱스의 삽입을 위한 설정값들의 설정이 모두 끝난다.

	//삽입을 시작한다.
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
	
	//복합인덱스 root의 offset을 구한다.
	dbMemcpyFreeIndian(&(cInode->complexIndexNodeIndexRootOffset), &oldRootOffset, sizeof(unsigned long), 1);
	
	//복합인덱스의 컬럼의 갯수를 구한다.
	dbMemcpyFreeIndian(&(cInode->complexIndexNodeColumnNumber), &columnNumber, sizeof(unsigned long), 1);

	//복합인덱스의 컬럼의 갯수만큼 접근하면서 
	//해당이름을 가지는 컬럼의 데이터 타입과 contents 중에서 몇번째의 컨텐츠인지를 알아내어.
	//dataTypes배열과 idxContents 배열에 집에 넣는다.
	{
		pColumnNode			pColumn;
		//COLUMN			columnNode;
		DB_VARCHAR		*columnName;
		unsigned long	columnNumberInAllColumn;
		
		//복합인덱스의 이름을 포인팅한다.
		columnName = (DB_VARCHAR*)(cInode + 1);

		for(currentColumn = 0 ; currentColumn < columnNumber ; currentColumn++)
		{
			//columnName을 가지는 컬럼이 몇번째 있는지 알아낸다.
			columnNumberInAllColumn = dbFindColumnName(columnStartOffset, &pColumn, columnName, DB_AREA_VS_AREA);
			
			//해당 컬럼의 키값을 저장한다.
			idxContents[currentColumn] = contents[columnNumberInAllColumn];
			
			//해당 컬럼키값의 데이터 타입을 저장한다.
			dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &dataTypes[currentColumn], sizeof(unsigned long), 1);
			
			columnName = columnName + (dbStringGetSize(columnName) + DB_VARCHAR_SIZE)/2;
		}
	}
	//위의 loop문을 빠져 나오면 해당 복합인덱스의 삽입을 위한 설정값들의 설정이 모두 끝난다.

	//삽입을 시작한다.
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
	
	// =======================지역변수선언(시작) =======================//
	pDictionaryHeader		pdictionaryHeader;	//dictionary header를 포인팅할 변수
	pTableNode				ptable;				//해당 테이블을 가르킬 녀석, 딕셔너리에 있으므로 위치가 변경될 일이 없다.
	pComplexIndexNode		pcomplexIndex;		//복합인덱스의 딕셔너리상의 노드를 가르킬 녀석이다.
	unsigned long			vComplexIndexOffset;
	unsigned long			vColumnStartOffset;
	// =======================지역변수선언(시작) =======================//

	pdictionaryHeader	= dbGetDictionaryHeader();							//딕셔너리 해더 포인팅
	ptable				= (pTableNode)(((char*)pdictionaryHeader) + dbTableOffset);	//테이블 포인팅
	
	//복합인덱스의 시작 offset을 구한다.
	dbMemcpyFreeIndian(&(ptable->tableNodeComplexIndexOffset),	&vComplexIndexOffset,	sizeof(unsigned long), 1);
	
	//컬럼의 시작 offset을 구한다.
	dbMemcpyFreeIndian(&(ptable->tableNodeColumnOffset),		&vColumnStartOffset,	sizeof(unsigned long), 1);

	//while문을 돌면서 해당 테이블의 각 복합인덱스를 순회하면서 실행한다.
	while (vComplexIndexOffset)
	{
		//딕셔너리에 있는 복합인덱스 컬럼을 찾는다.
		pcomplexIndex	= (pComplexIndexNode)((char*)pdictionaryHeader + vComplexIndexOffset);
		
		//해당 복합인덱스를 처리한다.
		dbIndexComplexUnitInsertPreProcessMS(pcomplexIndex, vColumnStartOffset, dbContents, dbRecordRowid);		// 060112_heechul, "MS" 추가. 
		
		//다음 복합인덱스가 있는 컬럼에 대한 연산을 수행하기 위하여
		//다음 offset을 저장하는 주소를 설정한다.
		dbMemcpyFreeIndian(&(pcomplexIndex->complexIndexNodeNext), &vComplexIndexOffset, sizeof(unsigned long), 1);
	}
}
/*
void dbIndexComplexAllInsert(unsigned long	tableNodeOffset, 
								 void*			contents[], 
								 unsigned long	recordRowid)
{
	// =======================지역변수선언(시작) =======================//
	pDictionaryHeader		pdicHeader;	//dictionary header를 포인팅할 변수
	pTableNode					ptable;			//해당 테이블을 가르킬 녀석, 딕셔너리에 있으므로 위치가 변경될 일이 없다.
	unsigned long			complexIndexOffset;
	unsigned long			columnStartOffset;
	pComplexIndexNode					cInode;		//복합인덱스의 딕셔너리상의 노드를 가르킬 녀석이다.
	// =======================지역변수선언(시작) =======================//

	pdicHeader = dbGetDictionaryHeader();							//딕셔너리 해더 포인팅
	ptable		= (pTableNode)(((char*)pdicHeader) + tableNodeOffset);	//테이블 포인팅
	
	//복합인덱스의 시작 offset을 구한다.
	dbMemcpyFreeIndian(&(ptable->tableNodeComplexIndexOffset), &complexIndexOffset, sizeof(unsigned long), 1);
	
	//컬럼의 시작 offset을 구한다.
	dbMemcpyFreeIndian(&(ptable->tableNodeColumnOffset), &columnStartOffset, sizeof(unsigned long), 1);

	//while문을 돌면서 해당 테이블의 각 복합인덱스를 순회하면서 실행한다.
	while (complexIndexOffset)
	{
		//딕셔너리에 있는 복합인덱스 컬럼을 찾는다.
		cInode			= (pComplexIndexNode)((char*)pdicHeader + complexIndexOffset);
		
		//해당 복합인덱스를 처리한다.
		dbIndexComplexUnitInsertPreProcess(cInode, columnStartOffset, contents, recordRowid);
		
		//다음 복합인덱스가 있는 컬럼에 대한 연산을 수행하기 위하여
		//다음 offset을 저장하는 주소를 설정한다.
		dbMemcpyFreeIndian(&(cInode->complexIndexNodeNext), &complexIndexOffset, sizeof(unsigned long), 1);
	}
}*/

void dbIndexSingleAllInsertMS(unsigned long	dbStartColumnOffset, 
							  void*			dbContents[], 
							  unsigned long	dbRecordRowid)
{
	pDictionaryHeader	pdictionaryHeader;		//딕셔너리 해더를 포인팅할 변수
	pColumnNode			pcolumn;				//dbStartColumnOffset으로부터 시작할 컬럼을 포인팅 할 변수
	ColumnNode			vColumnNode;			//컬럼 구조체 임시 변수로 접근하는 컬럼마다 컬럼의 값을 여기에 저장한다.
	unsigned long		vColumnCount;			//접근하는 컬럼의 순번을 저장할 변수이다. 인자로 받은 dbContents[]배열의 해당번째를 저장하는데 사용한다.
	unsigned long		vRootOffset;			//해당 컬럼에 인덱스를 추가한 후에 만들어진 세로운 인덱스 트리의 RootOffset을 저장하는 변수이다.
	
	vColumnCount = 0;

	pdictionaryHeader = dbGetDictionaryHeader();	//딕셔너리 해더를 받아온다.


	//각 Column에 접근하면서 각 컬럼마다 인덱스가 존재할 경우에 삽입을 시도한다.
	while(dbStartColumnOffset)
	{
		pcolumn = (pColumnNode)(((char*)pdictionaryHeader) + dbStartColumnOffset);	//해당 컬럼을 지정한다.
		dbMemcpyFreeIndian(pcolumn, &vColumnNode, sizeof(unsigned long), 3);

		if (vColumnNode.columnNodeIndexRootOffset != 0)		//해당 컬럼에 인덱스가 존재할때 처리
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

		dbStartColumnOffset = vColumnNode.columnNodeNextColumnOffset;	//다음컬럼으로 이동한다.
		vColumnCount++;
	}
}

/*
void dbIndexSingleAllInsert(unsigned long	dbStartColumnOffset, 
							void*			dbContents[], 
							unsigned long	dbRecordRowid)
{
	pDictionaryHeader	pdictionaryHeader;
	pColumnNode			pcolumn;		//딕셔너리에서의 컬럼을 포인팅 할 변수
	ColumnNode			vColumnNode; 
	unsigned long		vColumnCount;
	unsigned long		vRootOffset;
	
	vColumnCount = 0;

	pdictionaryHeader = dbGetDictionaryHeader();


	//각 Column에 접근하면서 각컬럼마다 인
	while(dbStartColumnOffset)
	{
		pcolumn = (pColumnNode)(((char*)pdictionaryHeader) + dbStartColumnOffset);	//해당 컬럼을 지정한다.
		dbMemcpyFreeIndian(pcolumn, &vColumnNode, sizeof(unsigned long), 3);

		if (vColumnNode.columnNodeIndexRootOffset != 0)		//해당 컬럼에 인덱스가 존재할때 처리
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

		dbStartColumnOffset = vColumnNode.columnNodeNextColumnOffset;	//다음컬럼으로 이동한다.
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

		//비교
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
	// =========================지역 변수 선언(시작) =========================//
	pDictionaryHeader		pdictionaryHeader;	//Dictionary file의 해더를 포인팅할 변수
	unsigned	long			*ptableOffset;		//Dictionary file이 올라오는 영역에서 테이블의 offset을 저장하고 위치를 포인팅할 변수
	pTableNode			pTable;				//Dictionary file이 올라오는 영역에서 Table의 취치를 포인팅할 변수
	pColumnNode			pcolumnNode;		//Dictionary file이 올라오는 영역에서 Column을 위치를 포인팅할 변수
	TableNode			vTableNode;			//Dictionary file에서의  Table노드의 내용을 복사할 변수
	unsigned long			vTableOffset,		//Dictionary file이 올라오는 영역의 테이블의 시작 offset을 저장할 변수
						vColumnOffset;		//Dictionary file이 올라오는 영역의 테이블에서 시작 컬럼의 offset을 저장하고 위치를 포인팅할 변수
	unsigned	long			vColumnCount;		//Table의 Column의 Total 갯수를 저장할 변수
	// =========================지역 변수 선언(끝) =========================//

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

	pdictionaryHeader	= dbGetDictionaryHeader();							//Dictionary를 지정한다.
	ptableOffset		= &(pdictionaryHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정
	
	//테이블을 찾는다.
	if (!dbTableFindTableName(&ptableOffset, dbTableName))
	{
		//ERROR : 입력으로 들어온 tableName의 table이 시스템에 존재하지않는 경우
		#if DB_SHOW_ERROR_CODE						//Error Check
		gDbErrorCode = DB_ERROR_TABLE_NOT_FOUND;		//Error Check
		#endif											//Error Check

		return 0;
	}


	//table의 offset이 저장되어 있다.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table의 메모리를 지정한다.
	pTable = (pTableNode)(((char*)pdictionaryHeader) + vTableOffset);
	
	//테이블의 내용을 TABLE_NODE에 복사한다.
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
	pDictionaryHeader		pdicHeader;	//Dictionary file의 해더를 포인팅할 변수
	pTableNode					pTable;			//Dictionary file이 올라오는 영역에서의 테이블을 포인팅할 변수
	TableNode					TABLE_NODE;
	pColumnNode					pColumn;
	unsigned long			*ptableOffset, vTableOffset;
	unsigned long			*pcolumnOffset, vColumnOffset;
	unsigned long			indexOffset;

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary를 지정한다.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정
	
	//테이블을 찾는다.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : 입력으로 들어온 tableName의 table이 시스템에 존재하지않는 경우
		return 0;
	}
	// ===========아래부터 해당 테이블을 찾았을때 처리 ===========//

	//table의 offset이 저장되어 있다.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table의 메모리를 지정한다.
	pTable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//테이블의 내용을 TABLE_NODE에 복사한다.
	dbMemcpyFreeIndian(pTable, &TABLE_NODE, sizeof(unsigned long), 6);

	if (TABLE_NODE.tableNodeStartPage == 0)
	{
		//테이터삽입이 없었다면 더이상 검색할것도 없다..
		return 0;
	}

	//인덱스의 시작 offset의 위치를 찾아낸다.
	{
		if (indexColumnNumber == 1)	//단일인덱스일 경우
		{
			//컬럼의 시작 offset이 저장되어 있는 주소를 포인팅한다.
			pcolumnOffset = &(pTable->tableNodeColumnOffset);

			//컬럼을 찾는다.
			if(!dbTableFindColumnName(&pcolumnOffset, indexColumnName[0]))
			{
				//ERROR : 해당 컬럼이 존재하지 않는 경우
				return 0;
			}

			//컬럼의 offset이 저장되어 있다.
			dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);

			pColumn =  (pColumnNode)(((char*)pdicHeader) + vColumnOffset);
			
			dbMemcpyFreeIndian(&(pColumn->columnNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);
		}
		else							//복합인덱스일 경우
		{
			unsigned long	complexOffset;
			pComplexIndexNode			pComplexIndex;
			ComplexIndexNode			complexIndex;
			dbMemcpyFreeIndian(&(pTable->tableNodeComplexIndexOffset), &complexOffset, sizeof(unsigned long), 1);

			//모든 복합인덱스 컬럼을 순차적으로 방문하면서 찾는다.
			while(complexOffset)
			{
				pComplexIndex = (pComplexIndexNode)(((char*)pdicHeader) + complexOffset);
				dbMemcpyFreeIndian(pComplexIndex, &complexIndex, sizeof(unsigned long), 3);

				if (complexIndex.complexIndexNodeColumnNumber == indexColumnNumber)
				{
					if (dbIndexCheckName((DB_VARCHAR*)(pComplexIndex + 1) , indexColumnName, indexColumnNumber) != 0)
					{
						//해당 복합인덱스를 찾은경우.
						break;
					}
				}

				complexOffset = complexIndex.complexIndexNodeNext;
			}

			if (complexOffset==DB_NULL) return 0;

			dbMemcpyFreeIndian(&(pComplexIndex->complexIndexNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);
		}
	}
	//indexOffset에는 인덱스의 시작 offset의 값이 저장되어있다.


	//인덱스 탐색을 시작한다.
	if (conditionColumnNumber == 0)
	{
		dbSearchArray = resultArray;
		dbSearchCount = 0;
		if (indexColumnNumber == (conditionColumnNumber + 1))
		{
			//데이터파일의 recordRowid를 결과로 리턴할 경우
			*refMemory = DB_DATA_FILE_LOAD;
			dbIndexTreeTraverse(indexOffset, traverseOrder, 1);	//결과를 리턴하는 부분
		}
		else
		{
			//검색된 결과는 인덱스 파일의 내용이다.
			*refMemory = DB_INDEX_FILE_LOAD;
			dbIndexTreeTraverse(indexOffset, traverseOrder, 0);	//결과를 리턴하는 부분
		}
	}
	else
	{
		unsigned long	i;
		unsigned long	columnDataType;
		for (i = 0 ; i < conditionColumnNumber ; i++)
		{
			//컬럼의 시작 offset이 저장되어 있는 주소를 포인팅한다.
			pcolumnOffset = &(pTable->tableNodeColumnOffset);
			
			//찾는 컬럼의 offset을 구한다.
			dbTableFindColumnName(&pcolumnOffset, indexColumnName[i]);
			dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);
						
			
			//해당 컬럼을 포인팅한다.
			pColumn =  (pColumnNode)(((char*)pdicHeader) + vColumnOffset);
			
			//컬럼의 dataType을 복사한다.
			dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &columnDataType, sizeof(unsigned long), 1);
				
			
			//검색을 실시한다.
			indexOffset = dbIndexSearchGetInode(indexOffset, columnDataType, conditionSet[i]);
			
			if (indexOffset == 0) return 0;
			
			//검색된 결과 다른 트리의 rootOffset이 검색된다.
			

			if (( (conditionColumnNumber - 1) == i ) && (indexColumnNumber != conditionColumnNumber))		//마지막에 한번만 일어난다. 결과를 리턴하는 부분
			{
				dbSearchArray = resultArray;
				dbSearchCount = 0;

				if (indexColumnNumber == (conditionColumnNumber + 1))
				{
					//데이터파일의 recordRowid를 결과로 리턴할 경우
					*refMemory = DB_DATA_FILE_LOAD;
					dbIndexTreeTraverse(indexOffset, traverseOrder, 1);
				}
				else
				{
					//검색된 결과는 인덱스 파일의 내용이다.
					*refMemory = DB_INDEX_FILE_LOAD;
					dbIndexTreeTraverse(indexOffset, traverseOrder, 0);
				}
			}
			else if(( (conditionColumnNumber - 1) == i ) && (indexColumnNumber == conditionColumnNumber))		//마지막에 한번만 일어난다. 결과를 리턴하는 부분
			{
				dbSearchArray = resultArray;
				dbSearchCount = 0;

				//검색된 결과는 인덱스 파일의 내용이다.
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
	pDictionaryHeader		pdicHeader;	//Dictionary file의 해더를 포인팅할 변수
	pColumnNode					pColumn;
	ColumnNode			columnNode;
	char					*pRecord;
	unsigned long			columnCount;

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary를 지정한다.
	
	if (columnStartOffset == 0)
	{
		return 0;
	}
	else
	{
		//실제 레코드를 가져온다.
		dbTouch(recordRowid, DB_DATA_FILE_LOAD);
		pRecord = gDbMmuDataPage->mmuMemoryAddress + recordRowid%gDbPageSizeDataFilePageSize;
		columnCount = 0;
	}

	while(columnStartOffset)
	{
		resultArray[columnCount] = pRecord;	//record의 해당 위치를 컬럼에 삽입한다.
		
		//다음컬럼으로의 진행
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
	pDictionaryHeader		pdicHeader;		//Dictionary file의 해더를 포인팅할 변수
	pTableNode			ptable;			//Dictionary file이 올라오는 영역에서의 테이블을 포인팅할 변수
	TableNode			tableNode;
	unsigned long			*ptableOffset, vTableOffset;

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary를 지정한다.
	ptableOffset		= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정

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

	
	//테이블을 찾는다.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : 입력으로 들어온 tableName의 table이 시스템에 존재하지않는 경우
		return;
	}
	// ===========아래부터 해당 테이블을 찾았을때 처리 ===========//

	//table의 offset이 저장되어 있다.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table의 메모리를 지정한다.
	ptable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//테이블의 내용을 TABLE_NODE에 복사한다.
	dbMemcpyFreeIndian(ptable, &tableNode, sizeof(unsigned long), 6);

	//해당 레코드의 값을 채워온다.
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

	pdicHeader			= dbGetDictionaryHeader();			//Dictionary를 지정한다.
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
		//처음으로 삭제된 노드일 경우 처리
		vDeleteNode.deleteNodeNext = 0;

		//데이터 Page해더에 삭제노드의 offset을 시작값으로 지정한다.
		dbMemcpyFreeIndian(&vDeleteRecordOffset, &(pDataHeader->dataHeaderDeleteRecordOffset), sizeof(unsigned long), 1);
	}
	else if (vDeleteRecordOffset > vDeleteOffset)
	{
		//처음으로 삭제된 노드일 경우 처리
		vDeleteNode.deleteNodeNext = vDeleteRecordOffset;

		//데이터 Page해더에 삭제노드의 offset을 시작값으로 지정한다.
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
		//whlie을 탈출할때
		//vDeleteRecordOffset에는 앞 삭제노드의 offset
		//vNextOffset에는 뒷노드의 offset이 저장된다.
		dbMemcpyFreeIndian(&vDeleteOffset, &(pCurDNode->deleteNodeNext), sizeof(unsigned long), 1);
		
		vDeleteNode.deleteNodeNext = vNextOffset;
	}

	//삭제노드를 셋팅한다.
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
	dbTouch(recordRowid, DB_DATA_FILE_LOAD);								//rowid가 존재하는 datapage를 Loading한다.
	pdatHeader = (pDataFilePageHeader)(gDbMmuDataPage->mmuMemoryAddress);	//해당 Data Page의 Header를 지정한다.
	dbMemcpyFreeIndian(pdatHeader, & datHeader, sizeof(unsigned long), 4);			//해당 Data Page의 Header를 복사한다.

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
	//해당 레코드가 차지하는 크기(recordSize)를 알수 있다.
	
	//해당 레코드의 슬랏이 가지는 값을 구한다.
		dbTouch(recordRowid, DB_DATA_FILE_LOAD);
	recordSlotValue = recordRowid%gDbPageSizeDataFilePageSize;

	//슬랏을 처리한다.
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
	//record를 삭제 노드로 처리를 한다.
	deleteNode.deleteNodeSize = recordSize;
	deleteNode.deleteNodeNext = datHeader.dataHeaderDeleteRecordOffset;

	dbMemcpyFreeIndian(&deleteNode, columnsValue[0], sizeof(unsigned long), 2);

	//딕셔너리해더의 삭제 노드에 대한 링크 갱신
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
		//해당번째의 복합인덱스를 찾는다.
		pcomplexIndex = (pComplexIndexNode)(((char*)pdicHeader) + startComplexIndexOffset);

		//해당 복합인덱스의 내용을 복사해온다.
		dbMemcpyFreeIndian(pcomplexIndex, &complexIndexNode, sizeof(unsigned long), 3);

		columnNumberInAllColumn = 0;

		columnName = (DB_VARCHAR*)(pcomplexIndex + 1);

		for(currentColumn = 0 ; currentColumn < complexIndexNode.complexIndexNodeColumnNumber ; currentColumn++)
		{
			//columnName을 가지는 컬럼이 몇번째 있는지 알아낸다.
			columnNumberInAllColumn = dbFindColumnName(startColumnOffset, &pColumn, columnName, DB_AREA_VS_AREA);
			
			//해당 컬럼의 키값을 저장한다.
			idxContents[currentColumn] = columnsValue[columnNumberInAllColumn];
			
			//해당 컬럼키값의 데이터 타입을 저장한다.
			dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &dataTypes[currentColumn], sizeof(unsigned long), 1);
			
			columnName = columnName + (dbStringGetSize(columnName) + DB_VARCHAR_SIZE)/2;
		}

		//해당 복합인덱스를 제거한다.
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
		//다음 복합인덱스 진행을 위하여 다음 복합인덱스의 offset을  startComplexIndexOffset에 삽입한다.
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
		//해당번째의 복합인덱스를 찾는다.
		pcomplexIndex = (pComplexIndexNode)(((char*)pdictionaryHeader) + dbStartComplexIndexOffset);

		//해당 복합인덱스의 내용을 복사해온다.
		dbMemcpyFreeIndian(pcomplexIndex, &vComplexIndexNode, sizeof(unsigned long), 3);

		vColumnNumberInAllColumn = 0;

		pColumnName = (DB_VARCHAR*)(pcomplexIndex + 1);

		for(vCurrentColumn = 0 ; vCurrentColumn < vComplexIndexNode.complexIndexNodeColumnNumber ; vCurrentColumn++)
		{
			//columnName을 가지는 컬럼이 몇번째 있는지 알아낸다.
			vColumnNumberInAllColumn = dbFindColumnName(dbStartColumnOffset, &pColumn, pColumnName, DB_AREA_VS_AREA);
			
			//해당 컬럼의 키값을 저장한다.
			vIdxContents[vCurrentColumn] = dbColumnsValues[vColumnNumberInAllColumn];
			
			//해당 컬럼키값의 데이터 타입을 저장한다.
			dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &vDataTypes[vCurrentColumn], sizeof(unsigned long), 1);
			
			pColumnName = pColumnName + (dbStringGetSize(pColumnName) + DB_VARCHAR_SIZE)/2;
		}

		//해당 복합인덱스를 제거한다.
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
		//다음 복합인덱스 진행을 위하여 다음 복합인덱스의 offset을  startComplexIndexOffset에 삽입한다.
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
	pDictionaryHeader		pdictionaryHeader;	//Dictionary file의 해더를 포인팅할 변수
	pTableNode			ptable;				//Dictionary file이 올라오는 영역에서의 테이블을 포인팅할 변수
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

	//Data File의 Page네에 해당 Rowid가 존재하는지를 체크한다.
	if (dbDataCheckRecordValidRowID(dbRecordRowid)!=DB_VALID_ROWID)
	{
		#if DB_SHOW_ERROR_CODE					//Error check
		gDbErrorCode = DB_ERROR_INVALID_ROWID;	//Error check
		#endif										//Error check
		
		return DB_FUNCTION_FAIL;
	}

	pdictionaryHeader	= dbGetDictionaryHeader();									//Dictionary를 지정한다.
	ptableOffset		= &(pdictionaryHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정
	
	//테이블을 찾는다.
	if (!dbTableFindTableName(&ptableOffset, dbTableName))
	{
		//ERROR : 입력으로 들어온 tableName의 table이 시스템에 존재하지않는 경우
		return DB_FUNCTION_FAIL;
	}
	// ===========아래부터 해당 테이블을 찾았을때 처리 ===========//

	//table의 offset이 저장되어 있다.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);


	//table의 메모리를 지정한다.
	ptable = (pTableNode)(((char*)pdictionaryHeader) + vTableOffset);


	//테이블의 내용을 TABLE_NODE에 복사한다.
	dbMemcpyFreeIndian(ptable, &vTableNode, sizeof(unsigned long), 6);


	//삭제를 하려는 Rowid가 해당 테이블 소속의 DataPage에 있는지를 검사한다.
	{
		unsigned long vDeleteRecordPageNumber;
		vDeleteRecordPageNumber = (dbRecordRowid/gDbPageSizeDataFilePageSize) + 1;
		if (!dbHaveDataPageNumberInDataPageChain(vTableNode.tableNodeStartPage, vDeleteRecordPageNumber))
		{
			//ERROR : 입력으로 들어온 tableName의 table이 시스템에 존재하지않는 경우
			return DB_FUNCTION_FAIL;
		}
	}


	//해당 컬럼내용을 배열에 받아온다.
	dbDataGetRecord(vTableNode.tableNodeColumnOffset, dbRecordRowid, vColumnsValues);


	//전체 컬럼의 갯수를 받아온다.
//	vTotalColumnNumber = dbColumnTotalCount(vTableNode.tableNodeColumnOffset);	<--이거 확인해봐야함.. 필요 없는거 같은데..


	//싱클인덱스 삭제 루틴을 실행한다
	dbIndexDeleteAllSingleMS(vTableNode.tableNodeColumnOffset, 
							 vColumnsValues,
							 dbRecordRowid);


	//복합인덱스 삭제 루틴을 실행한다
	dbIndexDeleteAllComplexMS(vTableNode.tableNodeColumnOffset, 
							  vTableNode.tableNodeComplexIndexOffset, 
							  vColumnsValues, 
							  dbRecordRowid);


	//레코드의 삭제 루틴을 실행한다.
	dbDataSetRecordDelete(vTableNode.tableNodeColumnOffset, 
						  vColumnsValues,
						  dbRecordRowid);
	//Time2 = OSTimeGet();

	//DbgPrintf(("DB Delete에 걸리는 시간[%d] \n", Time2 - Time1));
	return DB_FUNCTION_SUCCESS;
}

/*
void dbApiRecordDelete(DB_VARCHAR		*tableName,
					   unsigned long	recordRowid)
{
	pDictionaryHeader		pdicHeader;	//Dictionary file의 해더를 포인팅할 변수
	pTableNode					ptable;			//Dictionary file이 올라오는 영역에서의 테이블을 포인팅할 변수
	TableNode					tableNode;
//	pColumnNode					pColumn;
//	unsigned long			columnType;
	unsigned long			*ptableOffset, vTableOffset;
	unsigned long			totalColumnNumber;
	void*					columnsValue[DB_TABLE_MAX_COLUMN_NUMBER];

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary를 지정한다.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정
	
	//테이블을 찾는다.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : 입력으로 들어온 tableName의 table이 시스템에 존재하지않는 경우
		return;
	}
	// ===========아래부터 해당 테이블을 찾았을때 처리 ===========//

	//table의 offset이 저장되어 있다.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table의 메모리를 지정한다.
	ptable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//테이블의 내용을 TABLE_NODE에 복사한다.
	dbMemcpyFreeIndian(ptable, &tableNode, sizeof(unsigned long), 6);

	//해당 컬럼내용을 배열에 받아온다.
	dbDataGetRecord(tableNode.tableNodeColumnOffset, recordRowid, columnsValue);

	//전체 컬럼의 갯수를 받아온다.
	totalColumnNumber = dbColumnTotalCount(tableNode.tableNodeColumnOffset);

	//싱클인덱스 삭제 루틴을 실행한다
	dbIndexDeleteAllSingle(tableNode.tableNodeColumnOffset, columnsValue, recordRowid);

	//복합인덱스 삭제 루틴을 실행한다
	dbIndexDeleteAllComplex(tableNode.tableNodeColumnOffset, tableNode.tableNodeComplexIndexOffset, columnsValue, recordRowid);

	//레코드의 삭제 루틴을 실행한다.
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
			//DbgPrintf(("링크 삭제루틴으로 진입\n"));
			//삭제 링크로 처리하는 과정
			{
				pIndexFilePageHeader	pidxHeader;
				unsigned long			relativeOffset;
				unsigned long			indexHeaderDeleteLinkOffset;
				
				relativeOffset = linkOffset%gDbPageSizeIndexFilePageSize;
				pidxHeader = (pIndexFilePageHeader)(gDbMmuIndexPage->mmuMemoryAddress);
				
				dbMemcpyFreeIndian(&(pidxHeader->indexHeaderDeleteLinkOffset), &indexHeaderDeleteLinkOffset, sizeof(unsigned long), 1);
				//DbgPrintf(("indexHeaderDeleteLinkOffset = [%x]\n", indexHeaderDeleteLinkOffset));
				//인덱스 해더에 삭제link를 등록한다.
				dbMemcpyFreeIndian(&relativeOffset, &(pidxHeader->indexHeaderDeleteLinkOffset), sizeof(unsigned long), 1);
				
				//link의 linked list를 유지시킨다.
				dbMemcpyFreeIndian(&(indexHeaderDeleteLinkOffset), &(pLink->linkNodeNext), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
			}
			
			return linkNode.linkNodeNext;
		}
		else
		{
			unsigned long newNext;

			//DbgPrintf((" 이거 나오면 에러\n"));
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
						//연결하고
						vPreLinkNode.linkNodeNext	= vNextLinkNode.linkNodeNext;
						pPreLinkNode = dbIndexGetLink(vPreOffset);
						dbMemcpyFreeIndian(&vPreLinkNode, pPreLinkNode, sizeof(unsigned long), 2);
						gDbMmuIndexPage->mmuChanged = DB_CHANGED;
						
						//삭제하기
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
	//return이 되는 long 값은 tree의 root값이 된다.

	pIndexNode			pNode;
	IndexNode			indexNode;
//	unsigned	long		tempOffset,
//							preNodeOffset,
//							bak;

	if (rootNode == 0) return 0;

	//검색노드를 불러온다.
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

		//balance factor를 체크해서 rotation을 할수도 있다.
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
		//해당 rootNode의 높이를 다시 계산한다.
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

		//해당 트리의 root를 리턴한다.
		return rootNode;
	}
}

unsigned long dbIndexDeleteNodeMakeTreeUpMaxNode(unsigned long	rootNode, 
												 unsigned long	*maxNodeOffset)
{
	//return이 되는 long 값은 tree의 root값이 된다.

	pIndexNode					pNode;
	IndexNode					indexNode;
//	unsigned	long		tempOffset,
//							preNodeOffset;
//							bak;

	if (rootNode == 0) return 0;

	//검색노드를 불러온다.
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

		//balance factor를 체크해서 rotation을 할수도 있다.
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
		//해당 rootNode의 높이를 다시 계산한다.
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

		//해당 트리의 root를 리턴한다.
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

	vCompareResult = dbIndexCompareKeyAreaVsAreaMS(pindexNode + 1, dbKeyValue, dbDataType);	//<--------------------요기 디따 중요!!!!!!!!!!!

		#if ( AVLDB_DEBUG == 1 )
//DbgPrintf(("\n=============dbIndexCompareKeyAreaVsAreaMS값이 나와야 한다. [%d]\n", vCompareResult));	
		#endif

	//=======================Case 1.삭제를 하려는데... 해당 노드일 경우 처리 (begin)======================//
	if (vCompareResult == 0)
	{
	
		unsigned long	vNewLinkOffset;
//DbgPrintf(("\n해당 노드 삭제루틴 진입\n"));	
//showDatMemory(512);
		vNewLinkOffset = dbIndexLinkNodeDelete(vIndexNode.indexNodeLinkOffset, dbRecordRowid);
//DbgPrintf(("\ndbIndexCompareKeyAreaVsAreaMS 에서 링크 지운후 vNewLinkOffset의 값 [%x]\n", vNewLinkOffset));
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
//DbgPrintf(("\n해당 노드의 삭제 루틴으로 들어왔음.\n"));
			if ((vIndexNode.indexNodeLeft==0)&&(vIndexNode.indexNodeRight==0))
			{
//DbgPrintf(("\n여기로 들어오면 정상인데....\n"));

				//해당 index노드를 삭제 노드로 등록한다.
				dbIndexDeleteIndexNode(dbRootIndexOffset, dbDataType);
				
				dbRootIndexOffset = 0;

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
				
				return dbRootIndexOffset;
			}
			else if ((vIndexNode.indexNodeLeft==0)&&(vIndexNode.indexNodeRight!=0))
			{
				//해당 index노드를 삭제 노드로 등록한다.
				dbIndexDeleteIndexNode(dbRootIndexOffset, dbDataType);

				dbRootIndexOffset = vIndexNode.indexNodeRight;
				
				gDbMmuIndexPage->mmuChanged = DB_CHANGED;

				return dbRootIndexOffset;
			}
			else if((vIndexNode.indexNodeLeft!=0)&&(vIndexNode.indexNodeRight==0))
			{
				//해당 index노드를 삭제 노드로 등록한다.
				dbIndexDeleteIndexNode(dbRootIndexOffset, dbDataType);

				dbRootIndexOffset = vIndexNode.indexNodeLeft;

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;
				
				return dbRootIndexOffset;
			}
			else if(vIndexNode.indexNodeHeight==2)
			{
				//해당 index노드를 삭제 노드로 등록한다.
				dbIndexDeleteIndexNode(dbRootIndexOffset, dbDataType);
				
				dbRootIndexOffset = vIndexNode.indexNodeLeft;

				pindexNode = dbIndexGetNode(dbRootIndexOffset);
				
				//높이를 수정한다.
				dbMemcpyFreeIndian(&(vIndexNode.indexNodeHeight), &(pindexNode->indexNodeHeight), sizeof(unsigned long), 1);

				//오른쪽 서브 트리의 offset을 수정한다.
				dbMemcpyFreeIndian(&(vIndexNode.indexNodeRight), &(pindexNode->indexNodeRight), sizeof(unsigned long), 1);

				gDbMmuIndexPage->mmuChanged = DB_CHANGED;

				return dbRootIndexOffset;
			}
			else
			//subNode에서의 Max노드를 올리던지 아니면, subNode의 minNode를 올리던지 작업이 있어야 한다.
			{
				signed long		balanceFactor;
				
				IndexNode		tempRoot;
				
				balanceFactor = dbIndexGetBalanceFactor(vIndexNode.indexNodeLeft, vIndexNode.indexNodeRight);

				if (balanceFactor > 0)
				{
					//왼쪽이 더 큰 경우
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
					//오른쪽이 더 크거나 같은 경우
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
			//해당 index노드를 삭제 노드로 등록한다.
			dbIndexDeleteIndexNode(dbRootIndexOffset, dbDataType);

			dbRootIndexOffset = pTempRoot;
		}
		else{}
		
		return dbRootIndexOffset;
	}
	//=======================Case 1.삭제를 하려는데... 해당 노드일 경우 처리 ( end )======================//

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

		//rotation check를 한다.
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

		//indexNodeRootOffset의 높이를 다시 계산한다.
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

			//subNode에서의 Max노드를 올리던지 아니면, subNode의 minNode를 올리던지 작업이 있어야 한다.
			{
				signed long		balanceFactor;
				
				IndexNode			tempRoot;
				
				balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);

				if (balanceFactor > 0)
				{
					//왼쪽이 더 큰 경우
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
					//오른쪽이 더 크거나 같은 경우
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
			//해당 index노드를 삭제 노드로 등록한다.
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

		//rotation check를 한다.
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

		//indexNodeRootOffset의 높이를 다시 계산한다.
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
		//사실 이런상태까지는 오지 않을것이다. 그냥 에러 처리할라구 넣은 부분일뿐
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

			//subNode에서의 Max노드를 올리던지 아니면, subNode의 minNode를 올리던지 작업이 있어야 한다.
			{
				signed long		balanceFactor;
				
				IndexNode			tempRoot;
				
				balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);

				if (balanceFactor > 0)
				{
					//왼쪽이 더 큰 경우
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
					//오른쪽이 더 크거나 같은 경우
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
			//해당 index노드를 삭제 노드로 등록한다.
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

		//rotation check를 한다.
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

		//indexNodeRootOffset의 높이를 다시 계산한다.
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
		//사실 이런상태까지는 오지 않을것이다. 그냥 에러 처리할라구 넣은 부분일뿐
		return 0;
	}

	pIndexNode = dbIndexGetNode(dbRootIndexOffset);
	dbMemcpyFreeIndian(pIndexNode, &indexNode, sizeof(unsigned long), 4);

	vCompareResult = dbIndexCompareKeyAreaVsAreaMS( pIndexNode + 1, dbKeyValues[0], dbDataTypes[0]);//<-----요거이 디따중요!!!
	
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

			//subNode에서의 Max노드를 올리던지 아니면, subNode의 minNode를 올리던지 작업이 있어야 한다.
			{
				signed long		balanceFactor;
				
				IndexNode			tempRoot;
				
				balanceFactor = dbIndexGetBalanceFactor(indexNode.indexNodeLeft, indexNode.indexNodeRight);

				if (balanceFactor > 0)
				{
					//왼쪽이 더 큰 경우
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
					//오른쪽이 더 크거나 같은 경우
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
			//해당 index노드를 삭제 노드로 등록한다.
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

		//rotation check를 한다.
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

		//indexNodeRootOffset의 높이를 다시 계산한다.
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
	
	//인덱스 해더를 갱신한다.
	relationOffset = deleteIndexOffset%gDbPageSizeIndexFilePageSize;
	dbMemcpyFreeIndian(&relationOffset, &(pidxHeader->indexHeaderDeleteNodeOffset), sizeof(unsigned long), 1);

	pIndexNode = dbIndexGetNode(deleteIndexOffset);
	//삭제 노드를 기록한다.
	dbMemcpyFreeIndian(&deleteNode, pIndexNode, sizeof(unsigned long), 2);

	gDbMmuIndexPage->mmuChanged = DB_CHANGED;
}

#if 0
void dbIndexDeleteAllSingle(unsigned long	columnStartOffset, 
								void*			columnsValue[],
								unsigned long	recordRowid)
{
	pDictionaryHeader	pdicHeader;	//Dictionary file의 해더를 포인팅할 변수
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
	pDictionaryHeader	pdictionaryHeader;	//Dictionary file의 해더를 포인팅할 변수
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
			//DbgPrintf(("===================단일 인덱스에 대하여 인덱스 삭제를 시도한다..====================\n"));
		#endif
	
			vNewRoot = dbIndexDeleteUnitSingleMS(vColumnNode.columnNodeIndexRootOffset, 
												vColumnNode.columnNodeDataType, 
												dbColumnsValues[vColumnCount],
												dbRecordRowid);

		#if ( AVLDB_DEBUG == 1 )
			//DbgPrintf(("==================삭제후 인덱스의 시작 Offset의 값 vNewRoot = [%x]=====================\n", vNewRoot));
		#endif
		
			if (vNewRoot != vColumnNode.columnNodeIndexRootOffset)
			{
				if (vNewRoot == 0) vNewRoot = 1;

				dbMemcpyFreeIndian(&vNewRoot, &(pCurrentColumn->columnNodeIndexRootOffset), sizeof(unsigned long), 1);
				gDbDictionaryChanged = DB_CHANGED;
			}

		#if ( AVLDB_DEBUG == 1 )
			//DbgPrintf(("==================삭제후 인덱스의 시작 편집된 Offset의 값 vNewRoot = [%x]=====================\n", vNewRoot));
		#endif	
		}

		vColumnCount++;
		dbColumnStartOffset = vColumnNode.columnNodeNextColumnOffset;
	}
}

char* dbApiAllocKey(unsigned long dbKeyValueOffset, 
					   unsigned char dbFileType)
{
//이 함수는 Search결과로 얻은 Rowid로 해당 값이 존재하는 DB의 Area상의 Address를 넘기는 함수이다.
	char*	pKeyValueAddress;		//DB의 Area상의 Address를 Pointing할 변수
	
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
	pDictionaryHeader		pdicHeader;	//Dictionary file의 해더를 포인팅할 변수
	pTableNode					ptable;			//Dictionary file이 올라오는 영역에서의 테이블을 포인팅할 변수
	TableNode					tableNode;
	pColumnNode					pColumn;
	//unsigned long			columnType;
	unsigned long			*ptableOffset, vTableOffset;
	unsigned long			columnCount;
	void*					columnsValue[DB_TABLE_MAX_COLUMN_NUMBER];

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary를 지정한다.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정
	
	//테이블을 찾는다.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : 입력으로 들어온 tableName의 table이 시스템에 존재하지않는 경우
		return;
	}
	// ===========아래부터 해당 테이블을 찾았을때 처리 ===========//

	//table의 offset이 저장되어 있다.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table의 메모리를 지정한다.
	ptable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//테이블의 내용을 TABLE_NODE에 복사한다.
	dbMemcpyFreeIndian(ptable, &tableNode, sizeof(unsigned long), 6);


	//해당 record의 각 컬럼의 값들을 채워온다.
	if (dbDataGetRecord(tableNode.tableNodeColumnOffset, recordRowid, columnsValue) == 0)
	{
		//해당 데이터가 없는 경우
		return;
	}
	// ===========아래 부터 해당 테이블과 해당 값을 찾은경우이다.======//
	
	{
		ColumnNode					columnNode;
		unsigned long			newRootOffset;

		//몇번째 컬럼인지 찾는다.
		columnCount = dbFindColumnName(tableNode.tableNodeColumnOffset, &pColumn, columnName, DB_AREA_VS_STACK);
		
		//해당 컬럼을 받아온다.
		dbMemcpyFreeIndian(pColumn, &columnNode, sizeof(unsigned long), 3);

		if (state == DB_UPDATE_WITH_COMPLEX_INDEX)
		{
			//복합인덱스를 갱신한다.
		}

		if ((state == DB_UPDATE_WITH_SINGLE_INDEX)||(state == DB_UPDATE_WITH_COMPLEX_INDEX))
		{
			//싱글인덱스 갱신를 한다.
			{//기존의 싱글 인덱스를 삭제하는 루틴
				newRootOffset = dbIndexDeleteUnitSingle(columnNode.columnNodeIndexRootOffset, 
															  columnNode.columnNodeDataType, 
															  columnsValue[columnCount],
															  recordRowid);
				if (columnNode.columnNodeIndexRootOffset != newRootOffset)
				{
					columnNode.columnNodeIndexRootOffset = newRootOffset;

					//해당 컬럼의 dataType을 얻어온다
					dbMemcpyFreeIndian(&(columnNode.columnNodeIndexRootOffset), &(pColumn->columnNodeIndexRootOffset), sizeof(unsigned long), 1);
					gDbDictionaryChanged = DB_CHANGED;
				}
			}

			{//해당 값을 가지는 싱글인덱스를 추가하는 루틴
				newRootOffset = dbIndexSingleUnitInsert(columnNode.columnNodeIndexRootOffset,
															updateColumnValue,
															columnNode.columnNodeDataType,
															recordRowid);

				if (columnNode.columnNodeIndexRootOffset != newRootOffset)
				{
					columnNode.columnNodeIndexRootOffset = newRootOffset;

					//해당 컬럼의 dataType을 얻어온다
					dbMemcpyFreeIndian(&(columnNode.columnNodeIndexRootOffset), &(pColumn->columnNodeIndexRootOffset), sizeof(unsigned long), 1);
					gDbDictionaryChanged = DB_CHANGED;
				}
			}
		}
		
		if ((state == DB_UPDATE_WITHOUT_INDEX)||(state == DB_UPDATE_WITH_SINGLE_INDEX)||(state == DB_UPDATE_WITH_COMPLEX_INDEX))
		{
			//레코드의 해당 컬렁의 위치의 값을 갱신한다.
			dbDataRecordSetColumn( columnsValue[columnCount], updateColumnValue, columnNode.columnNodeDataType);
		}
	}
}
#endif
void dbIndexLinkTraverse(unsigned long dbLinkOffset)
{
	pLinkNode	pLink;		//Memory Area상의 Link의 위치를 포인팅 할 변수
	LinkNode		vLinkNode;	//Memory Area상의 Link의 내용을 복사할  Stack변수

	
	while(dbLinkOffset)
	{
		//Memory Area상의 Link의 위치를 포인한다.
		pLink = dbIndexGetLink(dbLinkOffset);

		//Stack변수에 해당 링크를 복사한다.
		dbMemcpyFreeIndian(pLink, &vLinkNode, sizeof(unsigned long), 2);

		//===================================================//
		//1. 검색결과를 저장할 Arrray에 해당 Record의 Rowid를 저장한다.
		#if 0
		*( dbSearchArray + dbSearchCount) = vLinkNode.linkNodeRecordRowid;
		//2. 검색된 갯수를 증가시킨다.
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
		
		//다음 링크의 Offset을 저장한다.
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
	pDictionaryHeader		pdicHeader;	//Dictionary file의 해더를 포인팅할 변수
	pTableNode					pTable;			//Dictionary file이 올라오는 영역에서의 테이블을 포인팅할 변수
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


	pdicHeader		= dbGetDictionaryHeader();			//Dictionary를 지정한다.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정
	
	//테이블을 찾는다.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : 입력으로 들어온 tableName의 table이 시스템에 존재하지않는 경우
		return 0;
	}
	// ===========아래부터 해당 테이블을 찾았을때 처리 ===========//

	//table의 offset이 저장되어 있다.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table의 메모리를 지정한다.
	pTable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//테이블의 내용을 TABLE_NODE에 복사한다.
	dbMemcpyFreeIndian(pTable, &TABLE_NODE, sizeof(unsigned long), 6);

	if (TABLE_NODE.tableNodeStartPage == 0)
	{
		//테이터삽입이 없었다면 더이상 검색할것도 없다..
		return 0;
	}

	//인덱스의 시작 offset의 위치를 찾아낸다.
	{
		//컬럼의 시작 offset이 저장되어 있는 주소를 포인팅한다.
		pcolumnOffset = &(pTable->tableNodeColumnOffset);

		//컬럼을 찾는다.
		if(!dbTableFindColumnName(&pcolumnOffset, indexColumnName))
		{
			//ERROR : 해당 컬럼이 존재하지 않는 경우
			return 0;
		}

		//컬럼의 offset이 저장되어 있다.
		dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);

		pColumn =  (pColumnNode)(((char*)pdicHeader) + vColumnOffset);
		dbMemcpyFreeIndian(pColumn, &columnNode, sizeof(unsigned long), 3);
	}
	//indexOffset에는 인덱스의 시작 offset의 값이 저장되어있다.


	//인덱스 탐색을 시작한다.
	dbSearchArray = resultArray;
	dbSearchCount = 0;
	
	//데이터파일의 recordRowid를 결과로 리턴할 경우
	*refMemory = DB_DATA_FILE_LOAD;
	
	dbIndexTreeTraverseConditonWithout(columnNode.columnNodeIndexRootOffset, columnNode.columnNodeDataType, columnValues, columnCount, traverseOrder);	//결과를 리턴하는 부분
	
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
//================= 이하의 내용은 PC에서 필요로 하는 부분임(begin)====================//
//====================================================================================//

int dbApiGetTotalRecordNum(DB_VARCHAR	*tableName, unsigned long *pTotal)
{
	pDictionaryHeader		pdicHeader;	//Dictionary file의 해더를 포인팅할 변수
	pTableNode			ptable;			//Dictionary file이 올라오는 영역에서의 테이블을 포인팅할 변수
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

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary를 지정한다.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정
	
	//테이블을 찾는다.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : 입력으로 들어온 tableName의 table이 시스템에 존재하지않는 경우
		#if DB_SHOW_ERROR_CODE							//error check
			gDbErrorCode = DB_ERROR_TABLE_NOT_FOUND;		//error check
		#endif												//error check

		return DB_FUNCTION_FAIL;
	}
	// ===========아래부터 해당 테이블을 찾았을때 처리 ===========//

	//table의 offset이 저장되어 있다.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

	//table의 메모리를 지정한다.
	ptable = (pTableNode)(((char*)pdicHeader) + vTableOffset);
	//테이블의 내용을 TABLE_NODE에 복사한다.
	dbMemcpyFreeIndian(&(ptable->tableNodeStartPage), &tableStartPage, sizeof(unsigned long), 1);

	recordTotalCount = 0;

	//테이블 소속의 모든 page를 돌면서 레코드의 갯수를 합친다.
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

	DbgPrintf(("\ndbApiAllSave 진입"));
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

	DbgPrintf(("\ndbApiAllSave : 1. Dictionary File Save 진입"));
	//딕셔너리에 변경사항이 있을경우 딕셔너리 파일을 저장한다.
	if (gDbDictionaryChanged == DB_CHANGED)
	{
		dbStoreDictionaryfile();
	}
	//DbgPrintf(("\ndbApiAllSave : 1. Dictionary File Save 탈출"));
	//DbgPrintf(("\ndbApiAllSave : 2. Data File Save 진입"));
	
	//데이터 파일이 로딩된 되었다면 판단하여 저장할 부분은 저장한다.
	if (gDbMmuDataPage->mmuPageNumber != DB_NULL)
	{
		dbStoreDatafile();
	}
	DbgPrintf(("\ndbApiAllSave : 2. Data File Save 탈출"));
	DbgPrintf(("\ndbApiAllSave : 3. Index File Save 진입"));
	//인덱스 파일이 로딩된 되었다면 판단하여 저장할 부분은 저장한다.
	if (gDbMmuIndexPage->mmuPageNumber != DB_NULL)
	{
		dbStoreIndexfile();
	}
	DbgPrintf(("\ndbApiAllSave : 3. Index File Save 탈출"));
	DbgPrintf(("\ndbApiAllSave 탈출"));
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
		//접근하려는 Rowid의 값이 너무 큰 경우
		#if DB_SHOW_ERROR_CODE						//Error Check
		gDbErrorCode = DB_ERROR_INVALID_ROWID_ZERO;	//Error Check
		#endif											//Error Check
		
		return DB_INVALID_ROWID_ZERO;
	}

	//Dictionary를 지정한다.
	pdicHeader	= dbGetDictionaryHeader();

	//딕셔너리해더의 정보중 DataPage의 마지막 Page번호값을 가져온다.
	dbMemcpyFreeIndian(&(pdicHeader->dictionaryHeaderNumberOfDataFileEndPage), &vEndDataPageInDictionary, sizeof(unsigned long), 1);

	//Datafile에서의 page 값을 저장한다.
	vDataPageNumber = (dbRecordRowid/gDbPageSizeDataFilePageSize)+1;

	if (vEndDataPageInDictionary<vDataPageNumber)
	{
		//접근하려는 Rowid의 값이 너무 큰 경우
		#if DB_SHOW_ERROR_CODE									//Error Check
		gDbErrorCode = DB_ERROR_INVALID_ROWID_TOO_VERY_LARGE	;	//Error Check
		#endif														//Error Check
		
		return DB_INVALID_ROWID_TOO_VERY_LARGE;
	}
	//Datafile에서의 상대 Offset 값을 저장한다.
	vRecordOffsetInPage = dbRecordRowid%gDbPageSizeDataFilePageSize;

	//해당 dbRecordRowid의 값을 가지는 Record의 Page를 Load한다.
	dbTouch(dbRecordRowid, DB_DATA_FILE_LOAD);

	//Data Page의 해더를 Pointing한다.
	pdataFilePageHeader = (pDataFilePageHeader)(gDbMmuDataPage->mmuMemoryAddress);

	//Data Page내의 레코드의 갯수를 저장한다.
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
	pDictionaryHeader		pdicHeader;		//Dictionary file의 해더를 포인팅할 변수
	pTableNode			ptable;			//Dictionary file이 올라오는 영역에서의 테이블을 포인팅할 변수
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

	//Record Copy를 실행하기 위해 들어온 recordRowid의 값이 가능한 값인지를 판단한다.
	if (dbDataCheckRecordValidRowID(recordRowid) != DB_VALID_ROWID)
	{
		#if DB_SHOW_ERROR_CODE					//Error check
		gDbErrorCode = DB_ERROR_INVALID_ROWID;	//Error check
		#endif										//Error check
		
		return DB_FUNCTION_FAIL;
	}

	{
	}
	
	pdicHeader	= dbGetDictionaryHeader();						//Dictionary를 지정한다.
	ptableOffset	= &(pdicHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정
	
	//테이블을 찾는다.
	if (!dbTableFindTableName(&ptableOffset, tableName))
	{
		//ERROR : 입력으로 들어온 tableName의 table이 시스템에 존재하지않는 경우
		#if DB_SHOW_ERROR_CODE					//Error check
		gDbErrorCode = DB_ERROR_TABLE_NOT_FOUND;	//Error check
		#endif										//Error check
		
		return DB_FUNCTION_FAIL;
	}
	//위의 if문을 통과하면 ptableOffset은 Dictionary File상의 Table Offset이 저장되어있는 곳을 포인팅한다.


	//vTableOffset에 Table의 Offset을 저장한다.
	dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);


	//ptable로 Dictionary의 Table을 Pointing한다.
	ptable = (pTableNode)(((char*)pdicHeader) + vTableOffset);

	
	//테이블의 내용을 TABLE_NODE에 복사한다.
	dbMemcpyFreeIndian(ptable, &vTableNode, sizeof(unsigned long), 6);


	//복사를 하려는 Rowid가 해당 테이블 소속의 DataPage에 있는지를 검사한다.
	{
		unsigned long vCopyRecordPageNumber;
		vCopyRecordPageNumber = (recordRowid/gDbPageSizeDataFilePageSize) + 1;
		if (!dbHaveDataPageNumberInDataPageChain(vTableNode.tableNodeStartPage, vCopyRecordPageNumber))
		{
			//ERROR : 입력으로 들어온 tableName의 table이 시스템에 존재하지않는 경우
			return DB_FUNCTION_FAIL;
		}
	}


	//해당 레코드의 값을 채워온다.
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
	pDictionaryHeader		pdicHeader;	//Dictionary file의 해더를 포인팅할 변수
	pColumnNode			pColumn;
	ColumnNode			columnNode;
	char					*pRecord;
	char					*pBuffer;
	unsigned long			columnCount;

	pdicHeader		= dbGetDictionaryHeader();			//Dictionary를 지정한다.
	
	if (columnStartOffset == 0)
	{
		return 0;
	}
	else
	{
		//실제 레코드를 가져온다.
		dbTouch(recordRowid, DB_DATA_FILE_LOAD);
		pRecord = gDbMmuDataPage->mmuMemoryAddress + recordRowid%gDbPageSizeDataFilePageSize;
		columnCount = 0;
	}

	while(columnStartOffset)
	{
		pBuffer = resultArray[columnCount];	//record가 기록될 지점을 넣는다.
		
		//다음컬럼으로의 진행
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
	pDictionaryHeader		pdicHeader;	//Dictionary file의 해더를 포인팅할 변수
	pColumnNode			pColumn;
	ColumnNode			columnNode;
	char					*pRecord;
	char					*pBuffer;
	unsigned long			columnCount;
//DbgPrintf(("===================================\n"));
	pdicHeader		= dbGetDictionaryHeader();			//Dictionary를 지정한다.
	
	if (columnStartOffset == 0)
	{
		return 0;
	}
	else
	{
		//실제 레코드를 가져온다.
		dbTouch(recordRowid, DB_DATA_FILE_LOAD);
		pRecord = gDbMmuDataPage->mmuMemoryAddress + recordRowid%gDbPageSizeDataFilePageSize;
		columnCount = 0;
	}

	while(columnStartOffset)
	{
		pBuffer = resultArray[columnCount];	//record가 기록될 지점을 넣는다.
		
		//다음컬럼으로의 진행
		columnCount++;

		pColumn = (pColumnNode)(((char*)pdicHeader) + columnStartOffset);
		dbMemcpyFreeIndian(pColumn, &columnNode, sizeof(unsigned long), 3);

		if (columnNode.columnNodeDataType == DB_VARCHAR_TYPE)
		{
//			struct mString	*pMtpString;
			unsigned long		vStringSize;

//DbgPrintf(("Dest[%x], 받기원함  ", pBuffer));
#if 0	//060110 15:02 희철
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
//================= 			이하의 내용은 PC에서 필요로 하는 부분임(e n d)			====================//
//====================================================================================//


//====================================================================================//
//================= 			이하의 내용은 디비기능의 확장 부분임(begin)			 =======================//
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
//================= 이하의 내용은 디비기능의 확장 부분임(e n d)=======================//
//====================================================================================//
unsigned long dbFindRowidIndexInResultArray(unsigned long *pArrayStart, unsigned long vStartIndex, unsigned long vArraySize, unsigned long vFindRowid)
{
	unsigned long * pArrayCurrent;// = pArrayStart + vStartIndex;

//	DbgPrintf(("[0x%x] 과 비교를 시작한다. : ", vFindRowid));
	
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

//			DbgPrintf(("Name별 Sorting Rowid : "));

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

//					DbgPrintf(("\n 찾았다!! \n"));
//					DbgPrintf(("%d번째와 %d번째 데이터 위치를 바꾼다.\n", *pSortedIndex, vSearchIndex));
					
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
		pDictionaryHeader		pdictionaryHeader;	//Dictionary file의 해더를 포인팅할 변수
		pTableNode			pTable;				//Dictionary file이 올라오는 영역에서의 테이블을 포인팅할 변수
		TableNode			TABLE_NODE;
		pColumnNode			pColumn;
		unsigned long			*ptableOffset, vTableOffset;
		unsigned long			*pcolumnOffset, vColumnOffset;
		unsigned long			indexOffset;
		
		//테이블을 찾는다.
		pdictionaryHeader	= dbGetDictionaryHeader();							//Dictionary를 지정한다.
		ptableOffset		= &(pdictionaryHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정
		
		if (!dbTableFindTableName(&ptableOffset, pTableName))
		{
			//ERROR : 입력으로 들어온 dbTableName의 table이 시스템에 존재하지않는 경우
			#if DB_SHOW_ERROR_CODE												//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_TABLE_NOT_FOUND;		//Error Check
			#endif																	//Error Check
			
			return DB_FUNCTION_FAIL;
		}
		// ===========아래부터 해당 테이블을 찾았을때 처리 ===========//

		//table의 offset이 저장되어 있다.
		dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

		//table의 메모리를 지정한다.
		pTable = (pTableNode)(((char*)pdictionaryHeader) + vTableOffset);
		//테이블의 내용을 TABLE_NODE에 복사한다.
		dbMemcpyFreeIndian(pTable, &TABLE_NODE, sizeof(unsigned long), 6);

		if (TABLE_NODE.tableNodeStartPage == 0)
		{
			//테이터삽입이 없었다면 더이상 검색할것도 없다..
			#if DB_SHOW_ERROR_CODE							//Error Check
			gDbErrorCode = DB_ERROR_TABLE_ALLOC_PAGE_NONE;	//Error Check
			#endif												//Error Check
			
			return DB_FUNCTION_FAIL;
		}


		//컬럼의 시작 offset이 저장되어 있는 주소를 포인팅한다.
		pcolumnOffset = &(pTable->tableNodeColumnOffset);

		//컬럼을 찾는다.
		if(!dbTableFindColumnName(&pcolumnOffset, pColumnName))
		{
			//ERROR : 해당 컬럼이 존재하지 않는 경우
			#if DB_SHOW_ERROR_CODE												//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_COLUMN_NOT_FOUND;	//Error Check
			#endif																	//Error Check
			
			return 0;
		}

		//컬럼의 offset이 저장되어 있다.
		dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);

		pColumn =  (pColumnNode)(((char*)pdictionaryHeader) + vColumnOffset);
		dbMemcpyFreeIndian(&(pColumn->columnNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);	//indexOffset에는 인덱스의 시작 offset의 값이 저장한다.

		//순회 하면서 Sorting을 실시한다.
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
	pDictionaryHeader		pdictionaryHeader;	//Dictionary file의 해더를 포인팅할 변수
	pTableNode			pTable;				//Dictionary file이 올라오는 영역에서의 테이블을 포인팅할 변수
	TableNode			TABLE_NODE;
	pColumnNode			pColumn;
	unsigned long			*ptableOffset, vTableOffset;
	unsigned long			*pcolumnOffset, vColumnOffset;
	unsigned long			indexOffset;
	unsigned char 		vDepth;

	#if DB_SHOW_ERROR_CODE				//Error Check
	gDbErrorCode = DB_ERROR_NO_ERROR;		//Error Check
	#endif									//Error Check
	
	//Database가 구동중인지를 체크한다.
	if (gDbStartUpDB != DB_DO_RUNNING)
	{
		#if DB_SHOW_ERROR_CODE								//Error Check
		gDbErrorCode = DB_ERROR_DATABASE_DO_NOT_RUNNING;		//Error Check
		#endif													//Error Check
		
		return 0;
	}
	else
	{
		//전달 Parameter가 정상적인지를 체크한다.
		if (indexColumnNumber <conditionColumnNumber)
		{
			#if DB_SHOW_ERROR_CODE															//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_CONDITION_PARAMETER_INCORRECT;	//Error Check
			#endif																				//Error Check

			return 0;
		}

		//오름차순으로 정렬할지 내림차순으로 정렬할지 전달받은 인자가 옳은가를 판단한다.
		if ((traverseOrder != DB_ASC) && (traverseOrder != DB_DESC))
		{
			#if DB_SHOW_ERROR_CODE											//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_ORDER_INVALID;	//Error Check
			#endif																//Error Check

			traverseOrder = DB_ASC;
		}

		

		//테이블을 찾는다.
		pdictionaryHeader	= dbGetDictionaryHeader();							//Dictionary를 지정한다.
		ptableOffset		= &(pdictionaryHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정
		
		if (!dbTableFindTableName(&ptableOffset, dbTableName))
		{
			//ERROR : 입력으로 들어온 dbTableName의 table이 시스템에 존재하지않는 경우
			#if DB_SHOW_ERROR_CODE												//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_TABLE_NOT_FOUND;		//Error Check
			#endif																	//Error Check
			
			return 0;
		}
		// ===========아래부터 해당 테이블을 찾았을때 처리 ===========//

		//table의 offset이 저장되어 있다.
		dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

		//table의 메모리를 지정한다.
		pTable = (pTableNode)(((char*)pdictionaryHeader) + vTableOffset);
		//테이블의 내용을 TABLE_NODE에 복사한다.
		dbMemcpyFreeIndian(pTable, &TABLE_NODE, sizeof(unsigned long), 6);

		if (TABLE_NODE.tableNodeStartPage == 0)
		{
			//테이터삽입이 없었다면 더이상 검색할것도 없다..
			#if DB_SHOW_ERROR_CODE							//Error Check
			gDbErrorCode = DB_ERROR_TABLE_ALLOC_PAGE_NONE;	//Error Check
			#endif												//Error Check
			
			return 0;
		}

		//인덱스의 시작 offset의 위치를 찾아낸다.
		{
			//단일인덱스일 경우 인덱스의 시작 offset의 위치를 찾아낸다.
			if (indexColumnNumber == 1)	
			{
				//컬럼의 시작 offset이 저장되어 있는 주소를 포인팅한다.
				pcolumnOffset = &(pTable->tableNodeColumnOffset);

				//컬럼을 찾는다.
				if(!dbTableFindColumnName(&pcolumnOffset, indexColumnName[0]))
				{
					//ERROR : 해당 컬럼이 존재하지 않는 경우
					#if DB_SHOW_ERROR_CODE												//Error Check
					gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_COLUMN_NOT_FOUND;	//Error Check
					#endif																	//Error Check
					
					return 0;
				}

				//컬럼의 offset이 저장되어 있다.
				dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);

				pColumn =  (pColumnNode)(((char*)pdictionaryHeader) + vColumnOffset);

				dbMemcpyFreeIndian(&(pColumn->columnNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);	//indexOffset에는 인덱스의 시작 offset의 값이 저장한다.
			}
			//복합인덱스일 경우 인덱스의 시작 offset의 위치를 찾아낸다.
			else
			{
				unsigned long			complexOffset;
				pComplexIndexNode	pComplexIndex;
				ComplexIndexNode	complexIndex;

				//complexOffset에 복합인덱스의 시작 offset의 값을 저장한다.
				dbMemcpyFreeIndian(&(pTable->tableNodeComplexIndexOffset), &complexOffset, sizeof(unsigned long), 1);

				//모든 복합인덱스 컬럼을 순차적으로 방문하면서 찾는다.
				while(complexOffset)
				{
					pComplexIndex = (pComplexIndexNode)(((char*)pdictionaryHeader) + complexOffset);
					dbMemcpyFreeIndian(pComplexIndex, &complexIndex, sizeof(unsigned long), 3);

					if (complexIndex.complexIndexNodeColumnNumber == indexColumnNumber)
					{
						if (dbIndexCheckName((DB_VARCHAR*)(pComplexIndex + 1) , indexColumnName, indexColumnNumber) != 0)
						{
							//해당 복합인덱스를 찾은경우.
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

				dbMemcpyFreeIndian(&(pComplexIndex->complexIndexNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);	//indexOffset에는 인덱스의 시작 offset의 값이 저장한다.
			}
		}
		//indexOffset에는 인덱스의 시작 offset의 값이 저장되어있다.


		vDepth = indexColumnNumber - conditionColumnNumber;

		//인덱스 탐색을 시작한다.
		if (conditionColumnNumber == 0)
		{
			dbSearchArray = resultArray;
			dbSearchCount = 0;

			dbIndexTreeTraverseAll(indexOffset, traverseOrder, vDepth);	//결과를 리턴하는 부분
		}
		else
		{
			unsigned long	i;
			unsigned long	columnDataType;
			for (i = 0 ; i < conditionColumnNumber ; i++)
			{
				//컬럼의 시작 offset이 저장되어 있는 주소를 포인팅한다.
				pcolumnOffset = &(pTable->tableNodeColumnOffset);
				
				//찾는 컬럼의 offset을 구한다.
				dbTableFindColumnName(&pcolumnOffset, indexColumnName[i]);
				dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);
							
				
				//해당 컬럼을 포인팅한다.
				pColumn =  (pColumnNode)(((char*)pdictionaryHeader) + vColumnOffset);
				
				//컬럼의 dataType을 복사한다.
				dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &columnDataType, sizeof(unsigned long), 1);
					
				
				//검색을 실시한다.
				indexOffset = dbIndexSearchGetInodeMS(indexOffset, columnDataType, conditionSet[i]);
				
				if (indexOffset == 0) return 0;
				
				//검색된 결과 다른 트리의 rootOffset이 검색된다.
				

				if (( (conditionColumnNumber - 1) == i ))		//마지막에 한번만 일어난다. 결과를 리턴하는 부분
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
	pDictionaryHeader		pdictionaryHeader;	//Dictionary file의 해더를 포인팅할 변수
	pTableNode			pTable;				//Dictionary file이 올라오는 영역에서의 테이블을 포인팅할 변수
	TableNode			TABLE_NODE;
	pColumnNode			pColumn;
	unsigned long			*ptableOffset, vTableOffset;
	unsigned long			*pcolumnOffset, vColumnOffset;
	unsigned long			indexOffset;

	#if DB_SHOW_ERROR_CODE				//Error Check
	gDbErrorCode = DB_ERROR_NO_ERROR;		//Error Check
	#endif									//Error Check
	
	//Database가 구동중인지를 체크한다.
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

		//전달 Parameter가 정상적인지를 체크한다.
		if (indexColumnNumber <conditionColumnNumber)
		{
			#if DB_SHOW_ERROR_CODE															//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_CONDITION_PARAMETER_INCORRECT;	//Error Check
			#endif																				//Error Check

			return 0;
		}

		//오름차순으로 정렬할지 내림차순으로 정렬할지 전달받은 인자가 옳은가를 판단한다.
		if ((traverseOrder != DB_ASC) && (traverseOrder != DB_DESC))
		{
			#if DB_SHOW_ERROR_CODE											//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_ORDER_INVALID;	//Error Check
			#endif																//Error Check

			traverseOrder = DB_ASC;
		}

		

		//테이블을 찾는다.
		pdictionaryHeader	= dbGetDictionaryHeader();							//Dictionary를 지정한다.
		ptableOffset		= &(pdictionaryHeader->dictionaryHeaderOffsetOfRootTable);	//table을 구성하는 트리의 시작 offset을 저장하는 위치를 지정
		
		if (!dbTableFindTableName(&ptableOffset, dbTableName))
		{
			//ERROR : 입력으로 들어온 dbTableName의 table이 시스템에 존재하지않는 경우
			#if DB_SHOW_ERROR_CODE												//Error Check
			gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_TABLE_NOT_FOUND;		//Error Check
			#endif																	//Error Check
			
			return 0;
		}
		// ===========아래부터 해당 테이블을 찾았을때 처리 ===========//

		//table의 offset이 저장되어 있다.
		dbMemcpyFreeIndian(ptableOffset, &vTableOffset, sizeof(unsigned long), 1);

		//table의 메모리를 지정한다.
		pTable = (pTableNode)(((char*)pdictionaryHeader) + vTableOffset);
		//테이블의 내용을 TABLE_NODE에 복사한다.
		dbMemcpyFreeIndian(pTable, &TABLE_NODE, sizeof(unsigned long), 6);

		if (TABLE_NODE.tableNodeStartPage == 0)
		{
			//테이터삽입이 없었다면 더이상 검색할것도 없다..
			#if DB_SHOW_ERROR_CODE							//Error Check
			gDbErrorCode = DB_ERROR_TABLE_ALLOC_PAGE_NONE;	//Error Check
			#endif												//Error Check
			
			return 0;
		}

		//인덱스의 시작 offset의 위치를 찾아낸다.
		{
			//단일인덱스일 경우 인덱스의 시작 offset의 위치를 찾아낸다.
			if (indexColumnNumber == 1)	
			{
				//컬럼의 시작 offset이 저장되어 있는 주소를 포인팅한다.
				pcolumnOffset = &(pTable->tableNodeColumnOffset);

				//컬럼을 찾는다.
				if(!dbTableFindColumnName(&pcolumnOffset, indexColumnName[0]))
				{
					//ERROR : 해당 컬럼이 존재하지 않는 경우
					#if DB_SHOW_ERROR_CODE												//Error Check
					gDbErrorCode = DB_ERROR_DBAPISEARCHMS_FUNCTION_COLUMN_NOT_FOUND;	//Error Check
					#endif																	//Error Check
					
					return 0;
				}

				//컬럼의 offset이 저장되어 있다.
				dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);

				pColumn =  (pColumnNode)(((char*)pdictionaryHeader) + vColumnOffset);
				
				dbMemcpyFreeIndian(&(pColumn->columnNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);	//indexOffset에는 인덱스의 시작 offset의 값이 저장한다.
			}
			//복합인덱스일 경우 인덱스의 시작 offset의 위치를 찾아낸다.
			else
			{
				unsigned long			complexOffset;
				pComplexIndexNode	pComplexIndex;
				ComplexIndexNode	complexIndex;

				//complexOffset에 복합인덱스의 시작 offset의 값을 저장한다.
				dbMemcpyFreeIndian(&(pTable->tableNodeComplexIndexOffset), &complexOffset, sizeof(unsigned long), 1);

				//모든 복합인덱스 컬럼을 순차적으로 방문하면서 찾는다.
				while(complexOffset)
				{
					pComplexIndex = (pComplexIndexNode)(((char*)pdictionaryHeader) + complexOffset);
					dbMemcpyFreeIndian(pComplexIndex, &complexIndex, sizeof(unsigned long), 3);

					if (complexIndex.complexIndexNodeColumnNumber == indexColumnNumber)
					{
						if (dbIndexCheckName((DB_VARCHAR*)(pComplexIndex + 1) , indexColumnName, indexColumnNumber) != 0)
						{
							//해당 복합인덱스를 찾은경우.
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

				dbMemcpyFreeIndian(&(pComplexIndex->complexIndexNodeIndexRootOffset), &indexOffset, sizeof(unsigned long), 1);	//indexOffset에는 인덱스의 시작 offset의 값이 저장한다.
			}
		}
		//indexOffset에는 인덱스의 시작 offset의 값이 저장되어있다.


		//인덱스 탐색을 시작한다.
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
					//데이터파일의 recordRowid를 결과로 리턴할 경우
					*refMemory = DB_DATA_FILE_LOAD;
					gDbSearchData.refMemory = DB_DATA_FILE_LOAD;
					dbIndexTreeTraverse(indexOffset, traverseOrder, 1);	//결과를 리턴하는 부분
				}
			}
			else
			{
				if ((indexOffset != 0)&&(indexOffset != 1))
				{
					//검색된 결과는 인덱스 파일의 내용이다.
					*refMemory = DB_INDEX_FILE_LOAD;
					gDbSearchData.refMemory = DB_INDEX_FILE_LOAD;
					dbIndexTreeTraverse(indexOffset, traverseOrder, 0);	//결과를 리턴하는 부분
				}
			}
		}
		else
		{
			unsigned long	i;
			unsigned long	columnDataType;
			for (i = 0 ; i < conditionColumnNumber ; i++)
			{
				//컬럼의 시작 offset이 저장되어 있는 주소를 포인팅한다.
				pcolumnOffset = &(pTable->tableNodeColumnOffset);
				
				//찾는 컬럼의 offset을 구한다.
				dbTableFindColumnName(&pcolumnOffset, indexColumnName[i]);
				dbMemcpyFreeIndian(pcolumnOffset, &vColumnOffset, sizeof(unsigned long), 1);
							
				
				//해당 컬럼을 포인팅한다.
				pColumn =  (pColumnNode)(((char*)pdictionaryHeader) + vColumnOffset);
				
				//컬럼의 dataType을 복사한다.
				dbMemcpyFreeIndian(&(pColumn->columnNodeDataType), &columnDataType, sizeof(unsigned long), 1);
					
				if ((indexOffset == 0)||(indexOffset == 1)) return 0;
				
				//검색을 실시한다.
				indexOffset = dbIndexSearchGetInodeMS(indexOffset, columnDataType, conditionSet[i]);
				
				if ((indexOffset == 0)||(indexOffset == 1)) return 0;
				
				//검색된 결과 다른 트리의 rootOffset이 검색된다.
				

				if (( (conditionColumnNumber - 1) == i ) && (indexColumnNumber != conditionColumnNumber))		//마지막에 한번만 일어난다. 결과를 리턴하는 부분
				{
					#if 0
					dbSearchArray = resultArray;
					dbSearchCount = 0;
					#endif
					
					if (indexColumnNumber == (conditionColumnNumber + 1))
					{
						//데이터파일의 recordRowid를 결과로 리턴할 경우
						*refMemory = DB_DATA_FILE_LOAD;
						gDbSearchData.refMemory = DB_DATA_FILE_LOAD;
						dbIndexTreeTraverse(indexOffset, traverseOrder, 1);
					}
					else
					{
						//검색된 결과는 인덱스 파일의 내용이다.
						*refMemory = DB_INDEX_FILE_LOAD;
						gDbSearchData.refMemory = DB_INDEX_FILE_LOAD;
						dbIndexTreeTraverse(indexOffset, traverseOrder, 0);
					}
				}
				else if(( (conditionColumnNumber - 1) == i ) && (indexColumnNumber == conditionColumnNumber))		//마지막에 한번만 일어난다. 결과를 리턴하는 부분
				{
					#if 0
					dbSearchArray = resultArray;
					dbSearchCount = 0;
					#endif
					//검색된 결과는 인덱스 파일의 내용이다.
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

		//비교
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
//									File 관련 처리 부분(Being)									//
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
	{//Data File Load를 시도할 경우 처리

		if (filePageNumber <= dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage)
		{//이 경우 Data File에 존재하는 Page를 요구할 경우 처리

			unsigned long vReadingSize = 0;

			//============ 1. file을 연다. (Begin) ============//
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
			//============ 1. file을 연다. (E n d) ============//


			//============ 2. file을 읽는다. (Begin) ============//
			{
				fseek(gDbFileDataFile, (filePageNumber - 1)*gDbPageSizeDataFilePageSize, SEEK_SET);
				vReadingSize = fread(targetMemoryAddress, gDbPageSizeDataFilePageSize, 1, gDbFileDataFile);
			}
			//============ 2. file을 읽는다. (E n d) ============//


			//============ 3. file을 닫는다. (Begin) ============//
			{
				fclose(gDbFileDataFile);
				gDbFileDataFile = 0;
			}
			//============ 3. file을 닫는다. (E n d) ============//
				
			if (vReadingSize != 1)
			{//이 경우 제대로 못 읽은 경우 처리
				#if DB_SHOW_ERROR_CODE															//Error Check
				gDbErrorCode = DB_ERROR_DATA_FILE_READ_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION;	//Error Check
				#endif

				//============ 1. file을 연다. (Begin) ============//
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
				//============ 1. file을 연다. (E n d) ============//
				
				//============ 2. file을 읽는다. (Begin) ============//
				{
					fseek(gDbFileDataFile, (filePageNumber - 1)*gDbPageSizeDataFilePageSize, SEEK_SET);
					vReadingSize = fread(targetMemoryAddress, gDbPageSizeDataFilePageSize, 1, gDbFileDataFile);
				}
				//============ 2. file을 읽는다. (E n d) ============//
				
				//============ 3. file을 닫는다. (Begin) ============//
				{
					fclose(gDbFileDataFile);
					gDbFileDataFile = 0;
				}
				//============ 3. file을 닫는다. (E n d) ============//
			}

			
			if (vReadingSize != 1)
			{//File을 다시 읽기를 시도했지만 실패를 한 경우
				#if DB_SHOW_ERROR_CODE															//Error Check
				gDbErrorCode = DB_ERROR_DATA_FILE_READ_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION;	//Error Check
				#endif
				printf("File Read Error");
			}
			
			gDbMmuDataPage->mmuChanged = DB_UNCHANGED;
		}
		else
		{//이 경우 Data File에 존재하지 않는 Page를 요구할 경우 처리.

			pDataFilePageHeader pDH = (pDataFilePageHeader)targetMemoryAddress;
			
			DataFilePageHeader	DH;
			
			//============= 1. 로딩할 DataPage의 Memory에 초기Header값을 셋팅한다. (Begin) =============//
			{
				DH.dataHeaderDeleteRecordOffset	= 0;
				DH.dataHeaderEndOfRecords		= sizeof(DataFilePageHeader);
				DH.dataHeaderNextSameTablePage	= 0;
				DH.dataHeaderNumberOfSlots		= 0;

				dbMemcpyFreeIndian(&DH, pDH, sizeof(unsigned long), 4);
			}
			//============= 1. 로딩할 DataPage의 Memory에 초기Header값을 셋팅한다. (E n d) =============//


			//============= 2. 새로 추가되 Data Page 이므로 Dictionary Header를 갱신한다. (Being) ===========//
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
			//============= 2. 새로 추가되 Data Page 이므로 Dictionary Header를 갱신한다. (E n d) ===========//
		}		
	}
	else if(fileType == DB_INDEX_FILE_LOAD)
	{//IndexFile Load를 시도할 경우 처리
		
		if (filePageNumber <= dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage)
		{//이 경우 Index File에 존재하는 Page를 요구할 경우 처리

			unsigned long vReadingSize = 0;

			//============ 1. file을 연다. (Begin) ============//
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
			//============ 1. file을 연다. (E n d) ============//
			

			//============ 2. file을 읽는다. (Begin) ============//
			{
				fseek(gDbFileIndexFile, (filePageNumber - 1)*gDbPageSizeIndexFilePageSize, SEEK_SET);
				vReadingSize = fread(targetMemoryAddress, gDbPageSizeIndexFilePageSize, 1, gDbFileIndexFile);
			}
			//============ 2. file을 읽는다. (E n d) ============//


			//============ 3. file을 닫는다. (Begin) ============//
			{
				fclose(gDbFileIndexFile);
				gDbFileIndexFile = 0;					
			}
			//============ 3. file을 닫는다. (E n d) ============//
				
			if (vReadingSize != 1)
			{//이 경우 제대로 못 읽은 경우 처리
				#if DB_SHOW_ERROR_CODE															//Error Check
				gDbErrorCode = DB_ERROR_INDEX_FILE_READ_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION;	//Error Check
				#endif																				//Error Check

				//============ 1. file을 연다. (Begin) ============//
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
				//============ 1. file을 연다. (E n d) ============//
				
				//============ 2. file을 읽는다. (Begin) ============//
				{
					fseek(gDbFileIndexFile, (filePageNumber - 1)*gDbPageSizeIndexFilePageSize, SEEK_SET);
					vReadingSize = fread(targetMemoryAddress, gDbPageSizeIndexFilePageSize, 1, gDbFileIndexFile);
				}
				//============ 2. file을 읽는다. (E n d) ============//

				//============ 3. file을 닫는다. (Begin) ============//
				{
					fclose(gDbFileIndexFile);
					gDbFileIndexFile = 0;
				}
				//============ 3. file을 닫는다. (E n d) ============//

				if (vReadingSize != 1)
				{//File을 다시 읽기를 시도했지만 실패를 한 경우
					#if DB_SHOW_ERROR_CODE															//Error Check
					gDbErrorCode = DB_ERROR_INDEX_FILE_READ_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION;	//Error Check
					#endif																				//Error Check
					printf("File Read Error");
				}
			}
			
			gDbMmuIndexPage->mmuChanged = DB_UNCHANGED;
		}
		else
		{//이 경우 Index File에 존재하는 Page를 요구할 경우 처리

			pIndexFilePageHeader pDH = (pIndexFilePageHeader)targetMemoryAddress;
			
			IndexFilePageHeader	 DH;
			
			//============= 1. 로딩할 IndexPage의 Memory에 초기Header값을 셋팅한다. (Begin) =============//
			{
				DH.indexHeaderDeleteNodeOffset	= 0;
				DH.indexHeaderDeleteLinkOffset	= 0;
				DH.indexHeaderEndOfDatas			= sizeof(IndexFilePageHeader);
			
				dbMemcpyFreeIndian(&DH, pDH, sizeof(unsigned long), 3);
			}
			//============= 1. 로딩할 IndexPage의 Memory에 초기Header값을 셋팅한다. (E n d) =============//

			//============= 2. 새로 추가되 Index Page 이므로 Dictionary Header를 갱신한다. (Being) ===========//
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
			//============= 2. 새로 추가되 Data Page 이므로 Dictionary Header를 갱신한다. (Being) ===========//
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

		//============= 1. File을 Open한다. (Begin) =============//
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
		//============= 1. File을 Open한다. (E n d) =============//


		//============= 2. File을 Write한다. (Begin) =============//
		{
			fseek(gDbFileDataFile, (filePageNumber-1)*gDbPageSizeDataFilePageSize, SEEK_SET);
			vWriteCount = fwrite(targetMemoryAddress, gDbPageSizeDataFilePageSize, 1, gDbFileDataFile);
		}
		//============= 2. File을 Write한다. (E n d) =============//

		//============= 3. File을 Close한다. (Begin) =============//
		{
			fclose(gDbFileDataFile);
			gDbFileDataFile = 0;
		}
		//============= 3. File을 Close한다. (E n d) =============//

		if (vWriteCount != 1 )
		{//기록에 성공하지 못한경우 

			#if DB_SHOW_ERROR_CODE																//Error Check
			gDbErrorCode = DB_ERROR_DATA_FILE_WRITE_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION;	//Error Check
			#endif																					//Error Check
			
			//============= 1. File을 Open한다. (Begin) =============//
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
			//============= 1. File을 Open한다. (E n d) =============//
			
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
			//Error없이 기록에 성공한 경우
		}
		else
		{
			#if DB_SHOW_ERROR_CODE																//Error Check
			gDbErrorCode = DB_ERROR_INDEX_FILE_WRITE_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION;	//Error Check
			#endif																					//Error Check
			
			//기록에 성공하지 못한경우 
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

	//이 함수는 database를 start하면서 db에서 사용하는 파일을 가져오는 함수이다.
	pDictionaryHeader		pDbDictionaryHeader;
	DictionaryHeader		dbDictionaryHeader;


	//=============================Dictionary File Loading (Begin)=============================//
	if (vDictionaryFileSize == 0)	//사용자가 강제로 사이즈를 결정해 주지 않았을 경우 처리
	{
		// Action 1. 딕셔너리의 해더의 내용이 올라올 곳에 가장 초기의 딕셔너리 내용을 올린다.
		{
			pDbDictionaryHeader = dbGetDictionaryHeader();

			dbDictionaryHeader.dictionaryHeaderDeleteNode				= 0;
			dbDictionaryHeader.dictionaryHeaderDeleteStartPage				= 0;
			dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage	= 0;
			dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage	= 0;
			dbDictionaryHeader.dictionaryHeaderOffsetOfRootTable			= 0;
			dbDictionaryHeader.dictionaryHeaderSizeOfDictionary			= sizeof(DictionaryHeader);
			

			//딕셔너리해더에 지역변수로 만든 딕셔너리 해더의 내용을 복사한다.
			dbMemcpyFreeIndian(&dbDictionaryHeader, pDbDictionaryHeader, sizeof(unsigned long), 6);
		}

		// Action 2. 딕셔너리 파일이 존재하면 그 내용을 올린다.
		{
			unsigned long vReadingSize = 0;

			// Action 2.1 딕셔너리 파일을 Open한다.
			{
				if (gDbFileDictionaryFile)
				{
					fclose(gDbFileDictionaryFile);
					gDbFileDictionaryFile = 0;
				}
				
				gDbFileDictionaryFile = fopen(gDbFileNameDictionaryFileName, "a+b");		//딕셔너리 파일을 Open한다.

				//Error Dictionary File을 Open하였는지를 판단한다.
				if (gDbFileDictionaryFile == 0)
				{
					#if DB_SHOW_ERROR_CODE																	//Error Check
					gDbErrorCode = DB_ERROR_DICTIONARY_FILE_OPEN_FAIL_AT_DBLOADDATABASEFILES_FUNTION;		//Error Check
					#endif																						//Error Check

					return DB_FUNCTION_FAIL;
				}
			}
			
			// Action 2.2. 딕셔너리 파일의 해더를 해당 위치에 복사한다.
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

			// Action 2.3. 딕셔너리 파일의 해더를 해당 구조체에 복사한다.
			dbMemcpyFreeIndian(pDbDictionaryHeader, &dbDictionaryHeader, sizeof(unsigned long), 6);

			// Action 2.3. 딕셔너리 파일의 Body를 해당 위치에 복사한다.
			{
				vReadingSize = 0;
				
				// Action 2.3.1. 딕셔너리 파일을 Open한다..
				{
					if (gDbFileDictionaryFile)
					{
						fclose(gDbFileDictionaryFile);
						gDbFileDictionaryFile = 0;
					}
					
					gDbFileDictionaryFile = fopen(gDbFileNameDictionaryFileName, "a+b");		//딕셔너리 파일을 Open한다.

					//Error Dictionary File을 Open하였는지를 판단한다.
					if (gDbFileDictionaryFile == 0)
					{
						#if DB_SHOW_ERROR_CODE																	//Error Check
						gDbErrorCode = DB_ERROR_DICTIONARY_FILE_OPEN_FAIL_AT_DBLOADDATABASEFILES_FUNTION;		//Error Check
						#endif																						//Error Check

						return DB_FUNCTION_FAIL;
					}
				}

				// Action 2.3.2. 딕셔너리 파일의 Body를 Read한다..
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
	else		//사용자가 강제로 사이즈를 결정해 주엇을 경우 처리
	{
		unsigned long vReadingSize = 0;

		// Action 1. 딕셔너리 파일의 내용이 올라올 곳의 주소를 찾는다.
		pDbDictionaryHeader = dbGetDictionaryHeader();

		// Action 2. 딕셔너리 파일을 Open한다.
		{
			if (gDbFileDictionaryFile)
			{
				fclose(gDbFileDictionaryFile);
				gDbFileDictionaryFile = 0;
			}

			gDbFileDictionaryFile = fopen(gDbFileNameDictionaryFileName, "a+b");		//딕셔너리 파일을 Open한다.

			//Error Dictionary File을 Open하였는지를 판단한다.
			if (gDbFileDictionaryFile == 0)
			{
				#if DB_SHOW_ERROR_CODE																	//Error Check
				gDbErrorCode = DB_ERROR_DICTIONARY_FILE_OPEN_FAIL_AT_DBLOADDATABASEFILES_FUNTION;		//Error Check
				#endif																						//Error Check

				return DB_FUNCTION_FAIL;
			}
		}

		// Action 3. 딕셔너리 파일을 Memory에 Load한다.
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
		//변경이 있던 page만을 저장한다.
		if (temp->mmuChanged == DB_CHANGED)
		{
			vWriteCount = 0;
			
			//이 함수는 메모리에 로드되어 있는 모든 datapage를 datafile에 저장한다.
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
				//기록에 성공한 경우 	
			}
			else
			{

				#if DB_SHOW_ERROR_CODE														//Error Check
				gDbErrorCode = DB_ERROR_DATA_FILE_WRITE_FAIL_AT_DBSTOREDATAFILE_FUNTION;		//Error Check
				#endif																			//Error Check

				//기록에 성공하지 못한 경우 
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
		//변경이 있던 page만을 저장한다.
		if (temp->mmuChanged == DB_CHANGED)
		{
			vWriteCount = 0;
			//이 함수는 메모리에 로드되어 있는 모든 datapage를 datafile에 저장한다.
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
				//기록에 성공한 경우 
			}
			else
			{
				#if DB_SHOW_ERROR_CODE														//Error Check
				gDbErrorCode = DB_ERROR_INDEX_FILE_WRITE_FAIL_AT_DBSTOREINDEXFILE_FUNTION;		//Error Check
				#endif						
				
				//기록에 성공하지 못한 경우 
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
//묵시적으로 여기가 TFS4

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
#ifdef CONFIG_HDD  //jaihong.kim noncache 사용않함 (E10)

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
#ifdef CONFIG_HDD  //jaihong.kim noncache 사용않함 (E10)

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
				//정상적인 경우에는 2번 만에 기록되어야 한다.
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

	//이 함수는 database를 start하면서 db에서 사용하는 파일을 가져오는 함수이다.
	pDictionaryHeader		pDbDictionaryHeader;
	DictionaryHeader		dbDictionaryHeader;


	//==========================Dictionary File Load (Being)==========================//
	if(vDictionaryFileSize == 0)
	{
		// Action 1. 딕셔너리의 해더의 내용이 올라올 곳에 가장 초기의 딕셔너리 내용을 올린다.
		{
			pDbDictionaryHeader = dbGetDictionaryHeader();

			//딕셔너리의 구조체에 초기 상태의 값을 셋팅한다.
			dbDictionaryHeader.dictionaryHeaderDeleteNode				= 0;
			dbDictionaryHeader.dictionaryHeaderDeleteStartPage				= 0;
			dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage	= 0;
			dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage	= 0;
			dbDictionaryHeader.dictionaryHeaderOffsetOfRootTable			= 0;
			dbDictionaryHeader.dictionaryHeaderSizeOfDictionary			= sizeof(DictionaryHeader);
			
			//딕셔너리해더에 지역변수로 만든 딕셔너리 해더의 내용을 복사한다.
			dbMemcpyFreeIndian(&dbDictionaryHeader, pDbDictionaryHeader, sizeof(unsigned long), 6);
		}

		// Action 2. 딕셔너리 파일이 존재하면 그 내용을 올린다.
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
				//무조건 파일 이 없는 경우
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
			//여기는 파일이  존재하는 경우
				// Action 2.2. 딕셔너리 파일의 해더를 해당 구조체에 복사한다.
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

		// 딕셔너리 파일을 Open한다.
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
		//딕셔너리 파일을 Read한다.
		dbFileRead((unsigned long)gDbFileDictionaryFile, (unsigned char*)pDbDictionaryHeader, vDictionaryFileSize);

		//딕셔너리 파일을 Close한다.
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
		tfs4_close(gDbFileDataFile);		//파일 항상 단아 둔다.
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
		tfs4_close(gDbFileIndexFile);	//파일 항상 단아 둔다.
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

	//이 함수는 database를 start하면서 db에서 사용하는 파일을 가져오는 함수이다.
	pDictionaryHeader		pDbDictionaryHeader;
	DictionaryHeader		dbDictionaryHeader;

//DbgPrintf(("dbLoadDatabaseFiles++\n"));
	//dictionaryFile load
	{
		// Action 1. 딕셔너리의 해더의 내용이 올라올 곳에 가장 초기의 딕셔너리 내용을 올린다.
		{
			pDbDictionaryHeader = dbGetDictionaryHeader();

			//딕셔너리의 구조체에 초기 상태의 값을 셋팅한다.
			dbDictionaryHeader.dictionaryHeaderDeleteNode				= 0;
			dbDictionaryHeader.dictionaryHeaderDeleteStartPage				= 0;
			dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage	= 0;
			dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage	= 0;
			dbDictionaryHeader.dictionaryHeaderOffsetOfRootTable			= 0;
			dbDictionaryHeader.dictionaryHeaderSizeOfDictionary			= sizeof(DictionaryHeader);
			
			//딕셔너리해더에 지역변수로 만든 딕셔너리 해더의 내용을 복사한다.
			dbMemcpyFreeIndian(&dbDictionaryHeader, pDbDictionaryHeader, sizeof(unsigned long), 6);
		}

		// Action 2. 딕셔너리 파일이 존재하면 그 내용을 올린다.
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
				//무조건 파일 이 없는 경우
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
			//여기는 파일이  존재하는 경우
				// Action 2.2. 딕셔너리 파일의 해더를 해당 구조체에 복사한다.
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
		tfs4_close(gDbFileDataFile);		//파일 항상 단아 둔다.
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
		tfs4_close(gDbFileIndexFile);	//파일 항상 단아 둔다.
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


//DbgPrintf(("dbApiShutDown:: 함수진입\n"));		

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
		//fclose(gDbFileDataFile);	//datafile을 close한다
		//tfs4_close(gDbFileDataFile);			

		if (gDbMmuIndexPage->mmuPageNumber != 0)
		{
			dbStoreIndexfile();
		}
		//fclose(gDbFileIndexFile);	//indexfile을 close한다
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
		DbgPrintf(("dbStoreDictionaryfile:: 딕셔너리 파일을 열지 못했습니다.\n"));
	}
	else
	{
		DbgPrintf(("=================[dbStoreDictionaryfile:: 딕녀서리 파일기록을 시도합니다.]=================\n"));

		DbgPrintf(("dbStoreDictionaryfile:: 딕녀서리 파일기록을 시도합니다.\n"));

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

		if (filePageNumber <= dbDictionaryHeader.dictionaryHeaderNumberOfDataFileEndPage)	//이 경우 datafile에서 읽어온다
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
		else	//이경우 datafile에는 존재하지 않으므로 셋팅만을 한다.
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

		if (filePageNumber <= dbDictionaryHeader.dictionaryHeaderNumberOfIndexFileEndPage)	//이 경우 indexfile에서 읽어온다
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
		else	//이경우 indexfile에는 존재하지 않으므로 셋팅만을 한다.
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

//DbgPrintf(("=================[dbStoreDatafile:: 데이터 파일기록을 시도합니다.]=================\n"));

	//DbgPrintf(("\n\n\nData file store"));

	while(temp != DB_NULL)
	{
		//변경이 있던 page만을 저장한다.
		if (temp->mmuPageNumber == 0) return;
		if (temp->mmuChanged == DB_CHANGED)
		{

//DbgPrintf(("[dbStoreDatafile:: %d Page 기록을 시도합니다.]\n", temp->mmuPageNumber));
//DbgPrintf(("[dbStoreDatafile:: 0x%x 주소에서  0x%x Size만큼 기록하는것을 시도합니다.]\n", temp->mmuMemoryAddress, gDbPageSizeDataFilePageSize));

			if (gDbFileDataFile != -1)
			{
				tfs4_close(gDbFileDataFile);
				gDbFileDataFile = -1;
			}
			
			gDbFileDataFile = tfs4_open((t_char*)gDbFileNameDataFileName, O_RDWR|O_TFS4_DIRECTIO);

			//이 함수는 메모리에 로드되어 있는 모든 datapage를 datafile에 저장한다.
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
	
//DbgPrintf(("=================[dbStoreIndexfile:: 인덱스 파일기록을 시도합니다.]=================\n"));

	
	//DbgPrintf(("\n\n\nIndex file write"));
	

	while(temp != DB_NULL)
	{
		if (temp->mmuPageNumber == 0) return;
		//변경이 있던 page만을 저장한다.
		if (temp->mmuChanged == DB_CHANGED)
		{
//DbgPrintf(("[dbStoreIndexfile:: %d Page 기록을 시도합니다.]\n", temp->mmuPageNumber));
//DbgPrintf(("[dbStoreIndexfile:: 0x%x 주소에서  0x%x Size만큼 기록하는것을 시도합니다.]\n", temp->mmuMemoryAddress, gDbPageSizeIndexFilePageSize));
			if (gDbFileIndexFile != -1)
			{
				tfs4_close(gDbFileIndexFile);
				gDbFileIndexFile = -1;
			}
			
			gDbFileIndexFile = tfs4_open((t_char*)gDbFileNameIndexFileName, O_RDWR|O_TFS4_DIRECTIO);

			//DbgPrintf(("\n DB index Changed!, dbStoreIndexfile() Page:%d", temp->mmuPageNumber));

			//이 함수는 메모리에 로드되어 있는 모든 datapage를 datafile에 저장한다.
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
//									File 관련 처리 부분( End )									//
//																								//
//////////////////////////////////////////////////////////////////////////////////////////////////

