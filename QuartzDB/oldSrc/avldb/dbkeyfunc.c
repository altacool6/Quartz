#ifndef _DBKEYFUNC_C		
#define _DBKEYFUNC_C
#include "config.h"
#ifdef CONFIG_AVLDB

/* ******************************************************************************
	File			: DBkeyFunc.c
	Author		: woong & billy (Wibro team)
	Description 	: function using db engine
******************************************************************************* */

#include "Basic_typedefs.h"
#include "api_main.h"
#include "NEW_ifp_local_function.h"
#include "DBKeyFunc.h"
#include "AvlDB.h"
//#include "AvlDB_ExportAPI.h"
#include "db_design.h"
#include "db_design_util.h"
#include "db3_basictype_define.h"
#include "NEW_db_config.h"
#include "FFileSys.h"
#include "safe_string.h"
#include "Uart_api.h"
#include "photo.h"
#include "browser.h"
#include "rtc.h"
#include "misclib.h"
#include "setup.h"
#include <ucos_ii.h>
#include "task.h"
#include "pm.h"

#ifdef CONFIG_FWUI
extern struct DirNode* g_BrowseDirNodeList;
#include "ui_action_music.h"

#include "MTPConfig.h"
#ifdef SUPPORT_MTP_DEVICE

#include "MTPDevice.h"
#include "MTPSpec.h"
#include "MTPStor.h"
#include "SymUSBExtVariables.h"
#endif

#endif		//#ifdef CONFIG_FWUI


//#include "basic_typedefs.h"

#include "stringsys.h"
#include "unicode_menu.h"
//051012



#ifdef U10_DEBUG
#define	DbgPrintf(x)	UART_Print x
#else
#define	DbgPrintf(x)
#endif

#define DB_FULL_DEBUG 0


extern unsigned char SeekData[4];


DB_DISPLAY *db_display = NULL;
uPlp_Info getPlp;

/////////////////////////////////////////////////////////////////////////////////////////////////
//	PARAMETER																		
/////////////////////////////////////////////////////////////////////////////////////////////////
static int curTotalTitleListNum=0;
static int AlarmBackupcurTotalTitleListNum = 0; //angelo - check
int usTotalTitleResultLinkNum; //for biget...
int tmpQuicListTotalNum = 1;
int curTrackTotalTime = 0;
int curRectlyPlayedTime = 0;
int remainRectlyTime = 0;
int rectlyTotalTitleNum = -1;
int favoritTotalTitleNum = -1;
unsigned long *pulPlayPtr = NULL; 
unsigned long pulAlarmBackupPlayPtr[TOTAL_TITLE_NUM];  //angelo - pointer로 사용시 Alarm Music재생시 Irplayer단의 Irque족에서 메모리 침범하여 대체 
unsigned long prevTempPtr=0; 
unsigned int g_artistStataus=0;
unsigned int g_artistAlbumStataus=0;
unsigned int g_prevArtistStataus=0;
unsigned int g_prevArtistAlbumStataus=0;
unsigned int g_albumStataus=0;
unsigned int g_genreStataus=0;
unsigned int g_genreArtistStataus=0;
unsigned int g_genreArtistAlbumStataus=0;
unsigned int g_prevGenreStataus=0;
unsigned int g_prevGenreArtistStataus=0;
unsigned int g_prevGenreArtistAlbumStataus=0;
unsigned int g_myPlaylistStataus=0;
unsigned int g_MyRatingStataus=0;
unsigned int changedIndexNum=0;
unsigned int curPlayingIndexNum=1;
unsigned char doEndDbFunc = 0;
volatile unsigned char ulQuickListNum=0;  //chk
unsigned char curTitlePath=0;
unsigned char curListsPath=0;
unsigned char prevTitlePath=0;
unsigned char curMovingPath=0;
unsigned char startShuffleMode=0;
unsigned char wasInAllDelQuickList=0;
unsigned char reStartPlayback = 1;
unsigned char makeShuffleFirst = 0;
unsigned char doMakeShuffleAgain = 0;
unsigned char didSomthingInQuickList=0;
unsigned char changedMyRatingVal = 0;
unsigned char prevMyRatingPos = 0;
unsigned char prevPlayModeStatus = 1;
unsigned char notFinishedPlay = 0;
unsigned char GoToPrevSong=0;
unsigned char curMusicPlayPos = 100;	 //0: Music Mode, 1: Browser Mode
unsigned char startGenreFirst=0;
unsigned char startArtistFirst=0;
unsigned char mustDoPlayAgain=0;
unsigned short ShuffleList[MAX_FILE_COUNT];
unsigned short randomtbl[MAX_FILE_COUNT];
volatile unsigned long pulDBQuickList[QUICKLIST_MAXNUM];
unsigned char changedTrackMyRatingVal=0;
unsigned char doCpmparePrevPtr= 0;
//051215..chungwook..

DB_VARCHAR* 	complex_index[4];
void*		  	condition_set[4];
unsigned short  condition_set1[MAX_UNICODE_PATH_BYTE];
unsigned short	  condition_set2[MAX_UNICODE_PATH_BYTE];
unsigned short	  condition_set3[MAX_UNICODE_PATH_BYTE];

unsigned long complex_index_column_number;
unsigned long condition_number;


///////////////////////////////////////////////////////////////////
//
//
//						new fwui variables
//
//
///////////////////////////////////////////////////////////////////
#ifdef CONFIG_FWUI
struct FWUI_DbData dbDataStruct;
struct FWUI_MusicAlarmDbData alarmMusicStruct;
struct FWUI_DbData *fwuiDbData = &dbDataStruct;
struct FWUI_MusicAlarmDbData *fwui_AlarmMusic = &alarmMusicStruct;
unsigned short fwuiDbData_playlistNum;
unsigned short fwuiDbData_starNum;
unsigned short checkNextSongFileName = 0;
unsigned char playReturnPos=0;
unsigned char isNextSong=0;
unsigned int gTempPlayingIndexNum;
#endif

///////////////////////////////////////////////////////////////////


//===========희철이가 추가한 부분(begin)================

void SpecialCopyString(unsigned short* pSourceStart, unsigned short* pSourceEnd, unsigned short* pDestnation)
{
	while(1)
	{
		(*pDestnation) = (*pSourceStart);

		if (pSourceEnd == 0)
		{
			if ((*pSourceStart)==0)
			{
				break;
			}
		}
		else
		{
			if(pSourceStart == pSourceEnd)
			{
				*(pDestnation+1) = 0;
				break;
			}
		}

		pSourceStart++;
		pDestnation++;
	}
}

#include "tfs4_api.h"
#include "db_design_insert.h"

enum
{
	CUL_FILE,
	CUL_FOLDER
};

unsigned long GetStartCluster(unsigned short* pPath, unsigned short* pName, unsigned char vFileOrFolder)
{
	
	tDir				sDIR;
	t_dirent 			*pEnt;
	t_stat 			stStat;

	FSOpenDir( &sDIR, 0, pPath);

	while(1)
	{
		pEnt = tfs4_readdir_stat(sDIR.ptDir, &stStat);

		if (pEnt == 0)
		{
			FSCloseDir( &sDIR );
			return 0;
		}

		if (vFileOrFolder == CUL_FILE)
		{
			if(!(stStat.st_mode & 0x10))
			{
				if( wcscmp(pEnt->d_name, pName) == 0 )	//CreateFileDirName에 찾고자 하는 playlist file을 넣는다. 
				{
					FSCloseDir( &sDIR );
					return stStat.st_ino;	//실제 startcluster를 얻어오는 곳이다. 
				}
			}
		}
		else if (vFileOrFolder == CUL_FOLDER)
		{
			if(stStat.st_mode & 0x10)
			{
				if( wcscmp(pEnt->d_name, pName) == 0 )	//CreateFileDirName에 찾고자 하는 playlist file을 넣는다. 
				{
					FSCloseDir( &sDIR );
					return stStat.st_ino;	//실제 startcluster를 얻어오는 곳이다. 
				}
			}			
		}
		else
		{
			return 0;
		}
	}
}
#include "tfs4_types.h"
int GetFileTypeFromDb(unsigned short* pFullPathName, unsigned short *pFileType)
{
	unsigned long vUid;
	unsigned long vRowid;

	//해당 파일의 Uid를 찾는다.
	{
		if (FindUid(pFullPathName, &vUid) == FUNCTION_FAILED)
		{
			return FUNCTION_FAILED;
		}
	}

	//해당 Uid를 가지는 파일의 Object Table상의 Uid를 찾는다.
	{
		vRowid = GetRowidOfObjectsTableByUid(vUid);
		if (vRowid==0)
		{
			return FUNCTION_FAILED;
		}
	}

	//Object Table에서 해당 레코드의 type을 받아온다.
	{
		void* 	aColumnValues[DB_OBJECT_TABLE_COUNT];
		memset(aColumnValues, 0, sizeof(aColumnValues));
		aColumnValues[OBJECT_TABLE_INDEX_FILETYPE]=pFileType;
		
		if (dbApiGetRecordCopy(gDbTableNameObjects, vRowid, aColumnValues)==DB_FUNCTION_FAIL)
		{
			return FUNCTION_FAILED;
		}
	}
	return FUNCTION_SUCCESS;
}
unsigned long GetFileStartCluster(unsigned short* pFullPathName)
{
	int			fp;
	uint32		RowID;
	t_stat  		*pStat;		
	t_int32 		dwRet;
	unsigned long	vStartCluster;
	
	fp = tfs4_open(pFullPathName, O_RDONLY);

	if(fp < 0)
	{
		return(1);
	}

	pStat = (t_stat*)malloc(sizeof(t_stat));

	if (pStat==0) return 0;
	
	dwRet = tfs4_fstat(fp, pStat);

	if( dwRet < 0)
	{
		free(pStat);
		
		return 0;	
	}
	
	vStartCluster = pStat->st_ino;
	
	tfs4_close(fp);
	
	free(pStat);

	return vStartCluster;
}

void FWDeleteDBObjectTable(unsigned short* pFullPath)
{
	void*			conditionSet[1];
	unsigned long		resultArray[10];
	unsigned short* 	indexColumnName[1];
	unsigned char		refMemory;
	unsigned long		vSearchTotal;
	unsigned long  	vUid = GetFileStartCluster(pFullPath);
		
	indexColumnName[0] 	= gDbColumnNameUid;
	conditionSet[0]		= &vUid;

	vSearchTotal = dbApiSearchMS(gDbTableNameObjects, indexColumnName, 1, conditionSet,1, DB_ASC, resultArray, 10, &refMemory);

	if (vSearchTotal == 1)
	{
		unsigned long ret;

		ret = dbApiRecordDeleteMS(gDbTableNameObjects, resultArray[0]);
		
		if (ret == 0)
		{
			DbgPrintf(("\n펌웨어 파일 삭제 실패\n"));
		}
		else
		{
			DbgPrintf(("\n펌웨어 파일 삭제 성공\n"));
		}
		
	}
	else
	{
		DbgPrintf(("\nUid가 올바르지 않습니다. 검색된 갯수는 [%d] Uid는[%d]\n", vSearchTotal, vUid));
	}
	dbApiAllSave();
}

int DeleteDeviceFileToDB(unsigned short* pFilePath, unsigned short* pFileName, unsigned long *pUid)
{
//
//return 값 :	<UMS컴파일시>	-1일 경우에는 디비에서 해당 파일의 찾지 못한 경우이다. 
//							( 음악화일을 삭제하려 하는데 이 리턴값이 -1이면 Error 상황이지만, 
//						  	음악화일이 아닌 화일에 대하여 삭제를 할 경우 Error가 아니다.)
//		  	<MTP/UMS모두>	1 일 경우 함수 수행이 성공적으로 동작한 경우이다.
//					   

	unsigned long 		vUid;
	unsigned long		vTotal;
	unsigned short*	indexColumnName[1];
	void*			conditionSet[1];
	unsigned short	aFullPath[260];
	unsigned long		aRowids[10];
	unsigned char		vRefMemory;

	//aFullPath를 만들어 낸다.
	{
		tfs4_wcscpy(aFullPath, pFilePath);
		tfs4_wcscat(aFullPath,  pFileName);
	}
	
	//File의 Uid를 알아낸다. (mtp 버젼과 UMS버젼의 알아내는 방법이 다르다.)
	{
		#if CONFIG_MTP
		vUid = GetFileStartCluster(aFullPath);
		#else
		if (FindUid(aFullPath, &vUid)==FUNCTION_FAILED)
		{
			return -1;
		}
		#endif
		
		*pUid = vUid;
	}
	
	
	//Object Table에서 삭제를 한다.
	{
		indexColumnName[0]	= gDbColumnNameUid;
		conditionSet[0]		= &vUid;
		
		vTotal = dbApiSearchMS(	gDbTableNameObjects, 
								indexColumnName, 	1, 
								conditionSet, 			1, 
								DB_ASC, 
								aRowids, 				10, 
								&vRefMemory);

		while(vTotal)
		{
			vTotal--;
			dbApiRecordDeleteMS(gDbTableNameObjects, aRowids[vTotal]);
		}
	}
	
	//Music Table에서 삭제를 한다.
	{
		indexColumnName[0]	= gDbColumnNameUid;
		conditionSet[0]		= &vUid;
		
		vTotal = dbApiSearchMS(	gDbTableNameMusic, 
								indexColumnName, 	1, 
								conditionSet, 			1, 
								DB_ASC, 
								aRowids, 				10, 
								&vRefMemory);
		while(vTotal)
		{
			vTotal--;
			dbApiRecordDeleteMS(gDbTableNameMusic, aRowids[vTotal]);
		}
	}
	
	//Album 테이블에서 삭제를 한다.
	{
		indexColumnName[0]	= gDbColumnNameUid;
		conditionSet[0]		= &vUid;
		
		vTotal = dbApiSearchMS(	gDbTableNameAlbum, 
								indexColumnName, 	1, 
								conditionSet, 			1, 
								DB_ASC, 
								aRowids, 				10, 
								&vRefMemory);


		while(vTotal)
		{
			vTotal--;
			dbApiRecordDeleteMS(gDbTableNameAlbum, aRowids[vTotal]);
		}
	}

	//Photo 테이블에서 삭제를 한다.
	{
		indexColumnName[0]	= gDbColumnNameUid;
		conditionSet[0]		= &vUid;
		
		vTotal = dbApiSearchMS(	gDbTableNamePhoto, 
								indexColumnName, 	1, 
								conditionSet, 			1, 
								DB_ASC, 
								aRowids, 				10, 
								&vRefMemory);


		while(vTotal)
		{
			vTotal--;
			dbApiRecordDeleteMS(gDbTableNamePhoto, aRowids[vTotal]);
		}
	}
	
	//video 테이블에서 삭제를 한다.
	{
		indexColumnName[0]	= gDbColumnNameUid;
		conditionSet[0]		= &vUid;
		
		vTotal = dbApiSearchMS(	gDbTableNameVideo, 
								indexColumnName, 	1, 
								conditionSet, 			1, 
								DB_ASC, 
								aRowids, 				10, 
								&vRefMemory);


		while(vTotal)
		{
			vTotal--;
			dbApiRecordDeleteMS(gDbTableNameVideo, aRowids[vTotal]);
		}
	}

	//Refferance Table에서 삭제한다.
	{
		//Child가 Uid인 Record를 삭제한다.
		{
			unsigned long		vRemaminRecords;
			
			indexColumnName[0]	= gDbColumnNameChildCluster;
			conditionSet[0]		= &vUid;
			
			do
			{
				vTotal = dbApiSearchMS(	gDbTableNameReferences, 
										indexColumnName, 	1, 
										conditionSet, 			1, 
										DB_ASC, 
										aRowids, 				10, 
										&vRefMemory);

				
				while(vTotal)
				{
					vTotal--;

					//지워야 한다면 Playlist지우고 Album File 지우는 루틴
					PlayListOrAlbumFileDeleteProcess(aRowids[vTotal]);
					dbApiRecordDeleteMS(gDbTableNameReferences, aRowids[vTotal]);
				}
				
				dbApiSearchMSRemainRecords(&vRemaminRecords);
			}while(vRemaminRecords);
		}
		//Parent가 Uid인 Record를 삭제한다.
		{
			unsigned long		vRemaminRecords;
			
			indexColumnName[0]	= gDbColumnNameParentCluster;
			conditionSet[0]		= &vUid;
			
			do
			{
				vTotal = dbApiSearchMS(	gDbTableNameReferences, 
										indexColumnName, 	1, 
										conditionSet, 			1, 
										DB_ASC, 
										aRowids, 				10, 
										&vRefMemory);


				while(vTotal)
				{
					vTotal--;
					dbApiRecordDeleteMS(gDbTableNameReferences, aRowids[vTotal]);
				}
				
				dbApiSearchMSRemainRecords(&vRemaminRecords);
			}while(vRemaminRecords);
		}
	}

	return 1;
}

void PlayListOrAlbumFileDeleteProcess(unsigned long vDataRowid)
{
	unsigned long		vParentUid;
	unsigned long		vSearchTotal;

	//해당 Rowid를 가지는 레코드의 Parent를 구한다.
	{
		void*	aRefferenceRecordSet[DB_REFERENCES_TABLE_COUNT];
		memset(aRefferenceRecordSet, 0, sizeof(aRefferenceRecordSet));
		vParentUid	= 0;
		aRefferenceRecordSet[REFERENCES_TABLE_INDEX_PARENTCLUSTER] = &vParentUid;

		dbApiGetRecordCopy(gDbTableNameReferences, vDataRowid, aRefferenceRecordSet);
	}

	//Parent가 지금 삭제하려는 레코드 하나만 가지는지를 본다.
	{
		unsigned long		vRowids[2];
		unsigned char		vRefferanceMemory;
		unsigned short*	aIndexColumnName[1];
		void*			aConditionValues[1];
		
		aIndexColumnName[0] 	= gDbColumnNameParentCluster;
		aConditionValues[0]	= &vParentUid;
		vSearchTotal = dbApiSearchMS(	gDbTableNameReferences, 
									aIndexColumnName, 1, 
									aConditionValues, 1, 
									DB_ASC, 
									vRowids, 2, &vRefferanceMemory);
		if (vSearchTotal==1)
		{
			//해당 파일을 지우는 작업을 해야한다.
			//삭제하려는 파일의 Uid는 vParentUid이다.
			//Album Table에 존재하는것을 찾아 지우고,
			//Object Table에서 찾아 지우고,
			//그것을 삭제한 파일 리스트에 추가하면 된다.
			{//Album Table에 존제하는지 찾는다.
				unsigned short*	aIndexCulumnName[1];
				void*			aConditionValue[1];
				unsigned long		aAlbumRecordRowids[2];

				aIndexCulumnName[0]	= gDbColumnNameUid;
				aConditionValue[0]	= &vParentUid;
				vSearchTotal =dbApiSearchMS(	gDbTableNameAlbum, 
											aIndexCulumnName, 	1, 
											aConditionValue, 		1, 
											DB_ASC, 
											aAlbumRecordRowids, 2, &vRefferanceMemory);
				if (vSearchTotal == 1)
				{
					dbApiRecordDeleteMS(gDbTableNameAlbum, aAlbumRecordRowids[0]);
				}
			}
			
			{//Object Table에서 찾아 삭제를 한다.
				unsigned short*	aIndexCulumnName[1];
				void*			aConditionValue[1];
				unsigned long		aObjectRecordRowids[2];

				aIndexCulumnName[0]	= gDbColumnNameUid;
				aConditionValue[0]	= &vParentUid;
				vSearchTotal =dbApiSearchMS(	gDbTableNameObjects, 
											aIndexCulumnName, 	1, 
											aConditionValue, 		1, 
											DB_ASC, 
											aObjectRecordRowids, 2, &vRefferanceMemory);
				if (vSearchTotal == 1)
				{
					unsigned long 	vDeleteFileCreateTime;
					unsigned short 	*pPath;
					unsigned short	*pName;

					pPath	= (unsigned short*)malloc(MAX_UNICODE_PATH_LENGTH);
					pName	= (unsigned short*)malloc(MAX_UNICODE_FILE_NAME_LENGTH);
					if ((pPath == 0) ||(pName == 0))
					{
						free(pPath);
						free(pName);
						return;
					}
					FillObjectPath(vParentUid, pPath, FILE_PATH);
					FillObjectName(vParentUid, pName);
					
					//삭제 파일의 생성시간을 받아온다.
					DeleteDeviceFileCreteTime(&vDeleteFileCreateTime,  pPath, pName);		

					TFS4_wcscat(pPath, pName);
					
					if(tfs4_unlink((t_char *)pPath) == 0)
					{
						dbApiRecordDeleteMS(gDbTableNameObjects, aObjectRecordRowids[0]);
						DeletedFileListAdd(vParentUid, vDeleteFileCreateTime);	//삭제된 파일의 정보를 기록한다.						
					}
					free(pPath);
					free(pName);
				}
			}
		}
	}
}

int RecordingFileDeleteDB(unsigned short* pFilePath, unsigned short* pFileName)
{
	unsigned long		vUid;
	unsigned short*	indexColumnName[1];
	void*			conditionSet[1];
	unsigned long		vRowids[10];
	unsigned long		vTotal;
	unsigned char		refMemory;
	
	vUid = GetStartCluster(pFilePath, pFileName, CUL_FILE);

	indexColumnName[0]	= gDbColumnNameUid;
	conditionSet[0]		= &vUid;
	
	#if 0
	vTotal = dbApiSearchMS(gDbTableNameMusic, indexColumnName, 1, conditionSet, 1, DB_ASC, vRowids, 10, &refMemory);

	if (vTotal==1)
	{
		dbApiRecordDeleteMS(gDbTableNameMusic, vRowids[0]);
	}
	else
	{
		return FUNCTION_FAILED;
	}
	#endif

	vTotal = dbApiSearchMS(gDbTableNameObjects, indexColumnName, 1, conditionSet, 1, DB_ASC, vRowids, 10, &refMemory);

	if (vTotal==1)
	{
		dbApiRecordDeleteMS(gDbTableNameObjects, vRowids[0]);
	}
	else
	{
		return FUNCTION_FAILED;
	}

	return FUNCTION_SUCCESS;
}

void DataBaseFileSafeSave(void)
{
	unsigned char low_batt = 1;


	low_batt = Get_CurBattLevel();
	
	if( (low_batt == 1) && !(pm_operate_get(OP_ADAPTOR))  &&  !isUSBCharging())
	{
		//파일 저장을 하면 않되는 경우
		//파일 시스템이 깨질수 있다.
	}
	else
	{
		//파일을 저장할수 있는 경우
		OSTaskSuspend(UMS_TASK_PRIO);
		
		dbApiAllSave();

		OSTaskResume(UMS_TASK_PRIO);
	}
}


extern unsigned long	gRecordingFileCreationDate;
int RecordingFileInsertDB(unsigned short* pFullPathAndFileName, unsigned long vFileSize)
{
	unsigned short* pFileName;			//파일의 이름이 시작하는 위치를 포인팅할 변수
	unsigned short* pParentFolderName;	//parent 폴더의 이름이 시작하는 위치를 포인팅할 변수
	unsigned short* pTemp;
	unsigned long	   vParentUid;
	unsigned long    vSelfUid;
	unsigned short* pTempBuffer1;
	unsigned short* pTempBuffer2;

	//File Name과 File Path를 찾는다.
	{
		//Full Path의 시작위치를 받는다.
		pTemp = pFullPathAndFileName;

		//초기값을 지정한다.
		pParentFolderName 	= 0;
		pFileName			= 0;
		
		//문자열의 끝까지 간다.
		while(*pTemp)
		{
			if ((*pTemp)=='/')
			{
				pParentFolderName = pFileName;
				pFileName = pTemp + 1;
			}
			pTemp++;
		}

		if (pFileName == 0)
		{
			//뭔가 이상한 FilePath임
			return FUNCTION_FAILED;
		}
	}

	//buffer를 할당 받는다.
	{
		pTempBuffer1 = (unsigned short*)malloc(512);
		if (pTempBuffer1==0)
		{return  FUNCTION_FAILED;}

		pTempBuffer2 = (unsigned short*)malloc(512);
		if (pTempBuffer2==0)
		{free(pTempBuffer1);return FUNCTION_FAILED;}
	}
	
	//Parent Folder의 Start Cluster를 찾는다.
	{
		//Buffer1에 Parent Folder의 Folder를 저장한다.
		SpecialCopyString(pFullPathAndFileName, pParentFolderName-1, pTempBuffer1);	
		//Buffer2에 Parent Folder의 이름을 저장한다.
		SpecialCopyString(pParentFolderName, pFileName-2, pTempBuffer2);
		vParentUid = GetStartCluster(pTempBuffer1, pTempBuffer2, CUL_FOLDER);
	}

	//Self File의 Start Cluster를 찾는다.
	{
		//Buffer1에 Parent Folder의 Folder를 저장한다.
		SpecialCopyString(pFullPathAndFileName, pFileName-1, pTempBuffer1);
		//Buffer2에 Parent Folder의 이름을 저장한다.
		SpecialCopyString(pFileName, 0, pTempBuffer2);
		vSelfUid = GetStartCluster(pTempBuffer1, pTempBuffer2, CUL_FILE);
	}

	//레코딩 파일을 DB의 Object에 등록한다.
	{
		unsigned long vRecordRowid;
		unsigned short vFileType =  REFERENCE_TABLE_FILE_FORMAT_MP3;
#ifndef CHANGE_FULLPATH_260CHAR
		unsigned short vAlias[256];
#else
		unsigned short vAlias[MAX_UNICODE_PATH_LENGTH];
#endif
		//Alias를 만든다.
		{
			int i=0;
			while(1)
			{
				vAlias[i] = *(pTempBuffer2+i);
				if (vAlias[i] == 0) break;
				i++;
			}
			RemoveFileExtString(vAlias);
		}
		
		//Object Table에 등록한다.
		{
			#ifdef MTP_OBJECT_TABLE_EXTENSION
			vRecordRowid = InsertObjectTableRecord(vParentUid, vSelfUid, 0xa0,  vFileType, pTempBuffer2, vAlias, vFileSize, gRecordingFileCreationDate, 0, 0, 0);
			#else
			vRecordRowid = InsertObjectTableRecord(vParentUid, vSelfUid, (unsigned char)0xa0, vFileType, pTempBuffer2);
			#endif
		}

		{
			RemoveFileExtString(pTempBuffer2);
		}
		
		//Music Table에 등록한다.
		#if 0
		{
			unsigned short aArtist[11]={'R','e','c','o','r','d','i','n','g','s',0};
			
			
			if (((*pTempBuffer2) == 'a')||((*pTempBuffer2) == 'A'))
			{
				unsigned short aAlbum[18]={'L','i','n','e',' ','i','n',' ','r','e','c','o','r','d','i','n','g',0};
				vRecordRowid = InsertMusicTableRecord(aArtist, aAlbum, 0, pTempBuffer2, 0, 0, 0, vFileType, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, vSelfUid, 0);

			}
			else if (((*pTempBuffer2) == 'v')||((*pTempBuffer2) == 'V'))
			{
				unsigned short aAlbum[16]={'V','o','i','c','e',' ','r','e','c','o','r','d','i','n','g',0};	
				vRecordRowid = InsertMusicTableRecord(aArtist, aAlbum, 0, pTempBuffer2, 0, 0, 0, vFileType, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, vSelfUid, 0);
			}
			else if (((*pTempBuffer2) == 't')||((*pTempBuffer2) == 'T'))
			{
				unsigned short aAlbum[19]={'F','M',' ','R','a','d','i','o',' ','r','e','c','o','r','d','i','n','g',0};	
				vRecordRowid = InsertMusicTableRecord(aArtist, aAlbum, 0, pTempBuffer2, 0, 0, 0, vFileType, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, vSelfUid, 0);
			}
			else
			{}
		}
		#endif

		//파일을 저장한다.
		DataBaseFileSafeSave();
	}
	
	free(pTempBuffer2);
	free(pTempBuffer1);
	
	return FUNCTION_SUCCESS;
}

/*
unsigned long GetRowidOfEachTableByUid(unsigned short* pTableName, unsigned long vUid)
{
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned long		vRowids[10];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	if (pTableName == gDbTableNameReferences)
	{
		vIndexColumn[0]	=gDbColumnNameChildCluster;
	}
	else
	{
		vIndexColumn[0]	= gDbColumnNameUid;		//인덱스를 지정한다.
	}
	vConditions[0]	= &vUid;
	
	vSearchTotal = dbApiSearchMS(pTableName, 
				 			  	vIndexColumn,	1, 
							      	vConditions,		1,
							      	DB_ASC, 
							      	vRowids, &vRefMemory);
	if (vSearchTotal == 1)
	{
		return vRowids[0];
	}
	else
	{
		return 0;
	}	
}


unsigned long GetRowidOfMusicTableByUid(unsigned long vUid)
{
	unsigned long vReturnValue;

	vReturnValue = GetRowidOfEachTableByUid(gDbTableNameMusic, vUid);
}

unsigned long GetRowidOfAlbumTableByUid(unsigned long vUid)
{
	unsigned long vReturnValue;

	vReturnValue = GetRowidOfEachTableByUid(gDbTableNameAlbum, vUid);
}

unsigned long GetRowidOfPhotoTableByUid(unsigned long vUid)
{
	unsigned long vReturnValue;

	vReturnValue = GetRowidOfEachTableByUid(gDbTableNamePhoto, vUid);
}

unsigned long GetRowidOfRefferenceTableByUid(unsigned long vUid)
{
	unsigned long vReturnValue;

	vReturnValue = GetRowidOfEachTableByUid(gDbTableNameReferences, vUid);
}

unsigned long GetRowidOfObjectsTableByUid(unsigned long vUid)
{
	unsigned long vReturnValue;

	vReturnValue = GetRowidOfEachTableByUid(gDbTableNameObjects, vUid);
}

void DeleteRecordUidInAllTable(unsigned long vUid)
{
	unsigned long vRowid;

	//Music 테이블에 해당 Uid를 가지는 record를 지운다.
	{
		vRowid = GetRowidOfMusicTableByUid(vUid);
		if (vRowid != 0)
		{
			dbApiRecordDeleteMS(gDbTableNameMusic, vRowid);
		}
	}
	//Album 테이블에 해당 Uid를 가지는 record를 지운다.
	{
		vRowid = GetRowidOfAlbumTableByUid(vUid);
		if (vRowid != 0)
		{
			dbApiRecordDeleteMS(gDbTableNameAlbum, vRowid);
		}
	}
	//Photo 테이블에 해당 Uid를 가지는 record를 지운다.
	{
		vRowid = GetRowidOfPhotoTableByUid(vUid);
		if (vRowid != 0)
		{
			dbApiRecordDeleteMS(gDbTableNamePhoto, vRowid);
		}
	}
	//Refference 테이블에 해당 Uid를 가지는 record를 지운다.
	{
		vRowid = GetRowidOfRefferenceTableByUid(vUid);
		if (vRowid != 0)
		{
			dbApiRecordDeleteMS(gDbTableNameReferences, vRowid);
		}
	}
	//Object 테이블에 해당 Uid를 가지는 record를 지운다.
	{
		vRowid = GetRowidOfObjectsTableByUid(vUid);
		if (vRowid != 0)
		{
			dbApiRecordDeleteMS(gDbTableNameObjects, vRowid);
		}
	}
}
*/
int GetPictureFolders(	unsigned long *pSearchArray, 
						#ifdef DB_SEARCH_CHANGED
						unsigned long vSearchArraySize,
						#endif
						unsigned long vIndex, 
						unsigned long *pTotal)
{
	//이 함수를 통하여 array에는 Object table index page의 rowid가 저장된다.
	unsigned short*	vIndexColumn[3];
	void*			vConditions[1];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;
	unsigned short	vFileType;
	
	vIndexColumn[0]	= gDbColumnNameFileType;		//인덱스를 지정한다.
	vIndexColumn[1] 	= gDbColumnNameParentUid;
	vIndexColumn[2] 	= gDbColumnNameProperties;

	vFileType			= OBJECT_TYPE_PHOTO_FILE;
	vConditions[0]	= &vFileType;
	
	vSearchTotal = dbApiSearchMS(gDbTableNameObjects, 
				 			  	vIndexColumn,	2, 
							      	vConditions,	1,
							      	DB_ASC, 
							      	pSearchArray, 
								#ifdef DB_SEARCH_CHANGED
								vSearchArraySize,
								#endif
								&vRefMemory);
	*pTotal = vSearchTotal;

	return vSearchTotal;
}

int GetPictureFoldersName(unsigned long *pSearchArray, unsigned long vIndex, unsigned short *pNameBuffer)
{
	//전달인자로 받은 pSearchArray에는 Object table index page의 rowid가 저장되어 있다.
	unsigned char *pParentUid;
	unsigned char *pTemp;
	unsigned long vParentUid;
	int			vReturnValue;	
	
	pParentUid = (unsigned char*)dbApiAllocKey(*(pSearchArray+vIndex) , 2);
	pTemp = (unsigned char*)(&vParentUid);
	
	*(pTemp+0) = *(pParentUid+0);
	*(pTemp+1) = *(pParentUid+1);
	*(pTemp+2) = *(pParentUid+2);
	*(pTemp+3) = *(pParentUid+3);
	//해당 File의 ParentUid의 값이 vParentUid에 저장된다.

	//해당 Parent의 네임을 받아 온다.
	vReturnValue = FillObjectName(vParentUid, pNameBuffer);

	//전달인자로 받은 pSearchArray에는 Object table index page의 rowid가 여전히 저장되어 있다.
	if (vReturnValue == FUNCTION_FAILED)
	{
		return FUNCTION_FAILED;
	}
	else
	{
		return FUNCTION_SUCCESS;
	}
}

int GetPictureFolderContents(	unsigned long *pSearchArray, 
								#ifdef DB_SEARCH_CHANGED
								unsigned long vSearchArraySize,
								#endif
								unsigned long vIndex, 
								unsigned long *pTotal)
{
	//전달인자로 받은 pSearchArray에는 Object table index page의 rowid가 저장되어 있다.
	//이 함수의 목적은 사진화일 중에 지금 선택한 인덱스의 Parent Uid 밑에 있는  화일을 가져오는 것이다.
	//정렬이 필요하다.
	
	unsigned long vParentUid;
	{
		unsigned char *pParentUid;
		unsigned char *pTemp;
		
		pParentUid = (unsigned char*)dbApiAllocKey(*(pSearchArray+vIndex) , 2);
		pTemp = (unsigned char*)(&vParentUid);
		
		*(pTemp+0) = *(pParentUid+0);
		*(pTemp+1) = *(pParentUid+1);
		*(pTemp+2) = *(pParentUid+2);
		*(pTemp+3) = *(pParentUid+3);
	}

	{
		unsigned short*	vIndexColumn[3];
		void*			vConditions[2];
		unsigned char		vRefMemory;
		unsigned long		vSearchTotal;
		unsigned short	vFileType;
		
		vIndexColumn[0]	= gDbColumnNameFileType;		//인덱스를 지정한다.
		vIndexColumn[1] 	= gDbColumnNameParentUid;
		vIndexColumn[2] 	= gDbColumnNameProperties;

		vFileType			= OBJECT_TYPE_PHOTO_FILE;
		vConditions[0]	= &vFileType;
		vConditions[1]	= &vParentUid;
		
		vSearchTotal = dbApiSearchMS(gDbTableNameObjects, 
					 			  	vIndexColumn,	3, 
								      	vConditions,	2,
								      	DB_ASC, 
								      	pSearchArray, 
								      	#ifdef DB_SEARCH_CHANGED
									vSearchArraySize,
									#endif
								      	&vRefMemory);
		*pTotal = vSearchTotal;
		//array에는 Object Table의 rowid가 저장되어 있다.
	}
	return FUNCTION_SUCCESS;
}

int GetPictureFolderContentName(unsigned long *pSearchArray, unsigned long vIndex, unsigned short *pNameBuffer)
{
	//array에는 Object Table의 rowid가 저장되어 있다.	
	void*		pColumnValue[DB_OBJECT_TABLE_COUNT];
	
	memset(pColumnValue, 0, sizeof(pColumnValue));
	pColumnValue[OBJECT_TABLE_INDEX_NAME] = pNameBuffer;
	
	dbApiGetRecordCopy(gDbTableNameObjects, *(pSearchArray+vIndex), pColumnValue);

	return 1;
}

int GetThumbnailPositionOfUidUseObjectTable(unsigned long *pSearchArray, unsigned long vIndex, unsigned long *pPosition)
{
	//array에는 Object Table의 rowid가 저장되어 있다.
	unsigned long		vUid;
	void*			pColumnValue[DB_OBJECT_TABLE_COUNT];
	int 				vReturnValue;
	
	memset(pColumnValue, 0, sizeof(pColumnValue));
	pColumnValue[OBJECT_TABLE_INDEX_UID] = &vUid;
	
	dbApiGetRecordCopy(gDbTableNameObjects, *(pSearchArray+vIndex), pColumnValue);

	vReturnValue = GetThumbnailPositionOfUid2(vUid, pPosition);

	if (vReturnValue == FUNCTION_FAILED)
	{
		return FUNCTION_FAILED;
	}
	else
	{
		return FUNCTION_SUCCESS;
	}
	
}

int GetThumbnailPositionOfUid2(unsigned long vUid, unsigned long *pPosition)
{
	unsigned short*	vIndexColumnNames[1];
	void*			vConditionSet[1];
	unsigned long		vSearchTotal;
	unsigned char		vRefMemory;
	unsigned long		pSearchArray[10];
	
	vIndexColumnNames[0]	= gDbColumnNameUid;	//인덱스를 지정한다.
	vConditionSet[0] 			= &vUid;					//조건을 지정한다.

	vSearchTotal = dbApiSearchMS(gDbTableNameObjects, 
				 			  	vIndexColumnNames,	1, 
							      	vConditionSet,		1,
							      	DB_ASC, 
							      	pSearchArray,
							      	#ifdef DB_SEARCH_CHANGED
								10,
								#endif
							      	&vRefMemory);
	if (vSearchTotal == 1)
	{
		unsigned long vPostion, vRowid;
		void* 		pColumnValues[DB_PHOTO_TABLE_COUNT];

		memset(pColumnValues, 0, sizeof(pColumnValues));		//stack을 할당받았기 때문에 초기화 한다.
		vPostion = 0;
		pColumnValues[PHOTO_TABLE_INDEX_UID] = &vPostion;		//검색을 원하는 컬럼인 Object Name에 해당 주소를 연결한다.
		
		vRowid = pSearchArray[0];

		//해당 Rowid를 가진 Record의 parentUId를 채워온다.
		dbApiGetRecordCopy(gDbTableNamePhoto, vRowid, pColumnValues);

		*pPosition = vPostion;
		return FUNCTION_SUCCESS;
	}
	else
	{
		return FUNCTION_FAILED;
	}
}


int GetThumbnailPositionOfUid(	unsigned long *pSearchArray, 
									#ifdef DB_SEARCH_CHANGED
									unsigned long vSearchArraySize,
									#endif	
									unsigned long vIndex, 
									unsigned long *pPosition)
{
	unsigned long		vUid;
	unsigned short*	vIndexColumnNames[1];
	void*			vConditionSet[1];
	unsigned long		vSearchTotal;
	unsigned char		vRefMemory;


	vUid = *(pSearchArray + vIndex);
	
	vIndexColumnNames[0]	= gDbColumnNameUid;	//인덱스를 지정한다.
	vConditionSet[0] 			= &vUid;					//조건을 지정한다.

	vSearchTotal = dbApiSearchMS(gDbTableNamePhoto, 
				 			  	vIndexColumnNames,	1, 
							      	vConditionSet,		1,
							      	DB_ASC, 
							      	pSearchArray, 
							      	#ifdef DB_SEARCH_CHANGED
								vSearchArraySize,
								#endif
							      	&vRefMemory);
	if (vSearchTotal == 1)
	{
		unsigned long vPostion, vRowid;
		void* 		pColumnValues[DB_PHOTO_TABLE_COUNT];

		memset(pColumnValues, 0, sizeof(pColumnValues));		//stack을 할당받았기 때문에 초기화 한다.
		vPostion = 0;
		pColumnValues[PHOTO_TABLE_INDEX_UID] = &vPostion;		//검색을 원하는 컬럼인 Object Name에 해당 주소를 연결한다.
		
		vRowid = *(pSearchArray + vIndex);

		//해당 Rowid를 가진 Record의 parentUId를 채워온다.
		dbApiGetRecordCopy(gDbTableNamePhoto, vRowid, pColumnValues);

		*pPosition = vPostion;
		return FUNCTION_SUCCESS;
	}
	else
	{
		return FUNCTION_FAILED;
	}
}

void GetPlayLists(	unsigned long *pSearchArray, 
					#ifdef DB_SEARCH_CHANGED
					unsigned long vSearchArraySize,
					#endif	
					unsigned long *pTatalListNumber, 
					unsigned short vListType)
{
	unsigned short*	vIndexColumnNames[1];
	void*			vConditionSet[1];
	unsigned char		vRefMemory;
	
	vIndexColumnNames[0]	= gDbColumnNameFileType;	//인덱스를 지정한다.
	vConditionSet[0] 			= &vListType;					//조건을 지정한다.
	
	*pTatalListNumber = dbApiSearchMS(gDbTableNameObjects, 
				     				  		vIndexColumnNames, 1, 
								      		vConditionSet, 1,
								      		DB_ASC, 
								      		pSearchArray, 
										#ifdef DB_SEARCH_CHANGED
										vSearchArraySize,
										#endif
								      		&vRefMemory);
}

void GetPlayListsFullPathFileName(unsigned long *pSearchArray, unsigned long vIndex, unsigned short *pFullPathFileNameBuffer)
{
	unsigned long vRowid;
	unsigned long vUid;
	void* 		pColumnValues[DB_OBJECT_TABLE_COUNT];

	vRowid = *(pSearchArray + vIndex);

	memset(pColumnValues, 0, sizeof(pColumnValues));
	pColumnValues[OBJECT_TABLE_INDEX_UID] = &vUid;

	//해당 Rowid를 가진 Record의 UId를 채워온다.
	dbApiGetRecordCopy(gDbTableNameObjects, vRowid, pColumnValues);

	FillObjectPath(vUid, pFullPathFileNameBuffer, FILE_PATH_AND_FILE_NAME);
}




void GetPlayListsFileName(unsigned long *pSearchArray, unsigned long vIndex, unsigned short *pFileNameBuffer)
{
	unsigned long vRowid;
	void* 		pColumnValues[DB_OBJECT_TABLE_COUNT];

	memset(pColumnValues, 0, sizeof(pColumnValues));					//stack을 할당받았기 때문에 초기화 한다.
	pColumnValues[OBJECT_TABLE_INDEX_NAME] = pFileNameBuffer;		//검색을 원하는 컬럼인 Object Name에 해당 주소를 연결한다.
	
	vRowid = *(pSearchArray + vIndex);

	//해당 Rowid를 가진 Record의 parentUId를 채워온다.
	dbApiGetRecordCopy(gDbTableNameObjects, vRowid, pColumnValues);
}


void GetPlayListContentsName(unsigned long *pSearchArray, unsigned long vIndex, unsigned short *pFileNameBuffer)
{
	FillObjectName(*(pSearchArray+vIndex), pFileNameBuffer);
}
enum
{
	PHOTO_VIEW_MODE,
	MUSIC_VIEW_MODE
};

void GetPlayListsFileNameContents(	unsigned long *pSearchArray, 
										#ifdef DB_SEARCH_CHANGED
										unsigned long vSearchArraySize,
										#endif	
										unsigned long vIndex, 
										unsigned long *pTotal, 
										int vMusicOrPhoto)
{
	unsigned long vParentUid;
	unsigned long vTotal;
	{
		unsigned long vUid;
		unsigned long vRowid;
		void* 		pColumnValues[DB_OBJECT_TABLE_COUNT];

		memset(pColumnValues, 0, sizeof(pColumnValues));		//stack을 할당받았기 때문에 초기화 한다.
		pColumnValues[OBJECT_TABLE_INDEX_UID] = &vUid;		//검색을 원하는 컬럼인 Object Name에 해당 주소를 연결한다.

		//Array에는 Objects table의 Rowid가 들어 있다.
		vRowid = *(pSearchArray + vIndex);

		//해당 PlayList의 Uid를 받아온다.
		dbApiGetRecordCopy(gDbTableNameObjects, vRowid, pColumnValues);

		vParentUid = vUid;
	}

	{
		//해당 uid를 parent uid로 가지는 File의 rowid를 찾는다.
		unsigned short*	vIndexColumnNames[1];
		void*			vConditionSet[1];
		unsigned char		vRefMemory;
		
		vIndexColumnNames[0]	= gDbColumnNameParentCluster;	//인덱스를 지정한다.
		vConditionSet[0] 			= &vParentUid;					//조건을 지정한다.

		//해당 playlist소속의 object들을 받아온다.
		vTotal = dbApiSearchMS(gDbTableNameReferences, 
					  		    vIndexColumnNames, 1, 
						 	    vConditionSet, 1,
							    DB_ASC, 
							    pSearchArray, 
							    #ifdef DB_SEARCH_CHANGED
							    vSearchArraySize,
							    #endif
							    &vRefMemory);
	}

	{
		unsigned long 		vLoopCount;
		unsigned long		vArrayIndex;
		unsigned long 		vUid;
		unsigned long 		vRowid;
		unsigned short	vFileFormat;
		void* 			pColumnValues[DB_REFERENCES_TABLE_COUNT];

		*pTotal		= 0;
		vLoopCount	= 0;
		vArrayIndex	= 0;
		
		//array의 내용을 uid로 채운다. 
		while(1)
		{
			memset(pColumnValues, 0, sizeof(pColumnValues));		//stack을 할당받았기 때문에 초기화 한다.
			vUid			= 0;
			vFileFormat	= 0;
			
			pColumnValues[REFERENCES_TABLE_INDEX_CHILDCLUSTER]	= &vUid;		//검색을 원하는 컬럼인 Object Name에 해당 주소를 연결한다.
			pColumnValues[REFERENCES_TABLE_INDEX_FILEFORMAT]	= &vFileFormat;

			//Array에는 Objects table의 Rowid가 들어 있다.
			vRowid = *(pSearchArray + vLoopCount);

			//해당 PlayList의 Uid와 File Format을 받아온다.
			dbApiGetRecordCopy(gDbTableNameReferences, vRowid, pColumnValues);

			if (vMusicOrPhoto == PHOTO_VIEW_MODE)
			{
				if (vFileFormat==REFERENCE_TABLE_FILE_FORMAT_EXIF_JPEG)
				{
					*(pSearchArray + vArrayIndex ) = vUid;
					vArrayIndex++;
				}
				else{}
				
				vLoopCount++;
			}
			else if (vMusicOrPhoto == MUSIC_VIEW_MODE)
			{
				if ((vFileFormat==REFERENCE_TABLE_FILE_FORMAT_MP3)
					||(vFileFormat==REFERENCE_TABLE_FILE_FORMAT_WMA)
					||(vFileFormat==REFERENCE_TABLE_FILE_FORMAT_OGG)
					||(vFileFormat==REFERENCE_TABLE_FILE_FORMAT_ASF))
				{
					*(pSearchArray + vArrayIndex ) = vUid;
					vArrayIndex++;
				}
				else{}

				vLoopCount++;
			}
			else{}

			if (vLoopCount >=vTotal)
			{
				break;
			}
		}
		
		*pTotal = vArrayIndex; 
	}

}

void GetMusicPlayListsFileNameContents(	unsigned long *pSearchArray, 
												#ifdef DB_SEARCH_CHANGED
												unsigned long vSearchArraySize,
												#endif
												unsigned long vIndex, 
												unsigned long *pTotal)
{
	GetPlayListsFileNameContents(	pSearchArray,
									#ifdef DB_SEARCH_CHANGED
									vSearchArraySize,
									#endif
									vIndex, pTotal, MUSIC_VIEW_MODE);
}

void GetPhotoPlayListsFileNameContents(	unsigned long *pSearchArray, 
												#ifdef DB_SEARCH_CHANGED
												unsigned long vSearchArraySize,
												#endif
												unsigned long vIndex, 
												unsigned long *pTotal)
{
	GetPlayListsFileNameContents(	pSearchArray,
									#ifdef DB_SEARCH_CHANGED
									vSearchArraySize,
									#endif
									vIndex, pTotal, PHOTO_VIEW_MODE);
}

void GetPlayListsFileUid(unsigned long *pSearchArray, unsigned long vIndex, unsigned long *pUid)
{
	unsigned long vRowid;
	void* 		pColumnValues[DB_OBJECT_TABLE_COUNT];

	memset(pColumnValues, 0, sizeof(pColumnValues));		//stack을 할당받았기 때문에 초기화 한다.
	pColumnValues[OBJECT_TABLE_INDEX_UID] = pUid;		//검색을 원하는 컬럼인 Uid에 해당 주소를 연결한다.
	
	vRowid = *(pSearchArray + vIndex);

	//해당 Rowid를 가진 Record의 parentUId를 채워온다.
	dbApiGetRecordCopy(gDbTableNameObjects, vRowid, pColumnValues);
}


void GetMusicPlayLists(	unsigned long *pSearchArray, 
							#ifdef DB_SEARCH_CHANGED
							unsigned long vSearchArraySize,
							#endif
							unsigned long *pTatalListNumber)
{
	unsigned long vSearchCount;

	//DbgPrintf(("\n*****************************************************\n"));
	
	//DbgPrintf(("\nGetMusicPlayLists실행\n"));
	GetPlayLists(pSearchArray, 
				#ifdef DB_SEARCH_CHANGED
				vSearchArraySize,
				#endif
				&vSearchCount, 
				OBJECT_TYPE_MUSIC_PLAYLIST);
	(*pTatalListNumber)	= vSearchCount;

	//DbgPrintf(("\nGetMusicPlayLists실행 :: Music Play List는 %d개\n", vSearchCount));
	GetPlayLists(pSearchArray+(*pTatalListNumber),
				#ifdef DB_SEARCH_CHANGED
				vSearchArraySize -(*pTatalListNumber),
				#endif
				&vSearchCount, 
				OBJECT_TYPE_MUSIC_PHOTO_PLAYLIST);
	(*pTatalListNumber)	+= vSearchCount;
	
	//DbgPrintf(("\nGetMusicPlayLists실행 :: Music + Photo Play List는 %d개\n", vSearchCount));	
	vSearchCount 		= (*pTatalListNumber);
	dbApiResultArraySorting(gDbTableNameObjects, 
							gDbColumnNameObjectName, 
							DB_ASC,
							pSearchArray, 
							*pTatalListNumber);
	//DbgPrintf(("\n*****************************************************\n"));
}



void GetPhotoPlayLists(	unsigned long *pSearchArray,
							#ifdef DB_SEARCH_CHANGED
							unsigned long vSearchArraySize,
							#endif
							unsigned long *pTatalListNumber)
{
	unsigned long vSearchCount = 0;

	GetPlayLists(pSearchArray, 
				#ifdef DB_SEARCH_CHANGED
				vSearchArraySize,
				#endif
				&vSearchCount, 
				OBJECT_TYPE_PHOTO_PLAYLIST);
	(*pTatalListNumber)	= vSearchCount;
	
	GetPlayLists(pSearchArray+(*pTatalListNumber), 
				#ifdef DB_SEARCH_CHANGED
				vSearchArraySize -(*pTatalListNumber),
				#endif
				&vSearchCount, 
				OBJECT_TYPE_MUSIC_PHOTO_PLAYLIST);
	(*pTatalListNumber)	+= vSearchCount;
	
	vSearchCount 		= (*pTatalListNumber);
	dbApiResultArraySorting(gDbTableNameObjects, 
							gDbColumnNameObjectName, 
							DB_ASC,
							pSearchArray, 
							*pTatalListNumber);
}





int CompareUnicodeString(unsigned short* targetBuffer, unsigned short* fileFullPathName)
{
	while(1)
	{
		if (((*targetBuffer)==0) && ((*fileFullPathName)==0))
		{
			//두 문자열이 같은 경우
			return 1;
		}
		else
		{
			if ((*targetBuffer)==(*fileFullPathName))
			{
				targetBuffer++;
				fileFullPathName++;
			}
			else
			{
				//두 문자열이 다른경恙?
				return 0;
			}
		
		}
	}
}


//충욱선배 관련 Rating 검색 middleware 함수
int FindRating(	int ratingValue, 
				unsigned long targetBuffer[], 
				#ifdef DB_SEARCH_CHANGED
				unsigned long vSearchArraySize,
				#endif	
				unsigned long *searchTotalNumber)
{
	//rating구해주는 함수 (MS 요구대로 수정)
	/******************/
	/*    0 = unratedn      */
	/*    1  = 1 star  	    */
	/*    2 - 25  = 2 star  */
	/*  26 - 50 = 3 star   */
	/*  51 - 75 = 4 star   */
	/*  76 - 99 = 5 star   */
	/******************/

	//함수설명  :  ratingValue로는 1,2,3,4,5 의 값이 들어 올수 있다.
	//			targetBuffer는 검색된 Rowid들이 저장된다.
	//			searchTotalNumber는 주소값으로 실제 검색된 총 갯수가 이 주소의 위치에 저장이 될것이다.
	
	DB_MUSIC_RATING_DATATYPE	ratingRangValue;
	unsigned short* 	indexColumnNames[1];
	void*			conditionSet[1];
	unsigned long		unitSearchNumber;
	unsigned char		refMemory;
	
	indexColumnNames[0]	= gDbColumnNameRating;
	conditionSet[0] 		= &ratingRangValue;
	*searchTotalNumber = 0;
	
	switch(ratingValue)
	{
		case 0:
			//이 경우 동작하는데 이상이 있는 루틴은 아니지만 이곳으로 들어올 경우는 있어서는 않된다.
			#if 0
			ratingRangValue=0;
			unitSearchNumber = dbApiSearchMS(gDbTableNameMusic, 
 				 					 indexColumnNames, 1, 
			     					 conditionSet, 1,
			      					 DB_ASC, 
			      					 targetBuffer, &refMemory);

			*searchTotalNumber = (*searchTotalNumber) + unitSearchNumber;
			#endif
			return FUNCTION_FAILED;
			break;
		case 1:
#ifdef CONFIG_MTP		//MTP 모델일 경우...  2006 3 26 heechul
			for(ratingRangValue=1; ratingRangValue<=99; ratingRangValue++)
#else					//UMS일 경우 일루...2006 3 26 heechul
			for(ratingRangValue=1; ratingRangValue<6; ratingRangValue++)
#endif
			{
					unitSearchNumber = dbApiSearchMS(gDbTableNameMusic, 
				     				 					 indexColumnNames, 1, 
								     					 conditionSet, 1,
								      					 PExtra_Type->SortMethod+1, 
								      					 targetBuffer+(*searchTotalNumber),
								      					 #ifdef DB_SEARCH_CHANGED
													 vSearchArraySize -(*searchTotalNumber),
													 #endif
								      					 &refMemory);
					
					*searchTotalNumber = (*searchTotalNumber) + unitSearchNumber;
			}
			break;
		case 2:
#ifdef CONFIG_MTP		//MTP 모델일 경우...  2006 3 26 heechul
			for(ratingRangValue=2; ratingRangValue<=99; ratingRangValue++)
#else					//UMS일 경우 일루...2006 3 26 heechul
			for(ratingRangValue=2; ratingRangValue<6; ratingRangValue++)
#endif
			{
					unitSearchNumber = dbApiSearchMS(gDbTableNameMusic, 
				     				 					 indexColumnNames, 1, 
								     					 conditionSet, 1,
								      					 PExtra_Type->SortMethod+1, 
								      					 targetBuffer+(*searchTotalNumber), 
								      					 #ifdef DB_SEARCH_CHANGED
													 vSearchArraySize -(*searchTotalNumber),
													 #endif
													 &refMemory);
					
					*searchTotalNumber = (*searchTotalNumber) + unitSearchNumber;
			}			
			break;
		case 3:
#ifdef CONFIG_MTP		//MTP 모델일 경우...  2006 3 26 heechul
			for(ratingRangValue=26; ratingRangValue<=99; ratingRangValue++)
#else					//UMS일 경우 일루...2006 3 26 heechul
			for(ratingRangValue=3; ratingRangValue<6; ratingRangValue++)
#endif
			{
					unitSearchNumber = dbApiSearchMS(gDbTableNameMusic, 
				     				 					 indexColumnNames, 1, 
								     					 conditionSet, 1,
								      					 PExtra_Type->SortMethod+1, 
								      					 targetBuffer+(*searchTotalNumber), 
								      					 #ifdef DB_SEARCH_CHANGED
													 vSearchArraySize -(*searchTotalNumber),
													 #endif
													 &refMemory);
					
					*searchTotalNumber = (*searchTotalNumber) + unitSearchNumber;
			}
			break;
		case 4:
#ifdef CONFIG_MTP		//MTP 모델일 경우...  2006 3 26 heechul
			for(ratingRangValue=51; ratingRangValue<=99; ratingRangValue++)
#else					//UMS일 경우 일루...2006 3 26 heechul
			for(ratingRangValue=4; ratingRangValue<6; ratingRangValue++)
#endif
			{
					unitSearchNumber = dbApiSearchMS(gDbTableNameMusic, 
				     				 					 indexColumnNames, 1, 
								     					 conditionSet, 1,
								      					 PExtra_Type->SortMethod+1, 
								      					 targetBuffer+(*searchTotalNumber), 
								      					 #ifdef DB_SEARCH_CHANGED
													 vSearchArraySize -(*searchTotalNumber),
													 #endif
								      					 &refMemory);
					
					*searchTotalNumber = (*searchTotalNumber) + unitSearchNumber;
			}

			break;
		case 5:
#ifdef CONFIG_MTP		//MTP 모델일 경우...  2006 3 26 heechul
			for(ratingRangValue=76; ratingRangValue<=99; ratingRangValue++)
#else					//UMS일 경우 일루...2006 3 26 heechul
			for(ratingRangValue=5; ratingRangValue<6; ratingRangValue++)
#endif

			{
					unitSearchNumber = dbApiSearchMS(gDbTableNameMusic, 
				     				 					 indexColumnNames, 1, 
								     					 conditionSet, 1,
								      					 PExtra_Type->SortMethod+1, 
								      					 targetBuffer+(*searchTotalNumber), 
								      					 #ifdef DB_SEARCH_CHANGED
													 vSearchArraySize -(*searchTotalNumber),
													 #endif
								      					 &refMemory);
					
					*searchTotalNumber = (*searchTotalNumber) + unitSearchNumber;
			}
			break;
		default:
			//여기로 들어올 경우에는 인자값으로 받은 ratingValue의 값이 0,1,2,3,4,5사이의 값이 아니다..
			return FUNCTION_FAILED;
	}

	return FUNCTION_SUCCESS;
}


int DBSearchFullPathGetOneItem(unsigned short* pTableName, unsigned short* pFileFullPath, void* pTaretAddress, unsigned long vColumnIndexNumber)
{
	unsigned long 		uId;					//해당 파일의 Uid를 얻어올 변수
	unsigned long 		vSearchNumber;		//검색된 갯수를 저장할 변수
	unsigned char		refMemory;
	unsigned short*	indexColumnNames[1];
	void*			conditionSet[1];
	unsigned long		rowIdsArray[10];
	
	//해당 파일Path을 가지는 uId를 받아온다.
	if (FindUid(pFileFullPath, &uId) == FUNCTION_FAILED)
	{
		return FUNCTION_FAILED;		
	}
	//위의 if문을 통과했다면 uid를 찾은 것이다. 


	//해당 uid를 가지는 record의 rowid를 찾는다.
	indexColumnNames[0]	= gDbColumnNameUid;
	conditionSet[0]		= &uId;
	
	vSearchNumber = dbApiSearchMS(pTableName, 
     				 				indexColumnNames, 1, 
				     				conditionSet, 1,
				      				DB_ASC, 
				      				rowIdsArray, 
				      				#ifdef DB_SEARCH_CHANGED
								10,
								#endif
				      				&refMemory);
	
	if (vSearchNumber != 1)
	{
		return FUNCTION_FAILED;		
	}
	else
	{
		if (pTableName == gDbTableNameMusic)
		{
			void * columnsValue[DB_MUSIC_TABLE_COUNT];
			
			memset(columnsValue, 0, sizeof(columnsValue));		//Stack으로 할당받은 공간이므로 초기화를 해준다.

			columnsValue[vColumnIndexNumber] = pTaretAddress;	//값을 받고자 하는 컬럼번호로 Data가 기록될 Address를 Mapping을 한다.
			
			//해당 uid를 가지는 레코의 rowid를 찾는다.
			if(dbApiGetRecordCopy(gDbTableNameMusic, rowIdsArray[0],  columnsValue)==DB_FUNCTION_FAIL)
			{
				return FUNCTION_FAILED;
			}
		}
		else if (pTableName == gDbTableNamePhoto)
		{
			void * columnsValue[DB_PHOTO_TABLE_COUNT];
			
			memset(columnsValue, 0, sizeof(columnsValue));		//Stack으로 할당받은 공간이므로 초기화를 해준다.

			columnsValue[vColumnIndexNumber] = pTaretAddress;	//값을 받고자 하는 컬럼번호로 Data가 기록될 Address를 Mapping을 한다.
			
			//해당 uid를 가지는 레코의 rowid를 찾는다.
			if(dbApiGetRecordCopy(gDbTableNamePhoto, rowIdsArray[0],  columnsValue)==DB_FUNCTION_FAIL)
			{
				return FUNCTION_FAILED;
			}
		}
		else
		{
			return FUNCTION_FAILED;		
		}
	}
	return FUNCTION_SUCCESS;
}


//김도연 과장님 관련 함수
/*
int MusicFullPathGetArtist(unsigned short* pFileFullPath, unsigned short *pArtist)
{
	//리턴값은 FUNCTION_SUCCESS , FUNCTION_FAILED
	return DBSearchFullPathGetOneItem(	gDbTableNameMusic,
										pFileFullPath, 
										pArtist, 
										MUSIC_TABLE_INDEX_ARTIST);
}


int MusicFullPathGetAlbum(unsigned short* pFileFullPath, unsigned short *pAlbum)
{
	//리턴값은 FUNCTION_SUCCESS , FUNCTION_FAILED
	return DBSearchFullPathGetOneItem(	gDbTableNameMusic,
										pFileFullPath, 
										pAlbum, 
										MUSIC_TABLE_INDEX_ALBUM);
}

int MusicFullPathGetTitle(unsigned short* pFileFullPath, unsigned short *pTitle)
{
	//리턴값은 FUNCTION_SUCCESS , FUNCTION_FAILED
	return DBSearchFullPathGetOneItem(	gDbTableNameMusic,
										pFileFullPath, 
										pTitle, 
										MUSIC_TABLE_INDEX_TITLE);
}

int MusicFullPathGetGenre(unsigned short* pFileFullPath, unsigned short *pGenre)
{
	//리턴값은 FUNCTION_SUCCESS , FUNCTION_FAILED
	return DBSearchFullPathGetOneItem(	gDbTableNameMusic,
										pFileFullPath, 
										pGenre, 
										MUSIC_TABLE_INDEX_GENRE);
}

int MusicFullPathGetAlbumPos(unsigned short* pFileFullPath, DB_MUSIC_ALBUMARTPOS_DATATYPE *pAlbumArtPos)
{
	//리턴값은 FUNCTION_SUCCESS , FUNCTION_FAILED
	return DBSearchFullPathGetOneItem(	gDbTableNameMusic,
										pFileFullPath, 
										pAlbumArtPos, 
										MUSIC_TABLE_INDEX_ALBUMARTPOS);
}

int MusicFullPathGetRating(unsigned short* pFileFullPath, DB_MUSIC_RATING_DATATYPE *pRating)
{
	//리턴값은 FUNCTION_SUCCESS , FUNCTION_FAILED
	return DBSearchFullPathGetOneItem(	gDbTableNameMusic,
										pFileFullPath, 
										pRating, 
										MUSIC_TABLE_INDEX_RATING);
}

int MusicFullPathGetAlbumYear(unsigned short* pFileFullPath, unsigned short *pTitle)
{
	// MUSIC_TABLE_INDEX_ORGRELEASEDATE
	return FUNCTION_FAILED;
	return FUNCTION_SUCCESS;
}

int MusicGetTotalRecord(unsigned long *pTotalNum)
{
	if (dbApiGetTotalRecordNum(gDbTableNameMusic, pTotalNum) == DB_FUNCTION_FAIL)
	{
		return FUNCTION_FAILED;
	}
	return FUNCTION_SUCCESS;
}
*/
int PhotoGetTotalRecord(unsigned long *pTotalNum)
{
	if (dbApiGetTotalRecordNum(gDbTableNamePhoto, pTotalNum) == DB_FUNCTION_FAIL)
	{
		return FUNCTION_FAILED;
	}
	return FUNCTION_SUCCESS;
}


int PhotoFullPathGetThumbnailPosition(unsigned short* pFileFullPath, unsigned short *pThumbnailPosition)
{
	//리턴값은 FUNCTION_SUCCESS , FUNCTION_FAILED
	return DBSearchFullPathGetOneItem(	gDbTableNamePhoto,
										pFileFullPath, 
										pThumbnailPosition, 
										PHOTO_TABLE_INDEX_POSITION);
}

//===========희철이가 추가한 부분( e n d )================

void Save_Memory_Quicklist(void)
{
	int i;
	
	for(i=0; i<200; i++)
	{
		PQuicklists_Type->pQuickLists[i] = pulDBQuickList[i];		
	}
	PQuicklists_Type->quickListNum = ulQuickListNum;	
}


void Check_Valid_QuickList(void)
{
	
//	unsigned long ulReturnDBDataCur;
	int i, count1=0;
	unsigned long ulTempSavePtr[200];
/*  실제 DB 존재 여부 파악  */  
	unsigned long ulTempPtr; 
	unsigned long vUId;
	int 	chkQuicklistDB = 0;

	void*	columns_value[DB_MUSIC_TABLE_COUNT];
	memset(columns_value, NULL, sizeof(void *)*DB_MUSIC_TABLE_COUNT);
	
	if(PQuicklists_Type->quickListNum == 0)
	{		
		ulQuickListNum = PQuicklists_Type->quickListNum;
		return;
	}
	else
	{
		for(i=0; i<PQuicklists_Type->quickListNum; i++)
		{
/*  실제 DB 존재 여부 파악  */			
			ulTempPtr = PQuicklists_Type->pQuickLists[i];

			//uid를 받아온다.
			columns_value[MUSIC_TABLE_INDEX_UID] = &vUId;
			chkQuicklistDB = dbApiGetRecordCopy(gDbTableNameMusic,
				   	   	ulTempPtr,
				   	   	columns_value);		
		
			if(chkQuicklistDB == 1 && PQuicklists_Type->pQuickLists[i] != 0)
			{
				ulTempSavePtr[count1] = PQuicklists_Type->pQuickLists[i];
				count1++;	
			}
		}

		ulQuickListNum = count1;
		
		for(i=0; i<ulQuickListNum; i++)
		{	
			pulDBQuickList[i] = ulTempSavePtr[i];			
		}			
	}
					
}  



void Check_Valid_QuickList_Entry(void)
{
	
//	unsigned long ulReturnDBDataCur;
	int i, count1=0;
	unsigned long ulTempSavePtr[200];
/*  실제 DB 존재 여부 파악  */  
	unsigned long ulTempPtr; 
	unsigned long vUId;
	int 	chkQuicklistDB = 0;

	void*	columns_value[DB_MUSIC_TABLE_COUNT];
	
	memset(columns_value, NULL, sizeof(void *)*DB_MUSIC_TABLE_COUNT);
	
	for(i=0; i<ulQuickListNum; i++)
	{
/*  실제 DB 존재 여부 파악  */			
		ulTempPtr = pulDBQuickList[i];

		//uid를 받아온다.
		columns_value[MUSIC_TABLE_INDEX_UID] = &vUId;
		chkQuicklistDB = dbApiGetRecordCopy(gDbTableNameMusic,
			   	   	ulTempPtr,
			   	   	columns_value);		
	
		if(chkQuicklistDB == 1 && ulTempPtr != 0)
		{
			ulTempSavePtr[count1] = pulDBQuickList[i];
			count1++;	
		}
	}

	ulQuickListNum = count1;
	
	for(i=0; i<ulQuickListNum; i++)
	{	
		pulDBQuickList[i] = ulTempSavePtr[i];			
	}			
}  

void Read_Memory_Quicklist(void)
{
	Check_Valid_QuickList();
}

/* ============================================================================
	Function name : int db_check_db_file_exist (void)
	Description      : 
	Parameter    	 : 
============================================================================ */
long dbDataFileLen = 0;
long dbIdxFileLen = 0;

int db_check_db_file_exist (void)
{
	
//
//chungwook..051219..아이리버 플러스2와 연동되면 필요한 부분
//	
	
/*
	t_int32 dwFD;
	
	doEndDbFunc = 0;
	dwFD = tfs4_open(usDBFullIdxFileName, O_RDONLY);
	if (dwFD >= 0)
	{
		dbIdxFileLen = tfs4_lseek(dwFD, 0, SEEK_END);
		tfs4_lseek(dwFD, 0, SEEK_SET);
		tfs4_close(dwFD);
		
		dwFD = tfs4_open(usDBFullDataFileName, O_RDONLY);
		if(dwFD >= 0)			
		{
			dbDataFileLen = tfs4_lseek(dwFD, 0, SEEK_END);
			tfs4_lseek(dwFD, 0, SEEK_SET);
			tfs4_close(dwFD);		
			return 1; //All DB files are exist. Ok!
		}
		else
		{
			DbgPrintf(("\n tfs4_open2 error  - in main\n"));	
			return 2;
		}
	}
	else
	{
		DbgPrintf(("\n tfs4_open1 error  - in main\n"));	
		return 2;
	}
*/
	//051013
	return 1; //All DB files are exist. Ok!
}

/* ============================================================================
	Function name : int db_check_generate_db (void)
	Description      : 
	Parameter    	 : 
============================================================================ */
#ifdef SUPPORT_MTP_DEVICE
extern char g_DBGenNoAct; //DB Genernerate 실행 
extern char g_LocalFunc_initF;
extern char g_DBCheckGenerateDbF;
#endif

#include "..\..\usb\mtpdevice\ifpfunc\mtp_interface_db.h"

#ifdef ALARM_MUSIC_NEW_LIST
int check_generate_alarm_music_db(void)
{
	if( pulAlarmBackupPlayPtr == NULL )
	{
#if 0	
		pulAlarmBackupPlayPtr = (unsigned long *)(malloc(0xFA0));
#endif 
	}

	return TRUE;
}

int make_empty_alarm_music_db(void)
{
	int k;
	
	if(AlarmBackupcurTotalTitleListNum)
	{
		if(pulAlarmBackupPlayPtr != NULL)
		{
	
		// 복원
			for(k = 0; k < AlarmBackupcurTotalTitleListNum; k++) 
			{
				pulPlayPtr[k] = pulAlarmBackupPlayPtr[k] ;
			}
			
			curTotalTitleListNum = AlarmBackupcurTotalTitleListNum;
			
		// 초기화 
			AlarmBackupcurTotalTitleListNum = 0;
		}
		else
		{
			AlarmBackupcurTotalTitleListNum = 0;
			curTotalTitleListNum = 0;	
		}
	}
	else
	{
		curTotalTitleListNum = 0;
	}
	return TRUE;
}

#endif


int db_check_generate_db (void)
{	
        Read_Memory_Quicklist();

	if( pulPlayPtr == NULL )
	{
		pulPlayPtr = (unsigned long *)(malloc(sizeof(unsigned long)*TOTAL_TITLE_NUM));
	}

	if( db_display == NULL )
	{
		db_display = (DB_DISPLAY *)(malloc(sizeof(DB_DISPLAY)));
	}

	wasInAllDelQuickList = 0;
	startGenreFirst = 0;
	startArtistFirst = 0;	
	mustDoPlayAgain = 0;
	didSomthingInQuickList = 0;
	prevTempPtr = 0;

	return 1;
}


#if 0
int db_check_music_file_exist (char *fullFileName)
{		
	t_int32 dwFD;
	
	dwFD = tfs4_open((const t_char *)fullFileName, O_RDONLY);
	
	if (dwFD >= 0)
	{
		tfs4_close(dwFD);
		return 1;		
	}
	else
	{
		return 2;
	}
}
#else //t_uchar
int db_check_music_file_exist (unsigned short* fullFileName)
{		
	t_int32 dwFD;
	
	dwFD = tfs4_open(fullFileName, O_RDONLY);
	
	if (dwFD >= 0)
	{
		tfs4_close(dwFD);
		return 1;		
	}
	else
	{
		return 2;
	}
}
#endif



unsigned long get_cur_myrating_val(unsigned long ulPtr)
{
//20051128..myrating
	void*	columns_value[DB_MUSIC_TABLE_COUNT];
	unsigned short retValue=0;
	unsigned short usColumnPtr[2];


	memset(columns_value, NULL, sizeof(columns_value));
	memset(usColumnPtr, NULL, sizeof(unsigned short));


	columns_value[MUSIC_TABLE_INDEX_RATING] = usColumnPtr;

	dbApiGetRecordCopy(gDbTableNameMusic,
				   	   ulPtr,
				   	   columns_value);

	retValue = *((unsigned short*)columns_value[MUSIC_TABLE_INDEX_RATING]);


	return (retValue);	

}

short get_myrating_cur_list_num(void)
{
	short retVal = 0;
	int i;

	retVal = 0;

	for(i=0; i<usTotalTitleResultLinkNum; i++)
	{
		if(db_display->pulResultLinkedListBuffer[i]==prevTempPtr)
		{			
			retVal = i+1;
			break;
		}
	}
	return retVal;	
}


int CheckStringSize(unsigned short *path, unsigned short *filename)
{
	int stringLength = 0;

	stringLength = TFS4_wcslen(path);
	stringLength += TFS4_wcslen(filename);

	if( stringLength >= MAX_UNICODE_PATH_LENGTH )
	{
		if(pm_operate_get(OP_PLAY_MODE))
		{
			audio_stop(FALSE);
		}
		
		return -1;
	}

	return 0;
}


/* ============================================================================
	Function name : GetPlaytrackByDBIndex(unsigned short usIndexLocal)
	Description      : 
	Parameter    	 : 
============================================================================ */


int GetPlaytrackByDBIndex(unsigned short usIndexLocal, unsigned char curPlayStatus)
{


//20051122..play하기 위해 루틴 수정 

//	unsigned long ulDataSize = 0;
//	unsigned short usDBPathName[MAX_UNICODE_PATH_LENGTH];
//	unsigned short usTempPathName[MAX_UNICODE_PATH_LENGTH];
//	unsigned short usTempArtistName[MAX_UNICODE_PATH_LENGTH];
//	unsigned short ucTmpBuff[MAX_UNICODE_PATH_LENGTH];
	unsigned short i;
//	unsigned char ucTotalPos;
	unsigned long ulTempPtr; 
	static unsigned char isNotFirst= 0;

	void*	columns_value[DB_MUSIC_TABLE_COUNT];

	#ifndef CHANGE_FULLPATH_260CHAR
	    t_char tempFilePath_n_Name[MAX_FILE_LEN];
	    memset(tempFilePath_n_Name ,0, MAX_FILE_LEN*sizeof(t_char));
	#else
		t_char tempFilePath_n_Name[MAX_UNICODE_PATH_LENGTH];
		memset(tempFilePath_n_Name ,0, MAX_UNICODE_PATH_LENGTH*sizeof(t_char));
	#endif
	
//	struct DirNode* pTempDirNode;	
	
	//useTmpPtr는 dbtask에서 myrating값이 바뀌면 true가 된다. 
	if (prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE && !GoToPrevSong && isNotFirst && useTmpPtr)
	{
		//play하고 있는 곡의 ptr이 display버퍼에 있으면 리턴값이 0이 아니기때문에 
		//else를 실행하고 
		if (get_myrating_cur_list_num() == 0)
			ulTempPtr = prevTempPtr;
		else
			ulTempPtr = pulPlayPtr[usIndexLocal];			
	}	
	else	
	{
		ulTempPtr = pulPlayPtr[usIndexLocal];
	}	

	i = 0;	




#ifdef CONFIG_FWUI	

	memset(columns_value, NULL, sizeof(columns_value));
#if (SUPPORT_MTP_OBJECT_CHANGED_NAME==0)	//2006 2 15 heechul table 구조변화에 따라 동작할수 읻도록 ....
	
	columns_value[MUSIC_TABLE_INDEX_FILEPATH] = g_BrowseDirNodeList[0].FileName;
	columns_value[MUSIC_TABLE_INDEX_FILENAME] = g_BrowseDirNodeList[1].FileName;
	dbApiGetRecordCopy(gDbTableNameMusic,
				   	   ulTempPtr,
				   	   columns_value);
#else	//2006 2 15 heechul table 구조변화에 따라 동작할수 읻도록 ....
{
	unsigned long vUId;

	//uid를 받아온다.
	columns_value[MUSIC_TABLE_INDEX_UID] = &vUId;
	dbApiGetRecordCopy(gDbTableNameMusic,
				   	   	ulTempPtr,
				   	   	columns_value);
	FillObjectPath(vUId, g_BrowseDirNodeList[0].FileName, FILE_PATH);		//file path를 받아온다.
	FillObjectName(vUId, g_BrowseDirNodeList[1].FileName);				//file name을 받아온다.
}
#endif	//2006 2 15 heechul table 구조변화에 따라 동작할수 읻도록 ....

	if( CheckStringSize(g_BrowseDirNodeList[0].FileName, g_BrowseDirNodeList[1].FileName) < 0 )
	{
		send_ui_eventw(pusU10_Msg_NotSupportFile[g_UnicodeMenuIdx]);
		return 0;
	}

	TFS4_wcscpy(tempFilePath_n_Name, g_BrowseDirNodeList[0].FileName);
	TFS4_wcscat(tempFilePath_n_Name, g_BrowseDirNodeList[1].FileName);
	if(pm_operate_get(OP_PLAY_MODE))
	{
		if(tfs4_wcscmp(tempFilePath_n_Name, FilePath_n_Name) != 0)		//핸재 재생되는 곡이랑 재생하려는 곡이랑 같지 않으면 
		{
			#ifdef SUPPORT_MTP_DEVICE
				#if (CONFIG_MTP_PREFETCH   == 1) 
	    			SetFirstLicenseAcq(1);
				#endif
			#endif
			audio_stop(FALSE);
			tfs4_wcscpy(FilePath_n_Name, tempFilePath_n_Name);
			return 1;
		}
		else
		{
			return 2;	//같은 곡이면 
		}
	}
	else
	{
		#ifdef SUPPORT_MTP_DEVICE
			#if defined(CONFIG_MTP_PREFETCH) 
	    			SetFirstLicenseAcq(1);
			#endif
		#endif
		//audio_stop(TRUE);
		memset(FilePath_n_Name, 0x0, sizeof(FilePath_n_Name));
		TFS4_wcscpy(FilePath_n_Name, g_BrowseDirNodeList[0].FileName);
		TFS4_wcscat(FilePath_n_Name, g_BrowseDirNodeList[1].FileName);
	}
#else
{
	void*	columns_value[DB_MUSIC_TABLE_COUNT];
	//memset(columns_value, NULL, sizeof(void *)*DB_MUSIC_TABLE_COUNT);	//heechul
	memset(columns_value, NULL, sizeof(columns_value));	//heechul

//	dbApiGetRecord(gDbTableNameMusic, ulTempPtr, columns_value);
//	db_api_get_record(db_t_music, ulTempPtr ,columns_value);
	

	 
	memset(usDBPathName, 0x0, sizeof(usDBPathName));
	memset(usTempPathName, 0x0, sizeof(usTempPathName));
	memset(usTempArtistName, 0x0, sizeof(usTempArtistName));	
	memset(ucTmpBuff, 0x0, sizeof(ucTmpBuff));
	
	//	path를 읽어와서 filename과 결합해서 path+filename을 넘긴다. 



	if(NEW_GetRealData((unsigned char*)columns_value[MUSIC_TABLE_INDEX_FILEPATH], (unsigned char*)usDBPathName, &ulDataSize))
	{
		ucTotalPos = 0;
		ucTotalPos = ulDataSize/2;
	}
	
	if(ulDataSize > 2 && (usDBPathName[ucTotalPos-1] != 0x002f))
	{
		usDBPathName[ucTotalPos] = 0x002f;
		ucTotalPos++;
	}	


	if(NEW_GetRealData((unsigned char*)columns_value[MUSIC_TABLE_INDEX_FILENAME], (((unsigned char*)usDBPathName)+(ucTotalPos*2)), &ulDataSize))
	{
		ucTotalPos += ulDataSize/2;
		usDBPathName[ucTotalPos] = 0;
	}


	
//	TFS4_mbstowcs(usTempPathName,"/a",3);//string->wide
//	TFS4_wcsncpy(usTempPathName+tfs4_wcslen(usTempPathName), usDBPathName, ucTotalPos+tfs4_wcslen(usTempPathName+1));
	memset(FilePath_n_Name, 0x0, sizeof(FilePath_n_Name));
	TFS4_wcscpy(FilePath_n_Name, usDBPathName);			


//	if (db_check_music_file_exist((char *)FilePath_n_Name) == 2) return (0);

	
	//Artist Field값을 얻어와서 Play시 화면에 Artist명을 나오게한다.

	if(NEW_GetRealData((unsigned char*)columns_value[1], (unsigned char*)usTempArtistName, &ulDataSize))
	{
		memset(usArtistName, 0x0, sizeof(usArtistName));			
		wc2utf8((unsigned char *)usArtistName, (unsigned short *)usTempArtistName);			

		if (prevTitlePath == MY_QUICKLISTS_TITLE) 
		{
			memset(tmpusArtistName, 0x0, sizeof(tmpusArtistName));
			wc2utf8((unsigned char *)tmpusArtistName, (unsigned short *)usTempArtistName);
		}	
	}


	if(NEW_GetRealData((unsigned char*)columns_value[0], (unsigned char*)ucTmpBuff, &ulDataSize))
	{
		memset(tmpCBuff, 0x0, sizeof(tmpCBuff));
		wc2utf8((unsigned char *)tmpCBuff, (unsigned short *)ucTmpBuff);						
	}

}

#endif



#if 1		
	if ((prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE) || prevTitlePath == MY_QUICKLISTS_TITLE)
	{
		curTrackMyRatingVal = get_cur_myrating_val(ulTempPtr);
	}

	isNotFirst = 1;
	prevTempPtr = ulTempPtr;		//20051128..playcount설정시 row_id로 쓸수 있음. 
#endif
	
	return(1);	
	

}

unsigned char db_check_filename_exist (int listnum, unsigned char checkMode)
{
//	unsigned long ulDataSize = 0;
	unsigned short usDBPathName[MAX_UNICODE_PATH_LENGTH];
//	unsigned short i;
//	unsigned char ucTotalPos;
	unsigned long ulTempPtr; 




#ifdef CONFIG_FWUI	
	void*	columns_value[DB_MUSIC_TABLE_COUNT];
	memset(columns_value, NULL, sizeof(columns_value));	//heechul
//	struct DirNode* pTempDirNode;	

// tonny
// Hdd 타입의 경우 Hdd가 꺼져있을경우 켜야하므로 다음 노래 재생시까지 시간지연 발생.
#if defined(CONFIG_HDD)
	return 1;
#endif

	if (listnum < 1)
	{
		listnum = 1;	
	}
	
	if (checkMode)
	{
		ulTempPtr= db_display->pulResultLinkedListBuffer[listnum -1];

	}
	else
	{
		ulTempPtr= pulPlayPtr[listnum-1];
	}


	memset(columns_value, NULL, sizeof(columns_value));

#if (SUPPORT_MTP_OBJECT_CHANGED_NAME==0)	//2006 2 15 heechul table 구조변화에 따라 동작할수 읻도록 ....
	columns_value[MUSIC_TABLE_INDEX_FILEPATH] = g_BrowseDirNodeList[0].FileName;
	columns_value[MUSIC_TABLE_INDEX_FILENAME] = g_BrowseDirNodeList[1].FileName;
	dbApiGetRecordCopy(gDbTableNameMusic,
				   	   ulTempPtr,
				   	   columns_value);
#else	//2006 2 15 heechul table 구조변화에 따라 동작할수 읻도록 ....
{
	unsigned long vUId;

	//uid를 받아온다.
	columns_value[MUSIC_TABLE_INDEX_UID] = &vUId;
	dbApiGetRecordCopy(gDbTableNameMusic,
				   	   	ulTempPtr,
				   	   	columns_value);
	FillObjectPath(vUId, g_BrowseDirNodeList[0].FileName, FILE_PATH);		//file path를 받아온다.
	FillObjectName(vUId, g_BrowseDirNodeList[1].FileName);				//file name을 받아온다.
}
#endif	//2006 2 15 heechul table 구조변화에 따라 동작할수 읻도록 ....

	if( CheckStringSize(g_BrowseDirNodeList[0].FileName, g_BrowseDirNodeList[1].FileName) < 0 )
	{
		return 2;
	}
	
	memset(usDBPathName, 0x0, sizeof(usDBPathName));
	TFS4_wcscpy(usDBPathName, g_BrowseDirNodeList[0].FileName);
	TFS4_wcscat(usDBPathName, g_BrowseDirNodeList[1].FileName);

	return  db_check_music_file_exist(usDBPathName);


#else
	void*	columns_value[DB_MUSIC_TABLE_COUNT];
	memset(columns_value, NULL, sizeof(columns_value));	//heechul
	
	if (listnum < 1)
	{
		listnum = 1;
	}

	if (checkMode)
	{
		{
			ulTempPtr= db_display->pulResultLinkedListBuffer[listnum-1];
		}
	}
	else
	{
		ulTempPtr= pulPlayPtr[listnum-1];
	}

	i = 0;		


//	여기에 맞는 루틴으로 갈아타야한다. 아래의 두 함수는 invalid한 함수들이다. 
//	db_api_get_record(db_t_music, ulTempPtr ,columns_value);
//	dbApiGetRecord(gDbTableNameMusic, ulTempPtr, columns_value);

	 
	memset(usDBPathName, 0x0, sizeof(usDBPathName));


	if(NEW_GetRealData((unsigned char*)columns_value[MUSIC_TABLE_INDEX_FILEPATH], (unsigned char*)usDBPathName, &ulDataSize))
	{
		ucTotalPos = 0;
		ucTotalPos = ulDataSize/2;
	}
	
	if(ulDataSize > 2 && (usDBPathName[ucTotalPos-1] != 0x002f))
	{
		usDBPathName[ucTotalPos] = 0x002f;
		ucTotalPos++;
	}	


	if(NEW_GetRealData((unsigned char*)columns_value[MUSIC_TABLE_INDEX_FILENAME], (((unsigned char*)usDBPathName)+(ucTotalPos*2)), &ulDataSize))
	{
		ucTotalPos += ulDataSize/2;
		usDBPathName[ucTotalPos] = 0;
	}

	
	memset(FilePath_n_Name, 0x0, sizeof(FilePath_n_Name));
	TFS4_wcscpy(FilePath_n_Name, usDBPathName);			

	return db_check_music_file_exist(FilePath_n_Name);

#endif


	
}


void db_update_playlists_val(void)
{	
/*	
	unsigned long ulplaycount = 0;
	unsigned long ulTempTime = 0;
	
	GetRecNumToPlayList((char*)prevTempPtr);

	ulTempTime = GetEpochTime();
	if(SetPlayListValue(LASTPLAYED_KEYWD, ulTempTime))
	{
		ulplaycount = ldlong((char*)prevTempPtr+PLAYCOUNT_KEYWD_OFFSET);
		ulplaycount ++;	
		stlong(ulplaycount, (char*)prevTempPtr+PLAYCOUNT_KEYWD_OFFSET); //tRACK nUMBER 를 바꿈.	

		ulplaycount = ldlong((char*)prevTempPtr+PLAYCOUNT_KEYWD_OFFSET);		
		SetPlayListValue(PLAYCOUNT_KEYWD, ulplaycount);
	}	
	doEndDbFunc= 1;
*/	
	unsigned long ulplaycount = 0;
	unsigned long ulTempTime = 0;
	void*	columns_value[DB_MUSIC_TABLE_COUNT];
	memset(columns_value, NULL, sizeof(columns_value));	//heechul
	
	ulTempTime = GetEpochTime();


	if(SetPlayListValue(prevTempPtr, &ulTempTime, LASTPLAYED_KEYWD))
	{

//		db_api_get_record(db_t_music, prevTempPtr ,columns_value);
//		dbApiGetRecord(gDbTableNameMusic, prevTempPtr, columns_value);
		ulplaycount = *((unsigned long*)columns_value[7]);	
		ulplaycount ++;	

		SetPlayListValue(prevTempPtr, &ulplaycount, PLAYCOUNT_KEYWD);
	}	
	doEndDbFunc= 1;
}

int get_total_title_num(void)
{
	return(curTotalTitleListNum);	
}	

unsigned char db_get_cur_myrating_val(unsigned short usIndexLocal)
{
	unsigned long ulTempPtr; 
	unsigned char retMyRating;

	if (changedMyRatingList 
		&& (prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE))
	{
		ulTempPtr = prevTempPtr;
	}	
	else if (prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath == MY_QUICKLISTS_TITLE 
			&& doCpmparePrevPtr)
	{
		ulTempPtr = prevTempPtr;
	}
	else
	{
		ulTempPtr = pulPlayPtr[usIndexLocal];
	}
	
	retMyRating = get_cur_myrating_val(ulTempPtr);
	
	return(retMyRating);	
}	


unsigned int db_get_cur_index_for_display(void)
{
	return changedIndexNum;
}

unsigned int db_get_index_for_ui(void)
{
	return changedIndexNum+1;
}




/************************************************/
//billy@add for temp 0721
void InitGetPlpPlaylist(void)
{
	int i;
	unsigned long totalIndex;

	#ifdef NEW_MUSIC_PLAYLIST_MENU
	getPlp.ulPlpHeadMenuCount = MAX_PLP_HEAD_MENU;

	// Playlist 위에 메뉴를 추가한다. 
	//
	getPlp.playlistType[0] = PLP_ITEM_MENU;
	getPlp.playlistID[0]  = (int)pusU10_MyRating;
	getPlp.playlistType[1] = PLP_ITEM_MENU;
	getPlp.playlistID[1]  = (int)pusU10_QuickList;
	#else
	getPlp.ulPlpHeadMenuCount = 0;
	#endif

	GetMusicPlayLists(	&getPlp.playlistID[getPlp.ulPlpHeadMenuCount],
						PLPTOTALNUM,
						&totalIndex);
	
	getPlp.ulPlpTotalCount = getPlp.ulPlpHeadMenuCount + totalIndex;
	
	
	if( getPlp.ulPlpTotalCount > PLPTOTALNUM )
	{
		getPlp.ulPlpTotalCount = PLPTOTALNUM;
	}

	for( i = 0; i < getPlp.ulPlpTotalCount - getPlp.ulPlpHeadMenuCount ; i++ )
	{
		getPlp.playlistType[getPlp.ulPlpHeadMenuCount + i] = PLP_ITEM_LIST;
	}
}

/* ============================================================================
	Function name : SaveParamPlaylist()
	Description   	 : save param playlist.
	Parameter    	 : 
	return		 : null

   SaveParamPlaylist ==> SaveParamPlaylistMyRating => SaveParamPlaylistMyRatingFile
				  ㄴ=> SaveParamPlaylistFile 	  => SaveParamPlaylistFilePath
============================================================================ */
void SaveParamPlaylist(int CurPos)
{
	if(CurPos == DB_PLAYLISTS)
	{
		usTotalTitleResultLinkNum = getPlp.ulPlpTotalCount;
	}
}


void GetPlaylistName(int currentIndex, unsigned short *currentPlaylist)
{
	if(getPlp.playlistType[currentIndex] == PLP_ITEM_MENU)
	{
		tfs4_wcscpy(currentPlaylist, 
		GetUnicodeString((unsigned long) getPlp.playlistID[currentIndex], 1));
	} 
	else if(getPlp.playlistType[currentIndex] == PLP_ITEM_LIST)
	{
		GetPlayListsFileName(getPlp.playlistID, currentIndex, currentPlaylist);
		RemoveFileExtString(currentPlaylist);
	}
}



#if 0
/* ============================================================================
	Function name : ComparePlaylistFileName(unsigned char *CompareFileName)
	Description  	 : compare brower playlist file position with music playlist file position.
	Parameter    	 : CompareFileName: brower playlist file full path.
	return		 : position index number, 0:fail
	Author		 : billy
============================================================================ */
unsigned short ComparePlaylistFileName(unsigned char *CompareFileName)
{
	unsigned long ulIndexNum = 0, dirLen=0;
	int i;
	t_char Browextension[5];
	t_char PLPextension[5] = { '.', 'P', 'L', 'P', '\0'};
//	t_char PLPextension[5] = { '.', 'P', 'L', 'A', '\0'};

	unsigned char wcCompareFileName[MAX_UNICODE_PATH_BYTE];

	memset(wcCompareFileName, 0x0, MAX_UNICODE_PATH_BYTE);

	//
	//check.. 1. CompareFileName: plp ?
	//
	dirLen = TFS4_strlen(CompareFileName)-4;
	
	DbgPrintf(("CompareFileName : %s\n",CompareFileName));
	DbgPrintf(("file length : %d\n",dirLen));

	for(i=0;i<dirLen;i++)
		wcCompareFileName[i]=CompareFileName[i];
	wcCompareFileName[i] = '\0';
	
	for(i=0;i<4;i++)
	{
		Browextension[i] = CompareFileName[dirLen +i];
		DbgPrintf(("%c %c\n",CompareFileName[dirLen +i],Browextension[i]));

		if((Browextension[i] >='a') && (Browextension[i] <='z'))
			Browextension[i]-=('a' - 'A');
	}
	Browextension[i] = '\0';

	if(TFS4_wcsncmp(Browextension, PLPextension,5) != 0 || TFS4_wcsncmp(Browextension, PLPextension,5) != 0)//fail..
	{
		DbgPrintf(("not plp file\n"));
		return 0;
	}

	DbgPrintf(("CompareFileName is plp file\n"));//ok	
	
	SaveParamPlaylist(0);//if don't save, save playlist...	

	//
	//check...2 compare file name...		
	//
{
	int j;
	unsigned char PlpFileName[MAX_UTF8_PATH_BYTE];
	unsigned char ConvertPlpFilePath[MAX_UTF8_PATH_BYTE];
	unsigned long DelPartLen = 0, TotalPathLen=0;

	DbgPrintf(("CompareFileName : [%s]\n",wcCompareFileName));	
	
	DelPartLen = TFS4_strlen("/a/Playlists/");
	DbgPrintf(("DelPartLen: %d\n", DelPartLen));
			
	for (i=0;i<getPlp->ulPlpTotalCount-1;i++)// 1번부터 저장..
	{
		memset(PlpFileName, 0x0, MAX_UTF8_PATH_BYTE);
		memset(ConvertPlpFilePath, 0x0, MAX_UTF8_PATH_BYTE);
	
		wc2utf8((unsigned char *)ConvertPlpFilePath, (unsigned short *)getPlp->PLP_LIST[i+1].FilePath);
		TotalPathLen = TFS4_strlen(ConvertPlpFilePath);
		DbgPrintf(("TotalPathLen: %d\n", TotalPathLen));
		
		for(j=DelPartLen ; j<TotalPathLen-4 ;j++)
			PlpFileName[j-DelPartLen] = ConvertPlpFilePath[j];
		PlpFileName[j] = '\0';
		
		DbgPrintf(("PlpFileName: %s\n", PlpFileName));

		if(!sfstrcmp((char *)PlpFileName, (char *)wcCompareFileName)) //same: return 0
		{
			DbgPrintf(("same file name num : [%d]\n",i+1));
			ulIndexNum = i+1;
			//break;
		}
		memset(PlpFileName, 0x0, MAX_UTF8_PATH_BYTE);
	}
}


	return ulIndexNum; //ok
}
#endif




/* ============================================================================
	Function name : navi_key_playlist(unsigned short* usDBFileName)
	Description  	 : Search Next/Privious contents
	Parameter    	 : usDBFileName: playlist file(plp file)
	return		 : 0: error , 1: success
	Author		 : billy
============================================================================ */
int navi_key_playlist(unsigned short* usDBFileName)
{

	int i, RealNum, TotalFileNum=0;
	t_int32 dwFD;
	unsigned long TempNum=0, NoPtrCounts=0;
	unsigned char temSeekdata[512];


	memset(temSeekdata, 0, sizeof(temSeekdata));

//	unsigned long ulResultPtr=0;

	dwFD = tfs4_open(usDBFileName, O_RDONLY);	

	if(dwFD < 0)
	{
		return 0;
	}

	//
	//check 1. 파일크기확인.... 총개수와 파일 크기 비교해서 구한다.
	//
	tfs4_lseek(dwFD, 0, SEEK_SET);
	TotalFileNum = tfs4_lseek(dwFD,  0,  SEEK_END);

	TotalFileNum = (TotalFileNum/4)-1;


	tfs4_lseek(dwFD, 0, SEEK_SET);

//	tfs4_read(dwFD, SeekData, 4);

	tfs4_read(dwFD, temSeekdata, 512);



	TempNum = (SeekData[0] | (SeekData[1]<<8) | (SeekData[2]<<16) | (SeekData[3]<<24));

	if(TempNum != TotalFileNum)
	{
		db_display->ulTotalLinkNumber = TotalFileNum;
	}else
	{
		db_display->ulTotalLinkNumber = TempNum;//save music title total number in playlist file.
	}
	
	RealNum = 0;
	for(i=1; i<db_display->ulTotalLinkNumber+1; i++)//because starting i=1..
	{
		tfs4_lseek(dwFD, 4*i, SEEK_SET);
		tfs4_read(dwFD, SeekData, 4);
		TempNum = (SeekData[0] | (SeekData[1]<<8) | (SeekData[2]<<16) | (SeekData[3]<<24));

		//
		//check 2.  ptr유무확인(존재하지않을 경우 스킵 ).
		//
		db_display->pulResultLinkedListBuffer[RealNum] = TempNum;
		RealNum++;

		
	}

	db_display->ulTotalLinkNumber = db_display->ulTotalLinkNumber - NoPtrCounts;
	db_display->ref_memory = DB_DATA_FILE_LOAD;
	tfs4_close(dwFD);
	return 1;

}


/* ============================================================================
	Function name : DelCurQuickListFile(unsigned long ulCurPos)
	Description  	 : del quick list file.
	Parameter    	 : ulCurPos: current index position, QUICKLIST_MAXNUM:200
	return		 : 2: not pointer value.
				   1: success.
	Author		 : billy
============================================================================ */
unsigned short DelCurQuickListFile(unsigned long ulCurPos)
{
	unsigned long ulQuicklistPos=0;

	if(ulCurPos >0)
	{
		ulQuicklistPos = ulCurPos-1;//because quick list start position 0.
	}
	else
	{
		return 2;
	}

	if(pulDBQuickList[ulQuicklistPos] != 0)
	{
		int i;

		ulQuickListNum--;
		for(i=ulQuicklistPos; i<ulQuickListNum; i++)
		{	        
			pulDBQuickList[i] = pulDBQuickList[i+1];		
		}
		pulDBQuickList[ulQuickListNum]=0;

		usTotalTitleResultLinkNum = ulQuickListNum;
		if (usTotalTitleResultLinkNum == 0) usTotalTitleResultLinkNum=-1;
		tmpQuicListTotalNum = usTotalTitleResultLinkNum;

#if 0  //ori
		if (prevTitlePath == MY_QUICKLISTS_TITLE)
		{
			for(i = 0; i < usTotalTitleResultLinkNum; i++) 
			{
				pulPlayPtr[i] = pulDBQuickList[i];
			}
//			
//			pulPlayPtr[i] = 0;			
//			curTotalTitleListNum = usTotalTitleResultLinkNum;
		}
#endif
		return 1;
	}
	else
	{
		return 2;
	}
}

/* ============================================================================
	Function name : CheckQuickList(unsigned long ulCurPos)
	Description  	 : check quick list file.
	Parameter    	 : ulCurPos: current index position, QUICKLIST_MAXNUM:200
	return		 : 2: exist same pointer value or not pointer value.
				   1: success.
	Author		 : billy
============================================================================ */

unsigned short CheckQuickList(unsigned long ulCurPos)
{
//초기값  0부터 시작 
	if(ulCurPos >0)
	{
		ulCurPos--;
	}

	//favorite와 recently는 계속 변하기 때문에 다른 포지션의 것과는 다른다. 
	if ((prevTitlePath == MY_FAVERITS_TITLE && curTitlePath == MY_FAVERITS_TITLE) 
		|| (prevTitlePath == RECENTLY_PLAYED_TITLE && curTitlePath == RECENTLY_PLAYED_TITLE)
		|| curTitlePath == MY_QUICKLISTS_TITLE  || playReturnPos == NOWPLAYING_TITLE) // angelo - added 
	{
		if(pulPlayPtr[ulCurPos] != 0)
		{
			//compare pointer value.
			int i;
			for(i=0;i<ulQuickListNum;i++)
			{
				if(pulDBQuickList[i]==pulPlayPtr[ulCurPos])
				{
					return 2;
				}
			}			
			//ok
			return 1;
		}
		else
		{
			return 2;
		}		
	}
	else
	{
		if(db_display->pulResultLinkedListBuffer[ulCurPos] != 0)
		{
			//compare pointer value.
			int i;
			for(i=0;i<ulQuickListNum;i++)
			{
				if(pulDBQuickList[i]==db_display->pulResultLinkedListBuffer[ulCurPos])
				{
					return 2;		//이미 있을때 인가? 중복을 피하기 위함.
				}
			}
			
			return 1;
		}
		else
		{
			return 2;				
		}	
	}
}


/* ============================================================================
	Function name : SaveQuickPtr(unsigned long ulCurPos)
	Description  	 : save quick playlist pointer.
	Parameter    	 : ulCurPos: current index position, QUICKLIST_MAXNUM:200
	return		 : 2: not pointer value.
				   1: success.
	Author		 : billy
============================================================================ */
unsigned short SaveQuickPtr(unsigned long ulCurPos)
{
//start position 0....
	if(ulCurPos > 0)
	{
		ulCurPos--;
	}

	if (curTitlePath == MY_QUICKLISTS_TITLE || playReturnPos == NOWPLAYING_TITLE) // angelo - added
	{
		if(pulPlayPtr[ulCurPos] != 0)
		{
			pulDBQuickList[ulQuickListNum]=pulPlayPtr[ulCurPos];
			ulQuickListNum++;

			return 1;						
		}
		else
		{
			return 2;
		}
	}
	else //ori
	{
		
		if(ulQuickListNum<QUICKLIST_MAXNUM)
		{
			if(db_display->pulResultLinkedListBuffer[ulCurPos] != 0)
			{	
				//save quick list	
				pulDBQuickList[ulQuickListNum] = db_display->pulResultLinkedListBuffer[ulCurPos];
				ulQuickListNum++;

				return 1;
			}
			else
			{
				return 2;
			}		

		}
		else
		{
			int i;

			for(i=0; i<QUICKLIST_MAXNUM-1; i++)
			{	        
				pulDBQuickList[i] = pulDBQuickList[i+1];		
			}

			pulDBQuickList[QUICKLIST_MAXNUM-1] = db_display->pulResultLinkedListBuffer[ulCurPos];
			
			ulQuickListNum = QUICKLIST_MAXNUM;

			return 1;
		}
	}

}

/* ============================================================================
	Function name : InitQuickPtr(void)
	Description  	 : delete quick playlist pointer.
	Parameter    	 : 
	return		 : 1: success.
				   2: not quick list file.
	Author		 : billy
============================================================================ */
unsigned short InitQuickPtr(void)
{
	int i;
	if(ulQuickListNum>0)
	{
		for (i=0;i<ulQuickListNum;i++)
			pulDBQuickList[i] = 0;
		
		ulQuickListNum=0;
		usTotalTitleResultLinkNum = -1;
		tmpQuicListTotalNum = usTotalTitleResultLinkNum;
		wasInAllDelQuickList = 1;

		return 1;
	}
	else
	{
		return 2;
	}
}





/* ============================================================================
	Function name : KeySearchNextPriHandle()
	Description  	 : 
	Parameter    	 : 
============================================================================ */


#if 0
/* ============================================================================
	Function name : SaveParamPlaylistFile()
	Description   	 : save parameter for step 1 myrating & playlists UI
	Parameter    	 : 
	return		 : null
	Author		 : billy

   SaveParamPlaylist ==> SaveParamPlaylistMyRating => SaveParamPlaylistMyRatingFile
				  ㄴ=> SaveParamPlaylistFile 	  => SaveParamPlaylistFilePath
============================================================================ */
void SaveParamPlaylistFile()
{
	//save data..
	DisplayPlaylistFile();	//draw file list..
	
	usTotalTitleResultLinkNum = db_display->ulTotalLinkNumber;
	if (usTotalTitleResultLinkNum == 0) usTotalTitleResultLinkNum=-1;
}
#endif




#ifdef NEW_PLAYALL_SEARCH
/*=========================================================*/
//#define		DB_ASC				1
//#define		DB_DESC				2
void db_get_list_total_num(int curPos, int curPlaylistsPos)
{		
	//db_g_searchValue[0]=0;

	switch(curPos)
	{
		case DB_ARTISTS:
			{
				complex_index[0] = gDbColumnNameArtist;
				complex_index[1] = gDbColumnNameAlbum;
				complex_index[2] = gDbColumnNameTrackNumber;
				complex_index_column_number = 3;
				condition_number = 0;
			}
			break; 
			
				
		case DB_ALBUMS:	
			{
				complex_index[0] = gDbColumnNameAlbum;
				complex_index[1] = gDbColumnNameTrackNumber;
				complex_index_column_number = 2;
				condition_number = 0;
			}
			break;  
			
			 	
		case DB_GENRES:	
			{
				complex_index[0] = gDbColumnNameGenre;
				complex_index[1] = gDbColumnNameArtist;
				complex_index[2] = gDbColumnNameTitle;

				complex_index_column_number = 3;				
				condition_number = 0;
			}
			break;	
			
						
		case DB_TITLES:	
			{
				complex_index[0] = gDbColumnNameTitle ;
				complex_index_column_number = 1;
				condition_number = 0;
			}						
			break; 
			
	#if defined(CONFIG_AUDIBLE)
		case DB_AUDIBLE:	
			{
				complex_index[0] = gDbColumnNameFileFormat;
				complex_index_column_number = 1;
				condition_number = 1;
			}						
			break; 
	#endif
			
		case DB_PLAYLISTS:
			SaveParamPlaylist(DB_PLAYLISTS);
			break;	
	}
}


void db_get_list_data2(int curPos, int listnum, int curPlaylistsPos, unsigned short* dbKeyOffset, int filetype)
{
	
	unsigned short usOrder;
	
	if (curPos == DB_PLAYLISTS)
	{
		switch(curPlaylistsPos)
		{
			case 1:
			{		//myrating				
				FindRating(	listnum, 
							db_display->pulResultLinkedListBuffer, 
							#ifdef DB_SEARCH_CHANGED
							TOTAL_TITLE_NUM, //sizeof(db_display->pulResultLinkedListBuffer)과 TOTAL_TITLE_NUM중에 어떤것을 이용해야 하는것이 올을까?
							#endif
							&db_display->ulTotalLinkNumber);
				break;
			}
						
			case 2:		//playcount
			{
				void * without_values[1];
				unsigned short	usExceptZero;
				condition_number = 0;
				usOrder = DB_DESC;	
				usExceptZero = 0;
				without_values[0] = &usExceptZero;

				break;		
			}
			
			case 3:		//lastedplay
			{
				void * without_values[1];
				unsigned short	usExceptZero;
				condition_number = 0;
				usExceptZero = 0;
				without_values[0] = &usExceptZero;
				usOrder = DB_DESC;

				break;	
			}
			
			case 4:		//playlist
			{
				#ifdef CONFIG_H10JDB
					if (!navi_key_playlist((unsigned short*)getPlp->PLP_LIST[listnum].FilePath))
					{
						return;
					}
				#else
					{
						#if 1
						fwui_playlist_getMusicRowId(&getPlp.playlistID[getPlp.ulPlpHeadMenuCount], listnum - 1);
						#else
						unsigned int retStartCluster;
					
						retStartCluster = fwui_playlist_plaFileGetStartCluster((unsigned short*)getPlp->PLP_FilePath[listnum]);

						fwui_playlist_musicFileGetCluster( retStartCluster );
						#endif
					}				
				#endif				
											
				break;	
			}	
			
			case 5:		//quicklist
			{
/*  Angelo */
				int i;
				for(i = 0; i < ulQuickListNum; i++) 
				{
					db_display->pulResultLinkedListBuffer[i]=pulDBQuickList[i];
				}	
/* end */
				break;	
			}							
		}	
		
	}
	else
	{
	#ifndef CONFIG_FWUI	
		if (listnum == 1)
		{
			unsigned long start_index;
			unsigned long end_index;
			
			start_index = condition_number;
			end_index = complex_index_column_number-1;
			for(; start_index<end_index; start_index++)
			{
				complex_index[start_index] = complex_index[start_index+1];
			}
			complex_index_column_number--;
			condition_number = 0;
		}
		else
	#else
		if(listnum == SELECT_PLAYALL)
		{
			unsigned long start_index;
			unsigned long end_index;
			
			start_index = condition_number;
			end_index = complex_index_column_number-1;
			for(; start_index<end_index; start_index++)
			{
				complex_index[start_index] = complex_index[start_index+1];
			}
			complex_index_column_number--;
			condition_number = 0;
		}
		else			
	#endif
		{
		    DB_VARCHAR	*source;
	
		#ifdef CONFIG_FWUI
		if(dbKeyOffset)
		{
//			source = (DB_VARCHAR*)dbApiAllocKey(dbKeyOffset, filetype);
			condition_set[0] = dbKeyOffset;
		}
		else
		{
			source = (DB_VARCHAR*)dbApiAllocKey(db_display->pulResultLinkedListBuffer[listnum-1], db_display->ref_memory);
			
			 condition_set[0] = condition_set1;	    
		    	dbStringCopy((DB_VARCHAR*)condition_set1, source);
		}
		#else									
		source = (DB_VARCHAR*)dbApiAllocKey(db_display->pulResultLinkedListBuffer[listnum-2], db_display->ref_memory);

              condition_set[0] = condition_set1;	    
	    	dbStringCopy((DB_VARCHAR*)condition_set1, source);
		#endif		    										


		    condition_number = 1;
		}
		
		usOrder = DB_ASC;
	}
}

//void db_get_next_list_data2(int curPos, int depth, int listnum, unsigned long dbKeyOffset)
void db_get_next_list_data2(int curPos, int depth, int listnum, unsigned short* dbKeyOffset)
{
#ifndef CONFIG_FWUI	
	if (listnum==1)
	{
		unsigned long start_index;
		unsigned long end_index;

		start_index = condition_number;
		end_index = complex_index_column_number-1;
		
		for(; start_index<end_index; start_index++)
		{
			complex_index[start_index] = complex_index[start_index+1];
		}
		complex_index_column_number--;
	}
	else
#else	
	if (listnum == SELECT_PLAYALL)
	{
		unsigned long start_index;
		unsigned long end_index;

		start_index = condition_number;
		end_index = complex_index_column_number-1;
		
		for(; start_index<end_index; start_index++)
		{
			complex_index[start_index] = complex_index[start_index+1];
		}
		complex_index_column_number--;
	}
	else
#endif	
	{
	    DB_VARCHAR		*source;

	#ifdef CONFIG_FWUI
	if(dbKeyOffset)
	{
//		source = (DB_VARCHAR*)dbApiAllocKey(dbKeyOffset, DB_INDEX_FILE_LOAD);//db_display->ref_memory);
		condition_set[1] = dbKeyOffset;
		condition_number = 2;
	}
	else	
	{
	    source = (DB_VARCHAR*)dbApiAllocKey(db_display->pulResultLinkedListBuffer[listnum-1], db_display->ref_memory);

	    if (depth==1)
	    {
		    	condition_set[1] = condition_set2;
		    	condition_number = 2;
			dbStringCopy((DB_VARCHAR*)condition_set2,  source);
	    }
	    else if (depth==2)
	    {
			condition_set[2] = condition_set3;
		    	condition_number =3;
              	dbStringCopy((DB_VARCHAR*)condition_set3,  source);
	    }
	}
	#else
	source = (DB_VARCHAR*)dbApiAllocKey(db_display->pulResultLinkedListBuffer[listnum-2], db_display->ref_memory);

	    if (depth==1)
	    {
		    	condition_set[1] = condition_set2;
		    	condition_number = 2;
			dbStringCopy((DB_VARCHAR*)condition_set2,  source);
	    }
	    else if (depth==2)
	    {
			condition_set[2] = condition_set3;
		    	condition_number =3;
              	dbStringCopy((DB_VARCHAR*)condition_set3,  source);
	    }
	#endif	
		
	}
}


int db_get_list_dataNtotalnum(int listnum)
{
	if((fwuiDbData->artistNum == SELECT_PLAYALL) ||(fwuiDbData->genreNum== SELECT_PLAYALL) ||(fwuiDbData->albumNum== SELECT_PLAYALL) )
	{
		db_display->ulTotalLinkNumber = dbApiSearchMSAllRecords(gDbTableNameMusic, 
										complex_index, complex_index_column_number, 
										condition_set, condition_number, 
										PExtra_Type->SortMethod+1, 
										db_display->pulResultLinkedListBuffer);
	}
	else
	{
			db_display->ulTotalLinkNumber = dbApiSearchMS(gDbTableNameMusic, 
										complex_index, complex_index_column_number, 
										condition_set, condition_number, 
										PExtra_Type->SortMethod+1, 
										db_display->pulResultLinkedListBuffer, 
										#ifdef DB_SEARCH_CHANGED
										TOTAL_TITLE_NUM,	//TOTAL_TITLE_NUM or sizeof(db_display->pulResultLinkedListBuffer)
										#endif
										&db_display->ref_memory);
	}

	return TRUE;
}


#else  // ori--(#ifdef NEW_PLAYALL_SEARCH)

#if 0 //=================================================> here

/*=========================================================*/
//#define		DB_ASC				1
//#define		DB_DESC				2
void db_get_list_total_num(int curPos, int curPlaylistsPos)
{		
	//db_g_searchValue[0]=0;

	switch(curPos)
	{
		case DB_ARTISTS:
			{
				complex_index[0] = gDbColumnNameArtist;
				complex_index[1] = gDbColumnNameAlbum;
				complex_index[2] = gDbColumnNameTrackNumber;
				complex_index_column_number = 3;
				condition_number = 0;
			}

			db_display->ulTotalLinkNumber = dbApiSearchMS(gDbTableNameMusic, 
														complex_index, complex_index_column_number, 
														condition_set, condition_number, 
													PExtra_Type->SortMethod+1, db_display->pulResultLinkedListBuffer, &db_display->ref_memory);
			break; 
			
				
		case DB_ALBUMS:	
			{
				complex_index[0] = gDbColumnNameAlbum;
				complex_index[1] = gDbColumnNameTrackNumber;
				complex_index_column_number = 2;
				condition_number = 0;
			}

			db_display->ulTotalLinkNumber = dbApiSearchMS(gDbTableNameMusic, 
														complex_index, complex_index_column_number, 
														condition_set, condition_number, 
														PExtra_Type->SortMethod+1, 
														db_display->pulResultLinkedListBuffer, 
														&db_display->ref_memory);
			break;  
			
			 	
		case DB_GENRES:	
			{
				complex_index[0] = gDbColumnNameGenre;
				complex_index[1] = gDbColumnNameArtist;
				complex_index[2] = gDbColumnNameTitle;

				complex_index_column_number = 3;				
				condition_number = 0;
			}

			db_display->ulTotalLinkNumber = dbApiSearchMS(gDbTableNameMusic, 
														complex_index, complex_index_column_number, 
														condition_set, condition_number, 
														PExtra_Type->SortMethod+1, 
														db_display->pulResultLinkedListBuffer, 
														&db_display->ref_memory);
			break;	
			
						
		case DB_TITLES:	
			{
				complex_index[0] = gDbColumnNameTitle ;
				complex_index_column_number = 1;
				condition_number = 0;
			}
						
			db_display->ulTotalLinkNumber = dbApiSearchMS(gDbTableNameMusic, 
														complex_index, complex_index_column_number, 
														condition_set, condition_number, 
														PExtra_Type->SortMethod+1, 
														db_display->pulResultLinkedListBuffer, 
														&db_display->ref_memory);
			break; 
			
			
		case DB_PLAYLISTS:
			SaveParamPlaylist(DB_PLAYLISTS);
			break;	
	}
}

void db_get_list_data2(int curPos, int listnum, int curPlaylistsPos)
{
	
	unsigned short usOrder;
	
	if (curPos == DB_PLAYLISTS)
	{
		
		switch(curPlaylistsPos)
		{
			case 1:
			{		//myrating				
				FindRating(listnum, db_display->pulResultLinkedListBuffer, &db_display->ulTotalLinkNumber);
				break;
			}
						
			case 2:		//playcount
			{
				void * without_values[1];
				unsigned short	usExceptZero;
				condition_number = 0;
				usOrder = DB_DESC;	
				usExceptZero = 0;
				without_values[0] = &usExceptZero;

				break;		
			}
			
			case 3:		//lastedplay
			{
				void * without_values[1];
				unsigned short	usExceptZero;
				condition_number = 0;
				usExceptZero = 0;
				without_values[0] = &usExceptZero;
				usOrder = DB_DESC;
				break;	
			}
			
			case 4:		//playlist
			{
				#ifdef CONFIG_H10JDB
				//#ifndef SUPPORT_MTP_DEVICE
					if (!navi_key_playlist((unsigned short*)getPlp->PLP_LIST[listnum].FilePath))
					{
						return;
					}
				#else
					{
						#if 1
						fwui_playlist_getMusicRowId(getPlp.playlistID, listnum - 1);
						#else
						unsigned int retStartCluster;
					
						retStartCluster = fwui_playlist_plaFileGetStartCluster((unsigned short*)getPlp->PLP_FilePath[listnum]);

						fwui_playlist_musicFileGetCluster( retStartCluster );
						#endif
					}				
				#endif				
											
				break;	
			}	
			
			case 5:		//quicklist
			{
				break;	
			}							
		}	
		
	}
	else
	{

	#ifndef CONFIG_FWUI	
		if (listnum == 1)
		{
			unsigned long start_index;
			unsigned long end_index;
			
			start_index = condition_number;
			end_index = complex_index_column_number-1;
			for(; start_index<end_index; start_index++)
			{
				complex_index[start_index] = complex_index[start_index+1];
			}
			complex_index_column_number--;
			condition_number = 0;
		}
		else
	#else
		if(listnum == SELECT_PLAYALL)
		{
			unsigned long start_index;
			unsigned long end_index;
			
			start_index = condition_number;
			end_index = complex_index_column_number-1;
			for(; start_index<end_index; start_index++)
			{
				complex_index[start_index] = complex_index[start_index+1];
			}
			complex_index_column_number--;
			condition_number = 0;
		}
		else			
	#endif
		{
		    DB_VARCHAR	*source;

	
		#ifdef CONFIG_FWUI	
		{
			source = (DB_VARCHAR*)dbApiAllocKey(db_display->pulResultLinkedListBuffer[listnum-1], db_display->ref_memory);
		}
		#else									
		source = (DB_VARCHAR*)dbApiAllocKey(db_display->pulResultLinkedListBuffer[listnum-2], db_display->ref_memory);
		#endif		    										

		    condition_set[0] = condition_set1;
	    
	    	dbStringCopy((DB_VARCHAR*)condition_set1, source);
		    condition_number = 1;
		}
		
		usOrder = DB_ASC;
		
		db_display->ulTotalLinkNumber = dbApiSearchMS(gDbTableNameMusic, 
										complex_index, complex_index_column_number, 
										condition_set, condition_number, 
										PExtra_Type->SortMethod+1, 
										db_display->pulResultLinkedListBuffer, 
										&db_display->ref_memory);
	}

}

void db_get_next_list_data2(int curPos, int depth, int listnum)
{
#ifndef CONFIG_FWUI	
	if (listnum==1)
	{
		unsigned long start_index;
		unsigned long end_index;

		start_index = condition_number;
		end_index = complex_index_column_number-1;
		
		for(; start_index<end_index; start_index++)
		{
			complex_index[start_index] = complex_index[start_index+1];
		}
		complex_index_column_number--;
	}
	else
#else	

	if (listnum == SELECT_PLAYALL)
	{
		unsigned long start_index;
		unsigned long end_index;

		start_index = condition_number;
		end_index = complex_index_column_number-1;
		
		for(; start_index<end_index; start_index++)
		{
			complex_index[start_index] = complex_index[start_index+1];
		}
		complex_index_column_number--;
	}
	else
#endif	
	{
	    DB_VARCHAR		*source;

	#ifdef CONFIG_FWUI
	source = (DB_VARCHAR*)dbApiAllocKey(db_display->pulResultLinkedListBuffer[listnum-1], db_display->ref_memory);
	#else
	source = (DB_VARCHAR*)dbApiAllocKey(db_display->pulResultLinkedListBuffer[listnum-2], db_display->ref_memory);
	#endif	
		
	    if (depth==1)
	    {
		    condition_set[1] = condition_set2;
		    condition_number = 2;
//		    db_string_copy((DB_VARCHAR*)condition_set2,  source);
			dbStringCopy((DB_VARCHAR*)condition_set2,  source);
	    }
	    else if (depth==2)
	    {
		    condition_set[2] = condition_set3;
		    condition_number =3;
//		    db_string_copy((DB_VARCHAR*)condition_set3,  source);
			dbStringCopy((DB_VARCHAR*)condition_set3,  source);
	    }
	}

	db_display->ulTotalLinkNumber = dbApiSearchMS(gDbTableNameMusic, 
												complex_index, complex_index_column_number, 
												condition_set, condition_number, 
												PExtra_Type->SortMethod+1, 
												db_display->pulResultLinkedListBuffer, 
												&db_display->ref_memory);

}
#endif //if 0

#endif  //#ifdef NEW_PLAYALL_SEARCH 



/* ============================================================================
	Function name : db_get_full_path_name (int listnum)
	Description   	 : 
	Parameter     	 : 
============================================================================ */
unsigned char db_get_full_path_name (int listnum, unsigned char curPlayStatus, unsigned char playmode)
{
	unsigned char retVal = 1;
	unsigned char ret;
	int orgPlayTrackNum = 0;
	
	if (listnum < 1)
	{
		listnum = 1;	
		retVal = 1;	
	}
	else
	{
		--listnum;
		orgPlayTrackNum = listnum;

		if ((playmode == playmode_shuffle || playmode == playmode_shuffle_repeat))
		{
			if (!reStartPlayback && curPlayStatus == TRUE)// && !startShuffleMode)
			{
				listnum = get_shuffle_index (listnum);
			}
		}
		
		changedIndexNum = listnum;
	
		ret = GetPlaytrackByDBIndex(listnum, curPlayStatus);

		if (!ret)
		{
			retVal = 0;
		}	
		else if (ret == 2)
		{
			retVal = 2;
		}
		else
		{
			retVal = 1;
		}
	}
	
	return retVal;
}

void set_alarm_totaltitlenum(int num)
{
	AlarmBackupcurTotalTitleListNum = num;
}

	

/* ============================================================================
	Function name : db_get_full_path_name (int listnum)
	Description   	 : 
	Parameter     	 : 
============================================================================ */
extern unsigned char saveQuickListTitle;
void db_save_title_list (void)
{
	unsigned short jj;

#ifdef ALARM_MUSIC_NEW_LIST	
	int chkalarm;

	chkalarm = pm_operate_get(OP_ALARM_MODE);

	if(chkalarm)
	{
#if 0 // angelo - chk 	
		check_generate_alarm_music_db();
#endif 
	}
#endif

	if (prevTitlePath == MY_QUICKLISTS_TITLE || (prevTitlePath == MY_QUICKLISTS_TITLE && saveQuickListTitle))
	{
#ifdef ALARM_MUSIC_NEW_LIST	
	if(chkalarm)
	{
		set_alarm_totaltitlenum(get_total_title_num());

		for(jj = 0; jj< AlarmBackupcurTotalTitleListNum; jj++)
		{
			pulAlarmBackupPlayPtr[jj] = pulPlayPtr[jj];
		}
	}		
#endif		
		curTotalTitleListNum = ulQuickListNum;
		for(jj = 0; jj < curTotalTitleListNum; jj++) 
		{
			pulPlayPtr[jj] = pulDBQuickList[jj];
		}		
		if (saveQuickListTitle)
			saveQuickListTitle = 0;
	}
	else
	{
//20051122...new db play	

#ifdef ALARM_MUSIC_NEW_LIST	
	if(chkalarm)
	{
		set_alarm_totaltitlenum(get_total_title_num());

		for(jj = 0; jj< AlarmBackupcurTotalTitleListNum; jj++)
		{
			pulAlarmBackupPlayPtr[jj] = pulPlayPtr[jj];
		}
	}		
#endif	
		curTotalTitleListNum = db_display->ulTotalLinkNumber;
		for(jj = 0; jj < curTotalTitleListNum; jj++) 
		{	
			pulPlayPtr[jj] = db_display->pulResultLinkedListBuffer[jj];
		}	
	}
}


/* ============================================================================
	Function name : db_get_full_path_name (int listnum)
	Description   	 : 
	Parameter     	 : 
============================================================================ */
void set_my_rating_value (int curIndex, int rating_val)
{
	unsigned long ulTempPtr; 

	if (changedMyRatingList && (prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE))
	{
		ulTempPtr = prevTempPtr;
	}
	else if (prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath == MY_QUICKLISTS_TITLE 
			&& doCpmparePrevPtr)
	{
		ulTempPtr = prevTempPtr;
	}
	else
	{
		ulTempPtr = pulPlayPtr[curIndex];
	}

	SetPlayListValue(ulTempPtr, (unsigned long *)&rating_val, MYRATING_KEYWD);
	
}

void set_cur_music_play_opsition(unsigned char value)
{
	curMusicPlayPos = value;
}


unsigned char get_cur_music_play_position (void) // 0: Music  1: Browser 2: FMrec 3: Saved
{
	return (curMusicPlayPos);			
}

unsigned short get_music_playing_index (void)
{
	return (curPlayingIndexNum);			//bi_set의 play할때 결정된다.
}



unsigned int save_rectly_fav_index_for_resume (unsigned char listPos)
{
	int i;
	int retIndex;

	retIndex = 1;
	//My Favorits
	if (listPos == 2)
	{
#ifdef NEW_PLAYALL_SEARCH
		db_get_list_data2(DB_PLAYLISTS, 0, 2, NULL, 0);	
#else
		db_get_list_data2(DB_PLAYLISTS, 0, 2);	
#endif
		for(i=0; i < get_total_title_num(); i++)
		{
			if(db_display->pulResultLinkedListBuffer[i] == prevTempPtr)
			{
				retIndex = PSound_Type->Favorit_Index = i+1;
				break;
			}
		}
	}
	else if (listPos == 3)
	{
		//recently played
#ifdef NEW_PLAYALL_SEARCH
		db_get_list_data2(DB_PLAYLISTS, 0, 3, NULL, 0);
#else
		db_get_list_data2(DB_PLAYLISTS, 0, 3);
#endif
		for(i=0; i < get_total_title_num(); i++)
		{
			if(db_display->pulResultLinkedListBuffer[i] == prevTempPtr)
			{
				retIndex = PSound_Type->Rectly_Index = i+1;
				break;
			}
		}
	}
	else 
	{
		retIndex = 1;
	}

	return retIndex;
}





/////////////////////////////////////////////////////////////////////////////////////////
//
//
//								new fwui functions
//	
//
/////////////////////////////////////////////////////////////////////////////////////////
#ifdef CONFIG_FWUI


void fwui_Init_Struct_db_display(void)
{
	memset(db_display, 0, sizeof(DB_DISPLAY));
}



#ifdef NEW_PLAYALL_SEARCH
void fwui_search_artist(void)
{
	unsigned char exitToPrevPath;
	static int artistFlag=0;
	
	db_get_list_total_num(DB_ARTISTS, 0); //Artist	

	curTitlePath = curListsPath = ARTIST_LISTS;

	if(fwuiDbData->artistNum != 0 && fwuiDbData->artistNum !=SELECT_PLAYALL)
	{		
		db_get_list_data2(DB_ARTISTS, fwuiDbData->artistNum, 0, fwuiDbData->artistDBOffset, DB_INDEX_FILE_LOAD);

		g_artistStataus = fwuiDbData->artistNum;	

		if (fwuiDbData->albumNum == 0 && artistFlag == 10)
			exitToPrevPath = 1;				
		else 
			exitToPrevPath = 0;

		curTitlePath = ARTIST_ALBUM;


		if (!exitToPrevPath)
		{							
			if (g_artistStataus != g_prevArtistStataus && startArtistFirst) 
			{
				mustDoPlayAgain=1;						
			}
			else
			{
				mustDoPlayAgain=0;	
			}
			g_prevArtistStataus = g_artistStataus;
		}
		artistFlag = 5;	


		if (fwuiDbData->albumNum != 0 && fwuiDbData->albumNum !=SELECT_PLAYALL)
		{
			db_get_next_list_data2(DB_ARTISTS_ALBUMS, 1, fwuiDbData->albumNum, fwuiDbData->albumDBOffset);

			g_artistAlbumStataus = fwuiDbData->albumNum;								
			if (g_artistAlbumStataus != g_prevArtistAlbumStataus && startArtistFirst) 
			{
				mustDoPlayAgain=1;
			}
			else
			{
				mustDoPlayAgain=0;
			}
			g_prevArtistAlbumStataus = g_artistAlbumStataus;
			artistFlag = 10;

			playReturnPos = curTitlePath = ARTIST_ALBUM_TITLE;				
		}
	}		

	fwui_Init_Struct_db_display();

	db_get_list_dataNtotalnum(1);  //인자값은 나중을 위해 임시로 넣어 둔다- (현재 필요 없다 )
}

#else

#if 0 //==========================> here
void fwui_search_artist(void)
{
	unsigned char exitToPrevPath;
	static int artistFlag=0;

	fwui_Init_Struct_db_display();
	
	db_get_list_total_num(DB_ARTISTS, 0); //Artist	

	curTitlePath = curListsPath = ARTIST_LISTS;
	
	if(fwuiDbData->artistNum != 0)
	{
		db_get_list_data2(DB_ARTISTS, fwuiDbData->artistNum, 0);

		g_artistStataus = fwuiDbData->artistNum;	

		if (fwuiDbData->albumNum == 0 && artistFlag == 10)
			exitToPrevPath = 1;				
		else 
			exitToPrevPath = 0;

		curTitlePath = ARTIST_ALBUM;


		if (fwuiDbData->albumNum != 0)
		{
			db_get_next_list_data2(DB_ARTISTS_ALBUMS, 1, fwuiDbData->albumNum);

			g_artistAlbumStataus = fwuiDbData->albumNum;								
			if (g_artistAlbumStataus != g_prevArtistAlbumStataus && startArtistFirst) 
			{
				mustDoPlayAgain=1;
			}
			else
			{
				mustDoPlayAgain=0;
			}
			g_prevArtistAlbumStataus = g_artistAlbumStataus;
			artistFlag = 10;

			playReturnPos = curTitlePath = ARTIST_ALBUM_TITLE;	
			
			return;

		}

		if (!exitToPrevPath)
		{							
			if (g_artistStataus != g_prevArtistStataus && startArtistFirst) 
			{
				mustDoPlayAgain=1;						
			}
			else
			{
				mustDoPlayAgain=0;	
			}
			g_prevArtistStataus = g_artistStataus;
		}
		artistFlag = 5;	

	}	
}
#endif // #if 0 

#endif //#ifdef NEW_PLAYALL_SEARCH


void fwui_Create_fwuiDbData(void)
{
	memset(fwuiDbData, 0, sizeof(struct FWUI_DbData));
}	

void fwui_Create_fwui_AlarmMusic(void)
{
	memset(fwui_AlarmMusic, 0, sizeof(struct FWUI_MusicAlarmDbData));
}




void fwui_Init_fwuiDbData(void)
{
	memset(fwuiDbData, 0, sizeof(struct FWUI_DbData));
	
	fwuiDbData_playlistNum = 0;
	fwuiDbData_starNum = 0;
}




void fwui_Generate_DB(void)
{
	db_check_generate_db();	

	if (PSound_Type->Play_Mode_SET == playmode_shuffle || PSound_Type->Play_Mode_SET == playmode_shuffle_repeat)
		startShuffleMode = 1;
	else
		startShuffleMode = 0;

	//InitGetPlpPlaylist();
}



void fwui_search_songs(void)
{

	fwui_Init_Struct_db_display();

	playReturnPos = curListsPath = curTitlePath = PURE_TITLE;
	db_get_list_total_num(DB_TITLES, 0);

#ifdef NEW_PLAYALL_SEARCH
	db_get_list_dataNtotalnum(1);  //인자값은 나중을 위해 임시로 넣어 둔다- (현재 필요 없다 )

	if(db_display->pulResultLinkedListBuffer[0] == NULL)
	{
		db_display->ulTotalLinkNumber = 0;
	}
#endif
}


#if defined(CONFIG_AUDIBLE)
void fwui_search_audible(void)
{
	unsigned short vFileFormat = MTP_ObjectFormat_AA;

	fwui_Init_Struct_db_display();

	playReturnPos = curListsPath = curTitlePath = AUDIBLE_TITLE;
	db_get_list_total_num(DB_TITLES, 0);

#ifdef NEW_PLAYALL_SEARCH
	complex_index[0] = gDbColumnNameFileFormat;
	complex_index_column_number = 1;
	condition_set[0] = &vFileFormat;
	condition_number = 1;

	//
	//	Audible type으로 search하고,
	//
	db_get_list_dataNtotalnum(1);  //인자값은 나중을 위해 임시로 넣어 둔다- (현재 필요 없다 )

	if(db_display->pulResultLinkedListBuffer[0] == NULL)
	{
		db_display->ulTotalLinkNumber = 0;
	}

	//
	//	결과를 다시 sorting한다.
	//
	dbApiResultArraySorting(gDbTableNameMusic, 
							gDbColumnNameTitle, 
							PExtra_Type->SortMethod+1, 
							db_display->pulResultLinkedListBuffer, 
							db_display->ulTotalLinkNumber);

	
#endif
}
#endif


void fwui_search_albums(void)
{
	fwui_Init_Struct_db_display();

	db_get_list_total_num(DB_ALBUMS, 0); //Albums	
	curTitlePath = curListsPath = ALBUM_LISTS;

#ifdef NEW_PLAYALL_SEARCH
	if (fwuiDbData->albumNum!=0 && fwuiDbData->albumNum !=SELECT_PLAYALL)
	{
		db_get_list_data2(DB_ALBUMS, fwuiDbData->albumNum, 0, fwuiDbData->albumDBOffset, DB_INDEX_FILE_LOAD);

		g_albumStataus = fwuiDbData->artistNum;
		playReturnPos = curTitlePath = ALBUMS_TITLE;
	}		

	db_get_list_dataNtotalnum(1);  //인자값은 나중을 위해 임시로 넣어 둔다- (현재 필요 없다 )

	if(db_display->pulResultLinkedListBuffer[0] == NULL)
	{
		db_display->ulTotalLinkNumber = 0;
	}
#else
	if (fwuiDbData->albumNum!=0)
	{
		db_get_list_data2(DB_ALBUMS, fwuiDbData->albumNum, 0);

		g_albumStataus = fwuiDbData->artistNum;
		playReturnPos = curTitlePath = ALBUMS_TITLE;
	}

#endif
		
}


#ifdef NEW_PLAYALL_SEARCH
void fwui_search_genres(void)
{

	unsigned char exitToPrevPath;
	static int genreFlag=0;

	db_get_list_total_num(DB_GENRES, 0); 
	curTitlePath = curListsPath = GENRES_LISTS;					


	if (fwuiDbData->genreNum != 0 && fwuiDbData->genreNum != SELECT_PLAYALL)
	{
		db_get_list_data2(DB_GENRES, fwuiDbData->genreNum, 0, fwuiDbData->genreDBOffset, DB_INDEX_FILE_LOAD);

		g_genreStataus = fwuiDbData->genreNum;


		if (fwuiDbData->artistNum == 0 && (genreFlag == 10 || genreFlag == 15))
			exitToPrevPath = 1;
		else 
			exitToPrevPath = 0;

		curTitlePath = GENRES_ARTIST;

		if (!exitToPrevPath)
		{							
			if (g_genreStataus != g_prevGenreStataus && startGenreFirst) 
			{
				mustDoPlayAgain=1;
			}
			else
			{
				mustDoPlayAgain=0;
			}
			g_prevGenreStataus = g_genreStataus;
		}
		genreFlag = 5;

		if (fwuiDbData->artistNum!=0 && fwuiDbData->artistNum != SELECT_PLAYALL)
		{
			db_get_next_list_data2(DB_GENRES_ARTISTS_ALBUMS, 1, fwuiDbData->artistNum, fwuiDbData->artistDBOffset);

			g_genreArtistStataus = fwuiDbData->artistNum;	

			if (fwuiDbData->albumNum == 0 && genreFlag == 15)
				exitToPrevPath = 1;
			else 
				exitToPrevPath = 0;


			if (!exitToPrevPath)
			{								
				if (g_genreArtistStataus != g_prevGenreArtistStataus && startGenreFirst) 
				{
					mustDoPlayAgain=1;
				}
				else
				{
					mustDoPlayAgain=0;
				}
				g_prevGenreArtistStataus = g_genreArtistStataus;
			}
			
			playReturnPos = curTitlePath = GENRES_A_A_TITLE;
			genreFlag = 10;
		}
	}

	fwui_Init_Struct_db_display();
	db_get_list_dataNtotalnum(1);  //인자값은 나중을 위해 임시로 넣어 둔다- (현재 필요 없다 )

	if(db_display->pulResultLinkedListBuffer[0] == NULL)
	{
		db_display->ulTotalLinkNumber = 0;
	}
}

#else
#if 0
void fwui_search_genres(void)
{

	unsigned char exitToPrevPath;
	static int genreFlag=0;

	fwui_Init_Struct_db_display();

	db_get_list_total_num(DB_GENRES, 0); 
	curTitlePath = curListsPath = GENRES_LISTS;					


	if (fwuiDbData->genreNum != 0)
	{
		db_get_list_data2(DB_GENRES, fwuiDbData->genreNum, 0);
		g_genreStataus = fwuiDbData->genreNum;


		if (fwuiDbData->artistNum == 0 && (genreFlag == 10 || genreFlag == 15))
			exitToPrevPath = 1;
		else 
			exitToPrevPath = 0;

		curTitlePath = GENRES_ARTIST;

		if (fwuiDbData->artistNum!=0)
		{
			db_get_next_list_data2(DB_GENRES_ARTISTS_ALBUMS, 1, fwuiDbData->artistNum);

			g_genreArtistStataus = fwuiDbData->artistNum;	

			if (fwuiDbData->albumNum == 0 && genreFlag == 15)
				exitToPrevPath = 1;
			else 
				exitToPrevPath = 0;


			if (!exitToPrevPath)
			{								
				if (g_genreArtistStataus != g_prevGenreArtistStataus && startGenreFirst) 
				{
					mustDoPlayAgain=1;
				}
				else
				{
					mustDoPlayAgain=0;
				}
				g_prevGenreArtistStataus = g_genreArtistStataus;
			}
			
			playReturnPos = curTitlePath = GENRES_A_A_TITLE;
			genreFlag = 10;
			
			return;
			

		}

		if (!exitToPrevPath)
		{							
			if (g_genreStataus != g_prevGenreStataus && startGenreFirst) 
			{
				mustDoPlayAgain=1;
			}
			else
			{
				mustDoPlayAgain=0;
			}
			g_prevGenreStataus = g_genreStataus;
		}
		genreFlag = 5;

	}

}
#endif
#endif


unsigned short fwui_totalNum_Search(void)
{
	return (db_display->ulTotalLinkNumber);
}



void fwui_search_playlist(unsigned short indexNum)
{

	curTitlePath = curListsPath = MY_PLAYLISTS_LISTS_1;

	db_get_list_total_num(DB_PLAYLISTS, 4);		// 4는 playlist일때					
	
	if(indexNum != 0)
	{	
		playReturnPos = curListsPath = curTitlePath = MY_PLAYLISTS_TITLE;
#ifdef NEW_PLAYALL_SEARCH	
		db_get_list_data2(DB_PLAYLISTS, indexNum - GetPlpHeadMenuCount(), 4, NULL, 0);	// 4는 playlist일때		
#else
		db_get_list_data2(DB_PLAYLISTS, indexNum - GetPlpHeadMenuCount(), 4);	// 4는 playlist일때		
#endif
		g_myPlaylistStataus = indexNum;							
	}	
}


unsigned short fwui_total_playlistfile(void)
{
	return (getPlp.ulPlpTotalCount);
}


void fwui_search_MyRating(unsigned short starNum)
{
	
	fwui_Init_Struct_db_display();	
	
	curListsPath = MY_PLAYLISTS_RATE;
	
	if(starNum != 0) 
	{
		playReturnPos = curListsPath = curTitlePath = MYRATING_TITLE;
#ifdef NEW_PLAYALL_SEARCH	
		db_get_list_data2(DB_PLAYLISTS, starNum, 1, NULL, 0);	//1은 myrating
#else
		db_get_list_data2(DB_PLAYLISTS, starNum, 1);	//1은 myrating
#endif
		g_MyRatingStataus = starNum;
	
	}
}


int fwui_search_Quicklist(void)
{
	playReturnPos = curListsPath = curTitlePath = MY_QUICKLISTS_TITLE;
#ifdef NEW_PLAYALL_SEARCH		
	db_get_list_data2(DB_PLAYLISTS, 0, 5, NULL, 0);
#else
	db_get_list_data2(DB_PLAYLISTS, 0, 5);
#endif
	db_display->ref_memory = 1;

	if(ulQuickListNum >=fwui_alarm_get_title() ) 
	{
		return TRUE;
	}

	return FALSE;

}

unsigned short fwui_total_quicklist(void)
{
	return (ulQuickListNum);
}

void fwui_quilist_test(void)
{
	
	ulQuickListNum = 2;
	pulDBQuickList[0] = 0x10;
	pulDBQuickList[1] = 0xa0;
	db_display->ref_memory = 1;
}


int fwui_add_quicklist(int selectIdx)
{
	int ret_val;

	if(selectIdx != 0) 
	{
		ret_val = SaveQuickPtr(selectIdx);	//1이면 ok, 2이면 error

		if (notFinishedPlay)
		{
			didAddInQuickList = 1;
		}	
		
		if(ret_val == 1)
		{
			return 1;
		}
		else
		{
			return 0;		//error
		}
		
	}	
	return 1;	/* 충욱씨 나중에 이거 리턴값 확인해 주세요 */
}


int fwui_delcur_quicklist(int selectIdx)
{
	
	if(selectIdx != 0) 
	{
		DelCurQuickListFile(selectIdx);
		//notFinishedPlay는 play/next/prev/again할때 1로 세팅된다. 
		//stop일때 0으로 세팅된다. 그러니까 play중일때는 무조건 1이다. 
	}
	return 1; /* 충욱씨 나중에 이거 리턴값 확인해 주세요 */
}

void fwui_delall_quicklist()
{
	InitQuickPtr();
}


unsigned char fwui_get_currentSong_ratingValue(unsigned short curIndex)
{
	
	unsigned char curRatingValue;


	//현재곡의 myrating값을 가지고 온다. 
	curRatingValue = db_get_cur_myrating_val(curIndex-1);		
	
	return(curRatingValue);
		
}





void fwui_set_myrating_value(unsigned short value, unsigned short curIndex, char mode)
{
	
	unsigned char compareRatingVal;


	if(mode)	//play중일때 사용
	{
		//현재곡의 myrating값을 가지고 온다.
		compareRatingVal = db_get_cur_myrating_val(curIndex-1);			
		
		fwui_write_myrating_value(curIndex-1, value, mode);
	}
	else		//리스트중일때 사용
	{
		compareRatingVal = fwui_get_cur_myrating_val(curIndex-1);		

		fwui_write_myrating_value(curIndex-1, value, mode);
	}

	

	
	if ((prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE) ||( prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath == MY_QUICKLISTS_TITLE))
	{
		changedTrackMyRatingVal = compareRatingVal = db_get_cur_myrating_val(curIndex-1);
		if (prevTitlePath == MYRATING_TITLE  && curTitlePath == MYRATING_TITLE)	
		{

		//051215
		#ifdef CONFIG_AVLDB
		//처리루틴 필요한가?
		
		#else
			db_get_list_data(DB_PLAYLISTS, 1, prevMyRatingPos);	
			db_save_title_list();
		#endif


			changedMyRatingVal = 1;	
		}
		else if (prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath == MY_QUICKLISTS_TITLE)
		{
			changedTrackMyRatingVal = compareRatingVal;								
		}
	}

}


//리스트에서 사용할 함수.....play상에서는 다른 함수 사용
//db_get_cur_myrating_val()와 동일한 기능인데 리스트에서만 사용한다.
unsigned long fwui_get_cur_myrating_val(unsigned short usIndexLocal)
{
	unsigned long ulTempPtr; 
	unsigned long retMyRating;



	if(curTitlePath == MY_QUICKLISTS_TITLE)
	{
		ulTempPtr = pulDBQuickList[usIndexLocal];
	}
	else
	{
		ulTempPtr = db_display->pulResultLinkedListBuffer[usIndexLocal];
	}

	
	retMyRating = get_cur_myrating_val(ulTempPtr);

	return(retMyRating);	
}	


#if 0 //ori
//이것도 리스트에서 사용한다. 
void fwui_write_myrating_value(int curIndex, int rating_val, char mode)
{
	
	unsigned long ulTempPtr; 


	if(mode)		//play중일때 rating 설정
	{
		if (changedMyRatingList && (prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE))
		{
			ulTempPtr = prevTempPtr;
		}
		else if (prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath == MY_QUICKLISTS_TITLE 
				&& doCpmparePrevPtr)
		{
			ulTempPtr = prevTempPtr;
		}
		else
		{
			ulTempPtr = pulPlayPtr[curIndex]; //실제 return value 적용
		}
	
		fwui_SetPlayListValue(ulTempPtr, (unsigned short)rating_val, MYRATING_KEYWD);		

	}
	else		//리스트에서 rating 설정
	{
		
		if(curTitlePath == MY_QUICKLISTS_TITLE)
		{
			ulTempPtr = pulDBQuickList[curIndex];
		}
		else
		{
			ulTempPtr = db_display->pulResultLinkedListBuffer[curIndex]; //실제 return value 적용
		}

		fwui_SetPlayListValue(ulTempPtr, (unsigned short)rating_val, MYRATING_KEYWD);		

		
	}
	
}


void fwui_write_playCount_value(int curIndex)
{
	
	unsigned long ulTempPtr; 

	if(curIndex == 0) return;


	if (changedMyRatingList && (prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE))
	{
		ulTempPtr = prevTempPtr;
	}
	else if (prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath == MY_QUICKLISTS_TITLE 
			&& doCpmparePrevPtr)
	{
		ulTempPtr = prevTempPtr;
	}
	else
	{
		ulTempPtr = pulPlayPtr[curIndex-1];  //실제 return value 적용
	}

	fwui_SetPlayListValue(ulTempPtr, 0, PLAYCOUNT_KEYWD);			
	
}


void fwui_write_buy_value(int curIndex, char mode)
{
	
	unsigned long ulTempPtr; 


	if(mode)		//play중일때 rating 설정
	{
		if (changedMyRatingList && (prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE))
		{
			ulTempPtr = prevTempPtr;
		}
		else if (prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath == MY_QUICKLISTS_TITLE 
				&& doCpmparePrevPtr)
		{
			ulTempPtr = prevTempPtr;
		}
		else
		{
			ulTempPtr = pulPlayPtr[curIndex];  //실제 return value 적용 
		}
	
		fwui_setBuyValue(ulTempPtr);		

	}
	else		//리스트에서 rating 설정
	{
		
		if(curTitlePath == MY_QUICKLISTS_TITLE)
		{
			ulTempPtr = pulDBQuickList[curIndex];
		}
		else
		{
			ulTempPtr = db_display->pulResultLinkedListBuffer[curIndex];  //실제 return value 적용
		}

		fwui_setBuyValue(ulTempPtr);		

		
	}
	
}
#else  // /* Angelo - rowID 변경값 업데이트  */

//이것도 리스트에서 사용한다. 
void fwui_write_myrating_value(int curIndex, int rating_val, char mode)
{
	
	unsigned long ulTempPtr, ulReturnPtr; 


	if(mode)		//play중일때 rating 설정
	{
		if (changedMyRatingList && (prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE))
		{
			ulTempPtr = prevTempPtr;
		}
		else if (prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath == MY_QUICKLISTS_TITLE 
				&& doCpmparePrevPtr)
		{
			ulTempPtr = prevTempPtr;
		}
		else
		{
			ulTempPtr = pulPlayPtr[curIndex]; //실제 return value 적용
		}
	
		ulReturnPtr = fwui_SetPlayListValue(ulTempPtr, (unsigned short)rating_val, MYRATING_KEYWD);		

/* Angelo - rowID 변경값 업데이트  */
		if((ulReturnPtr != ulTempPtr) && ulReturnPtr)
		{
/* quicklist - rowID 변경값 업데이트 */
			{
				int i;
				
				if(fwui_total_quicklist())
				{
					for(i=0;  i < fwui_total_quicklist(); i++)
					{
						if(pulDBQuickList[i] == pulPlayPtr[curIndex])
						{
							pulDBQuickList[i] = ulReturnPtr;
							break;
						}
					}
				}
			}
/* */		
			 pulPlayPtr[curIndex] = ulReturnPtr;
		}
	}
	else		//리스트에서 rating 설정
	{
		
		if(curTitlePath == MY_QUICKLISTS_TITLE)
		{
			ulTempPtr = pulDBQuickList[curIndex];
		}
		else
		{
			ulTempPtr = db_display->pulResultLinkedListBuffer[curIndex]; //실제 return value 적용
		}

		ulReturnPtr = fwui_SetPlayListValue(ulTempPtr, (unsigned short)rating_val, MYRATING_KEYWD);		

/* Angelo - rowID 변경값 업데이트  */
		if((ulReturnPtr != ulTempPtr) && ulReturnPtr)
		{
/* quicklist - rowID 변경값 업데이트 */
			{
				int i;
				if(fwui_total_quicklist())
				{
					for(i=0;  i < fwui_total_quicklist(); i++)
					{
						if(pulDBQuickList[i] == db_display->pulResultLinkedListBuffer[curIndex])
						{
							pulDBQuickList[i] = ulReturnPtr;
							break;
						}
					}
				}
			}
/* */
			db_display->pulResultLinkedListBuffer[curIndex] = ulReturnPtr;
		}	
	}	
}


void fwui_write_playCount_value(int curIndex)
{
	
	unsigned long ulTempPtr, ulReturnPtr; 

	if(curIndex == 0) return;


	if (changedMyRatingList && (prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE))
	{
		ulTempPtr = prevTempPtr;
	}
	else if (prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath == MY_QUICKLISTS_TITLE 
			&& doCpmparePrevPtr)
	{
		ulTempPtr = prevTempPtr;
	}
	else
	{
		ulTempPtr = pulPlayPtr[curIndex-1];  //실제 return value 적용
	}

	ulReturnPtr = fwui_SetPlayListValue(ulTempPtr, 0, PLAYCOUNT_KEYWD);			

/* Angelo - rowID 변경값 업데이트  */
	if((ulReturnPtr != ulTempPtr) && ulReturnPtr)
	{
/* quicklist - rowID 변경값 업데이트 */
		{
			int i;
			if(fwui_total_quicklist())
			{
				for(i=0;  i < fwui_total_quicklist(); i++)
				{
					if(pulDBQuickList[i] == pulPlayPtr[curIndex-1])
					{
						pulDBQuickList[i] = ulReturnPtr;
						break;
					}
				}
			}
		}
/* */	
	
		pulPlayPtr[curIndex-1]= ulReturnPtr;
	}
	
}


void fwui_write_buy_value(int curIndex, char mode)
{
	
	unsigned long ulTempPtr, ulReturnPtr =0; 


	if(mode)		//play중일때 rating 설정
	{
		if (changedMyRatingList && (prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE))
		{
			ulTempPtr = prevTempPtr;
		}
		else if (prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath == MY_QUICKLISTS_TITLE 
				&& doCpmparePrevPtr)
		{
			ulTempPtr = prevTempPtr;
		}
		else
		{
			ulTempPtr = pulPlayPtr[curIndex];  //실제 return value 적용 
		}
	
		ulReturnPtr = fwui_setBuyValue(ulTempPtr);	

/* Angelo - rowID 변경값 업데이트  */
		if((ulReturnPtr != ulTempPtr) && ulReturnPtr)
		{
/* quicklist - rowID 변경값 업데이트 */
			{
				int i;
				if(fwui_total_quicklist())
				{
					for(i=0;  i < fwui_total_quicklist(); i++)
					{
						if(pulDBQuickList[i] == pulPlayPtr[curIndex])
						{
							pulDBQuickList[i] = ulReturnPtr;
							break;
						}
					}
				}
			}
/* */		
			pulPlayPtr[curIndex]= ulReturnPtr;
		}		

	}
	else		//리스트에서 rating 설정
	{
		
		if(curTitlePath == MY_QUICKLISTS_TITLE)  // 실제 이 부분은 필요 없다 -> quicklist는 sub 메뉴에서 리스트 삭제만 존재 
		{
			ulTempPtr = pulDBQuickList[curIndex];
		}
		else
		{
			ulTempPtr = db_display->pulResultLinkedListBuffer[curIndex];  //실제 return value 적용
		}

		ulReturnPtr =fwui_setBuyValue(ulTempPtr);		

/* Angelo - rowID 변경값 업데이트  */
		if((ulReturnPtr != ulTempPtr) && ulReturnPtr)
		{
/* quicklist - rowID 변경값 업데이트 */
			{
				int i;
				if(fwui_total_quicklist())
				{
					for(i=0;  i < fwui_total_quicklist(); i++)
					{
						if(pulDBQuickList[i] == db_display->pulResultLinkedListBuffer[curIndex])
						{
							pulDBQuickList[i] = ulReturnPtr;
							break;
						}
					}
				}
			}
/* */
		
			db_display->pulResultLinkedListBuffer[curIndex]= ulReturnPtr;
		}		
	}
	
}

#endif


unsigned short fwui_addGroupQuicklist(unsigned short total)
{

//	ulCurPos--;//start position 0....
	unsigned short UsefulQuicklistNum, 
					NewStartNum,
					StartNum;
	int i, j, k, l, cValue;

	
	StartNum = ulQuickListNum;

	UsefulQuicklistNum = QUICKLIST_MAXNUM - ulQuickListNum;

	l = 0;
	cValue = 0;
	
	if(total < UsefulQuicklistNum)
	{
		for(i=0; i<total; i++)
		{
			// 이미 추가된 것은 다시  추가 하지 않음  
			for(k=0;  k< StartNum; k++)
			{  
				if( pulDBQuickList[k] == db_display->pulResultLinkedListBuffer[i])
				{
					cValue = 1;
					break;
				}
			}

					
			if(db_display->pulResultLinkedListBuffer[i] != 0 && cValue == 0) 
			{					
				//save quick list	
				pulDBQuickList[StartNum+l] = db_display->pulResultLinkedListBuffer[i];
				ulQuickListNum++;
				l++;
			}

			cValue = 0;
						
		}		
	}
	else
	{
		if(total >= QUICKLIST_MAXNUM)
		{
			for(i=0; i<QUICKLIST_MAXNUM; i++)
			{
				pulDBQuickList[i] = db_display->pulResultLinkedListBuffer[i];
			}

			ulQuickListNum = QUICKLIST_MAXNUM;
		}
		else
		{
			NewStartNum =QUICKLIST_MAXNUM - total;  

			//기존 데이터 백업
			for(i=0; i<NewStartNum; i++)
			{	        
				pulDBQuickList[i] = pulDBQuickList[ulQuickListNum-NewStartNum + i];		
			}


			ulQuickListNum = NewStartNum;  //Total 값을 재 세팅 해야  함 !!!
				
			//새로 채워질 데이터 삽입
			for(j=0; j<total; j++)
			{
				// 이미 추가된 것은 다시  추가 하지 않음  
				for(k=0;  k< NewStartNum; k++)
				{
					if( pulDBQuickList[k] == db_display->pulResultLinkedListBuffer[j])
					{
						cValue = 1;
						break;
					}
				}			
					

				if(db_display->pulResultLinkedListBuffer[j] != 0 && cValue == 0) 
				{										
					//save quick list	
					pulDBQuickList[NewStartNum+l] = db_display->pulResultLinkedListBuffer[j];
					ulQuickListNum++;
					l++;
				}

				cValue = 0;			
			}
		}		
	}

	return TRUE;		
}




void fwui_add_queue(int curIndex)
{
	unsigned short UsefulQueueNum, 
					NewStartNum,
					StartNum;
	int i, errorCounter=0;

	curIndex--;

	{

		StartNum = curTotalTitleListNum;	
		
		if(curTotalTitleListNum  < TOTAL_TITLE_NUM)
		{
			
			if(db_display->pulResultLinkedListBuffer[curIndex] != 0)
			{
				//save quick list	
				pulPlayPtr[StartNum] = db_display->pulResultLinkedListBuffer[curIndex];
			#if( BUGFIX_060614 == 1)
				if (PSound_Type->Play_Mode_SET == playmode_shuffle || PSound_Type->Play_Mode_SET == playmode_shuffle_repeat) 
				{
					ShuffleList[StartNum] = StartNum;
				}
			#endif
			}
			else
			{
				pulPlayPtr[StartNum] = NULL;
				//ERROR
				errorCounter++;
			}
	
			curTotalTitleListNum = curTotalTitleListNum + 1 - errorCounter;			
		}
		else
		{
			//기존 데이터 백업
			for(i=0; i<TOTAL_TITLE_NUM-1; i++)
			{	        
				pulPlayPtr[i] = pulPlayPtr[i+1];		
			}
	
			//새로 채워질 데이터 삽입
			if(db_display->pulResultLinkedListBuffer[curIndex] != 0)
			{
				pulPlayPtr[TOTAL_TITLE_NUM-1] = db_display->pulResultLinkedListBuffer[curIndex];
			}
			else
			{
				pulPlayPtr[TOTAL_TITLE_NUM-1] = NULL;
				//ERROR
				errorCounter++;
			}

			curTotalTitleListNum = TOTAL_TITLE_NUM- errorCounter;			
		}
	}	
}




void fwui_make_empty_queue(void)
{
	int i;

	if(pulPlayPtr != NULL)
	{
		for(i=0; i<TOTAL_TITLE_NUM; i++)
		{
			pulPlayPtr[i] = NULL;
		}
	}

	curTotalTitleListNum = 0;
}


void fwui_addGroupQueueToPlayPtrList(unsigned short total)
{

//	ulCurPos--;//start position 0....
	unsigned short UsefulQueueNum, 
					NewStartNum,
					StartNum;
	int i=0, l= 0, j=0, errorCounter=0;


//	if(fwui_checkPlayingListBuff() != 0)
	{

		StartNum = curTotalTitleListNum;
	
		UsefulQueueNum = TOTAL_TITLE_NUM - curTotalTitleListNum;
	
		
		if(total < UsefulQueueNum)
		{	
			for(i=0; i<total; i++)
			{
				if(db_display->pulResultLinkedListBuffer[i] != 0)
				{	
					//save quick list	
					pulPlayPtr[StartNum+l] = db_display->pulResultLinkedListBuffer[i];

				#if( BUGFIX_060614 == 1)
					if (PSound_Type->Play_Mode_SET == playmode_shuffle || PSound_Type->Play_Mode_SET == playmode_shuffle_repeat) 
					{
						ShuffleList[StartNum+l] = StartNum+l;
					}
				#endif
					l++;
				}
				else
				{
					//ERROR
					errorCounter++;
				}				
			}
	
			curTotalTitleListNum = curTotalTitleListNum + total - errorCounter;	
		}
		else
		{
			if(total >= TOTAL_TITLE_NUM)
			{
				for(i=0; i<TOTAL_TITLE_NUM; i++)
				{
					pulPlayPtr[i] = db_display->pulResultLinkedListBuffer[i];
				}

				curTotalTitleListNum = TOTAL_TITLE_NUM;
			}
			else
			{						
				NewStartNum = TOTAL_TITLE_NUM  - total;
		
				//기존 데이터 백업
				for(i=0; i<NewStartNum; i++)
				{	        
					pulPlayPtr[i] = pulPlayPtr[curTotalTitleListNum - NewStartNum + i];		
				}
		
				//새로 채워질 데이터 삽입
				for(j=0; j<total; j++)
				{
					if(db_display->pulResultLinkedListBuffer[j] != 0)
					{
						pulPlayPtr[NewStartNum+l] = db_display->pulResultLinkedListBuffer[j];

						l++;
					}
					else
					{
						//ERROR
						errorCounter++;
					}
				}
				
				curTotalTitleListNum = TOTAL_TITLE_NUM - errorCounter;
			}
		}
	}

		
}



////////////////////////////////////////////////////////////////
//
//	alarm set functions
//
/////////////////////////////////////////////////////////////////
void fwui_alarm_set_music(unsigned short value)
{
	fwui_AlarmMusic->music = value;
}

void fwui_alarm_set_playlists(unsigned short value)
{
//	fwui_AlarmMusic->playlists = value;
}

void fwui_alarm_set_playlists_rating(unsigned short value)
{
	fwui_AlarmMusic->playlists_rating = value;	
}

void fwui_alarm_set_playlists_myplaylist(unsigned short value)
{
	fwui_AlarmMusic->playlists_myplaylist = value;
}


void fwui_alarm_set_playlists_quicklist(unsigned short value)
{
	fwui_AlarmMusic->playlists_quicklist = value;	
}

void fwui_alarm_set_artist(unsigned short value)
{
	fwui_AlarmMusic->artist = value;	
}


void fwui_alarm_set_album(unsigned short value)
{
	fwui_AlarmMusic->album = value;	
}


void fwui_alarm_set_genre(unsigned short value)
{
	fwui_AlarmMusic->genre = value;	
}

////////
void fwui_alarm_set_artist_DBOffset(unsigned short value)
{
	dbStringCopy(fwui_AlarmMusic->artistDBOffset, (DB_VARCHAR*)dbApiAllocKey(db_display->pulResultLinkedListBuffer[value-1], db_display->ref_memory));
}

void fwui_alarm_set_album_DBOffset(unsigned short value)
{
	dbStringCopy(fwui_AlarmMusic->albumDBOffset, (DB_VARCHAR*)dbApiAllocKey(db_display->pulResultLinkedListBuffer[value-1], db_display->ref_memory));
}

void fwui_alarm_set_genre_DBOffset(unsigned short value)
{
	dbStringCopy(fwui_AlarmMusic->genreDBOffset, (DB_VARCHAR*)dbApiAllocKey(db_display->pulResultLinkedListBuffer[value-1], db_display->ref_memory));
}
///////////

void fwui_alarm_set_title(unsigned short value)
{
	fwui_AlarmMusic->title = value;	
}


void fwui_alarm_set_music_uid(unsigned long value)
{
	fwui_AlarmMusic->uid= value;
}

////////////////////////////////////////////////////////////////
//
//	alarm get functions
//
/////////////////////////////////////////////////////////////////
unsigned short fwui_alarm_get_music(void)
{
	return(fwui_AlarmMusic->music);
}

unsigned short fwui_alarm_get_playlists(void)
{
//	return(fwui_AlarmMusic->playlists);	
	return 0;
}

unsigned short fwui_alarm_get_playlists_rating(void)
{
	return(fwui_AlarmMusic->playlists_rating);	
}

unsigned short fwui_alarm_get_playlists_myplaylist(void)
{
	return(fwui_AlarmMusic->playlists_myplaylist);	
}


unsigned short fwui_alarm_get_playlists_quicklist(void)
{
	return(fwui_AlarmMusic->playlists_quicklist);	
}

unsigned short fwui_alarm_get_artist(void)
{
	return(fwui_AlarmMusic->artist);	
}

unsigned short* fwui_alarm_get_artist_DBOffset(void)
{
	return(fwui_AlarmMusic->artistDBOffset);	
}

unsigned short fwui_alarm_get_album(void)
{
	return(fwui_AlarmMusic->album);	
}

unsigned short* fwui_alarm_get_album_DBOffset(void)
{
	return(fwui_AlarmMusic->albumDBOffset);	
}


unsigned short fwui_alarm_get_genre(void)
{
	return(fwui_AlarmMusic->genre);	
}

unsigned short* fwui_alarm_get_genre_DBOffset(void)
{
	return(fwui_AlarmMusic->genreDBOffset);	
}


unsigned short fwui_alarm_get_title(void)
{
	return(fwui_AlarmMusic->title);	
}

unsigned long  fwui_alarm_get_music_uid(void)
{
	return(fwui_AlarmMusic->uid);	
}


/////////////////////////////////////////////////////////////
//
//			new playlist functions
//
/////////////////////////////////////////////////////////////
unsigned long fwui_playlist_DBSearchClusRowid(unsigned short* mtpTableName,
						      	  				unsigned long   Cluster)
{
	unsigned short*		vColumns[1];
	void*				vConditions[1];
	unsigned long		vResults[1];
	unsigned char		vRef;
	unsigned long		vTotalSearchCount;
	
	vColumns[0] = gDbColumnNameStartClusM;
	
	vConditions[0]	= &Cluster;
	
	vTotalSearchCount = dbApiSearchMS(gDbTableNameMusic, 
									vColumns,		1, 
									vConditions,	1, 
									PExtra_Type->SortMethod+1, 
									vResults,
									#ifdef DB_SEARCH_CHANGED
									1,
									#endif
									&vRef);
				
	if (vTotalSearchCount == 0)
	{
		//error;
		return 0;
	}
	else if (vTotalSearchCount == 1)
	{
		return vResults[0];
	}
	else
	{
		return 0;
	}

}



#if 1
int fwui_playlist_getMusicRowId(unsigned long *playlistArray, unsigned long currentIndex)
{
	int i;
	
	memcpy(db_display->pulResultLinkedListBuffer, playlistArray, sizeof(unsigned long)*PLPTOTALNUM);
	
	GetMusicPlayListsFileNameContents(db_display->pulResultLinkedListBuffer, 
										#ifdef DB_SEARCH_CHANGED
										TOTAL_TITLE_NUM,
										#endif
									  currentIndex, &db_display->ulTotalLinkNumber);

	for( i = 0; i < db_display->ulTotalLinkNumber; i++ )
	{
		db_display->pulResultLinkedListBuffer[i] = GetRowidOfMusicTableByUid(db_display->pulResultLinkedListBuffer[i]);
	}

	db_display->ref_memory = DB_DATA_FILE_LOAD;

	return 1;
}
#else
unsigned int fwui_playlist_plaFileGetStartCluster( unsigned short *CreateFileDirName )
{
	wchar	*Filename = (wchar*)(MTP_BASE_ADDRESS+3072+512);
	tDir	sDIR;
	t_stat 	stSTAT;
	wchar PlaylistPath[] = {'/', 'a', '/', 'P','l','a','y','l','i','s','t','s','/',0};
	unsigned char PlpFileName[MAX_UTF8_PATH_BYTE], *tempPlaylistName;
	unsigned long DelPartLen = 0, TotalPathLen=0, j;


	tempPlaylistName = (unsigned char *)CreateFileDirName;

	DelPartLen = TFS4_strlen("/a/Playlists/");


	TotalPathLen = tfs4_wcslen((unsigned short *)CreateFileDirName);		

	for(j=(DelPartLen*2); j<(TotalPathLen*2); j++)
		PlpFileName[j-(DelPartLen*2)] = tempPlaylistName[j];		


    FSOpenDir( &sDIR, 0, PlaylistPath);	//Pathname = /a/playlist/를 준다. 

	while( FSReadDirStat(&sDIR, Filename, &stSTAT) )
	{
		if( wcscmp(Filename, (wchar*)PlpFileName) == 0 )	//CreateFileDirName에 찾고자 하는 playlist file을 넣는다. 
		{
			FSCloseDir( &sDIR );
			return stSTAT.st_ino;	//실제 startcluster를 얻어오는 곳이다. 
		}	
    }
	FSCloseDir( &sDIR );

	return 0;
}


unsigned int fwui_playlist_musicFileGetCluster( unsigned int StartCluster )
{

	uint32	*lmtpChildClusters;
	uint32	i, FileNum;
	static unsigned long StartClusterEntry, lChildCluster;
	unsigned long rowID;

	lmtpChildClusters = (uint32*)malloc(4*1000);


	memset(lmtpChildClusters, 0, 4*1000);

	StartClusterEntry = (unsigned long)StartCluster;


	FileNum = mtpDBSearchReferencesChild( (unsigned long *)&StartClusterEntry, (unsigned long *)lmtpChildClusters);
							 


	for( i=0; i<FileNum; i++ )
	{
		lChildCluster = lmtpChildClusters[i];

		rowID = fwui_playlist_DBSearchClusRowid(gDbTableNameMusic, lChildCluster);

		db_display->pulResultLinkedListBuffer[i] = rowID;
	}

	db_display->ulTotalLinkNumber = FileNum;
	db_display->ref_memory = DB_DATA_FILE_LOAD;

	free(lmtpChildClusters);
	
	return 1;
	
}
#endif



/*
int fwui_parse_playlistfile(unsigned short* usDBFileName)
{

	int i, RealNum, TotalFileNum=0;
	t_int32 dwFD;
	unsigned long TempNum=0, NoPtrCounts=0;
	unsigned char temSeekdata[512];



	memset(temSeekdata, 0, sizeof(temSeekdata));

//	unsigned long ulResultPtr=0;

	dwFD = tfs4_open(usDBFileName, O_RDONLY);	

	if(dwFD < 0)
	{
		return 0;
	}

	//
	//check 1. 파일크기확인.... 총개수와 파일 크기 비교해서 구한다.
	//
	tfs4_lseek(dwFD, 0, SEEK_SET);
	TotalFileNum = tfs4_lseek(dwFD,  0,  SEEK_END);

	TotalFileNum = (TotalFileNum/4)-1;


	tfs4_lseek(dwFD, 0, SEEK_SET);

//	tfs4_read(dwFD, SeekData, 4);

	tfs4_read(dwFD, temSeekdata, 512);



	TempNum = (SeekData[0] | (SeekData[1]<<8) | (SeekData[2]<<16) | (SeekData[3]<<24));

	if(TempNum != TotalFileNum)
	{
		db_display->ulTotalLinkNumber = TotalFileNum;
	}else
	{
		db_display->ulTotalLinkNumber = TempNum;//save music title total number in playlist file.
	}
	
	RealNum = 0;
	for(i=1; i<db_display->ulTotalLinkNumber+1; i++)//because starting i=1..
	{
		tfs4_lseek(dwFD, 4*i, SEEK_SET);
		tfs4_read(dwFD, SeekData, 4);
		TempNum = (SeekData[0] | (SeekData[1]<<8) | (SeekData[2]<<16) | (SeekData[3]<<24));

		//
		//check 2.  ptr유무확인(존재하지않을 경우 스킵 ).
		//
		db_display->pulResultLinkedListBuffer[RealNum] = TempNum;
		RealNum++;

		
	}

	db_display->ulTotalLinkNumber = db_display->ulTotalLinkNumber - NoPtrCounts;
	db_display->ref_memory = DB_DATA_FILE_LOAD;
	tfs4_close(dwFD);
	return 1;

}
*/


unsigned char fwui_quicklist_filename_exist(int listnum)
{
	unsigned short usDBPathName[MAX_UNICODE_PATH_LENGTH];
	unsigned long ulTempPtr; 


	void*	columns_value[DB_MUSIC_TABLE_COUNT];

	memset(columns_value, NULL, sizeof(columns_value));	//heechul

	if (listnum < 1)
	{
		listnum = 1;	
	}


	ulTempPtr= pulDBQuickList[listnum-1];



	memset(columns_value, NULL, sizeof(columns_value));
	
#if (SUPPORT_MTP_OBJECT_CHANGED_NAME==0)	//2006 2 15 heechul table 구조변화에 따라 동작할수 읻도록 ....	

	columns_value[MUSIC_TABLE_INDEX_FILEPATH] = g_BrowseDirNodeList[0].FileName;
	columns_value[MUSIC_TABLE_INDEX_FILENAME] = g_BrowseDirNodeList[1].FileName;
	dbApiGetRecordCopy(gDbTableNameMusic,
				   	   ulTempPtr,
				   	   columns_value);
#else	//2006 2 15 heechul table 구조변화에 따라 동작할수 읻도록 ....
{
	unsigned long vUId;

	//uid를 받아온다.
	columns_value[MUSIC_TABLE_INDEX_UID] = &vUId;
	dbApiGetRecordCopy(gDbTableNameMusic,
				   	   	ulTempPtr,
				   	   	columns_value);
	FillObjectPath(vUId, g_BrowseDirNodeList[0].FileName, FILE_PATH);		//file path를 받아온다.
	FillObjectName(vUId, g_BrowseDirNodeList[1].FileName);				//file name을 받아온다.
}
#endif	//2006 2 15 heechul table 구조변화에 따라 동작할수 읻도록 ....

	memset(usDBPathName, 0x0, sizeof(usDBPathName));
	TFS4_wcscpy(usDBPathName, g_BrowseDirNodeList[0].FileName);
	TFS4_wcscat(usDBPathName, g_BrowseDirNodeList[1].FileName);

	return db_check_music_file_exist(usDBPathName);

}



char fwui_getNextMusicFileName(unsigned short currentIndex, unsigned short* fileName)
{

	unsigned short usDBPathName[MAX_UNICODE_PATH_LENGTH];
	unsigned short i=0, tempSaveCurIdx=0;
	unsigned long ulTempPtr; 
	int 		chkmode=1;
	void*	columns_value[DB_MUSIC_TABLE_COUNT];
	memset(columns_value, NULL, sizeof(columns_value));	//heechul


	if((PSound_Type->Play_Mode_SET== playmode_shuffle_repeat)  ||
		(PSound_Type->Play_Mode_SET== playmode_repeat))
	{
		chkmode = 0;
	}
		
//	if(get_total_title_num() <=1) //ori
	if((get_total_title_num() <=1) ||((chkmode== 1) && currentIndex >= get_total_title_num()) )
	{
		return FALSE;
	}
	else
	{
		{
			i = GetPrevOrNextPlaybackIndex(PSound_Type->Play_Mode_SET, currentIndex, DB_PLAY_NEXT);

			if( i <= 0 )
			{
				return FALSE;
			}
			
			--i;

			if ((PSound_Type->Play_Mode_SET == playmode_shuffle || PSound_Type->Play_Mode_SET == playmode_shuffle_repeat))
			{ 
				if (!reStartPlayback /*&& curPlayStatus == TRUE*/)// && !startShuffleMode)
				{
					i = get_shuffle_index(i);
				}
			
			}

			ulTempPtr = pulPlayPtr[i];
	
			memset(columns_value, NULL, sizeof(columns_value));

#if (SUPPORT_MTP_OBJECT_CHANGED_NAME==0)	//2006 2 15 heechul table 구조변화에 따라 동작할수 읻도록 ....

			columns_value[MUSIC_TABLE_INDEX_FILEPATH] = g_BrowseDirNodeList[0].FileName;
			columns_value[MUSIC_TABLE_INDEX_FILENAME] = g_BrowseDirNodeList[1].FileName;
	
			dbApiGetRecordCopy(gDbTableNameMusic,
						   	   ulTempPtr,
						   	   columns_value);
#else	//2006 2 15 heechul table 구조변화에 따라 동작할수 읻도록 ....
			{
				unsigned long vUId;

				//uid를 받아온다.
				columns_value[MUSIC_TABLE_INDEX_UID] = &vUId;
				dbApiGetRecordCopy(gDbTableNameMusic,
							   	   	ulTempPtr,
							   	   	columns_value);
				FillObjectPath(vUId, g_BrowseDirNodeList[0].FileName, FILE_PATH);		//file path를 받아온다.
				FillObjectName(vUId, g_BrowseDirNodeList[1].FileName);				//file name을 받아온다.
			}
#endif	//2006 2 15 heechul table 구조변화에 따라 동작할수 읻도록 ....

			if( CheckStringSize(g_BrowseDirNodeList[0].FileName, g_BrowseDirNodeList[1].FileName) < 0 )
			{
				return FALSE;
			}
			
			memset(usDBPathName, 0x0, sizeof(usDBPathName));
			TFS4_wcscpy(usDBPathName, g_BrowseDirNodeList[0].FileName);
			TFS4_wcscat(usDBPathName, g_BrowseDirNodeList[1].FileName);
	
			TFS4_wcscpy(fileName, usDBPathName);
		}
		return TRUE;
	}

}

/////////////////////////////////////////////////////////////////
//
//
//				now playing check
//
//
//////////////////////////////////////////////////////////////////
unsigned short fwui_checkPlayingListBuff(void)
{
	
	if((pulPlayPtr[0] == NULL) || get_total_title_num()==0)
		return 0;
	else
		return 1;
	
}

void fwui_initPlayingListBuff(void)
{
	memset(pulPlayPtr, 0, 0xFA0);
}


int CheckValidAlarmMusicFile(unsigned long uid)
{
	if(FillObjectPath(uid, g_BrowseDirNodeList->FileName,FILE_PATH_AND_FILE_NAME))
	{
		if(db_check_music_file_exist(g_BrowseDirNodeList->FileName) == 1)
		{
			return 1;
		}				
	}

	return 0;
}


int GetPlpHeadMenuCount(void)
{
	return getPlp.ulPlpHeadMenuCount;
}

#if defined(CONFIG_AUDIBLE)
//=================================================================
//	060911woody
//
//	개요:
//	file play시 DB update가 발생하면,
//	Row ID가 바뀌면서 list깨지는 현상 발생. 
//	이를 막기위해서, 
//	DB update이후 다시한번 list들을 갱신해 줄 필요가 있음 
//
//	요약: 
//	oldRowID에 변경되기 전 row ID를 넣으면, 
//	리스트에서 이놈을 찾아 newRowID로 변경함. 
//
//	찝찝:
//  아래 세가지 경우 말고 더 있을까?
//=================================================================
void UpdateRowIDOfCurrentList(unsigned long newRowID, unsigned long oldRowID)
{
	int i;
	
	//
	// 1. quicklist에서 rowID 변경값 업데이트. 
	//
	if(fwui_total_quicklist())
	{
		for(i=0;  i < fwui_total_quicklist(); i++)
		{
			if(pulDBQuickList[i] == oldRowID)
			{
				pulDBQuickList[i] = newRowID;
				break;
			}
		}
	}

	//
	// 2. 현재 play 되고 있는 리스트 update. 이건 music모드에서만 해당되겠지 
	//
	if(get_cur_music_play_position() == MUSIC_MODE)
	{
		for ( i=0 ; i < get_total_title_num() ; i++)
		{
			if(pulPlayPtr[i] == oldRowID)
			{
				pulPlayPtr[i] = newRowID;
				break;
			}
		}
	}

	//
	// 3. 현재 search된 display list update
	//
	for ( i=0 ; i < fwui_totalNum_Search() ; i++)
	{
		if(db_display->pulResultLinkedListBuffer[i] == oldRowID)
		{
			db_display->pulResultLinkedListBuffer[i] = newRowID;
			break;
		}
	}
}


//=====================================================================
// 060918woody, 오더블 파일 판단 조건! 복잡해서 죄송!
//
// CheckFilenameIsAudible()
//	- 현재 play하려는, 또는 play 되고있는 파일의 path name으로 부터
//	  Audible인지 아닌지 판단. 
//	- UI에서 곡을 선택하자 마자 path name은 바뀌기 때문에,
//	  Audible판단 기준으로 적합함. 
//	- 가급적이면 이 함수를 사용할 것. 
//	  
// GetAudibleFileFlag()
//	- 이것 역시 path name의 ext로 판단하지만,
//	  song -> Audible 선택시 문제 발생. 이전 파일을 닫기 때문임. 
//	  따라서 별로 권장하지 않음. 아직 다 없에지 못함. 
//	  
// GetAudibleFileOpenedFlag()
//	- Audible API에서 open success했을 경우 1로 세팅함. 
//	  실제 audible파일이 open되고 close되는 기준으로 사용한다. 
//  why? : When follow below sequence, 
//			check file extention -> close file -> open file. 
//			"AA"가 check된 상태에서 "mp3"를 close하기 위함. 
//===================================================================================
//	060913woody, file확장자가 Audible인지 판단하는 함수. 
//	FilePath_n_Name에는 현재 play하는, 또는 play하려는 file의 full path name이 들어있다. 
//===================================================================================
#include "ui_api.h"
int CheckFilenameIsAudible(void)
{
	t_char *fileExtPtr;
	
	fileExtPtr = find_dot_next_backwarding(FilePath_n_Name);
	
	if(fi_wcsicmp(fileExtPtr, L"AA") == 0)
	{
		// Audible 파일!
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}



#endif   // config_audible


#endif	//#ifdef CONFIG_FWUI


#endif  //#ifdef CONFIG_AVLDB

#endif //_DBKEYFUNC_C		

