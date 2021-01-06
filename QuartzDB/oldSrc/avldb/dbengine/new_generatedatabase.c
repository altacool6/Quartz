#include "config.h"
#ifdef CONFIG_AVLDB

#include "Basic_typedefs.h"
#include "tfs4_types.h"
#include "tfs4_api.h"
#include "ulist.h"
#include "udir.h"
#include "browser.h"
#include "malloc.h"


//051012
//#include "NEW_db_common_api.h"			//chungwook..20050826..
#include "NEW_ifp_local_function.h"
//#include "AvlDB_ExportAPI.h"
#include "avldb.h"


#ifdef U10_DEBUG
#define	DbgPrintf(x)	UART_Print x
#else
#define	DbgPrintf(x)
#endif

#if 0		//2006 2 11 heechul avldb에서는 unknown_data라고 입력하여 넣지 않는다.
static const unsigned short usUnknownString[] = {'u','n','k','n','o','w','n','_','d','a','t','a',0};
#else
static const unsigned short usUnknownString[] = {0};
#endif
extern struct type_table all_type_tbl[];
#define ALL_TYPE_TBL_NUM	9


enum
{
    CODEC_MP3,
    CODEC_IRM,
    CODEC_WMA,
    CODEC_ASF,
    CODEC_OGG,
#if defined(CONFIG_AUDIBLE)
	CODEC_AA,
#endif
    NUMCODECS
};



//extern int g_nFileHandle;
void DB_FillNullData(RECORDDATA *recData);
extern long ID3TagExtract(t_int32 dwFD, RECORDDATA *recData, unsigned char *pucData);

#ifdef SUPPORT_MTP_DEVICE
extern long WMATagExtract(t_int32 dwFD, RECORDDATA *recData, unsigned char *pucData, unsigned char mtpstrf);
#else
extern long WMATagExtract(t_int32 dwFD, RECORDDATA *recData, unsigned char *pucData);
#endif
extern long OGGTagExtract(t_int32 dwFD, RECORDDATA *recData, unsigned char *pucData);
extern int  AddRecord(int iHandle, RECORDDATA *pData, unsigned char *pBuffer);

//****************************************************************************
//
// Fill null data into RECORD
//
//****************************************************************************
void DB_FillNullData(RECORDDATA *recData)
{
    memcpy(&recData->aArtist[0], (char *)usUnknownString, sizeof(usUnknownString));
    memcpy(&recData->aAlbum[0], (char *)usUnknownString, sizeof(usUnknownString));
    memcpy(&recData->aGenre[0], (char *)usUnknownString, sizeof(usUnknownString));
    recData->ulTrackNumber = 0xffffffff;
    recData->ulReleaseYear = 0;
}

//****************************************************************************
//
// Get DB information and Fill into the record
//
//****************************************************************************
/*static*/ unsigned long 
GetDBInfoFromTAG(unsigned short *usFileName, RECORDDATA *recData,
                 unsigned char *pucBuffer, unsigned long ulCodecType)
{
    t_int32 dwFD;
    unsigned long ulStringLength;
    int i;
    long lReturn;

    //
    //  Get Filename string length 
    //
    ulStringLength = 0;
    while(usFileName[ulStringLength])
    {
        ulStringLength++;
    };
    i = ulStringLength;

    //
    // Find Path position
    //
    while(i)
    {
        i--;
        if(usFileName[i] == (unsigned short)'/')
            break;
    }

    //
    // Insert Path and filename
    //
    if(i)
    {
        memcpy((char *)&recData->aFilePath[0], (char *)usFileName, i*2);

#ifdef FILL_DB_DEBUG
        DbgPrintf("Path:%s",usFileName);
        DbgPrintf("Name:%s",usFileName[i+1]);
#endif
        memcpy((char *)&recData->aFileName[0], (char *)&usFileName[i+1], (ulStringLength-i)*2);
    }
    else
    {
        unsigned short usRoot = (unsigned short)'/';
        
        memcpy((char *)&recData->aFilePath[0], (char *)&usRoot, 2);
        memcpy((char *)&recData->aFileName[0], (char *)usFileName, ulStringLength*2);
    }

    //
    // Open file to extract TAG
    //
    dwFD = tfs4_open(usFileName, O_RDONLY);
    if( dwFD < 0)
    {
#ifdef FILL_DB_DEBUG
        DbgPrintf("FSOpen Error\r\n");
#endif
        return(0);
    }

    //
    // Insert Tag information : Title, Artist, Album, Genre, TrackNo, Release Year
    //    
    switch(ulCodecType)
    {
        case CODEC_MP3:
        {
             if(ID3TagExtract(dwFD, recData, pucBuffer) == -1)
             {
#ifdef FILL_DB_DEBUG
                DbgPrintf("ID3Tag Extraction Error\r\n");
#endif
                tfs4_close(dwFD);
                return(0);
             }
             break;
        }
             
        case CODEC_WMA:
        case CODEC_ASF:
        {
#ifdef SUPPORT_MTP_DEVICE
            lReturn = WMATagExtract(dwFD, recData, pucBuffer, 0);
#else
            lReturn = WMATagExtract(dwFD, recData, pucBuffer);
#endif            
            if(lReturn == -1)
            {
#ifdef FILL_DB_DEBUG
                DbgPrintf("WMATag Extraction Error\r\n");
#endif
                tfs4_close(dwFD);
                return(0);
            }
            else if(lReturn == 0)
            {
                DB_FillNullData(recData);
            }
            
            break;
        }
            
        case CODEC_OGG:
        {
            lReturn = OGGTagExtract(dwFD, recData, pucBuffer);
            if(lReturn == -1)
            {
#ifdef FILL_DB_DEBUG
                DbgPrintf("OGGTag Extraction Error\r\n");
#endif
                tfs4_close(dwFD);
                return(0);
            }
            else if(lReturn == 0)
            {
                DB_FillNullData(recData);
            }
             
            break;
        }

	#if defined(CONFIG_AUDIBLE)
		case CODEC_AA:
		{
            lReturn = AudibleTagExtract(dwFD, recData);
            if(lReturn == -1)
            {
                tfs4_close(dwFD);
                return(0);
            }
            else if(lReturn == 0)
            {
                DB_FillNullData(recData);
            }
            break;
		}
    #endif
	
        default:
        {
            tfs4_close(dwFD);
            return(0);
        }
    }

    //
    // Write "unknown_data" string into empry string field.
    //
    if(!recData->aTitle[0] && !recData->aTitle[1])
    {
        unsigned short *usFile, *usTitle;
        int i;
        
        //
        // if there is no title, insert filename after remove extension.
        //
        usFile = (unsigned short *)&recData->aFileName;
        usTitle = (unsigned short *)pucBuffer;
        i = 0;
        for(i = 0; i < 256; i++)
        {
            if(!usFile[i]) break;
            usTitle[i] = usFile[i];
        }
        
        while(usFile[i] != '.')
        {
            i--;
        }

        memcpy(&recData->aTitle[0], (char *)usFile, i<<1);
    }
#if 0		//2006 2 11 Heechul
    if(!recData->aArtist[0] && !recData->aArtist[1])
        memcpy(&recData->aArtist[0], (char *)usUnknownString, sizeof(usUnknownString));
    if(!recData->aAlbum[0] && !recData->aAlbum[1])
        memcpy(&recData->aAlbum[0], (char *)usUnknownString, sizeof(usUnknownString));
    if(!recData->aGenre[0] && !recData->aGenre[1])
        memcpy(&recData->aGenre[0], (char *)usUnknownString, sizeof(usUnknownString));
#endif
    //
    // File Close
    //
    tfs4_close(dwFD);

    return(1);
}

//****************************************************************************
//
// Get codec number according to extension of file
//
//****************************************************************************
/*static*/ unsigned long get_codec_number(t_char *name)
{
    t_char *ptr;
    
    ptr = find_dot_next_backwarding(name);
    if(!ptr)
    {
        return(NUMCODECS);
    }
    if( ((ptr[0] == 'm') || (ptr[0] == 'M'))
     && ((ptr[1] == 'p') || (ptr[1] == 'P'))
     &&  (ptr[2] == '3') )
    {
        return(CODEC_MP3);
    }
    else if( ((ptr[0] == 'w') || (ptr[0] == 'W'))
          && ((ptr[1] == 'm') || (ptr[1] == 'M'))
          && ((ptr[2] == 'a') || (ptr[2] == 'A')) )
    {
        return(CODEC_WMA);
    }
    else if( ((ptr[0] == 'a') || (ptr[0] == 'A'))
          && ((ptr[1] == 's') || (ptr[1] == 'S'))
          && ((ptr[2] == 'f') || (ptr[2] == 'F')) )
    {
        return(CODEC_ASF);
    }
    else if( ((ptr[0] == 'o') || (ptr[0] == 'O'))
          && ((ptr[1] == 'g') || (ptr[1] == 'G'))
          && ((ptr[2] == 'g') || (ptr[2] == 'G')) )
    {
        return(CODEC_OGG);
    }
#if defined(CONFIG_AUDIBLE)
    else if( ((ptr[0] == 'a') || (ptr[0] == 'A'))
          && ((ptr[1] == 'a') || (ptr[1] == 'A')) )
    {
        return(CODEC_AA);
    }
#endif	
    else
        return(NUMCODECS);
}

//****************************************************************************
//
// Search and add music files in this folder
//
//****************************************************************************

#include "db_design.h"
#include "db3_basictype_define.h"
#include "id3.h"

static int search_music_file(browser_st *current, int total_index, t_char *path)
{
	//char testString1[]="well do";
	//char testString2[100];

	RECORDDATA recData, *tmp_recData;
    int i, dir_exist;
    t_char *nameptr;
    unsigned char *pucBuffer;
    unsigned long ulCodecNumber, ulResult;
	extern unsigned char	pucData[2048];

//	DB_VARCHAR music[]={'m','u','s','i','c',0};
	void* contents[22];


	unsigned long contests6 = 0;
	unsigned long contests7 = 0;
	unsigned long contests8 = 0;

	contents[6] = &contests6;
	contents[7] = &contests7;
	contents[8] = &contests8;


    dir_exist = 0;
  //  pucBuffer = (unsigned char *)malloc(2048);
  	pucBuffer = (unsigned char*)pucData;
    if(!pucBuffer)
    {
        return(-1);
    }
   
    tmp_recData = (RECORDDATA *)malloc(sizeof(RECORDDATA));	
    if(!tmp_recData)
    {
      //  free(pucBuffer);
        return(-1);
    }

    for(i = 1; i <= total_index; i++)
    {
        if(ulgettype(current->ulist_handle, ALL_TYPE_TBL_NUM, all_type_tbl, i) != 20)
        {
    		// Clear RECORDDATA structure
    		memset((char *)&recData, 0, sizeof(RECORDDATA));

            // Get filename
            nameptr = ulgetnamebyindex(current->ulist_handle, i);
            ulCodecNumber = get_codec_number(nameptr);

            // make path+filename
			tfs4_wcscat((t_char *)path, nameptr);

            // extract TAG
            ulResult = GetDBInfoFromTAG(path, &recData, pucBuffer, ulCodecNumber);

	    {
		     unsigned long i=0;
		     for (;i<512;i+=2)
		     	{
	            		if ((recData.aFilePath[i]==0)&&(recData.aFilePath[i+1]==0))
	            		{
	            			recData.aFilePath[i+1] = '/';
					break;
	            		}
		     	}
            }
            
            // cut the file name;
            nameptr = find_slash_next_backwarding(path);
            nameptr[0] = 0;


            
            // ADD record
            if(ulResult)
            {
				      //(recData, contents);
	            memset(contents, 0 , sizeof(contents));

	            contents[0] = recData.aArtist;
	            contents[1] = recData.aAlbum;
	            contents[2] = recData.aGenre;
	            contents[3] = recData.aTitle;
	            contents[4] = recData.aFilePath;
	            contents[5] = recData.aFileName;
	            contents[7] = &recData.ulMyRating;
//	            contents[] = &recData->ulPlayCount;
//	            contents[] = &recData->ulLastPlayed;
	            contents[9] = &recData.ulFileFormat;
	            contents[10] = &recData.ulTrackNumber;
//	            contents[] = &recData->ulDrm;
//	            contents[] = &recData->ulReleaseYear;
//	            contents[] = &recData->ulFileSize;
//	            contents[] = &recData->ulTotalTime;
//	            contents[] = &recData->ulSampleRate;
//	            contents[] = &recData->ulBitrate;
//	            contents[] = &recData->ulMyRatingTime;
//	            contents[] = &recData->ulLyrics;
//	            contents[] = recData->aAdbProID;
//	            contents[] = recData->aAdbBookMark;
//	            contents[] = &recData->ulAdbPlaybackPos;
//	            contents[] = &recData->ulAdbCodecType;                   

				//
				//insert record data to db memory
				//
//				api_insert(music, pApiInsertDataPtr);
				dbApiRecordInsertMS(gDbTableNameMusic, contents, DB_NOT_USING_IN_UPDATE);	//Heechul수정한곳
            }

/*
            if(ulResult)
            {
                if(AddRecord(g_nFileHandle, &recData , (unsigned char *)tmp_recData) == -1)
                {
                    free(tmp_recData);
//                    free(pucBuffer);
                    return(-1);
                }
            }
*/            
        }
        else
        {
            dir_exist = 1;
        }
    }
    
    free(tmp_recData);
  //  free(pucBuffer);
    return(dir_exist);
}

//****************************************************************************
//
// Scan folder to get music database. scan_folder() is recursive function
//
//****************************************************************************
static int scan_folder(t_char *path)
{
    int i, total_index;
    browser_st *current;
    t_char *nameptr;
    
    current = (browser_st *)malloc(sizeof(browser_st));
    if(!current)
    {
        return 0;
    }

    //
    // Open Root Folder
    //
	current->udir_handle = udir_open(path);
	current->ulist_handle = udir_get(current->udir_handle, path, 0, 0, 1);

    // Get total file/dir number in this folder
    total_index = ultotalindex(current->ulist_handle);

    if(search_music_file(current, total_index, path) == 1)
    {
		for(i = 1; i <= total_index; i++)
        {
			if(ulgettype(current->ulist_handle, ALL_TYPE_TBL_NUM, all_type_tbl, i) == 20)
            {
                nameptr = ulgetnamebyindex(current->ulist_handle, i);
                tfs4_wcscat(path, nameptr);

                scan_folder(path);
                
               	nameptr = find_slash_next_backwarding(path);
                nameptr[0] = 0;
            }        
        }
    }

    //
    // free of folder
    //
	if(current->ulist_handle){
		udir_free(current->ulist_handle);
	}

	if(current->udir_handle){
		udir_close(current->udir_handle);
	}

    free(current);
	
	return 1;
}

//****************************************************************************
//
// Scan hole storage, extact TAG and make music dB 
//
//****************************************************************************
unsigned long NEW_AddID3TagToDB(void)
{
    t_char root[257] = { '/', 'a', '/', 0, };


    DbgPrintf(("\n여기로 들어와야 하는데..\n"));
    if(!scan_folder(root))
    {
		return 0;
    }

    return(1);
}

//****************************************************************************
//
// 	파일명 : ifp_local_function.c
//	작성자 : Woody Lee
//	목적   : 한개 파일의 record를 DB에 Add 한다. 
//	사용방식
//	사용파일 : 없음 
//	제한사항	: 
//	오류처리	: 
//	이력 사항 
//			1. 2005_04_27 처음 작성 
//  	Parameters:
//
//	return value:
//		1 - ok
//		0 - error
//
//****************************************************************************
/* Deleted by woong */

#if 0

unsigned long AddRecordedTagToDB(unsigned short * pRecName)
{
//	int i = 0;
	RECORDDATA recData;
	unsigned char *pTagBuff = 0;

//	unsigned short *pTagLen = 0;
	unsigned long ulOffset = 0;
//	unsigned short usTagLen = 0;
//	unsigned short usUnicodeFlag = 0;
	
	#ifdef AddRecordedTagToDB_DEBUG	
	DbgPrintf(("\n AddRecordedTagToDB start in AddRecordedTagToDB \n"));					
	#endif
	
	ulOffset = 0;	
	memset(&recData, 0, sizeof(recData));	
	if(GetTagOfRecordedFile(&recData, pRecName) == 1)
	{		

		#ifdef WATCHDOG
	        Watchdog_Reload();
		#endif

		pTagBuff = (unsigned char*)g_DataBaseTmpEndOfRam;
		g_DataBaseTmpEndOfRam += sizeof(RECORDDATA);
		
		if(AddRecord(g_nFileHandle, &recData , pTagBuff) == -1)
		{
			#ifdef AddRecordedTagToDB_DEBUG				
			DbgPrintf(("\n Add record fail"));				
			#endif
			
			g_DataBaseTmpEndOfRam -= sizeof(RECORDDATA);	
			
			return 0;				
		}
		#ifdef AddRecordedTagToDB_DEBUG				
		else
		{
			DbgPrintf(("\n Addrecord OK %d in AddID3TagToDB  \n", i+1));				
		}
		#endif			
	}
	else
	{
		#ifdef AddRecordedTagToDB_DEBUG				
		DbgPrintf(("\n GetID3TagByEntNum error in AddID3TagToDB \n"));			
		#endif		
		
		return 0;
	}

	#ifdef AddRecordedTagToDB_DEBUG	
	DbgPrintf(("\n step10 in AddRecordedTagToDB \n"));					
	#endif
	g_DataBaseTmpEndOfRam -= sizeof(RECORDDATA);		
	return 1;	
}

#endif
/* end */

#endif	//#ifdef CONFIG_AVLDB
