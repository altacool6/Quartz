#include ".\dbengine\avldb.h"
#include ".\dbengine\new_db_config.h"
#include "db_design.h"


unsigned short gDbTableNameMusic[]				= {'M','u','s','i','c',0};
unsigned short gDbTableNamePhoto[]				= {'P','h','o','t','o',0};
unsigned short gDbTableNameAlbum[]				= {'A','l','b','u','m',0};
unsigned short gDbColumnNameArtist[]			= {'A','r','t','i','s','t',0};
unsigned short gDbColumnNameAlbum[]				= {'A','l','b','u','m',0};
unsigned short gDbColumnNameGenre[]				= {'G','e','n','r','e',0};
unsigned short gDbColumnNameTitle[]				= {'T','i','t','l','e',0};
unsigned short gDbColumnNameFilePath[]			= {'F','i','l','e','P','a','t','h',0};
unsigned short gDbColumnNameFileName[]			= {'F','i','l','e','N','a','m','e',0};
unsigned short gDbColumnNameDuration[]			= {'D','u','r','a','t','i','o','n',0};
unsigned short gDbColumnNameRating[]			= {'R','a','t','i','n','g',0};
unsigned short gDbColumnNameUseCount[]			= {'U','s','e','C','o','u','n','t',0};
unsigned short gDbColumnNameFileFormat[]		= {'F','i','l','e','F','o','r','m','a','t',0};
unsigned short gDbColumnNameTrackNumber[]		= {'T','r','a','c','k','N','u','m','b','e','r',0};
unsigned short gDbColumnNameDrm[]				= {'D','r','m',0};
unsigned short gDbColumnNameLyric[]				= {'L','y','r','i','c',0};
unsigned short gDbColumnNamePurchase[]			= {'P','u','r','c','h','a','s','e',0};
unsigned short gDbColumnNameProtectionStatus[]	= {'P','r','o','t','e','c','t','i','o','n','S','t','a','t','u','s',0};
unsigned short gDbColumnNameSampleRate[]		= {'S','a','m','p','l','e','R','a','t','e',0};
unsigned short gDbColumnNameBitRate[]			= {'B','i','t','R','a','t','e',0};
unsigned short gDbColumnNameChangedFlag[]		= {'C','h','a','n','g','e','d','F','l','a','g',0};
unsigned short gDbColumnNameWidth[]				= {'W','i','d','t','h',0};
unsigned short gDbColumnNameHeight[]			= {'H','e','i','g','h','t',0};
unsigned short gDbColumnNameAudioWAVECodec[]	= {'A','u','d','i','o','W','A','V','E','C','o','d','e','c',0};
unsigned short gDbColumnNameSampleFormat[]		= {'F','o','r','m','a','t',0};
unsigned short gDbColumnNameSampleWidth[]		= {'S','a','m','p','l','e','W','i','d','t','h',0};
unsigned short gDbColumnNameSampleHeight[]		= {'S','a','m','p','l','e','H','e','i','g','h','t',0};
unsigned short gDbColumnNameSampleData[]		= {'S','a','m','p','l','e','D','a','t','a',0};
unsigned short gDbColumnNamePosition[]			= {'S','a','m','p','l','e','P','o','s',0};
unsigned short gDbColumnNameSampleSize[]		= {'S','a','m','p','l','e','S','i','z','e',0};
unsigned short gDbColumnNameStartClusM[]		= {'C','l','u','s','M',0};
unsigned short gDbColumnNameStartClusA[]		= {'C','l','u','s','A',0};
unsigned short gDbColumnNameAlbumArtPos[]		= {'A','l','b','u','m','A','r','t','P','o','s',0};
unsigned short gDbColumnNameOrgReleaseDate[]	= {'O','r','g','R','e','l','e','a','s','e','D','a','t','e',0};	// 060208woody//#if (ORG_RELEASEDATE_SUPPORT == 1)
unsigned short gDbColumnNameAlbumArtist[]		= {'A','l','b','u','m','A','r','t','i','s','t',0};

unsigned short gDbColumnNameUid[]				= {'U','I','D',0};
#ifdef DB_SUPPORT_E10
unsigned short gDbColumnNameRaingTime[]			= {'R','a','t','i','n','g','t','i','m','e',0};
#endif

#if defined(CONFIG_AUDIBLE)
unsigned short gDbColumnNameAudibleCodec[]				= L"gDbColumnNameAudibleCodec";
unsigned short gDbColumnNameAudibleProductID[]			= L"gDbColumnNameAudibleProductID";
unsigned short gDbColumnNameAudibleImageOffset[]		= L"gDbColumnNameAudibleImageOffset";
unsigned short gDbColumnNameAudibleImageSize[]			= L"gDbColumnNameAudibleImageSize";
unsigned short gDbColumnNameAudibleDuration[]			= L"gDbColumnNameAudibleDuration";
unsigned short gDbColumnNameAudiblePlaybackPosition[]	= L"gDbColumnNameAudiblePlaybackPosition";
unsigned short gDbColumnNameAudiblePlayedThrough[]		= L"gDbColumnNameAudiblePlayedThrough";
unsigned short gDbColumnNameAudibleTitleSegment[]		= L"gDbColumnNameAudibleTitleSegment";
unsigned short gDbColumnNameAudibleSection[]			= L"gDbColumnNameAudibleSection";
unsigned short gDbColumnNameAudibleBookmark[]			= L"gDbColumnNameAudibleBookmark";
#endif

//unsigned short *pDateModified;
//unsigned short *pAlbumArtist;
//unsigned short *pBitRateType;
//unsigned short *pOrignalReleaseDate;

unsigned short gDictionaryFileName[17]	= { '/','a','/','S','y','s','t','e','m','/','d', 'b','.', 'd', 'i', 'c', '\0'};
unsigned short gDataFileName[17] 		= { '/','a','/','S','y','s','t','e','m','/','d', 'b','.', 'd', 'a', 't', '\0'};
unsigned short gIndexFileName[17]		= { '/','a','/','S','y','s','t','e','m','/','d', 'b','.', 'i', 'd', 'x', '\0'};

//==========================================================================================
//	
//	주의!!!!! 새로운 column을 추가할때 mtpconfig.h의 MTP_WRITE_SCRATCH_SIZE 를 고려해야 함!
//
//==========================================================================================
void CreateMusicSchma(void)
{
	unsigned short*		pColumnNames[5];

	dbApiCreateTable(gDbTableNameMusic);

	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameArtist/*0*/,			DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameAlbum/*1*/,			DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameGenre/*2*/,			DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameTitle/*3*/,			DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameFilePath/*4*/,			DB_VARCHAR_TYPE);

	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameFileName/*5*/,			DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameDuration/*6*/,			DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameRating/*7*/,			DB_UINT16_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameUseCount/*8*/,			DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameFileFormat/*9*/,		DB_UINT16_TYPE);

	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameTrackNumber/*10*/,		DB_UINT16_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameDrm/*11*/,				DB_UINT8_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameLyric/*12*/,			DB_UINT8_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNamePurchase/*13*/,		DB_UINT8_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameProtectionStatus/*14*/,DB_UINT16_TYPE);
	
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameSampleRate/*15*/,		DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameBitRate/*16*/,			DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameChangedFlag/*17*/,		DB_UINT8_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameAudioWAVECodec/*18*/,	DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameStartClusM/*19*/,		DB_UINT32_TYPE);

	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameStartClusA/*20*/,		DB_UINT32_TYPE);	// 060113 add album link
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameAlbumArtPos/*21*/,		DB_UINT32_TYPE);	// not used
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameOrgReleaseDate /*22*/,	DB_VARCHAR_TYPE);	// 060208woody
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameAlbumArtist	/*23*/,	DB_VARCHAR_TYPE);	

#if SUPPORT_MTP_OBJECT_CHANGED_NAME
	dbApiCreateColumn(gDbTableNameMusic,	gDbColumnNameUid/*24*/,				DB_UINT32_TYPE);	// 060207 Heechul
#endif
#ifndef CONFIG_MTP //UMS일 경우 처리
dbApiCreateColumn(gDbTableNameMusic,             gDbColumnNameRaingTime/*25*/,                         DB_UINT32_TYPE);
#endif


#if defined(CONFIG_AUDIBLE) 
	dbApiCreateColumn(gDbTableNameMusic,    gDbColumnNameAudibleCodec,            	DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,    gDbColumnNameAudibleProductID,        	DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,    gDbColumnNameAudibleImageOffset,      	DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,    gDbColumnNameAudibleImageSize,        	DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,    gDbColumnNameAudibleDuration,         	DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,    gDbColumnNameAudiblePlaybackPosition, 	DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,    gDbColumnNameAudiblePlayedThrough,    	DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,    gDbColumnNameAudibleTitleSegment,     	DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNameMusic,    gDbColumnNameAudibleSection,     		DB_VARCHAR_TYPE);		// 060802, 사용안함. 
	dbApiCreateColumn(gDbTableNameMusic,    gDbColumnNameAudibleBookmark,     		DB_VARCHAR_TYPE);
#endif

	dbApiCreateIndexSingle(gDbTableNameMusic, gDbColumnNameTitle);
	dbApiCreateIndexSingle(gDbTableNameMusic, gDbColumnNameStartClusM);	// 060120 add reference link
	dbApiCreateIndexSingle(gDbTableNameMusic, gDbColumnNameRating);
	dbApiCreateIndexSingle(gDbTableNameMusic, gDbColumnNameChangedFlag);	//060125 심화석 대리의 요청에 따라 김희철 사원이 추가
	dbApiCreateIndexSingle(gDbTableNameMusic, gDbColumnNameTrackNumber);	//060125 심화석 대리의 요청에 따라 김희철 사원이 추가
#if defined(CONFIG_AUDIBLE) 
	dbApiCreateIndexSingle(gDbTableNameMusic, gDbColumnNameFileFormat);		
#endif

#if SUPPORT_MTP_OBJECT_CHANGED_NAME
	dbApiCreateIndexSingle(gDbTableNameMusic, gDbColumnNameUid);		// 060207 Heechul
#endif

/*
	pColumnNames[0] = gDbColumnNameFileName;
	pColumnNames[1] = gDbColumnNameFilePath;

	dbApiCreateIndexComplex(gDbTableNameMusic, pColumnNames, 2);
*/
	{
		pColumnNames[0] = gDbColumnNameGenre;
		pColumnNames[1] = gDbColumnNameArtist;
		pColumnNames[2] = gDbColumnNameTitle;
	}
	dbApiCreateIndexComplex(gDbTableNameMusic, pColumnNames, 3);

	{
		pColumnNames[0] = gDbColumnNameArtist;
		pColumnNames[1] = gDbColumnNameAlbum;
		pColumnNames[2] = gDbColumnNameTrackNumber;			
	}
	dbApiCreateIndexComplex(gDbTableNameMusic, pColumnNames, 3);

// heechul test
	{
		pColumnNames[0] = gDbColumnNameArtist;
		pColumnNames[1] = gDbColumnNameTrackNumber;
	}	
	dbApiCreateIndexComplex(gDbTableNameMusic, pColumnNames, 2);

	{
		pColumnNames[0] = gDbColumnNameAlbum;
		pColumnNames[1] = gDbColumnNameTrackNumber;			
	}
	dbApiCreateIndexComplex(gDbTableNameMusic, pColumnNames, 2);

	{
		pColumnNames[0] = gDbColumnNameArtist;
		pColumnNames[1] = gDbColumnNameTitle;
	}	
	dbApiCreateIndexComplex(gDbTableNameMusic, pColumnNames, 2);


	{
		pColumnNames[0] = gDbColumnNameGenre;
		pColumnNames[1] = gDbColumnNameTitle;
	}
	dbApiCreateIndexComplex(gDbTableNameMusic, pColumnNames, 2);
//
}

void CreateAlbumSchma(void)
{
	unsigned short*		pColumnNames[5];

	dbApiCreateTable(gDbTableNameAlbum);

	dbApiCreateColumn(gDbTableNameAlbum,	gDbColumnNameFilePath,			DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNameAlbum,	gDbColumnNameFileName,			DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNameAlbum,	gDbColumnNameArtist,			DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNameAlbum,	gDbColumnNameAlbum,				DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNameAlbum,	gDbColumnNameGenre,				DB_VARCHAR_TYPE);

	dbApiCreateColumn(gDbTableNameAlbum,	gDbColumnNameSampleFormat,		DB_UINT16_TYPE);
	dbApiCreateColumn(gDbTableNameAlbum,	gDbColumnNameSampleWidth,		DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameAlbum,	gDbColumnNameSampleHeight,		DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameAlbum,	gDbColumnNamePosition,			DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameAlbum,	gDbColumnNameSampleSize,		DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameAlbum,	gDbColumnNameStartClusA,		DB_UINT32_TYPE);	// 060113 add album link
	dbApiCreateColumn(gDbTableNameAlbum, 	gDbColumnNameUid,				DB_UINT32_TYPE);		// 060207 Heechul

	dbApiCreateIndexSingle(gDbTableNameAlbum, gDbColumnNamePosition);

	dbApiCreateIndexSingle(gDbTableNameAlbum, gDbColumnNameStartClusA);

	dbApiCreateIndexSingle(gDbTableNameAlbum, gDbColumnNameUid);		// 060207 Heechul
	
#ifdef CONFIG_MTP
	pColumnNames[0] = gDbColumnNameFileName;
	pColumnNames[1] = gDbColumnNameFilePath;
	dbApiCreateIndexComplex(gDbTableNameAlbum, pColumnNames, 2);
#endif
}


void CreatePhotoSchma(void)
{
	unsigned short*		pColumnNames[5];
	
	dbApiCreateTable(gDbTableNamePhoto);

	dbApiCreateColumn(gDbTableNamePhoto,	gDbColumnNameFilePath,			DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNamePhoto,	gDbColumnNameFileName,			DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNamePhoto,	gDbColumnNameWidth,				DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNamePhoto,	gDbColumnNameHeight,			DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNamePhoto,	gDbColumnNameSampleFormat,		DB_UINT16_TYPE);
	dbApiCreateColumn(gDbTableNamePhoto,	gDbColumnNameSampleWidth,		DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNamePhoto,	gDbColumnNameSampleHeight,		DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNamePhoto,	gDbColumnNamePosition,			DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNamePhoto,	gDbColumnNameSampleSize,		DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNamePhoto,	gDbColumnNameUid,				DB_UINT32_TYPE);		// 060207 Heechul


	dbApiCreateIndexSingle(gDbTableNamePhoto, gDbColumnNamePosition);
	dbApiCreateIndexSingle(gDbTableNamePhoto, gDbColumnNameUid);		// 060207 Heechul

#ifdef CONFIG_MTP
	pColumnNames[0] = gDbColumnNameFileName;
	pColumnNames[1] = gDbColumnNameFilePath;

	dbApiCreateIndexComplex(gDbTableNamePhoto, pColumnNames, 2);
#endif
}




unsigned short gDbTableNameReferences[]		= {'R','e','f','e','r','e','n','c','e','s',0};
unsigned short gDbColumnNameParentCluster[]	= {'P','a','r','e','n','t','C','l','u','s','t','e','r',0};
unsigned short gDbColumnNameChildCluster[]	= {'C','h','i','l','d','C','l','u','s','t','e','r',0};

void CreateReferencesTableSchma(void)
{
	dbApiCreateTable(gDbTableNameReferences);

	dbApiCreateColumn(gDbTableNameReferences,	 gDbColumnNameParentCluster,	DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameReferences,	 gDbColumnNameChildCluster,		DB_UINT32_TYPE);
#if( SUPPORT_FILEFORMAT_REFERENCES == 1 )
	dbApiCreateColumn(gDbTableNameReferences,	 gDbColumnNameFileFormat,		DB_UINT16_TYPE);
	//mtpspec.h에 정의되어있는 타입을 따라감  
#endif
	dbApiCreateIndexSingle(gDbTableNameReferences, gDbColumnNameParentCluster);
	dbApiCreateIndexSingle(gDbTableNameReferences, gDbColumnNameChildCluster);
}


unsigned short gDbTableNameObjects[]		= {'O','b','j','e','c','t','s',0};

unsigned short gDbColumnNameParentUid[] 	= {'P','a','r','e','n','t','U','i','d',0};
unsigned short gDbColumnNameProperties[] 	= {'P','r','o','p','e','r','t','i','e','s',0};
unsigned short gDbColumnNameFileType[] 		= {'F','i','l','e','T','y','p','e',0};
unsigned short gDbColumnNameObjectName[] 	= {'O','b','j','e','c','t','N','a','m','e',0};

unsigned short gDbColumnNameName[]			= {'N','a','m','e',0};
unsigned short gDbColumnNameFileSize[]		= {'F','i','l','e','S','i','z','e',0};
unsigned short gDbColumnNameDateCreated[]	= {'D','a','t','e','C','r','e','a',0};
unsigned short gDbColumnNameRowID[]			= {'R','o','w','I','D',0};
unsigned short gDbColumnNamePUOID1[]			= {'P','U','O','I','D','1',0};
unsigned short gDbColumnNamePUOID2[]			= {'P','U','O','I','D','2',0};


void CreateFilesObjectsSchma(void)
{
	unsigned short*		pColumnNames[5];

	dbApiCreateTable(gDbTableNameObjects);

	dbApiCreateColumn(gDbTableNameObjects,	gDbColumnNameUid,				DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameObjects,	gDbColumnNameParentUid,		DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameObjects,	gDbColumnNameProperties,		DB_UINT8_TYPE);
	dbApiCreateColumn(gDbTableNameObjects,	gDbColumnNameFileType,			DB_UINT16_TYPE);
	dbApiCreateColumn(gDbTableNameObjects,	gDbColumnNameObjectName,		DB_VARCHAR_TYPE);
#ifdef MTP_OBJECT_TABLE_EXTENSION
	dbApiCreateColumn(gDbTableNameObjects,	gDbColumnNameName,			DB_VARCHAR_TYPE);
	dbApiCreateColumn(gDbTableNameObjects,	gDbColumnNameFileSize,			DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameObjects,	gDbColumnNameDateCreated,		DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameObjects,	gDbColumnNameRowID,			DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameObjects,	gDbColumnNamePUOID1,			DB_UINT32_TYPE);
	dbApiCreateColumn(gDbTableNameObjects,	gDbColumnNamePUOID2,			DB_UINT32_TYPE);
#endif

	dbApiCreateIndexSingle(gDbTableNameObjects, gDbColumnNameUid);
	dbApiCreateIndexSingle(gDbTableNameObjects, gDbColumnNameObjectName);
	dbApiCreateIndexSingle(gDbTableNameObjects, gDbColumnNameFileType);
//#ifdef MTP_OBJECT_TABLE_EXTENSION
//	dbApiCreateIndexSingle(gDbTableNameObjects, gDbColumnNameRowID);
//	dbApiCreateIndexSingle(gDbTableNameObjects, gDbColumnNamePUOID);
//#endif

	pColumnNames[0] = gDbColumnNameFileType;
	pColumnNames[1] = gDbColumnNameParentUid;
	pColumnNames[2] = gDbColumnNameProperties;

	dbApiCreateIndexComplex(gDbTableNameObjects, pColumnNames, 3);
}

#if SUPPORT_AVI_FORMAT

unsigned short gDbTableNameVideo[]							= {'V','i','d','e','o',0};
unsigned short gDbColumnNameAudioSampleRate[]				= {'A','u','d','i','o','S','a','m','p','l','e','R','a','t','e',0};
unsigned short gDbColumnNameAudioBitRate[]					= {'A','u','d','i','o','B','i','t','R','a','t','e',0};
//unsigned short gDbColumnNameWidth
//unsigned short gDbColumnNameHeight
unsigned short gDbColumnNameTotalBitRate[]					= {'T','o','t','a','l','B','i','t','R','a','t','e',0};
unsigned short gDbColumnNameVideoBitRate[]					= {'V','i','d','e','o','B','i','t','R','a','t','e',0};
unsigned short gDbColumnNameFramesPerThousandSeconds[]	= {'F','r','a','m','e','s','P','e','r','T','h','o','u','s','a','n','d','S','e','c','o','n','d','s',0};

unsigned short gDbColumnNameVideoFourCCCodec[]				= {'V','i','d','e','o','F','o','u','r','C','C','C','o','d','e','c',0};
//unsigned short gDbColumnNameAudioWAVECodec

void CrateFilesVideoSchma(void)
{
	dbApiCreateTable(gDbTableNameVideo); 																	// 24 + 12
	dbApiCreateColumn(gDbTableNameVideo,	gDbColumnNameAudioSampleRate,			DB_UINT32_TYPE);		// 12 + 32
	dbApiCreateColumn(gDbTableNameVideo,	gDbColumnNameAudioBitRate,					DB_UINT32_TYPE);		// 12 + 26
	dbApiCreateColumn(gDbTableNameVideo,	gDbColumnNameWidth,						DB_UINT32_TYPE);		// 12 + 12
	dbApiCreateColumn(gDbTableNameVideo,	gDbColumnNameHeight,						DB_UINT32_TYPE);		// 12 + 14
	dbApiCreateColumn(gDbTableNameVideo,	gDbColumnNameTotalBitRate,					DB_UINT32_TYPE);		// 12 + 26
	dbApiCreateColumn(gDbTableNameVideo,	gDbColumnNameVideoBitRate,					DB_UINT32_TYPE);		// 12 + 26
	dbApiCreateColumn(gDbTableNameVideo,	gDbColumnNameFramesPerThousandSeconds,	DB_UINT32_TYPE);		// 12 + 50
	dbApiCreateColumn(gDbTableNameVideo,	gDbColumnNameVideoFourCCCodec,			DB_UINT32_TYPE);		// 12 + 34
	dbApiCreateColumn(gDbTableNameVideo,	gDbColumnNameAudioWAVECodec,				DB_UINT32_TYPE);		// 12 + 30
	dbApiCreateColumn(gDbTableNameVideo,	gDbColumnNameUid,							DB_UINT32_TYPE);		// 12 + 8
//	dbApiCreateColumn(gDbTableNameVideo,	gDbColumnNameAlbumUId,						DB_UINT32_TYPE);
	
	
	dbApiCreateIndexSingle(gDbTableNameVideo, gDbColumnNameUid);		// 060207 Heechul
}
#endif
