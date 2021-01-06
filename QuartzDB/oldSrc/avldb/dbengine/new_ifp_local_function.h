
#define MALLOC_TEMP


#define DISPLAY_LINE_COUNT 7

#include "stringsys.h"

typedef struct  _RECORD
{
#ifdef SUPPORT_MTP_DEVICE
	#ifndef CHANGE_FULLPATH_260CHAR
	    unsigned char aFilePath[512];
	    unsigned char aFileName[512];
	    unsigned char aTitle[512];
	    unsigned char aArtist[512];
	    unsigned char aAlbum[512];
	    unsigned char aGenre[512];
	#else	
		unsigned char aFilePath[MAX_UNICODE_PATH_BYTE];
		unsigned char aFileName[MAX_UNICODE_FILE_NAME_LENGTH_BYTE];
		unsigned char aTitle[MAX_UNICODE_STRING_LENGTH_BYTE];
		unsigned char aArtist[MAX_UNICODE_STRING_LENGTH_BYTE];
		unsigned char aAlbum[MAX_UNICODE_STRING_LENGTH_BYTE];
		unsigned char aGenre[MAX_UNICODE_STRING_LENGTH_BYTE];
	#endif
#else	
	#ifndef CHANGE_FULLPATH_260CHAR
	    unsigned char aFilePath[256];
	    unsigned char aFileName[256];
	    unsigned char aTitle[256];
	    unsigned char aArtist[256];
	    unsigned char aAlbum[256];
	    unsigned char aGenre[256];
	#else
		unsigned char aFilePath[MAX_UNICODE_PATH_LENGTH];
		unsigned char aFileName[MAX_UNICODE_FILE_NAME_LENGTH];
		unsigned char aTitle[MAX_UNICODE_STRING_LENGTH];
		unsigned char aArtist[MAX_UNICODE_STRING_LENGTH];
		unsigned char aAlbum[MAX_UNICODE_STRING_LENGTH];
		unsigned char aGenre[MAX_UNICODE_STRING_LENGTH];
	#endif
#endif	
	unsigned long ulMyRating;
	unsigned long ulPlayCount;
	unsigned long ulLastPlayed;
	unsigned long ulFileFormat;
	unsigned long ulTrackNumber;
	unsigned long ulDrm;
	unsigned long ulReleaseYear;
	unsigned long ulFileSize;
	unsigned long ulTotalTime;
	unsigned long ulSampleRate;
	unsigned long ulBitrate;
	unsigned long ulMyRatingTime;
	unsigned long ulLyrics;
	unsigned char aAdbProID[64];
	unsigned char aAdbBookMark[80];
	unsigned long ulAdbPlaybackPos;
	unsigned long ulAdbCodecType;					
}RECORDDATA;

#define DB_UNICODE_SUPPORT

#ifdef DB_UNICODE_SUPPORT
//	#define db_strlen(x) 2*wcslen((unsigned short*)x)	
	#define db_strlen(x) db_wcslen(x)	
	#define NULL_CHAR_SIZE 2
	#define TAG_CHAR_SIZE	2	//UNICODE인 경우 
#else
	#define db_strlen(x) strlen((char*)x)
	#define NULL_CHAR_SIZE 1
	#define TAG_CHAR_SIZE	1	//LOCAL 인 경우 
#endif


unsigned short NEW_SaveColumnValueToBuffer(RECORDDATA *recData, void* contents[5]);
#if 0
void fwui_setBuyValue(unsigned long ulRow_Id);
#else
unsigned long fwui_setBuyValue(unsigned long ulRow_Id);
#endif

//extern unsigned long db_wcslen(unsigned char *ucData);
extern unsigned short usDBFullDicFileName[];
extern unsigned short usDBFullDataFileName[];
extern unsigned short usDBFullIdxFileName[];
extern void DB_FillNullData(RECORDDATA *recData);
//extern unsigned long NEW_AddID3TagToDB(void);


unsigned long NEW_db_LocalFunc_init( unsigned long param1, unsigned long param2, unsigned long param3, 
						unsigned long param4, unsigned long param5);
unsigned long NEW_GenerateDB(unsigned long ulType);
unsigned char NEW_GetRealData(unsigned char* pucRealPtr, unsigned char* pucBuffer, unsigned long* pusDataSize);
unsigned long SetPlayListValue(unsigned long ulRow_Id, unsigned long* pulValue, unsigned short usKeyType);



////////////////////////////////////////////////////////////////////////////////////////////
//
//
//		fwui functions.
//
//
////////////////////////////////////////////////////////////////////////////////////////////
#if 0  //ori
void fwui_SetPlayListValue(unsigned long ulRow_Id, 
									unsigned short myratingValue,
									unsigned short usKeyType);
#else //angelo - chk
unsigned long  fwui_SetPlayListValue(unsigned long ulRow_Id, 
									unsigned short myratingValue,
									unsigned short usKeyType);

#endif
unsigned long fwui_get_roundTripData(unsigned long ulPtr, unsigned short columnType);






