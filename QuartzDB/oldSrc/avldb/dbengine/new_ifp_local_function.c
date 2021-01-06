#include "config.h"
#ifdef CONFIG_AVLDB

//051012
#include "Basic_typedefs.h"
//#include "ifp_globals.h"

//#include "../config.h"
//#include "../globals.h"

//#ifdef NEW_DB_ENGINE
#include "NEW_db_config.h"
//#include "NEW_db_common_api.h"			//chungwook..20050826..
#include "NEW_ifp_local_function.h"
//#include "AvlDB_ExportAPI.h"
#include "AvlDB.h"
#include "DBKeyFunc.h"
//#endif




#include "mtp_interface_db.h"
#include "..\..\..\db\avldb\dbengine\avldb.h"
#include "..\..\..\db\avldb\dbengine\db3_basictype_define.h"
#include "..\..\..\db\avldb\dbengine\new_db_config.h"
#include "..\..\..\db\avldb\db_design.h"



#ifdef U10_DEBUG
#include "uart_api.h"
#endif

//////////////////////////////////////////////////////////////////////////////////
//
//
//			ifp 관련 function
//
//
//////////////////////////////////////////////////////////////////////////////////

#ifdef U10_DEBUG
#define	DbgPrintf(x)	UART_Print x
#else
#define	DbgPrintf(x)
#endif

//tDir sDir;
unsigned short int usiLocal_Error_num;
unsigned long g_DataBaseTmpEndOfRam;	//tmp arae


//unsigned short usDBFullDataFileName[19] = {'\\','S','y','s','t','e','m','\\','H','1','0','_','J','r','.','D','A','T',0};
//unsigned short usDBFullIdxFileName[19] = {'\\','S','y','s','t','e','m','\\','H','1','0','_','J','r','.','I','D','X',0};


unsigned short usDBFullDicFileName[19] = {'\\','S','y','s','t','e','m','\\','H','1','0','_','J','r','.','D','I','C',0};
unsigned short usDBFullDataFileName[19] = {'\\','S','y','s','t','e','m','\\','H','1','0','_','J','r','.','D','A','T',0};
unsigned short usDBFullIdxFileName[19] = {'\\','S','y','s','t','e','m','\\','H','1','0','_','J','r','.','I','D','X',0};



//unsigned short usDBFullDataFileName[19] = {'\\','S','y','s','t','e','m','\\','H','1','0','_','1','G','.','D','A','T',0};
//unsigned short usDBFullIdxFileName[19] = {'\\','S','y','s','t','e','m','\\','H','1','0','_','1','G','.','I','D','X',0};

unsigned long ulDBFolderLen = 7+1;	//DB Folder + 1 byte(\\  크기)
unsigned short usDBName[7] = {'H','1','0','_','J','r',0};

//static const unsigned short usUnknownString[] = {'u','n','k','n','o','w','n','_','d','a','t','a',0};

static const unsigned short usUnknownString[] = {0xffff,0xff00};
//#define TIME_MEASURE
//#define DB_WRITE_UNIT		512
#define DB_WRITE_UNIT		2048
//#define DB_WRITE_UNIT		4096
//#define DB_WRITE_UNIT		10240
//#define DB_WRITE_UNIT		20480
//#define DB_WRITE_UNIT		102400


//db_common
//extern LINKEDLIST_HEAD* pResultLinkedList[2];
extern int mo_malloc(unsigned long x);
extern void db_api_record_insert(DB_VARCHAR *table_name, void* contents[]);
extern unsigned long NEW_AddID3TagToDB(void);


//////////////////////////////////////////////////////////////////////////////////
//
//
//			COMMON 관련 function
//
//
//////////////////////////////////////////////////////////////////////////////////


//unsigned long NEW_AddID3TagToDB(void);


#ifdef MALLOC_TEST
unsigned long g_ulCount = 0;
unsigned long mo_left = 0;/*(UINT) (mo_end - mo_start); */
unsigned long mo_used = 0;
static unsigned char *mo_start;
static unsigned char *mo_end;
unsigned char ucInitFlag = 0;

void mo_init(unsigned char * mo_start_addr, unsigned long mo_size)
{

  mo_start = mo_start_addr;
  mo_end   = mo_start_addr + mo_size;
  mo_left  = (unsigned long)(mo_end - mo_start);
  mo_used = 0;
  ucInitFlag = 1;
}

int mo_malloc(unsigned long x)
{
	unsigned char *ret = 0;
	unsigned char ucRemain = 0;

	if(ucInitFlag != 1)
	{
		mo_init((unsigned char *)DB_MALLOC_START,  DB_MALLOC_AREA_SIZE);
	}

	if (x > mo_left)
	    return 0;

		/*
	if(x <=2 )
	{
		if(((unsigned long)mo_start +mo_used) % 2 != 0)
		{
			mo_used++;
			mo_left--;
		}
	}
	else		//4byte allign
	{
	*/
		ucRemain = ((unsigned long)mo_start +mo_used) % 4;
		if( ucRemain != 0)
		{
			mo_used += (4-ucRemain);
			mo_left -= (4-ucRemain);
		}		
	//}
	
	ret = mo_start + mo_used;	
	mo_used += x;
	mo_left -= x;	
	return (unsigned long)ret;
}



#endif




//****************************************************************************
//
// NEW DB Engine Functions
//
//****************************************************************************

unsigned long NEW_db_LocalFunc_init( unsigned long param1, unsigned long param2, unsigned long param3, 
						unsigned long param4, unsigned long param5)
{
	unsigned short i;
	
	memset((char*)DICTIONARYFILE_START_ADDRESS, 0,  DBAREA_SIZE );		//SDRAM 초기화 
//	memset(&sFiles, 0, sizeof(sFiles));
//	memset(&sDir, 0, sizeof(sDir));	
//	memset(fileHandle, 0, sizeof(fileHandle));

	#ifdef MY_MALLOC
		// Malloc Table Init; by dckim
		MyMalloc_init( DB_MALLOC_START,  DB_MALLOC_AREA_SIZE );
	#endif
		
	usiLocal_Error_num = 0;
	g_DataBaseTmpEndOfRam = TMP_FREE;
	//memset(pResultLinkedList, 0, sizeof(pResultLinkedList));

	
//	for(i=0; i< KEY_TYPE_SIZE; i++)
	for(i = 0; i < 2; i++)	
	{
//		pResultLinkedList[i] = (LINKEDLIST_HEAD*)malloc(sizeof(LINKEDLIST_HEAD));
//		memset(pResultLinkedList[i], 0, sizeof(LINKEDLIST_HEAD));
//		DbgPrintf(("\n pResultLinkedList[i] : %x \n ", pResultLinkedList[i]));		
	}


//	memset((unsigned char*)DBSORTBUFFER_START_ADDRESS, 0, DBSORTBUFFER_SIZE); //sort할 데이타 버퍼 초기화 
//	g_sResultData.pSortData = (SORTDATA *)((unsigned char*)DBSORTBUFFER_START_ADDRESS);

	#ifdef db_LocalFunc_init_DEBUG
	DbgPrintf(("\n g_sResultData.pSortData 0x%x  in db_LocalFunc_init\n ", g_sResultData.pSortData));	
	DbgPrintf(("\n DBSORTBUFFER_START_ADDRESS 0x%x  in db_LocalFunc_init\n ", DBSORTBUFFER_START_ADDRESS));		
	#endif	
	
//	g_ulEndOfFixDataAddr = 0;	//Fix Data area 의 end address
//	g_ulEndOfVarDataAddr = DBDATAFILE_END_ADDRESS;	//variable data area 의 end address	
	return 1;
}




///////////////////////////////////////////////////////////////////////////////////////////
//
//
//	This Function is maked for only Test
//
//	2005.08.22
//
//
////////////////////////////////////////////////////////////////////////////////////////////
#define NEW_SaveColumnValueToBuffer_debug

unsigned short NEW_SaveColumnValueToBuffer(RECORDDATA *recData, void* contents[6])
{
	
	contents[0] = &(recData->aTitle[0]);
	contents[1] = &(recData->aArtist[0]);
	contents[2] = &(recData->aAlbum[0]);
	contents[3] = &(recData->aGenre[0]);
	contents[4] = &(recData->aFilePath[0]);
	contents[5] = &(recData->aFileName[0]);				 				 				 				 

	return 0;

/*
	unsigned short kk, jj = 0, usTotalCount = 0;
	unsigned short *pTempColumnValueBuffer;		//chungwook..20050822
	unsigned short *temp;
	unsigned short tempShort;
	unsigned short NULLSTRING = 0xFFFE;
	unsigned short COLUMN_SEPERATE = '|';
	
	pTempColumnValueBuffer = (unsigned short*)SAVE_COLUMNVALUE_TEMPBUFFER;
	
	memset(pTempColumnValueBuffer, 0, SAVE_COLUMNVALUE_TEMPBUFFER_SIZE);


	
	//pTempColumnValueBuffer에 타이틀을 집어넣는다.
	{
		temp = (unsigned short*)(&(recData->aTitle[0]));
		{//디버깅용코드
			DB_VARCHAR tempVARCHAR;
			unsigned long i=0;
			debug("\nrecData->aAlbum[0]=");
			while(1){
				db_memcpy(temp+i, &tempVARCHAR, DB_VARCHAR_SIZE);
				debug(" %x ", tempVARCHAR);
				i++;
				if (tempVARCHAR==0) break;
			}
		}
		db_memcpy(temp, &tempShort, sizeof(unsigned short));

		if (tempShort == 0)
		{
			//null값이 들어왔을때 처리
			db_memcpy(&NULLSTRING,		pTempColumnValueBuffer,	DB_VARCHAR_SIZE);
			pTempColumnValueBuffer++;
			db_memcpy(&COLUMN_SEPERATE,	pTempColumnValueBuffer,	DB_VARCHAR_SIZE);
			pTempColumnValueBuffer++;
		}
		else
		{
			//data가 들어왔을때 처리
			unsigned long string_size = writeString(temp, pTempColumnValueBuffer);	//buffet에 삽입한다.
			
			pTempColumnValueBuffer += string_size/2;
			
			db_memcpy(&COLUMN_SEPERATE, pTempColumnValueBuffer-1, sizeof(unsigned short));			
		}

	}

	//pTempColumnValueBuffer에 아티스트을 집어넣는다.
	{
		
		temp = (unsigned short*)(&(recData->aArtist[0]));
		{//디버깅용코드
			DB_VARCHAR tempVARCHAR;
			unsigned long i=0;
			debug("\nrecData->aArtist[0]=");
			while(1){
				db_memcpy(temp+i, &tempVARCHAR, DB_VARCHAR_SIZE);
				debug(" %x ", tempVARCHAR);
				i++;
				if (tempVARCHAR==0) break;
			}
		}
		db_memcpy(temp, &tempShort, sizeof(unsigned short));

		if (tempShort == 0)
		{
			//null값이 들어왔을때 처리
			db_memcpy(&NULLSTRING,		pTempColumnValueBuffer,	DB_VARCHAR_SIZE);
			pTempColumnValueBuffer++;
			db_memcpy(&COLUMN_SEPERATE,	pTempColumnValueBuffer,	DB_VARCHAR_SIZE);
			pTempColumnValueBuffer++;
		}
		else
		{
			//data가 들어왔을때 처리
			unsigned long string_size = writeString(temp, pTempColumnValueBuffer);	//buffet에 삽입한다.
			
			pTempColumnValueBuffer += string_size/2;

			db_memcpy(&COLUMN_SEPERATE, pTempColumnValueBuffer-1, sizeof(unsigned short));			
		}
	}

	//pTempColumnValueBuffer에 앨범을 집어넣는다.
	{
		temp = (unsigned short*)(&(recData->aAlbum[0]));
		{//디버깅용코드
			DB_VARCHAR tempVARCHAR;
			unsigned long i=0;
			debug("\nrecData->aAlbum[0]=");
			while(1){
				db_memcpy(temp+i, &tempVARCHAR, DB_VARCHAR_SIZE);
				debug(" %x ", tempVARCHAR);
				i++;
				if (tempVARCHAR==0) break;
			}
		}
		db_memcpy(temp, &tempShort, sizeof(unsigned short));

		if (tempShort == 0)
		{
			//null값이 들어왔을때 처리
			db_memcpy(&NULLSTRING,		pTempColumnValueBuffer,	DB_VARCHAR_SIZE);
			pTempColumnValueBuffer++;
			db_memcpy(&COLUMN_SEPERATE,	pTempColumnValueBuffer,	DB_VARCHAR_SIZE);
			pTempColumnValueBuffer++;
		}
		else
		{
			//data가 들어왔을때 처리
			unsigned long string_size = writeString(temp, pTempColumnValueBuffer);	//buffet에 삽입한다.
			
			pTempColumnValueBuffer += string_size/2;

			db_memcpy(&COLUMN_SEPERATE,	pTempColumnValueBuffer-1,	DB_VARCHAR_SIZE);
		}
	}
	//pTempColumnValueBuffer에 장르을 집어넣는다.
	{
		temp = (unsigned short*)(&(recData->aGenre[0]));
		{//디버깅용코드
			DB_VARCHAR tempVARCHAR;
			unsigned long i=0;
			debug("\nrecData->aGenre[0]=");
			while(1){
				db_memcpy(temp+i, &tempVARCHAR, DB_VARCHAR_SIZE);
				debug(" %x ", tempVARCHAR);
				i++;
				if (tempVARCHAR==0) break;
			}
		}
		db_memcpy(temp, &tempShort, sizeof(unsigned short));

		if (tempShort == 0)
		{
			//null값이 들어왔을때 처리
			db_memcpy(&NULLSTRING,		pTempColumnValueBuffer,	DB_VARCHAR_SIZE);
			pTempColumnValueBuffer++;
			db_memcpy(&COLUMN_SEPERATE,	pTempColumnValueBuffer,	DB_VARCHAR_SIZE);
			pTempColumnValueBuffer++;
		}
		else
		{
			//data가 들어왔을때 처리
			unsigned long string_size = writeString(temp, pTempColumnValueBuffer);	//buffet에 삽입한다.
			
			pTempColumnValueBuffer += string_size/2;

			db_memcpy(&COLUMN_SEPERATE,	pTempColumnValueBuffer-1,	DB_VARCHAR_SIZE);
		}
	}

	//pTempColumnValueBuffer에 path을 집어넣는다.
	{
		temp = (unsigned short*)(&(recData->aFilePath[0]));
		{//디버깅용코드
			DB_VARCHAR tempVARCHAR;
			unsigned long i=0;
			debug("\nrecData->aFilePath[0]=");
			while(1){
				db_memcpy(temp+i, &tempVARCHAR, DB_VARCHAR_SIZE);
				debug(" %x ", tempVARCHAR);
				i++;
				if (tempVARCHAR==0) break;
			}
		}
		db_memcpy(temp, &tempShort, sizeof(unsigned short));

		if (tempShort == 0)
		{
			//null값이 들어왔을때 처리
			db_memcpy(&NULLSTRING,		pTempColumnValueBuffer,	DB_VARCHAR_SIZE);
			pTempColumnValueBuffer++;
			db_memcpy(&COLUMN_SEPERATE,	pTempColumnValueBuffer,	DB_VARCHAR_SIZE);
			pTempColumnValueBuffer++;
		}
		else
		{
			//data가 들어왔을때 처리
			unsigned long string_size = writeString(temp, pTempColumnValueBuffer);	//buffet에 삽입한다.
			
			pTempColumnValueBuffer += string_size/2;

			db_memcpy(&COLUMN_SEPERATE,	pTempColumnValueBuffer-1,	DB_VARCHAR_SIZE);
		}
	}

	//pTempColumnValueBuffer에 name을 집어넣는다.
	{
		temp = (unsigned short*)(&(recData->aFileName[0]));
		{//디버깅용코드
			DB_VARCHAR tempVARCHAR;
			unsigned long i=0;
			debug("\nrecData->aFileName[0]=");
			while(1){
				db_memcpy(temp+i, &tempVARCHAR, DB_VARCHAR_SIZE);
				debug(" %x ", tempVARCHAR);
				i++;
				if (tempVARCHAR==0) break;
			}
			debug("\n");
		}
		db_memcpy(temp, &tempShort, sizeof(unsigned short));

		if (tempShort == 0)
		{
			//null값이 들어왔을때 처리
			db_memcpy(&NULLSTRING,		pTempColumnValueBuffer,	DB_VARCHAR_SIZE);
			pTempColumnValueBuffer++;
			db_memcpy(&COLUMN_SEPERATE,	pTempColumnValueBuffer,	DB_VARCHAR_SIZE);
			pTempColumnValueBuffer++;
		}
		else
		{
			//data가 들어왔을때 처리
			unsigned long string_size = writeString(temp, pTempColumnValueBuffer);	//buffet에 삽입한다.
			
			//pTempColumnValueBuffer += string_size/2;

			//db_memcpy(pTempColumnValueBuffer-1, &COLUMN_SEPERATE, sizeof(unsigned short));			
		}
	}

	return (unsigned short*)SAVE_COLUMNVALUE_TEMPBUFFER;
*/

/*
	for(kk = 0; kk < 6; kk++)
	{
		switch(kk)
		{
			case 0:		//artist

				while(1)
				{
					if(recData->aArtist[jj] == 0 && recData->aArtist[jj+1] == 0) break;
					jj+=2;
				}

				if(jj == 0)
					break;
				else
					memcpy((unsigned char*)pTempColumnValueBuffer, &(recData->aArtist[0]), jj);
				
				*((unsigned char*)pTempColumnValueBuffer+jj) = 0;		//2byte				
				*((unsigned char*)pTempColumnValueBuffer+jj+1) = 0x7C;		//2byte				
				usTotalCount = jj+2;	

				#if 0				
				{	
					unsigned char kk;
//					DbgPrintf(("\n NEW_SaveColumnValueToBuffer_[1] "));												
//					DbgPrintf(("\n usTotalCount:%d ", usTotalCount));

					kk = 0;
					while(1) {
						
						DbgPrintf(("\n 아티[%d]:%x ", kk, *((unsigned char*)pTempColumnValueBuffer+kk)));							
						if(usTotalCount-1 == kk) break;
						kk++;
					}				
					
				}	
				#endif				

				break;


			case 1:		//album
			
				while(1)
				{
					if(recData->aAlbum[jj] == 0 && recData->aAlbum[jj+1] == 0) break;				
					jj+=2;
				}

			
				if(jj == 0)
					break;
				else
					memcpy((unsigned char*)pTempColumnValueBuffer+usTotalCount, &(recData->aAlbum[0]), jj);

					
				*((unsigned char*)pTempColumnValueBuffer + usTotalCount + jj) = 0;		
				*((unsigned char*)pTempColumnValueBuffer + usTotalCount + jj + 1) = 0x7C;		
				usTotalCount += jj+2;
				
				#if 0				
				{	
					unsigned char kk;
//					DbgPrintf(("\n NEW_SaveColumnValueToBuffer_[2] "));												
//					DbgPrintf(("\n usTotalCount:%d ", usTotalCount));

					kk = usTotalCount-jj-2;
					while(1) {
						
						DbgPrintf(("\n 앨범2[%d]:%x ", kk, *((unsigned char*)pTempColumnValueBuffer+kk)));							
						if(usTotalCount-1 == kk) break;
						kk++;
					}				
					
				}	
				#endif						
				
				break;				

				
			case 2:		//genre
			
				while(1)
				{
					if(recData->aGenre[jj] == 0 && recData->aGenre[jj+1] == 0) break;				
					jj+=2;
				}
				
				if(jj == 0)
					break;
				else
					memcpy((unsigned char*)pTempColumnValueBuffer+usTotalCount, &(recData->aGenre[0]), jj);					

				*((unsigned char*)pTempColumnValueBuffer + usTotalCount + jj) = 0;		
				*((unsigned char*)pTempColumnValueBuffer + usTotalCount + jj + 1) = 0x7C;		
				usTotalCount += jj+2;

				#if 0					
				{	
					unsigned char kk;
//					DbgPrintf(("\n NEW_SaveColumnValueToBuffer_[3] "));												
//					DbgPrintf(("\n usTotalCount:%d ", usTotalCount));

					kk = usTotalCount-jj-2;
					while(1) {
						
						DbgPrintf(("\n 장르[%d]:%x ", kk, *((unsigned char*)pTempColumnValueBuffer+kk)));							
						if(usTotalCount-1 == kk) break;
						kk++;
					}				
					
				}	
				#endif	

				break;



			case 3:		//title

				debug("\n========================================================================\n");
			
				while(1)
				{
					if(recData->aTitle[jj] == 0 && recData->aTitle[jj+1] == 0) break;				

			debug(" %x ", recData->aTitle[jj]);
			debug(" %x ", recData->aTitle[jj+1]);


					jj+=2;
				}
				debug("\n========================================================================\n");
				
				if(jj == 0)
					break;
				else
					memcpy((unsigned char*)pTempColumnValueBuffer+usTotalCount, &(recData->aTitle[0]), jj);	

				*((unsigned char*)pTempColumnValueBuffer+usTotalCount+jj) = 0;		
				*((unsigned char*)pTempColumnValueBuffer+usTotalCount+jj+1) = 0x7C;		
				usTotalCount += jj+2;

				#if 0				
				{	
					unsigned char kk;
//					DbgPrintf(("\n NEW_SaveColumnValueToBuffer_[4] "));												
//					DbgPrintf(("\n usTotalCount:%d ", usTotalCount));

					kk = usTotalCount-jj-2;
					while(1) {
						
						DbgPrintf(("\n 타이[%d] : %x ", kk, *((unsigned char*)pTempColumnValueBuffer+kk)));							
						if(usTotalCount-1 == kk) break;
						kk++;
					}				
					
				}	
				#endif	

				break;

			//
			// save path
			//
			case 4:		
			
				while(1)
				{
					if(recData->aFilePath[jj] == 0 && recData->aFilePath[jj+1] == 0) break;				
					jj+=2;
				}
				
				if(jj == 0)
					break;
				else
					memcpy((unsigned char*)pTempColumnValueBuffer+usTotalCount, &(recData->aFilePath[0]), jj);	

				*((unsigned char*)pTempColumnValueBuffer+usTotalCount+jj) = 0;		
				*((unsigned char*)pTempColumnValueBuffer+usTotalCount+jj+1) = 0x7C;		
				usTotalCount += jj+2;

				#if 0				
				{	
					unsigned char kk;
					DbgPrintf(("\n NEW_SaveColumnValueToBuffer_[path] "));												
					DbgPrintf(("\n usTotalCount:%d ", usTotalCount));

					kk = usTotalCount-jj-2;
					while(1) {
						
						DbgPrintf(("\n path [%d] : %x ", kk, *((unsigned char*)pTempColumnValueBuffer+kk)));							
						if(usTotalCount-1 == kk) break;
						kk++;
					}				
					
				}	
				#endif	

				break;

			//
			// save filename
			//
			case 5:
			
				while(1)
				{
					if(recData->aFileName[jj] == 0 && recData->aFileName[jj+1] == 0) break;				
					jj+=2;
				}
				
				if(jj == 0)
					break;
				else
					memcpy((unsigned char*)pTempColumnValueBuffer+usTotalCount, &(recData->aFileName[0]), jj);	

//				*((unsigned char*)pTempColumnValueBuffer+usTotalCount+jj) = 0;		
//				*((unsigned char*)pTempColumnValueBuffer+usTotalCount+jj+1) = 0x7C;		
				usTotalCount += jj;

				#if 0				
				{	
					unsigned char kk;
					DbgPrintf(("\n NEW_SaveColumnValueToBuffer_[filename] "));												
					DbgPrintf(("\n usTotalCount:%d ", usTotalCount));

					kk = usTotalCount-jj;
					while(1) {
						
						DbgPrintf(("\n filename [%d] : %x ", kk, *((unsigned char*)pTempColumnValueBuffer+kk)));							
						if(usTotalCount-1 == kk) break;
						kk++;
					}				
					
				}	
				#endif	

				break;

		}
		jj = 0;

	}
	
	return (pTempColumnValueBuffer);*/
}	





unsigned long NEW_GenerateDB(unsigned long ulType)
{
	
/*	
	unsigned short db_music[]={'m','u','s','i','c',0};
	void*		contents[6];
	unsigned short no1[]={'n','o','1',0};
	unsigned short boa[]={'B','O','A',0};
	unsigned short album1[]={'#','1',0};
	unsigned short pop[]={'P','o','p',0};
	unsigned short path[]={'/','a','/','b','/','c',0};
	unsigned short name[]={'a','b','c','.','m','p','3',0};
	
	contents[0] = no1;
	contents[1] = boa;
	contents[2] = album1;
	contents[3] = pop;
	contents[4] = path;
	contents[5] = name;

	debug("\n디비 생성 코드\n");
	db_api_record_insert(db_music, contents);

*/
	
#if 1	
	//DB 영역 초기화 
//	memset((char*)DICTIONARYFILE_START_ADDRESS, 0,  DBAREA_SIZE);	//new:200507011..kcw..db의 size는 0x60_0000(6.3MB)


	//Db 파일 로드 
	#ifdef WATCHDOG
	Watchdog_Reload();
	#endif

/*
	if(!LoadDBFile(usDBFullDataFileName, 0, (unsigned char*)cVBNode[0], 512))
	{
		#ifdef GenerateDB_DEBUG
		DbgPrintf(("\n error1 - LoadDBDataFile\n"));	
		#endif
		return 0;
	}
	else
	{
		#ifdef GenerateDB_DEBUG				
		{
			DbgPrintf(("\n ok1 - LoadDBDataFile\n"));			
		}
		#endif				
		
		if(!LoadDBFile(usDBFullIdxFileName, 1,(unsigned char*)cVBNode[0], 512))
		{
			return 0;
		}
		
		#ifdef GenerateDB_DEBUG				
		{
			unsigned long ulTestFileSize;
			memcpy(&ulTestFileSize, (unsigned char*) DBINDEXFILE_START_ADDRESS, 4);
			DbgPrintf(("\n index file size3 %d  in iVBBlockWrite \n", ulTestFileSize));				
			memcpy(&ulTestFileSize, (unsigned char*) DBDATAFILE_START_ADDRESS, 4);
			DbgPrintf(("\n data file size3 %d  in iVBBlockWrite \n", ulTestFileSize));					
			DbgPrintf(("\n ok2 - LoadDBDataFile\n"));			
		}
		#endif	


		#ifdef main_DEBUG				
		DbgPrintf(("\n db file open ok \n"));			
		#endif		

		//DB Generate 된 경우만 tag를 Extract 한다. 
		if((ulType == 0) && !AddID3TagToDB())
		{
//				DisplayColorString((char *)"DB generation error", 0x10,
//									   31, 65, 0, 0);
			#ifdef main_DEBUG				
			DbgPrintf(("\n db Data Add Error in Main \n"));			
			#endif	
		}
		else
		{
			#ifdef main_DEBUG				
			DbgPrintf(("\n db Data Add ok in Main \n"));			
			#endif	
			
		}

	}
	
*/
	{
//		unsigned long BeforeGlobalTimer, AfterGlobalTimer;				
	
		
//		BeforeGlobalTimer = ulGlobalTimer;

		if((ulType == 0) && !NEW_AddID3TagToDB())
		{
//				DisplayColorString((char *)"DB generation error", 0x10,
//									   31, 65, 0, 0);
//			#ifdef main_DEBUG				
			DbgPrintf(("\n db Data Add [Error] in Main \n"));			
//			#endif	
		}
		else
		{
//			#ifdef main_DEBUG				
			DbgPrintf(("\n db Data Add [ok] in Main \n"));			
//			#endif	
			
		}

//		AfterGlobalTimer = ulGlobalTimer;

//		DbgPrintf(("\n\n ===========new_addid3tag=============="));
//		DbgPrintf(("\n\n BeforeGlobalTimer : [%d] ", BeforeGlobalTimer));
//		DbgPrintf(("\n AfterGlobalTimer : [%d] ", AfterGlobalTimer));
//		DbgPrintf(("\n ========================================"));
	}


#endif
		
	return 1;
}




unsigned char NEW_GetRealData(unsigned char* pucRealPtr, unsigned char* pucBuffer, unsigned long* pusDataSize)
{
	
	unsigned short usCounter=0;

//	DbgPrintf(("\n\n <<<< NEW_GetRealData >>>> \n"));

	while(1)
	{
		if(*(pucRealPtr+usCounter) == 0 && *(pucRealPtr+usCounter+1) == 0) 
			break;
		else if(*(pucRealPtr+usCounter) == 0 && *(pucRealPtr+usCounter+1) == 0x7c)
			break;
		usCounter += 2;
//		DbgPrintf(("\n pucRealPtr_[NEW_GetRealData] : %x \n", *pucRealPtr));		
	}

	*pusDataSize = (unsigned long)usCounter;

	if(usCounter == 0)
		return 0;
	else
		memcpy(pucBuffer, pucRealPtr, usCounter);

//	DbgPrintf(("\n  pucRealPtr [NEW_GetRealData] : %x ", pucRealPtr));		
//	DbgPrintf(("\n *pusDataSize [NEW_GetRealData] : %d ", *pusDataSize));

	
	#if 0				
	{	
		unsigned char kk;
//		DbgPrintf(("\n\n <<<< NEW_GetRealData >>>> \n"));												
		
		DbgPrintf(("\n  pucRealPtr [NEW_GetRealData]:%x ", pucRealPtr));		
		DbgPrintf(("\n *pusDataSize [NEW_GetRealData]:%d ", *pusDataSize));

		kk = 0;
		while(kk<*pusDataSize) {
			
			DbgPrintf(("\n pucBuffer[%d]:%x ", kk, *(pucBuffer+kk)));							
//			if(usCounter == kk) break;
			kk++;
		}				
		
	}	
	#endif	
	
	return 1;			
}


//20051128..myrating
extern DB_VARCHAR db_t_music[];
extern DB_VARCHAR db_c_rating[];
extern DB_VARCHAR db_c_playcount[];
extern DB_VARCHAR db_c_lastplaydate[];
//extern struct DirNode* g_BrowseDirNodeList;
unsigned long SetPlayListValue(unsigned long ulRow_Id, unsigned long *pulValue, unsigned short usKeyType)
{		 
	
	
	return 0;
	
#if 0		
	unsigned short* usColumnPtr;
	void   *columns_value[22], *updaterecordbuf[22]; 
	unsigned char* tempBuff;



	usColumnPtr = (unsigned short *)malloc(2);
	tempBuff    = (unsigned char *)malloc(4*1024);


	memset(updaterecordbuf, NULL, sizeof(void *)*22);
	memset(columns_value, NULL, sizeof(void *)*22);
	memset(usColumnPtr, NULL, sizeof(unsigned short));
	memset(tempBuff, NULL, 4*1024);


	*usColumnPtr = *((unsigned short*)pulValue);

	columns_value[7] = usColumnPtr;




	getMtpMusicUpdateBuffer((unsigned long*)updaterecordbuf, (unsigned char *)tempBuff);



	switch(usKeyType)
	{
		case MYRATING_KEYWD:
			usColumnPtr = db_c_rating;
			break;
		
		case PLAYCOUNT_KEYWD:
			usColumnPtr = db_c_playcount;
			break;

		case LASTPLAYED_KEYWD:
			usColumnPtr = db_c_lastplaydate;
			break;	
	}			


	db_api_record_update_with_single_index_key(db_t_music, 
												ulRow_Id,
												usColumnPtr,
												pulValue);



    mtpDBUpdate(gDbTableNameMusic, ulRow_Id, columns_value, updaterecordbuf);

	get_cur_myrating_val(ulRow_Id);

	free(usColumnPtr);
	free(tempBuff);
#endif												
}
												



#if 0 //ori
void fwui_SetPlayListValue(unsigned long ulRow_Id, 
									unsigned short myratingValue,
									unsigned short usKeyType)
#else /* Angelo - rowID 변경값 업데이트  */
unsigned long fwui_SetPlayListValue(unsigned long ulRow_Id, 
									unsigned short myratingValue,
									unsigned short usKeyType)
#endif
{		 
	
	unsigned short myratingColumn = 0;
	unsigned long  usedCount = 0;
	unsigned char  changeFlag = 1;
	

	void   *columns_value[DB_MUSIC_TABLE_COUNT], *updaterecordbuf[DB_MUSIC_TABLE_COUNT]; 
	unsigned char* tempBuff;
	unsigned long retValue=0, tempPlayCount=0;



	tempBuff    = (unsigned char *)malloc(6*1024);


	memset(updaterecordbuf, 0, sizeof(updaterecordbuf));
	memset(columns_value, 0, sizeof(columns_value));
	memset(tempBuff, NULL, 6*1024);

#ifdef CONFIG_MTP
	if(myratingValue == 0) myratingValue = 0;
	else if(myratingValue == 1) myratingValue = 1;
	else if(myratingValue == 2) myratingValue = 25;
	else if(myratingValue == 3) myratingValue = 50;
	else if(myratingValue == 4) myratingValue = 75;
	else if(myratingValue == 5) myratingValue = 99;	
#endif

	switch(usKeyType)
	{
		case MYRATING_KEYWD:

			myratingColumn = myratingValue;
			columns_value[MUSIC_TABLE_INDEX_RATING] = &myratingColumn;
			columns_value[MUSIC_TABLE_INDEX_CHANGEDFLAG] = &changeFlag;				
			getMtpMusicUpdateBuffer((unsigned long*)updaterecordbuf, (unsigned char *)tempBuff);
		       retValue = mtpDBUpdate(gDbTableNameMusic, ulRow_Id, columns_value, updaterecordbuf);

			if(retValue != ulRow_Id)
			{
				UpdateRowIDOfCurrentList(retValue, ulRow_Id);
			}

			//값이 변했는지 체크하는 루틴.....나중에 삭제
#if 0
			fwui_get_roundTripData(ulRow_Id, 1);
#endif
			break;
	
		
		case PLAYCOUNT_KEYWD:

			tempPlayCount = fwui_get_roundTripData(ulRow_Id, 2);

			tempPlayCount++;

			usedCount = tempPlayCount;
			columns_value[MUSIC_TABLE_INDEX_USECOUNT] = &usedCount;
			columns_value[MUSIC_TABLE_INDEX_CHANGEDFLAG] = &changeFlag;				
			getMtpMusicUpdateBuffer((unsigned long*)updaterecordbuf, (unsigned char *)tempBuff);
		    	retValue = mtpDBUpdate(gDbTableNameMusic, ulRow_Id, columns_value, updaterecordbuf);

			if(retValue != ulRow_Id)
			{
				UpdateRowIDOfCurrentList(retValue, ulRow_Id);
			}

			//값이 변했는지 체크하는 루틴.....나중에 삭제
#if 0			
			fwui_get_roundTripData(retValue, 3);
#endif 
			doEndDbFunc= 1;

			break;
	}

	free(tempBuff);

/* Angelo - rowID 변경값 업데이트  */
	return  retValue;
/* End */											
}



unsigned long fwui_get_roundTripData(unsigned long ulPtr, unsigned short columnType)
{

	void*	columns_value[DB_MUSIC_TABLE_COUNT];
	unsigned short retValue=0;
	unsigned long retValue1=0;
	unsigned char retValue2=0;
	unsigned char retValue3=0;
	unsigned short myratingColumn = 0;
	unsigned long usedCount = 0;
	unsigned char  changeFlag = 0;
	unsigned char  buyValue = 0;

	memset(columns_value, NULL, sizeof(columns_value));


	columns_value[MUSIC_TABLE_INDEX_RATING] = &myratingColumn;
	columns_value[MUSIC_TABLE_INDEX_USECOUNT] = &usedCount;
	columns_value[MUSIC_TABLE_INDEX_PURCHASE] = &buyValue;
	columns_value[MUSIC_TABLE_INDEX_CHANGEDFLAG] = &changeFlag;


	dbApiGetRecordCopy(gDbTableNameMusic,
				   	   ulPtr,
				   	   columns_value);

	switch(columnType)
	{
		case 1:		//myrating return

			retValue = *((unsigned short*)columns_value[MUSIC_TABLE_INDEX_RATING]);
			return (unsigned long)retValue;
		
			break;
			
			
		case 2:		//usedcountptr return

			retValue1 = *((unsigned long*)columns_value[MUSIC_TABLE_INDEX_USECOUNT]);
			return (unsigned long)retValue1;
		
			break;
			
			
			
		case 3:		//change flag return
		
			retValue2 = *((unsigned char*)columns_value[MUSIC_TABLE_INDEX_CHANGEDFLAG]);		
			return (unsigned long)retValue2;
					
			break;

		case 4:		//buy flag return
		
			retValue3 = *((unsigned char*)columns_value[MUSIC_TABLE_INDEX_PURCHASE]);		
			return (unsigned long)retValue3;
					
			break;

			
	}

	return 0;
}



unsigned long fwui_setBuyValue(unsigned long ulRow_Id)
{		 
	
	unsigned char  buyValue = 0;
	unsigned char  changeFlag = 1;
	
	
	void   *columns_value[DB_MUSIC_TABLE_COUNT], *updaterecordbuf[DB_MUSIC_TABLE_COUNT]; 
	unsigned char* tempBuff;
	unsigned long retValue=0;

	tempBuff    = (unsigned char *)malloc(4*1024);

	memset(updaterecordbuf, 0, sizeof(updaterecordbuf));
	memset(columns_value, 0, sizeof(columns_value));
	memset(tempBuff, NULL, 4*1024);


	buyValue = 1;
	columns_value[13] = &buyValue;
	columns_value[17] = &changeFlag;
	getMtpMusicUpdateBuffer((unsigned long*)updaterecordbuf, (unsigned char *)tempBuff);
    	retValue = mtpDBUpdate(gDbTableNameMusic, ulRow_Id, columns_value, updaterecordbuf);
		
	if(retValue != ulRow_Id)
	{
		UpdateRowIDOfCurrentList(retValue, ulRow_Id);
	}


#if 0 //ori 
	fwui_get_roundTripData(ulRow_Id, 4);
#endif 
	free(tempBuff);

/* Angelo - rowID 변경값 업데이트  */
	return  retValue;
/* End */													
}

												
#endif	//#ifdef CONFIG_AVLDB							

