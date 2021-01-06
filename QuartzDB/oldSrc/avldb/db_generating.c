#include "region.h"
#include "avldb.h"
#include "db_design.h"
#include "tfs4_api.h"
#include "tfs4_types.h"
#include "new_db_config.h"
#include "malloc.h"
#include "Uart_api.h"
#include "new_ifp_local_function.h"
#include "id3.h"
#include "db_util.h"
#include "config.h"		//060409challen.jo - @DEBUG_PRELOAD
#include "mtpspec.h"
#include "stringsys.h"

#if UART_DISPLAY
#define	DbgPrintf(x)	UART_Print x
#else
#define	DbgPrintf(x)	//UART_Print x
#endif
#include "mtp_interface_db.h"

extern unsigned long  NEW_AddID3TagToDB(void);
extern unsigned char	gDbStartUpDB;


const unsigned short	gCurrentDirectory[]	= {'.' , 0};
const unsigned short	gParentDirectory[] 	= {'.','.' ,0};
unsigned short 		*gTempFullPath;
unsigned short		*gTempFullPathEnd;
unsigned short		*gTempCurrentPath;

/*
void printTwobyteString(unsigned short* pSource)
{
	DbgPrintf(("\n"));

	while(*pSource)
	{
		DbgPrintf(("%c", *pSource));
		pSource++;
	}
	DbgPrintf(("\n"));
}
*/

unsigned short GetMTPFileType(unsigned short *pFileName)
{
	unsigned short	vLowerCaseMask = 0x20;
	unsigned short* 	pFileExtension;
	unsigned short* 	pFileNameEnd;
	
	pFileExtension = pFileName;

	//문자열 끝까지 간다.
	while(*pFileExtension)
	{
		pFileExtension++;
	}
	
	pFileNameEnd = pFileExtension;

	if (pFileName == pFileNameEnd) return 0;

	//문자열 끝에서 거슬러 올라오면서 처음 .을 만나는 곳에서 종료한다.
	while(1)
	{
		pFileExtension--;

		if (pFileExtension<pFileName)
		{
			return 0;
		}
		if ((*pFileExtension)=='.')
		{
			pFileExtension++;
			break;
		}
	}

	if       ((((*(pFileExtension+0))|vLowerCaseMask) == 'm')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'p')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == '3')
		&&(((*(pFileExtension+3))) == 0))
	{
		return PTP_ObjectFormat_MP3;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'w')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'm')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 'a')
		&&(((*(pFileExtension+3))) == 0))
	{
		return MTP_ObjectFormat_WMA;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'o')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'g')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 'g')
		&&(((*(pFileExtension+3))) == 0))
	{
		return MTP_ObjectFormat_OGG;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 't')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'x')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 't')
		&&(((*(pFileExtension+3))) == 0))
	{
		return PTP_ObjectFormat_Text;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'w')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'a')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 'v')
		&&(((*(pFileExtension+3))) == 0))
	{
		return PTP_ObjectFormat_WAV;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'j')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'p')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 'g')
		&&(((*(pFileExtension+3))) == 0))
	{
		return PTP_ObjectFormat_EXIF_JPEG;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'a')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'v')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 'i')
		&&(((*(pFileExtension+3))) == 0))
	{
		return PTP_ObjectFormat_AVI;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'w')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'm')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 'v')
		&&(((*(pFileExtension+3))) == 0))
	{
		return MTP_ObjectFormat_WMV;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'a')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 's')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 'f')
		&&(((*(pFileExtension+3))) == 0))
	{
		return PTP_ObjectFormat_ASF;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'm')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'p')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 'g')
		&&(((*(pFileExtension+3))) == 0))
	{
		return PTP_ObjectFormat_MPEG;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'b')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'm')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 'p')
		&&(((*(pFileExtension+3))) == 0))
	{
		return PTP_ObjectFormat_BMP;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'g')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'i')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 'f')
		&&(((*(pFileExtension+3))) == 0))
	{
		return PTP_ObjectFormat_GIF;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'h')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 't')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 'm')
		&&(((*(pFileExtension+3))) == 0))
	{
		return PTP_ObjectFormat_HTML;
	}
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'x')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'm')
		&&(((*(pFileExtension+2))|vLowerCaseMask) == 'l')
		&&(((*(pFileExtension+3))) == 0))
	{
		return MTP_ObjectFormat_XML;
	}
#if defined(CONFIG_AUDIBLE)
	else if((((*(pFileExtension+0))|vLowerCaseMask) == 'a')
		&&(((*(pFileExtension+1))|vLowerCaseMask) == 'a')
		&&(((*(pFileExtension+2))) == 0))
	{
		return MTP_ObjectFormat_AA;
	}
#endif

	else
	{
		return PTP_ObjectFormat_Undefined;
	}
}

void dbUsingDeleteDatabaseFiles(void)
{
  //이 함수는 database 파일을 지우는 함수이다.
  //전제조건  :  디비파일이 Close 되어 있어야 한다.
	tfs4_unlink(gDictionaryFileName);
  tfs4_unlink(gDataFileName);
  tfs4_unlink(gIndexFileName);
}

void dbUsingInintialDatabaseMemory(void)
{
  //이 함수는 디비가 사용하는 Memory를 초기화 한다.
	memset((char*)DICTIONARYFILE_START_ADDRESS, 0, DICTIONARYFILE_START_ADDRESS_SIZE);
	memset((char*)DATAFILE_START_ADDRESS,       0, DATAFILE_START_ADDRESS_SIZE);
	memset((char*)INDEXFILE_START_ADDRESS,      0, INDEXFILE_START_ADDRESS_SIZE);
	memset((char*)MEMORY_MANAGEMENT_DATAPAGE_ADDRESS,   0, DATAFILE_START_ADDRESS_SIZE/DATAFILE_PAGE_SIZE);
	memset((char*)MEMORY_MANAGEMENT_INDEXPAGE_ADDRESS,  0, INDEXFILE_START_ADDRESS_SIZE/INDEXFILE_PAGE_SIZE);
	gDbStartUpDB = DB_DO_NOT_RUNNING;
}

void dbUsingCreateSchmas(void)
{
  //이 함수는 디비의 스키마들을 생성하는 함수이다.
  #if 0
 	CreateMusicSchma();
	CreateAlbumSchma();
	CreatePhotoSchma();
	CreateReferencesTableSchma();
   #else
 	 CreateMusicSchma();
	CreateAlbumSchma();
	CreatePhotoSchma();
	CreateReferencesTableSchma();
	#if SUPPORT_AVI_FORMAT
	CrateFilesVideoSchma();
	#endif
	CreateFilesObjectsSchma();
	{
		void*  contents[DB_OBJECT_TABLE_COUNT];
		unsigned long rootUid = 0xffffffff;
		unsigned long rootParentUid = 0;
		unsigned char rootProperty = PROPERTY_FOLDER;
		unsigned short rootName[] = {'/','a','/',0};

		memset(contents, 0 , sizeof(contents));
		contents[OBJECT_TABLE_INDEX_UID] 			= &rootUid;
		contents[OBJECT_TABLE_INDEX_PARENTUID]	= &rootParentUid;
		contents[OBJECT_TABLE_INDEX_PROPERTIES]	= &rootProperty;
		contents[OBJECT_TABLE_INDEX_FILETYPE]		= 0;
		contents[OBJECT_TABLE_INDEX_NAME]		= rootName;
		dbApiRecordInsertMS(gDbTableNameObjects, contents, DB_NOT_USING_IN_UPDATE);
	}
   #endif
}



void AppendOneFileName(unsigned short **ppFullPath, unsigned short	*pSource)
{
	while (1)
	{
		**ppFullPath = *pSource;

		if ((*pSource) == 0)
		{
			break;
		}

		(*ppFullPath)++;
		pSource++;
	}
}


void AppendOneDepth(unsigned short **ppFullPath, unsigned short	*pSource)
{
	while (1)
	{
		**ppFullPath = *pSource;

		if ((*pSource) == 0)
		{
			**ppFullPath = '/';
			(*ppFullPath)++;
			**ppFullPath = 0;
			break;
		}

		(*ppFullPath)++;
		pSource++;
	}
}

void EreaseOneDepth(unsigned short **ppFullPath)
{
	(*ppFullPath)--;
	
	while(1)
	{
		if (*((*ppFullPath)-1) == '/')
		{
			**ppFullPath = 0;
			break;
		}
		**ppFullPath = 0;
		(*ppFullPath)--;
	}

}

enum
{
    FILE_TYPE_MP3,
    FILE_TYPE_IRM,
    FILE_TYPE_WMA,
    FILE_TYPE_ASF,
    FILE_TYPE_OGG,
    FILE_TYPE_NUMCODECS
};


unsigned long getFileType(unsigned short *pFileName)
{
	unsigned short *pFileNameStart;
	unsigned short *pEndDot;
	unsigned short *pEndDotNext;

	//1. 파일명에서 확장자의 시작번지를 찾는다.(Begin)
	pFileNameStart	= pFileName;
	pEndDot 			= pFileName;
	
	while(*pFileName)
	{
		if  ((*pFileName)=='.')
		{
			pEndDot = pFileName;
		}
		pFileName++;
	}
	pEndDotNext = pEndDot + 1;
	//1. 파일명에서 확장자의 시작번지를 찾는다.(End)

	
	if (pEndDot == pFileNameStart)
	{
		//파일명에 . 이 없는 경우
		return(FILE_TYPE_NUMCODECS);
	}
	else if (pEndDotNext == pFileNameStart)
	{
		//파일명 끝에 . 이 있는 경우
		return(FILE_TYPE_NUMCODECS);
	}
	else
	{
		if( ((pEndDotNext[0] == 'm') || (pEndDotNext[0] == 'M')) && ((pEndDotNext[1] == 'p') || (pEndDotNext[1] == 'P')) &&  (pEndDotNext[2] == '3') )
		{
			return(FILE_TYPE_MP3);
		}
		else if( ((pEndDotNext[0] == 'w') || (pEndDotNext[0] == 'W')) && ((pEndDotNext[1] == 'm') || (pEndDotNext[1] == 'M')) && ((pEndDotNext[2] == 'a') || (pEndDotNext[2] == 'A')) )
		{
			return(FILE_TYPE_WMA);
		}
		else if( ((pEndDotNext[0] == 'a') || (pEndDotNext[0] == 'A')) && ((pEndDotNext[1] == 's') || (pEndDotNext[1] == 'S')) && ((pEndDotNext[2] == 'f') || (pEndDotNext[2] == 'F')) )
		{
			return(FILE_TYPE_ASF);
		}
		else if( ((pEndDotNext[0] == 'o') || (pEndDotNext[0] == 'O')) && ((pEndDotNext[1] == 'g') || (pEndDotNext[1] == 'G')) && ((pEndDotNext[2] == 'g') || (pEndDotNext[2] == 'G')) )
		{
			return(FILE_TYPE_OGG);
		}
		else
		{
			return(FILE_TYPE_NUMCODECS);
		}	
	}
}


unsigned long GetMusicFilesTAG(unsigned short *usFileFullPathName, RECORDDATA *recData,  unsigned char *pucBuffer, unsigned long ulFileType)
{
	int 			vFileHandle;
	int 			i;
	unsigned long	ulStringLength;
	unsigned long 	lReturn;

    
	vFileHandle = tfs4_open(usFileFullPathName, O_RDONLY);

	if( vFileHandle < 0)
	{
		//File Open Error
		return(0);
	}

	//
	// Insert Tag information : Title, Artist, Album, Genre, TrackNo, Release Year
	//    
	switch(ulFileType)
	{
		case CODEC_MP3:
			if(ID3TagExtract(vFileHandle, recData, pucBuffer) == -1)
			{
				#ifdef FILL_DB_DEBUG
				DbgPrintf("ID3Tag Extraction Error\r\n");
				#endif
				tfs4_close(vFileHandle);
				return(0);
			}

			break;

             
		case CODEC_WMA:
		case CODEC_ASF:
			#ifdef SUPPORT_MTP_DEVICE
			lReturn = WMATagExtract(vFileHandle, recData, pucBuffer, 0);
			#else
			lReturn = WMATagExtract(vFileHandle, recData, pucBuffer);
			#endif

			if(lReturn == -1)
			{
				#ifdef FILL_DB_DEBUG
				DbgPrintf(x)("WMATag Extraction Error\r\n");
				#endif
				tfs4_close(vFileHandle);
				return(0);
			}
			else if(lReturn == 0)
			{
				DB_FillNullData(recData);
			}
			
 			break;


		case CODEC_OGG:

			lReturn = OGGTagExtract(vFileHandle, recData, pucBuffer);
			
			if(lReturn == -1)
			{
				#ifdef FILL_DB_DEBUG
				DbgPrintf("OGGTag Extraction Error\r\n");
				#endif
				tfs4_close(vFileHandle);
				return(0);
			}
			else if(lReturn == 0)
			{
				DB_FillNullData(recData);
			}
             
            		break;
        
		default:
			tfs4_close(vFileHandle);
			return(0);
	}


	tfs4_close(vFileHandle);

	return(1);
}

void DbProcessByFile(unsigned short *pFileFullPath, unsigned short *pFileName, unsigned short *pFileFullPathEnd, unsigned long uId)
{
	//이 함수의 기능은 파일별로 해당 파일에 대한 디비에 삽입을 하는 작업을 수행하는 함수이다.
	//전달인자인 	pFileFullPath에는 파일명을 포함한 file의 fullpath가 들어오고	
	//			pFileFullPathEnd에는 해당 문자열의 끝 주소가 들어오고
	//			uId로는 해당 File의 ClusterID값이 전달된다.

	unsigned long 		vFileType;
	unsigned char		*pFileInfo;
	RECORDDATA 	vRecordData;
	unsigned long		vTagExtrectResult;
	unsigned long		vRowId;
	unsigned short 	vFileFormat;
	
	//file의 확장자를 보고 종류를 알아낸다.
	vFileType = getFileType(pFileName);

	//file의 정보를 받아올 buffer(2000byte)공간을 확보한다.
	pFileInfo = (unsigned char*)malloc(2000);

	// extract TAG
	vTagExtrectResult = GetMusicFilesTAG(pFileFullPath, &vRecordData, pFileInfo, vFileType);

	
       if (vTagExtrectResult ==NUMCODECS)
       {
	   	vFileFormat  =  0;
       }
	else if(vTagExtrectResult == CODEC_MP3)
	{
		vFileFormat = 	REFERENCE_TABLE_FILE_FORMAT_MP3;
	}
	else if(vTagExtrectResult == CODEC_WMA)
	{
		vFileFormat = 	REFERENCE_TABLE_FILE_FORMAT_WMA;
	}
	else if(vTagExtrectResult == CODEC_ASF)
	{
		vFileFormat = 	REFERENCE_TABLE_FILE_FORMAT_ASF;
	}
	else if(vTagExtrectResult == CODEC_OGG)
	{
		vFileFormat = 	REFERENCE_TABLE_FILE_FORMAT_OGG;
	}
	else if(vTagExtrectResult == NUMCODECS)
	{
	   	vFileFormat  =  0;
	}
	else
	{
	   	vFileFormat  =  0;
	}
   
	//file의 정보를 받아올 buffer(2000byte)공간을 제거한다.	
	free(pFileInfo);

	// ADD record
	if(vTagExtrectResult)
	{
		void*	vContents[DB_MUSIC_TABLE_COUNT];
            
		memset(vContents, 0 , sizeof(vContents));

		vContents[MUSIC_TABLE_INDEX_ARTIST] = vRecordData.aArtist;
		vContents[MUSIC_TABLE_INDEX_ALBUM] = vRecordData.aAlbum;
		vContents[MUSIC_TABLE_INDEX_GENRE] = vRecordData.aGenre;
		vContents[MUSIC_TABLE_INDEX_TITLE] = vRecordData.aTitle;
		vContents[MUSIC_TABLE_INDEX_FILEFORMAT] = &vFileFormat;

#if 	SUPPORT_MTP_OBJECT_CHANGED_NAME

		vContents[MUSIC_TABLE_INDEX_UID] = &uId;

#endif		
		vRowId = dbApiRecordInsertMS(gDbTableNameMusic, vContents, DB_NOT_USING_IN_UPDATE);	//Heechul수정한곳

		if (vRowId==0)
		{
			//Error
		}
	}
}

#define IS_DIRECTORY	0x10

extern unsigned long get_codec_number(unsigned short *name);
extern unsigned long GetDBInfoFromTAG(unsigned short *usFileName, RECORDDATA *recData, unsigned char *pucBuffer, unsigned long ulCodecType);

extern unsigned char	pucData[2048];

enum
{
	NONE_RECORDING_SUB_FOLDER,	
	RECORDING_SUB_FOLDER
};
int vRecordingSubFolder;

void RecursiveCallSubDirectory(unsigned short *pFullPath, 				unsigned short	**pFullPathEnd, 
								    unsigned short *pNameBuffer, 
								    unsigned long 	vParentClusterNumber,	unsigned long 		vSelfClusterNumber,
								    int 			vDeleteFile,  			unsigned long 		vCreateDate)
{
	unsigned int	RowID;
	t_DIR 		*pDIR;
	t_dirent		*pEntry;
	t_stat 		stSTAT;	
	unsigned long vRootUid = 0xffffffff;
	//=============================================================//
	//							1. Directory DB에 삽입 (Begin)							//
	//=============================================================//
	void*	vColumnValues[DB_OBJECT_TABLE_COUNT];
	int 		recording_sub = NONE_RECORDING_SUB_FOLDER;
	//여긴 디렉토리에 처음 접근한 경우이다.  이때디렉토리에 대한 삽입을 한다.

	memset(vColumnValues, 0 , sizeof(vColumnValues));

	if ((pNameBuffer[0]=='/')&&(pNameBuffer[1]=='a')&&(pNameBuffer[2]=='/')&&(pNameBuffer[3]==0))
	{
		unsigned long		resultArray[10];
		unsigned short*	indexColumnName[1];
		void*			conditionSet[1];
		unsigned char		refMemory;
		unsigned long		vSearchNumber;
	
		indexColumnName[0]	= gDbColumnNameUid;
		conditionSet[0]		= &vRootUid;
		
		vSearchNumber = dbApiSearchMS(	gDbTableNameObjects, 
										indexColumnName, 1, 
										conditionSet, 1, 
										DB_ASC, 
										resultArray, 
										#ifdef DB_SEARCH_CHANGED
										10,
										#endif
										&refMemory);

		if (vSearchNumber==0)
		{

			void*  contents[DB_OBJECT_TABLE_COUNT];

			unsigned long rootParentUid = 0;
			unsigned char rootProperty = PROPERTY_FOLDER;
			unsigned short rootName[] = {'/','a','/',0};
			unsigned short rooltAliasName[] = {'a',0};

			memset(contents, 0 , sizeof(contents));
			contents[OBJECT_TABLE_INDEX_UID] 			= &vRootUid;
			contents[OBJECT_TABLE_INDEX_PARENTUID]	= &rootParentUid;
			contents[OBJECT_TABLE_INDEX_PROPERTIES]	= &rootProperty;
			contents[OBJECT_TABLE_INDEX_FILETYPE]		= 0;
			contents[OBJECT_TABLE_INDEX_NAME]		= rootName;
			contents[OBJECT_TABLE_INDEX_ALIAS]			= rooltAliasName;			
			contents[OBJECT_TABLE_INDEX_FILESIZE]			= 0;
			contents[OBJECT_TABLE_INDEX_DATECREATED]	= 0;

			
			dbApiRecordInsertMS(gDbTableNameObjects, contents, DB_NOT_USING_IN_UPDATE);
		}
		else if (vSearchNumber==1)
		{
			;
		}
		else
		{
			;//이경우에는 시스템이 오동작을 할 것이다.
		}

	}
	else
	{
#ifndef ERASE_ALIAS_NAME
#ifndef CHANGE_FULLPATH_260CHAR
		unsigned short vAliasBuf[256];
#else
		unsigned short vAliasBuf[MAX_UNICODE_STRING_LENGTH];
#endif
		unsigned short *pAliasBufferDetector;
#endif	/*#ifndef ERASE_ALIAS_NAME*/
		vColumnValues[OBJECT_TABLE_INDEX_UID]			= &vSelfClusterNumber;

		if (vParentClusterNumber==0)
		{
			vColumnValues[OBJECT_TABLE_INDEX_PARENTUID]	= &vRootUid;
		}
		else
		{
			vColumnValues[OBJECT_TABLE_INDEX_PARENTUID]	= &vParentClusterNumber;
		}
		
		vColumnValues[OBJECT_TABLE_INDEX_PROPERTIES]	= 0;
		vColumnValues[OBJECT_TABLE_INDEX_FILETYPE]		= 0;
		vColumnValues[OBJECT_TABLE_INDEX_NAME]			= pNameBuffer;


#ifndef ERASE_ALIAS_NAME

#if !defined(BUG_FIX_STRCPY_TO_WCSCPY)
		TFS4_strcpy(vAliasBuf, pNameBuffer);	//060409challen.jo - 확장자 짤리는 문제 수정
#else
		tfs4_wcscpy(vAliasBuf, pNameBuffer);	//060409challen.jo - 확장자 짤리는 문제 수정
#endif

		pAliasBufferDetector = vAliasBuf;

		while(*pAliasBufferDetector)
		{
			pAliasBufferDetector++;
		}
		*(pAliasBufferDetector - 1) = 0;
		
		vColumnValues[OBJECT_TABLE_INDEX_ALIAS]			= vAliasBuf;
#endif	/*#ifndef ERASE_ALIAS_NAME*/		
		vColumnValues[OBJECT_TABLE_INDEX_DATECREATED]	= &vCreateDate;		
		

		if (vDeleteFile==DB_FILE_ERROR)
		{
			RowID = dbApiRecordInsertMS(gDbTableNameObjects, vColumnValues, DB_NOT_USING_IN_UPDATE);
		}
		else
		{
			unsigned long		resultArray[10];
			unsigned short*	indexColumnName[1];
			void*			conditionSet[1];
			unsigned char		refMemory;
			unsigned long		vSearchNumber;
			
			indexColumnName[0]	= gDbColumnNameUid;
			conditionSet[0]		= vColumnValues[OBJECT_TABLE_INDEX_UID];
			
			vSearchNumber = dbApiSearchMS(	gDbTableNameObjects, 
											indexColumnName, 1, 
											conditionSet, 1, 
											DB_ASC, 
											resultArray, 
											#ifdef DB_SEARCH_CHANGED
											10,
											#endif
											&refMemory);

			if (vSearchNumber==0)
			{
				RowID = dbApiRecordInsertMS(gDbTableNameObjects, vColumnValues, DB_NOT_USING_IN_UPDATE);
			}
			else if (vSearchNumber==1)
			{
				;
			}
			else
			{
				;//이경우에는 시스템이 오동작을 할 것이다.
			}
		}

		if (RowID == 0)						//error처리 부분
		{									//error처리 부분
			//DbgPrintf(("\nDB Insert Error\n"));	//error처리 부분
		}									//error처리 부분

		
		{
			DB_VARCHAR			vLowerCaseMask = 0x20;
			//Recording 폴더 하위에 대해서는 작업을 하지 않는다.
			if (	    ((pNameBuffer[0]|vLowerCaseMask)=='r')
				&&((pNameBuffer[1]|vLowerCaseMask)=='e')
				&&((pNameBuffer[2]|vLowerCaseMask)=='c')
				&&((pNameBuffer[3]|vLowerCaseMask)=='o')
				&&((pNameBuffer[4]|vLowerCaseMask)=='r')
				&&((pNameBuffer[5]|vLowerCaseMask)=='d')
				&&((pNameBuffer[6]|vLowerCaseMask)=='i')
				&&((pNameBuffer[7]|vLowerCaseMask)=='n')
				&&((pNameBuffer[8]|vLowerCaseMask)=='g')
				&&((pNameBuffer[9]|vLowerCaseMask)=='s')
				&&(pNameBuffer[10]=='/')
				&&(pNameBuffer[11]==0))
			{
				recording_sub 			= RECORDING_SUB_FOLDER;
				vRecordingSubFolder 	= RECORDING_SUB_FOLDER;
			}
		}


	/*
	DbgPrintf(("====================="));
	printTwobyteString(pFullPath);				//debugging용 Code
	printTwobyteString(pNameBuffer);			//debugging용 Code
	DbgPrintf(("====================="));
	*/
		
	}

	//=============================================================//
	//							1.Directory DB에 삽입 (E n d)							//
	//=============================================================//	



	//=============================================================//
	//						2. 현재 Directory하위의 Object처리 (Begin)						//
	//=============================================================//	
	pDIR = tfs4_opendir(pFullPath);
	
	while( 1 )
	{
		pEntry = tfs4_readdir_stat(pDIR,&stSTAT);

		if (!pEntry)
		{
			break;
		}
		//=========================================================//
		//						2.1 하위 Directory처리 (Begin)							//
		//=========================================================//
		if(stSTAT.st_mode & IS_DIRECTORY)
		{
			unsigned short *tempName;
			
			//폴더마다 존재하는 자신디렉토리와 부모 디렉토리를 가르키는 정보는 건너 뛴다. (begin)
			if ( wcscmp(pEntry->d_name, gCurrentDirectory)==0 )	continue;
			if ( wcscmp(pEntry->d_name, gParentDirectory) ==0 )	continue;
			//폴더마다 존재하는 자신디렉토리와 부모 디렉토리를 가르키는 정보는 건너 뛴다. (e n d)

			//하위 Directory에 대한 검색을 실시한다.
			//FullPath와 FullPath의 끝을 증가 시킨다.
			AppendOneDepth(pFullPathEnd, pEntry->d_name);
			
			//현재의 이름을 저장한다.
			tempName = pNameBuffer;
			AppendOneDepth(&tempName, pEntry->d_name);

			//====================재귀 호출을 한다.(BEGIN)====================//
			RecursiveCallSubDirectory(pFullPath, pFullPathEnd, pNameBuffer, vSelfClusterNumber, stSTAT.st_ino, vDeleteFile, stSTAT.st_ctime);
			//====================재귀 호출을 한다.(E N D)====================//
			
			//FullPath와 FullPath의 끝을 감소 시킨다.
			EreaseOneDepth(pFullPathEnd);
		}
		//==========================================================//
		//						2.1 하위 Directory처리 (End)								//
		//==========================================================//

		//==========================================================//
		//						2.2 하위 File 처리 (Begin)								//
		//==========================================================//
		else 
		{
			
			//==========================================================//
			//					2.2.1 Object Table 등록작업 (Begin)							//
			//==========================================================//
			//if (!(stSTAT.st_mode & ATTR_HIDDEN))
			{
#ifndef ERASE_ALIAS_NAME
	#ifndef CHANGE_FULLPATH_260CHAR
				unsigned short vAliasName[256];
	#else
				unsigned short vAliasName[MAX_UNICODE_STRING_LENGTH];
	#endif
#endif /*#ifndef ERASE_ALIAS_NAME*/
				unsigned short vMtpFileType;
				memset(vColumnValues, 0 , sizeof(vColumnValues));

#ifndef ERASE_ALIAS_NAME

#if !defined(BUG_FIX_STRCPY_TO_WCSCPY)
				TFS4_strcpy(vAliasName, 	pEntry->d_name);	//060409challen.jo - 확장자 짤리는 문제 수정 
#else
				TFS4_wcscpy(vAliasName, 	pEntry->d_name);	//060409challen.jo - 확장자 짤리는 문제 수정
#endif

				RemoveFileExtString(vAliasName);
#endif /*#ifndef ERASE_ALIAS_NAME*/
				vMtpFileType = GetMTPFileType(pEntry->d_name);
				//Object Table에 등록하는 작업을 한다.
				vColumnValues[OBJECT_TABLE_INDEX_UID]			= &stSTAT.st_ino;
				vColumnValues[OBJECT_TABLE_INDEX_PARENTUID]		= &vSelfClusterNumber;
				vColumnValues[OBJECT_TABLE_INDEX_PROPERTIES]	= 0;
				vColumnValues[OBJECT_TABLE_INDEX_FILETYPE]		= &vMtpFileType;
				vColumnValues[OBJECT_TABLE_INDEX_NAME]			= pEntry->d_name;
#ifndef ERASE_ALIAS_NAME
				vColumnValues[OBJECT_TABLE_INDEX_ALIAS]			= vAliasName;			
#endif /*#ifndef ERASE_ALIAS_NAME*/
				vColumnValues[OBJECT_TABLE_INDEX_FILESIZE]		= &stSTAT.st_size;
				vColumnValues[OBJECT_TABLE_INDEX_DATECREATED]	= &stSTAT.st_ctime;

				//vColumnValues[OBJECT_TABLE_INDEX_PARENTUID]		= &vParentClusterNumber;

				if (vDeleteFile==DB_FILE_ERROR)
				{
					RowID = dbApiRecordInsertMS(gDbTableNameObjects, vColumnValues, DB_NOT_USING_IN_UPDATE);
				}
				else
				{
					unsigned long		resultArray[10];
					unsigned short*	indexColumnName[1];
					void*			conditionSet[1];
					unsigned char		refMemory;
					unsigned long		vSearchNumber;
					
					indexColumnName[0]	= gDbColumnNameUid;
					conditionSet[0]		= vColumnValues[OBJECT_TABLE_INDEX_UID];
					
					vSearchNumber = dbApiSearchMS(	gDbTableNameObjects, 
													indexColumnName, 1, 
													conditionSet, 1, 
													DB_ASC, 
													resultArray, 
													#ifdef DB_SEARCH_CHANGED
													10,
													#endif
													&refMemory);

					if (vSearchNumber==0)
					{
						RowID = dbApiRecordInsertMS(gDbTableNameObjects, vColumnValues, DB_NOT_USING_IN_UPDATE);
					}
					else if (vSearchNumber==1)
					{
						;
					}
					else
					{
						;//이경우에는 시스템이 오동작을 할 것이다.
					}
				}
			

			
				if (RowID == 0)						//error처리 부분
				{									//error처리 부분
					DbgPrintf(("\nDB Insert Error\n"));	//error처리 부분
				}									//error처리 부분
				//==========================================================//
				//					2.2.1 Object Table 등록작업 ( E n d )							//
				//==========================================================//

				//==========================================================//
				//					2.2.2 File 별로 디비에 추가하는 작업 (Begin)						//
				//==========================================================//
				
				AppendOneFileName(pFullPathEnd, pEntry->d_name);	//해당 Object에 FileName을 포함한 FullPath를 만든다.

				//파일의 종류를 검사하고 이를 디비에 추가한다.
				/*
				DbgPrintf(("====================="));
				printTwobyteString(pFullPath);		//debugging용 Code
				printTwobyteString(pEntry->d_name);	//debugging용 Code
				DbgPrintf(("====================="));
				*/
				//if (!(stSTAT.st_mode & ATTR_HIDDEN))
				{

					if (vDeleteFile==DB_FILE_ERROR)
					{
						//file의 tag정보를 받아온다.
						void*			vMusicColums[DB_MUSIC_TABLE_COUNT];
						unsigned long 		ulCodecNumber, ulResult;
						RECORDDATA 	recData;
						unsigned char		*pucBuffer;

						memset(&recData, 0, sizeof(RECORDDATA));

						pucBuffer = pucData;

						ulCodecNumber = get_codec_number(pEntry->d_name);

						// extract TAG
						ulResult = GetDBInfoFromTAG(pFullPath, &recData, pucBuffer, ulCodecNumber);\

						if(ulResult)
						{
							vMusicColums[MUSIC_TABLE_INDEX_ARTIST]	= recData.aArtist;
							vMusicColums[MUSIC_TABLE_INDEX_ALBUM]	= recData.aAlbum;
							vMusicColums[MUSIC_TABLE_INDEX_GENRE]	= recData.aGenre;
							vMusicColums[MUSIC_TABLE_INDEX_TITLE]	= recData.aTitle;
							vMusicColums[MUSIC_TABLE_INDEX_UID]		= &stSTAT.st_ino;

							if (vRecordingSubFolder == NONE_RECORDING_SUB_FOLDER)
							{
								dbApiRecordInsertMS(gDbTableNameMusic, vMusicColums, DB_NOT_USING_IN_UPDATE);	//Heechul수정한곳
							}
						}
					}
					else
					{
						unsigned long		resultArray[10];
						unsigned short*	indexColumnName[1];
						void*			conditionSet[1];
						unsigned char		refMemory;
						unsigned long		vSearchNumber;
						
						indexColumnName[0]	= gDbColumnNameUid;
						conditionSet[0]		= &stSTAT.st_ino;

						vSearchNumber = dbApiSearchMS(	gDbTableNameMusic, 
														indexColumnName, 1, 
														conditionSet, 1, 
														DB_ASC, 
														resultArray, 
														#ifdef DB_SEARCH_CHANGED
														10,
														#endif
														&refMemory);
					

						if (vSearchNumber==0)
						{
							//file의 tag정보를 받아온다.
							void*			vMusicColums[DB_MUSIC_TABLE_COUNT];
							unsigned long 		ulCodecNumber, ulResult;
							RECORDDATA 	recData;
							unsigned char		*pucBuffer;

							memset(&recData, 0, sizeof(RECORDDATA));
							
							pucBuffer = pucData;

							ulCodecNumber = get_codec_number(pEntry->d_name);

							// extract TAG
							ulResult = GetDBInfoFromTAG(pFullPath, &recData, pucBuffer, ulCodecNumber);

							if(ulResult)
							{
								vMusicColums[MUSIC_TABLE_INDEX_ARTIST]	= recData.aArtist;
								vMusicColums[MUSIC_TABLE_INDEX_ALBUM]	= recData.aAlbum;
								vMusicColums[MUSIC_TABLE_INDEX_GENRE]	= recData.aGenre;
								vMusicColums[MUSIC_TABLE_INDEX_TITLE]	= recData.aTitle;
								vMusicColums[MUSIC_TABLE_INDEX_UID]		= &stSTAT.st_ino;

								if (vRecordingSubFolder == NONE_RECORDING_SUB_FOLDER)
								{
									dbApiRecordInsertMS(gDbTableNameMusic, vMusicColums, DB_NOT_USING_IN_UPDATE);	//Heechul수정한곳
								}
							}
						}
						else if (vSearchNumber==1)
						{
							;
						}
						else
						{
							;//이경우에는 시스템이 오동작을 할 것이다.
						}
					}
				}


				EreaseOneDepth(pFullPathEnd);						//해당 Object의 FullPath에서 FileName을 제거한다.
			}
			//==========================================================//
			//					2.2.2 File 별로 디비에 추가하는 작업 ( E n d )						//
			//==========================================================//

		}
		//==========================================================//
		//						2.2 하위 File 처리 (End)								//
		//==========================================================//
	}

	if (recording_sub == RECORDING_SUB_FOLDER)
	{
		vRecordingSubFolder = NONE_RECORDING_SUB_FOLDER;
	}
	

	tfs4_closedir( pDIR );
	//=============================================================//
	//						2. 현재 Directory하위의 Object처리 (E n d)						//
	//=============================================================//	

}


void dbUsingRegeneratingDatabase(void)
{
	int 	vDeleteFile;

	unsigned long vDictionaryLogicalSize = DICTIONARY_LOGICAL_SIZE;
/*
	#ifdef CONFIG_MTP
	unsigned long vDictionaryLogicalSize = 0x00000918;
	#else
	unsigned long vDictionaryLogicalSize = 0x0000093a;
	#endif
*/
	//디비파일을 삭제한다.
#ifdef SUPPORT_AVI_FORMAT
	if ( dbApiUtilCheckDatabaseFiles((char*)gDictionaryFileName, (char*)gDataFileName, (char*)gIndexFileName, 
									vDictionaryLogicalSize,
									DATAFILE_PAGE_SIZE, 
									INDEXFILE_PAGE_SIZE,
									6) ==DB_FILE_ERROR)

#else
	if ( dbApiUtilCheckDatabaseFiles((char*)gDictionaryFileName, (char*)gDataFileName, (char*)gIndexFileName, 
									vDictionaryLogicalSize,
									DATAFILE_PAGE_SIZE, 
									INDEXFILE_PAGE_SIZE,
									5) ==DB_FILE_ERROR)
#endif
	{
		vDeleteFile = DB_FILE_ERROR;
		dbUsingDeleteDatabaseFiles();
	}
	else
	{
		vDeleteFile = DB_FILE_NO_ERROR;
	}

	//디비가 사용하는 메모리를 초기화 한다.
	dbUsingInintialDatabaseMemory();
  

  //디비가 사용하는 전역변수를 초기화 한다.
  dbInitialSetting();


  //Database를 스타트 시킨다.
  	if (vDeleteFile == DB_FILE_ERROR)
  	{
	 	dbApiStartUp((char*)gDictionaryFileName,
		  				 (char*)gDataFileName,
			  			 (char*)gIndexFileName,
				  		 (char*)DICTIONARYFILE_START_ADDRESS,
	  					 (char*)DATAFILE_START_ADDRESS,
	  					 (char*)INDEXFILE_START_ADDRESS,
	  					 (char*)MEMORY_MANAGEMENT_DATAPAGE_ADDRESS,
	  					 (char*)MEMORY_MANAGEMENT_INDEXPAGE_ADDRESS,
	  					 (unsigned long)DICTIONARYFILE_START_ADDRESS_SIZE,
	  					 (unsigned long)DATAFILE_START_ADDRESS_SIZE,
	  					 (unsigned long)INDEXFILE_START_ADDRESS_SIZE,
	  					 (unsigned long)DATAFILE_PAGE_SIZE,
	  					 (unsigned long)INDEXFILE_PAGE_SIZE,
	  					 (unsigned char)DB_SAME_INDIAN, 0);
  	}
	else
	{
	 	dbApiStartUp((char*)gDictionaryFileName,
		  				 (char*)gDataFileName,
			  			 (char*)gIndexFileName,
				  		 (char*)DICTIONARYFILE_START_ADDRESS,
	  					 (char*)DATAFILE_START_ADDRESS,
	  					 (char*)INDEXFILE_START_ADDRESS,
	  					 (char*)MEMORY_MANAGEMENT_DATAPAGE_ADDRESS,
	  					 (char*)MEMORY_MANAGEMENT_INDEXPAGE_ADDRESS,
	  					 (unsigned long)DICTIONARYFILE_START_ADDRESS_SIZE,
	  					 (unsigned long)DATAFILE_START_ADDRESS_SIZE,
	  					 (unsigned long)INDEXFILE_START_ADDRESS_SIZE,
	  					 (unsigned long)DATAFILE_PAGE_SIZE,
	  					 (unsigned long)INDEXFILE_PAGE_SIZE,
	  					 (unsigned char)DB_SAME_INDIAN, vDictionaryLogicalSize);
	}
  
  if (vDeleteFile == DB_FILE_ERROR)
  {
	//디비 스키마를 생성한다.
  	dbUsingCreateSchmas();
  }
  
  //시스템을 뒤지면서 음악화일에 대하여 삽입을 시도한다.
#if (SUPPORT_MTP_OBJECT_CHANGED_NAME==0)
  NEW_AddID3TagToDB();

	{
		unsigned int Formatted = 0xFFFFFFFF, record2[2];
		// For WMPMetadataRoundTrip
		record2[0] = (unsigned int)&Formatted;
		record2[1] = (unsigned int)&Formatted;
		mtpDBInsert(gDbTableNameReferences, (void *)record2);
	}
#else

   //현재의 접근하는 Object의 Full Path를 저장할 임시공간이다.
   gTempFullPath		= (unsigned short*)malloc(514);
   //현재의 접근하는 Object의 이름을 저장할 임시 공간이다.
   gTempCurrentPath	= (unsigned short*)malloc(514);	
   

   *(gTempFullPath+0)	= '/';
   *(gTempFullPath+1)	= 'a';
   *(gTempFullPath+2)	= '/';
   *(gTempFullPath+3)	= 0;
   gTempFullPathEnd	 	= gTempFullPath+3;


   *(gTempCurrentPath+0)	= '/';
   *(gTempCurrentPath+1)	= 'a';
   *(gTempCurrentPath+2)	= '/';
   *(gTempCurrentPath+3)	= 0;
   
	vRecordingSubFolder = NONE_RECORDING_SUB_FOLDER;
   
#ifdef BUG_FIX_REGENERATING_DB
	RecursiveCallSubDirectory(gTempFullPath, &gTempFullPathEnd, gTempCurrentPath, 0, 0xffffffff, vDeleteFile,0);
#else
	RecursiveCallSubDirectory(gTempFullPath, &gTempFullPathEnd, gTempCurrentPath, 0, 0, vDeleteFile,0);
#endif

   free(gTempFullPath);
   free(gTempCurrentPath);

#if SUPPORT_AVI_FORMAT

   #if 0
   DataBaseFileSafeSave();
   #else
   dbApiAllSave();
   #endif
   
#else

	DataBaseFileSafeSave();
	//dbApiAllSave();
	
#endif
   
#endif
}

