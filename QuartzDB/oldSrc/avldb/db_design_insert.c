#include "avldb.h"
#include "db_design.h"
#include "db_design_insert.h"

//이 파일에는 db_design.c에 정의되어 있는 테이블의 Data 삽입을 처리하는 함수이다.

#ifdef MTP_OBJECT_TABLE_EXTENSION
unsigned long InsertObjectTableRecord(	unsigned long 		vParentUid, 
										unsigned long 		vUid, 
										unsigned char 	vProperties, 
										unsigned short 	vFileType, 
										unsigned short* 	pObjectName,
										unsigned short*	pAliasName,
										unsigned long		vFileSize,
										unsigned long		vDateCreated,
										unsigned long		vRowid,
										unsigned long 		vPuoid1,
										unsigned long 		vPuoid2)
{
	void* dbContents[DB_OBJECT_TABLE_COUNT];
	memset(dbContents, 0, sizeof(dbContents));
	
	dbContents[OBJECT_TABLE_INDEX_UID]			= &vUid;
	dbContents[OBJECT_TABLE_INDEX_PARENTUID]	= &vParentUid;
	dbContents[OBJECT_TABLE_INDEX_PROPERTIES]	= &vProperties;
	dbContents[OBJECT_TABLE_INDEX_FILETYPE]		= &vFileType;
	dbContents[OBJECT_TABLE_INDEX_NAME]			= pObjectName;
	dbContents[OBJECT_TABLE_INDEX_ALIAS]			= pAliasName;
	dbContents[OBJECT_TABLE_INDEX_FILESIZE]		= &vFileSize;
	dbContents[OBJECT_TABLE_INDEX_DATECREATED]	= &vDateCreated;
	dbContents[OBJECT_TABLE_INDEX_ROWID]		= &vRowid;
	dbContents[OBJECT_TABLE_INDEX_PUOID1]		= &vPuoid1;
	dbContents[OBJECT_TABLE_INDEX_PUOID2]		= &vPuoid2;

	return dbApiRecordInsertMS(gDbTableNameObjects, dbContents, DB_NOT_USING_IN_UPDATE);
}
#else
unsigned long InsertObjectTableRecord(	unsigned long 		vParentUid, 
										unsigned long 		vUid, 
										unsigned char 	vProperties, 
										unsigned short 	vFileType, 
										unsigned short* 	pObjectName)
{
	void* dbContents[DB_OBJECT_TABLE_COUNT];
	memset(dbContents, 0, sizeof(dbContents));
	
	dbContents[OBJECT_TABLE_INDEX_UID]			= &vUid;
	dbContents[OBJECT_TABLE_INDEX_PARENTUID]	= &vParentUid;
	dbContents[OBJECT_TABLE_INDEX_PROPERTIES]	= &vProperties;
	dbContents[OBJECT_TABLE_INDEX_FILETYPE]		= &vFileType;
	dbContents[OBJECT_TABLE_INDEX_NAME]			= pObjectName;

	return dbApiRecordInsertMS(gDbTableNameObjects, dbContents, DB_NOT_USING_IN_UPDATE);
}
#endif
unsigned long InsertReferenceTableRecord(	unsigned long 		vParentUid, 
											unsigned long 		vChildUid, 
											unsigned short 	vFileFormat)
{
	void* dbContents[DB_REFERENCES_TABLE_COUNT];
	memset(dbContents, 0, sizeof(dbContents));
	
	dbContents[REFERENCES_TABLE_INDEX_PARENTCLUSTER]	= &vParentUid;
	dbContents[REFERENCES_TABLE_INDEX_CHILDCLUSTER]		= &vChildUid;
	dbContents[REFERENCES_TABLE_INDEX_FILEFORMAT]		= &vFileFormat;

	return dbApiRecordInsertMS(gDbTableNameReferences, dbContents, DB_NOT_USING_IN_UPDATE);
}

unsigned long InsertPhotoTableRecord(	unsigned long		vWidth, 
										unsigned long		vHeight, 
										unsigned short	vSampleFormat, 
										unsigned long 		vSampleWidth, 
										unsigned long 		vSampleHeight, 
										unsigned long 		vPosition, 
										unsigned long 		vSampleSize, 
										unsigned long 		vUid)
{	
	void* dbContents[DB_PHOTO_TABLE_COUNT];
	memset(dbContents, 0, sizeof(dbContents));
	
	dbContents[PHOTO_TABLE_INDEX_FILEPATH]		= 0;
	dbContents[PHOTO_TABLE_INDEX_FILENAME]		= 0;
	dbContents[PHOTO_TABLE_INDEX_WIDTH]			= &vWidth;
	dbContents[PHOTO_TABLE_INDEX_HEIGHT]		= &vHeight;
	dbContents[PHOTO_TABLE_INDEX_SAMPLEFORMAT]	= &vSampleFormat;
	dbContents[PHOTO_TABLE_INDEX_SAMPLEWIDTH]	= &vSampleWidth;
	dbContents[PHOTO_TABLE_INDEX_SAMPLEHEIGHT]	= &vSampleHeight;
	dbContents[PHOTO_TABLE_INDEX_POSITION]		= &vPosition;
	dbContents[PHOTO_TABLE_INDEX_SAMPLESIZE]	= &vSampleSize;
	dbContents[PHOTO_TABLE_INDEX_UID]			= &vUid;

	return dbApiRecordInsertMS(gDbTableNamePhoto, dbContents, DB_NOT_USING_IN_UPDATE);	
}

unsigned long InsertAlbumTableRecord(	unsigned short* pArtist, 
										unsigned short* pAlbum, 
										unsigned short* pGenre, 
										unsigned short   vSampleFormat, 
										unsigned long 	   vSampleWidth, 
										unsigned long    vSampleHeight, 
										unsigned long    vPosition, 
										unsigned long    vSampleSize, 
										unsigned long    vStartClusA, 
										unsigned long    vUid)
{	
	void* dbContents[DB_ALBUM_TABLE_COUNT];
	memset(dbContents, 0, sizeof(dbContents));
	
	dbContents[ALBUM_TABLE_INDEX_FILEPATH]		= 0;
	dbContents[ALBUM_TABLE_INDEX_FILENAME]		= 0;
	dbContents[ALBUM_TABLE_INDEX_ARTIST]			= pArtist;
	dbContents[ALBUM_TABLE_INDEX_ALBUM]			= pAlbum;
	dbContents[ALBUM_TABLE_INDEX_GENRE]			= pGenre;
	dbContents[ALBUM_TABLE_INDEX_SAMPLEFORMAT]	= &vSampleFormat;
	dbContents[ALBUM_TABLE_INDEX_SAMPLEWIDTH]	= &vSampleWidth;
	dbContents[ALBUM_TABLE_INDEX_SAMPLEHEIGHT]	= &vSampleHeight;
	dbContents[ALBUM_TABLE_INDEX_POSITION]		= &vPosition;
	dbContents[ALBUM_TABLE_INDEX_SAMPLESIZE]	= &vSampleSize;
	dbContents[ALBUM_TABLE_INDEX_STARTCLUSA]	= &vStartClusA;
	dbContents[ALBUM_TABLE_INDEX_UID]			= &vUid;


	return dbApiRecordInsertMS(gDbTableNameAlbum, dbContents, DB_NOT_USING_IN_UPDATE);	
}

unsigned long InsertMusicTableRecord(	unsigned short* pArtist, 
										unsigned short* pAlbum, 
										unsigned short* pGenre, 
										unsigned short* pTitle, 
										unsigned long 	   vDuration, 
										unsigned short   vRating, 
										unsigned long 	   vUseCount, 
										unsigned short   vFileFormat,
										unsigned short   vTrackNumber,
										unsigned char    vDrm,
										unsigned char    vLyric,
										unsigned char    vPurchase,
										unsigned short   vProtectionStatus,
										unsigned long    vSampleRate,
										unsigned long    vBitRate,
										unsigned char    vChangedFlag,
										unsigned long    vAudioWAVECodec,
										unsigned long    vStartClusM,
										unsigned long    vStartClusA,
										unsigned long    vAlbumArtPos,
										unsigned short* pOrgReleaseDate,
										unsigned short* pAlbumArtist,
										unsigned long    vUid,
										unsigned long    vRaingTime)
{	
	void* dbContents[DB_MUSIC_TABLE_COUNT];
	memset(dbContents, 0, sizeof(dbContents));

	dbContents[MUSIC_TABLE_INDEX_ARTIST]				= pArtist;
	dbContents[MUSIC_TABLE_INDEX_ALBUM]				= pAlbum;
	dbContents[MUSIC_TABLE_INDEX_GENRE]				= pGenre;
	dbContents[MUSIC_TABLE_INDEX_TITLE]				= pTitle;
	dbContents[MUSIC_TABLE_INDEX_FILEPATH]			= 0;
	
	dbContents[MUSIC_TABLE_INDEX_FILENAME]			= 0;
	dbContents[MUSIC_TABLE_INDEX_DURATION]			= &vDuration;
	dbContents[MUSIC_TABLE_INDEX_RATING]			= &vRating;
	dbContents[MUSIC_TABLE_INDEX_USECOUNT]			= &vUseCount;
	dbContents[MUSIC_TABLE_INDEX_FILEFORMAT]			= &vFileFormat;
	
	dbContents[MUSIC_TABLE_INDEX_TRACKNUMBER]		= &vTrackNumber;
	dbContents[MUSIC_TABLE_INDEX_DRM]				= &vDrm;
	dbContents[MUSIC_TABLE_INDEX_LYRIC]				= &vLyric;
	dbContents[MUSIC_TABLE_INDEX_PURCHASE]			= &vPurchase;
	dbContents[MUSIC_TABLE_INDEX_PROTECTIONSTATUS]	= &vProtectionStatus;
	
	dbContents[MUSIC_TABLE_INDEX_SAMPLERATE]		= &vSampleRate;
	dbContents[MUSIC_TABLE_INDEX_BITRATE]			= &vBitRate;
	dbContents[MUSIC_TABLE_INDEX_CHANGEDFLAG]		= &vChangedFlag;
	dbContents[MUSIC_TABLE_INDEX_AUDIOWAVECODEC]	= &vAudioWAVECodec;
	dbContents[MUSIC_TABLE_INDEX_STARTCLUSM]		= &vStartClusM;

	dbContents[MUSIC_TABLE_INDEX_STARTCLUSA]		= &vStartClusA;
	dbContents[MUSIC_TABLE_INDEX_ALBUMARTPOS]		= &vAlbumArtPos;
	dbContents[MUSIC_TABLE_INDEX_ORGRELEASEDATE]	= pOrgReleaseDate;
	dbContents[MUSIC_TABLE_INDEX_ALBUMARTIST]		= pAlbumArtist;
	dbContents[MUSIC_TABLE_INDEX_UID]				= &vUid;
//	dbContents[MUSIC_TABLE_INDEX_RATINGTIME]		= &vRaingTime;

	return dbApiRecordInsertMS(gDbTableNameMusic, dbContents, DB_NOT_USING_IN_UPDATE);	
}

