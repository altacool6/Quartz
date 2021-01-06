#ifndef __DB_DESIGN_H__
#define __DB_DESIGN_H__

#include "config.h"
//#include "mtpconfig.h"

#if defined(CONFIG_E10REAL)
#define DB_SUPPORT_E10	
#endif
#define	SUPPORT_MTP_OBJECT_CHANGED_NAME		1
#define	SUPPORT_FILEFORMAT_REFERENCES			1
#define	MTP_OBJECT_TABLE_EXTENSION 				1


//==========================================
//060410challen.jo
//
#define SUPPORT_AVI_FORMAT		1

#define ERASE_ALIAS_NAME			//20060419 김희철 : 주석으로 막았던 것을 활성화 시킴


//#ifdef CONFIG_PRELOAD_FW			20060419 김희철 : 주석으로 막아 항상 활성화 시킴
//차후 main firmware에 반영되어야 함
#define BUG_FIX_STRCPY_TO_WCSCPY
#define BUG_FIX_REGENERATING_DB
//#endif								20060419 김희철 : 주석으로 막아 항상 활성화 시킴

#if SUPPORT_AVI_FORMAT
//차후 main firmware에 반영되어야 함
#define DB_STARTUP_CHANGE
#endif

#ifndef DB_STARTUP_CHANGE
#define FSFORMAT_INSERT_SYSTEM_FOLDER
#endif

//==========================================

enum
{
	FUNCTION_FAILED,
	FUNCTION_SUCCESS
};

/*(0x00000918+0x9e+0x19e)*/
#ifdef CONFIG_MTP
	#ifdef MTP_OBJECT_TABLE_EXTENSION
		#if SUPPORT_AVI_FORMAT
			#if defined(CONFIG_AUDIBLE)
				#define DICTIONARY_LOGICAL_SIZE		0x00000e34
			#else
				#define DICTIONARY_LOGICAL_SIZE		0x00000B54
			#endif
		#else
			#define DICTIONARY_LOGICAL_SIZE		(0x00000918+0x9e)
		#endif
	#else
	#define DICTIONARY_LOGICAL_SIZE		0x00000918
	#endif
#else
	#ifdef MTP_OBJECT_TABLE_EXTENSION
		#if SUPPORT_AVI_FORMAT
			#define DICTIONARY_LOGICAL_SIZE		(0x000008da+0x9e+0x19e)
		#else
			#define DICTIONARY_LOGICAL_SIZE		(0x000008da+0x9e)
		#endif
	#else
	#define DICTIONARY_LOGICAL_SIZE		0x000008da
	#endif
#endif


//현재를 스키마 기준으로 3000곡이 드어올 경우 대략 File Size는 12M byte를 잡으면 된다.
#define  CURRENT_SCHMA_ESTIMATED_FILE_SIZE_OF_3000_RECORDS 	0xd00000

//=======TABLE의 각 COLUMN에 대한 DATA TYPE을 정의한다.(BEGIN)=======//
	//==================MUSIC TABLE(BEGIN)================//
	#define	DB_MUSIC_ARTIST_DATATYPE				unsigned short*
	#define	DB_MUSIC_ALBUM_DATATYPE					unsigned short*
	#define	DB_MUSIC_GENRE_DATATYPE					unsigned short*
	#define	DB_MUSIC_TITLE_DATATYPE					unsigned short*
	#define	DB_MUSIC_FILEPATH_DATATYPE				unsigned short*
	#define	DB_MUSIC_FILENAME_DATATYPE				unsigned short*
	#define	DB_MUSIC_DURATION_DATATYPE				unsigned long
	#define	DB_MUSIC_RATING_DATATYPE				unsigned short
	#define	DB_MUSIC_USECOUNT_DATATYPE				unsigned long
	#define	DB_MUSIC_FILEFORMAT_DATATYPE			unsigned short
	#define	DB_MUSIC_TRACKNUMBER_DATATYPE			unsigned short
	#define	DB_MUSIC_DRM_DATATYPE					unsigned char
	#define	DB_MUSIC_LYRIC_DATATYPE					unsigned char
	#define	DB_MUSIC_PURCHASE_DATATYPE				unsigned char
	#define	DB_MUSIC_PROTECTIONSTATUS_DATATYPE	unsigned short
	#define	DB_MUSIC_SAMPLERATE_DATATYPE			unsigned long
	#define	DB_MUSIC_BITRATE_DATATYPE				unsigned long
	#define	DB_MUSIC_CHANGEDFLAG_DATATYPE			unsigned char
	#define	DB_MUSIC_AUDIOWAVECODEC_DATATYPE		unsigned long
	#define	DB_MUSIC_STARTCLUSM_DATATYPE			unsigned long
	#define	DB_MUSIC_STARTCLUSA_DATATYPE			unsigned long
	#define	DB_MUSIC_ALBUMARTPOS_DATATYPE			unsigned long
	#define	DB_MUSIC_UID_DATATYPE					unsigned long
	//==================MUSIC TABLE(E N D)================//

	//==================ALBUM TABLE(BEGIN)================//
	#define	DB_ALBUM_FILEPATH_DATATYPE				unsigned short*
	#define	DB_ALBUM_FILENAME_DATATYPE				unsigned short*
	#define	DB_ALBUM_ARTIST_DATATYPE				unsigned short*
	#define	DB_ALBUM_ALBUM_DATATYPE					unsigned short*
	#define	DB_ALBUM_GENRE_DATATYPE					unsigned short*
	#define	DB_ALBUM_SAMPLEFORMAT_DATATYPE			unsigned short
	#define	DB_ALBUM_SAMPLEWIDTH_DATATYPE			unsigned long
	#define	DB_ALBUM_SAMPLEHEIGHT_DATATYPE			unsigned long
	#define	DB_ALBUM_POSITION_DATATYPE				unsigned long
	#define	DB_ALBUM_SAMPLESIZE_DATATYPE			unsigned long
	#define	DB_ALBUM_STARTCLUSA_DATATYPE			unsigned long
	#define	DB_ALBUM_UID_DATATYPE					unsigned long
	//==================ALBUM TABLE(E N D)================//

	//==================PHOTO TABLE(BEGIN)================//
	#define	DB_PHOTO_FILEPATH_DATATYPE				unsigned short*
	#define	DB_PHOTO_FILENAME_DATATYPE				unsigned short*
	#define	DB_PHOTO_WIDTH_DATATYPE					unsigned long
	#define	DB_PHOTO_HEIGHT_DATATYPE				unsigned long
	#define	DB_PHOTO_SAMPLEFORMAT_DATATYPE			unsigned short
	#define	DB_PHOTO_SAMPLEWIDTH_DATATYPE			unsigned long
	#define	DB_PHOTO_SAMPLEHEIGHT_DATATYPE			unsigned long
	#define	DB_PHOTO_POSITION_DATATYPE				unsigned long
	#define	DB_PHOTO_SAMPLESIZE_DATATYPE			unsigned long
	#define	DB_PHOTO_UID_DATATYPE					unsigned long
	//==================PHOTO TABLE(E N D)================//
	
//=======TABLE의 각 COLUMN에 대한 DATA TYPE을 정의한다.(E N D)=======//



//=======TABLE의 각 COLUMN에 대한 INDEX을 정의한다.(BEGIN)=======//
	//==========MUSIC TABLE(BEGIN)=========//
	enum MUSIC_TABLE_INDEX
	{
	/*0*/	MUSIC_TABLE_INDEX_ARTIST,
	/*1*/	MUSIC_TABLE_INDEX_ALBUM,
	/*2*/	MUSIC_TABLE_INDEX_GENRE,
	/*3*/	MUSIC_TABLE_INDEX_TITLE,
	/*4*/	MUSIC_TABLE_INDEX_FILEPATH,
	
	/*5*/	MUSIC_TABLE_INDEX_FILENAME,
	/*6*/	MUSIC_TABLE_INDEX_DURATION,
	/*7*/	MUSIC_TABLE_INDEX_RATING,
	/*8*/	MUSIC_TABLE_INDEX_USECOUNT,
	/*9*/	MUSIC_TABLE_INDEX_FILEFORMAT,
	
	/*10*/	MUSIC_TABLE_INDEX_TRACKNUMBER,
	/*11*/	MUSIC_TABLE_INDEX_DRM,
	/*12*/	MUSIC_TABLE_INDEX_LYRIC,
	/*13*/	MUSIC_TABLE_INDEX_PURCHASE,
	/*14*/	MUSIC_TABLE_INDEX_PROTECTIONSTATUS,
	
	/*15*/	MUSIC_TABLE_INDEX_SAMPLERATE,
	/*16*/	MUSIC_TABLE_INDEX_BITRATE,
	/*17*/	MUSIC_TABLE_INDEX_CHANGEDFLAG,
	/*18*/	MUSIC_TABLE_INDEX_AUDIOWAVECODEC,
	/*19*/	MUSIC_TABLE_INDEX_STARTCLUSM,

	/*20*/	MUSIC_TABLE_INDEX_STARTCLUSA,
	/*21*/	MUSIC_TABLE_INDEX_ALBUMARTPOS,
	/*22*/	MUSIC_TABLE_INDEX_ORGRELEASEDATE,		// 060208woody
	/*23*/	MUSIC_TABLE_INDEX_ALBUMARTIST,		

	#if SUPPORT_MTP_OBJECT_CHANGED_NAME
	/*22*/	MUSIC_TABLE_INDEX_UID,
	#endif
	
	#ifndef CONFIG_MTP	//ums일 경우 추가
	MUSIC_TABLE_INDEX_RATINGTIME,
	#endif

#if defined(CONFIG_AUDIBLE)
			MUSIC_TABLE_INDEX_AUDIBLE_CODEC,
			MUSIC_TABLE_INDEX_AUDIBLE_PRODUCTID,
			MUSIC_TABLE_INDEX_AUDIBLE_IMAGEOFFSET,
			MUSIC_TABLE_INDEX_AUDIBLE_IMAGESIZE,
			MUSIC_TABLE_INDEX_AUDIBLE_IMAGEWH,
			MUSIC_TABLE_INDEX_AUDIBLE_PLAYBACKPOS,
			MUSIC_TABLE_INDEX_AUDIBLE_PLAYTHROUTH,
			MUSIC_TABLE_INDEX_AUDIBLE_TITLESEGMENT,
			MUSIC_TABLE_INDEX_AUDIBLE_SECTION,
			MUSIC_TABLE_INDEX_AUDIBLE_BOOKMARK,
#endif
	
	/*23*/	DB_MUSIC_TABLE_COUNT
	};
	//==========MUSIC TABLE(E N D)=========//

	//==========ALBUM TABLE(BEGIN)=========//
	enum ALBUM_TABLE_INDEX
	{
	/*0*/	ALBUM_TABLE_INDEX_FILEPATH,
	/*1*/	ALBUM_TABLE_INDEX_FILENAME,
	/*2*/	ALBUM_TABLE_INDEX_ARTIST,
	/*3*/	ALBUM_TABLE_INDEX_ALBUM,
	/*4*/	ALBUM_TABLE_INDEX_GENRE,
	/*5*/	ALBUM_TABLE_INDEX_SAMPLEFORMAT,
	/*6*/	ALBUM_TABLE_INDEX_SAMPLEWIDTH,
	/*7*/	ALBUM_TABLE_INDEX_SAMPLEHEIGHT,
	/*8*/	ALBUM_TABLE_INDEX_POSITION,
	/*9*/	ALBUM_TABLE_INDEX_SAMPLESIZE,
	/*10*/	ALBUM_TABLE_INDEX_STARTCLUSA,
	#if SUPPORT_MTP_OBJECT_CHANGED_NAME
	/*11*/	ALBUM_TABLE_INDEX_UID,
	/*12*/	DB_ALBUM_TABLE_COUNT
	#else
	/*11*/	DB_ALBUM_TABLE_COUNT
	#endif
	};
	//==========ALBUM TABLE(E N D)=========//

	//==========PHOTO TABLE(BEGIN)=========//
	enum PHOTO_TABLE_INDEX
	{
	/*0*/	PHOTO_TABLE_INDEX_FILEPATH,
	/*1*/	PHOTO_TABLE_INDEX_FILENAME,
	/*2*/	PHOTO_TABLE_INDEX_WIDTH,
	/*3*/	PHOTO_TABLE_INDEX_HEIGHT,
	/*4*/	PHOTO_TABLE_INDEX_SAMPLEFORMAT,
	/*5*/	PHOTO_TABLE_INDEX_SAMPLEWIDTH,
	/*6*/	PHOTO_TABLE_INDEX_SAMPLEHEIGHT,
	/*7*/	PHOTO_TABLE_INDEX_POSITION,
	/*8*/	PHOTO_TABLE_INDEX_SAMPLESIZE,
	#if SUPPORT_MTP_OBJECT_CHANGED_NAME
	/*9*/	PHOTO_TABLE_INDEX_UID,
	/*10*/	DB_PHOTO_TABLE_COUNT
	#else
	/*9*/	DB_PHOTO_TABLE_COUNT
	#endif
	};
	//==========PHOTO TABLE(E N D)=========//

	//==========REFERENCES TABLE(BEGIN)=========//
	enum REFERENCES_TABLE_INDEX
	{
	/*0*/	REFERENCES_TABLE_INDEX_PARENTCLUSTER,
	/*1*/	REFERENCES_TABLE_INDEX_CHILDCLUSTER,
#if( SUPPORT_FILEFORMAT_REFERENCES == 1)
	/*2*/	REFERENCES_TABLE_INDEX_FILEFORMAT,
#endif
	/*3*/	DB_REFERENCES_TABLE_COUNT
	};
	//==========REFERENCES TABLE(E N D)=========//

	//==========OBJECT TABLE(BEGIN)=========//
	enum OBJECT_TABLE_INDEX
	{
	/*0*/	OBJECT_TABLE_INDEX_UID,
	/*1*/	OBJECT_TABLE_INDEX_PARENTUID,
	/*2*/	OBJECT_TABLE_INDEX_PROPERTIES,
	/*3*/	OBJECT_TABLE_INDEX_FILETYPE,
	/*4*/	OBJECT_TABLE_INDEX_NAME,
	#ifdef MTP_OBJECT_TABLE_EXTENSION
	/*5*/	OBJECT_TABLE_INDEX_ALIAS,
	/*6*/	OBJECT_TABLE_INDEX_FILESIZE,
	/*7*/	OBJECT_TABLE_INDEX_DATECREATED,
	/*8*/	OBJECT_TABLE_INDEX_ROWID,
	/*9*/	OBJECT_TABLE_INDEX_PUOID1,
	/*10*/	OBJECT_TABLE_INDEX_PUOID2,
	#endif
	/*11*/	DB_OBJECT_TABLE_COUNT
	};
	//==========OBJECT TABLE(E N D)=========//
//=======TABLE의 각 COLUMN에 대한 INDEX을 정의한다.(E N D)=======//


//MTP쪽의 데이터를 참조합니다.
enum _REFERENCE_TABLE_FILE_FORMAT_
{
	REFERENCE_TABLE_FILE_FORMAT_Undefined		= 0x3000,	// undefiend object
	REFERENCE_TABLE_FILE_FORMAT_Association,					// for PTP compability. MTP uses ObjectReference
	REFERENCE_TABLE_FILE_FORMAT_Script,						// }
	REFERENCE_TABLE_FILE_FORMAT_Execuable,					// } Device-specific
	REFERENCE_TABLE_FILE_FORMAT_Text,						//
	REFERENCE_TABLE_FILE_FORMAT_HTML,						// Hypertext Markup Language file (text)
	REFERENCE_TABLE_FILE_FORMAT_DPOF,						// Digital Print Order Format File (text)
	REFERENCE_TABLE_FILE_FORMAT_AIFF,						//
	REFERENCE_TABLE_FILE_FORMAT_WAV,						//0x3008	//
	REFERENCE_TABLE_FILE_FORMAT_MP3,						//
	REFERENCE_TABLE_FILE_FORMAT_AVI,						//
	REFERENCE_TABLE_FILE_FORMAT_MPEG,						//
	REFERENCE_TABLE_FILE_FORMAT_ASF,						// Microsoft Advanced Streaming Format
	REFERENCE_TABLE_FILE_FORMAT_defined		= 0x3800,	// Unknown image object
	REFERENCE_TABLE_FILE_FORMAT_EXIF_JPEG,					// Exchangeable File Format, JEIDA standard
	REFERENCE_TABLE_FILE_FORMAT_TIFF_EP,					// TIFF for Electronic Photography
	REFERENCE_TABLE_FILE_FORMAT_FlashPix,					// Structured Storage Image Format
	REFERENCE_TABLE_FILE_FORMAT_BMP,						// Microsoft Windows Bitmap file
	REFERENCE_TABLE_FILE_FORMAT_CIFF,						// Canon Camera Image File Format
	REFERENCE_TABLE_FILE_FORMAT_GIF			= 0x3807,	// Graphics Interchange Format
	REFERENCE_TABLE_FILE_FORMAT_JFIF,						// JPEG File Interchange Format
	REFERENCE_TABLE_FILE_FORMAT_PCD,						// PhotoCD Image Pac
	REFERENCE_TABLE_FILE_FORMAT_PICT,						// Quickdraw Image Format
	REFERENCE_TABLE_FILE_FORMAT_PNG,						// Portable Network Graphics
	REFERENCE_TABLE_FILE_FORMAT_TIFF			= 0x380D,	// Tag Image File Format
	REFERENCE_TABLE_FILE_FORMAT_TIFF_IT,					// TIFF for Information Technology (graphic arts)
	REFERENCE_TABLE_FILE_FORMAT_JP2,						// JPEG2000 Baseline File Format
	REFERENCE_TABLE_FILE_FORMAT_JPX,						// JPEG2000 Extended File Format
	REFERENCE_TABLE_FILE_FORMAT_UndefinedFirmware	= 0xB802,
	REFERENCE_TABLE_FILE_FORMAT_WindowsImageFormat	= 0xB881,
	REFERENCE_TABLE_FILE_FORMAT_UndefinedAudio		= 0xB900,
	REFERENCE_TABLE_FILE_FORMAT_WMA,
	REFERENCE_TABLE_FILE_FORMAT_OGG,
	REFERENCE_TABLE_FILE_FORMAT_UndefinedVideo		= 0xB980,
	REFERENCE_TABLE_FILE_FORMAT_WMV,
	REFERENCE_TABLE_FILE_FORMAT_UndefinedCollection =0xBA00,
	REFERENCE_TABLE_FILE_FORMAT_AbstractMultimediaAlbum,
	REFERENCE_TABLE_FILE_FORMAT_AbstractImageAlbum,
	REFERENCE_TABLE_FILE_FORMAT_AbstractAudioAlbum,
	REFERENCE_TABLE_FILE_FORMAT_AbstractVideoAlbum,
	REFERENCE_TABLE_FILE_FORMAT_AbstractAudioandVideoPlaylist,
	REFERENCE_TABLE_FILE_FORMAT_AbstractContactGroup,
	REFERENCE_TABLE_FILE_FORMAT_AbstractMessageFolder,
	REFERENCE_TABLE_FILE_FORMAT_AbstractChapteredProduction,
	REFERENCE_TABLE_FILE_FORMAT_WPLPlaylist = 0xBA10,
	REFERENCE_TABLE_FILE_FORMAT_M3UPlaylist,
	REFERENCE_TABLE_FILE_FORMAT_MPLPlaylist,
	REFERENCE_TABLE_FILE_FORMAT_ASXPlaylist,
	REFERENCE_TABLE_FILE_FORMAT_PLSPlaylist,
	REFERENCE_TABLE_FILE_FORMAT_UndefinedDocument = 0xBA80,
	REFERENCE_TABLE_FILE_FORMAT_XML = 0xBA82,		//hjnam..20060203..new created by MS	XML format(0xBA82) 
	REFERENCE_TABLE_FILE_FORMAT_AbstractDocument,
	REFERENCE_TABLE_FILE_FORMAT_UndefinedMessage = 0xBB00,
	REFERENCE_TABLE_FILE_FORMAT_AbstractMessage,
	REFERENCE_TABLE_FILE_FORMAT_UndefinedContact = 0xBB80,
	REFERENCE_TABLE_FILE_FORMAT_AbstractContact,
	REFERENCE_TABLE_FILE_FORMAT_vCard2,
	REFERENCE_TABLE_FILE_FORMAT_vCard3,
	REFERENCE_TABLE_FILE_FORMAT_UndefinedCalendarItem = 0xBE00,
	REFERENCE_TABLE_FILE_FORMAT_AbstractCalendarItem,
	REFERENCE_TABLE_FILE_FORMAT_vCalendar1,
	REFERENCE_TABLE_FILE_FORMAT_UndefinedWindowsExecutable = 0xBE80
};

enum PROPERTY_VALUES
{
	/*폴더*/
	PROPERTY_FOLDER,

	/*앨범파일*/
	PROPERTY_FILE_ALB,		
	
	/*음악파일*/
	PROPERTY_FILE_MP3,
	PROPERTY_FILE_WMA,
	PROPERTY_FILE_OGG,

	/*그림파일*/
	PROPERTY_FILE_JPG,
	PROPERTY_FILE_GIF,
	PROPERTY_FILE_BMP,
	
	/*문서 포맷*/
	PROPERTY_FILE_TEXT,

	/*동영상 포맷*/
	PROPERTY_FILE_AVI,
	PROPERTY_FILE_ASF,
	PROPERTY_FILE_WMV,
	PROPERTY_FILE_MPEG,
	PROPERTY_FILE_MOV,

	/*기타의 포맷*/
	PROPERTY_ORDER_FILE
};



enum OBJECT_TYPE_INDEX
{
	OBJECT_TYPE_OTHER = 0,
	OBJECT_TYPE_FOLDER = 1,
	OBJECT_TYPE_MUSIC_FILE = 2,
	OBJECT_TYPE_PHOTO_FILE = REFERENCE_TABLE_FILE_FORMAT_EXIF_JPEG,
	OBJECT_TYPE_MUSIC_PLAYLIST = 4,
	OBJECT_TYPE_PHOTO_PLAYLIST = 5,
	OBJECT_TYPE_MUSIC_PHOTO_PLAYLIST = 6,
	OBJECT_TYPE_COUNT = 7
};

#define MAX_OBJECT_COUNT 	3300		//이 값은 시스템에 존재하는 모든 파일 및 

extern unsigned short gDictionaryFileName[];
extern unsigned short gDataFileName[];
extern unsigned short gIndexFileName[];

extern unsigned short gDbTableNameMusic[];
extern unsigned short gDbColumnNameArtist[];
extern unsigned short gDbColumnNameAlbum[];
extern unsigned short gDbColumnNameGenre[];
extern unsigned short gDbColumnNameTitle[];
extern unsigned short gDbColumnNameFilePath[];
extern unsigned short gDbColumnNameFileName[];
extern unsigned short gDbColumnNameDuration[];
extern unsigned short gDbColumnNameRating[];
extern unsigned short gDbColumnNameUseCount[];
extern unsigned short gDbColumnNameFileFormat[];
extern unsigned short gDbColumnNameTrackNumber[];
extern unsigned short gDbColumnNameDrm[];
extern unsigned short gDbColumnNameLyric[];
extern unsigned short gDbColumnNamePurchase[];
extern unsigned short gDbColumnNameProtectionStatus[];
extern unsigned short gDbColumnNameSampleRate[];
extern unsigned short gDbColumnNameBitRate[];
extern unsigned short gDbColumnNameChangedFlag[];
extern unsigned short gDbColumnNamePosition[];
extern unsigned short gDbColumnNameStartClusM[];
extern unsigned short gDbColumnNameStartClusA[];
extern unsigned short gDbColumnNameUid[];

extern unsigned short gDbColumnNameAlbumArtPos[];		// 060115woody

extern unsigned short gDbTableNamePhoto[];
extern unsigned short gDbTableNameAlbum[];

extern unsigned short gDictionaryFileName[];
extern unsigned short gDataFileName[];
extern unsigned short gIndexFileName[];

extern unsigned short gDbTableNameReferences[];
extern unsigned short gDbColumnNameParentCluster[];
extern unsigned short gDbColumnNameChildCluster[];

extern unsigned short gDbTableNameObjects[];

extern unsigned short gDbColumnNameParentUid[];
extern unsigned short gDbColumnNameProperties[];
extern unsigned short gDbColumnNameFileType[];
extern unsigned short gDbColumnNameObjectName[];
#ifdef MTP_OBJECT_TABLE_EXTENSION
extern unsigned short gDbColumnNameName[];
extern unsigned short gDbColumnNameFileSize[];
extern unsigned short gDbColumnNameDateCreated[];
extern unsigned short gDbColumnNameRowID[];
extern unsigned short gDbColumnNamePUOID[];
#endif

#if SUPPORT_AVI_FORMAT

	//==========ALBUM TABLE(BEGIN)=========//
	enum VIDEO_TABLE_INDEX
	{
	/*0*/	VIDEO_TABLE_INDEX_SAMPLE_RATE,
	/*1*/	VIDEO_TABLE_INDEX_BIT_RATE,
	/*2*/	VIDEO_TABLE_INDEX_WIDTH,
	/*3*/	VIDEO_TABLE_INDEX_HEIGHT,
	/*4*/	VIDEO_TABLE_INDEX_TOTAL_BIT_RATE,
	/*5*/	VIDEO_TABLE_INDEX_VIDEO_BIT_RATE,
	/*6*/	VIDEO_TABLE_INDEX_FRAMES_PER_THOUSAND_SECONDS,
	/*7*/	VIDEO_TABLE_INDEX_VIDEO_FOUR_CCCODEC,
	/*8*/	VIDEO_TABLE_INDEX_AUDIO_WAVE_CODEC,
	/*9*/	VIDEO_TABLE_INDEX_UID,
	/*10*/	DB_VIDEO_TABLE_COUNT
	};
	//==========ALBUM TABLE(E N D)=========//

extern unsigned short gDbTableNameVideo[];
extern unsigned short gDbColumnNameAudioSampleRate[];
extern unsigned short gDbColumnNameAudioBitRate[];
extern unsigned short gDbColumnNameTotalBitRate[];
extern unsigned short gDbColumnNameVideoBitRate[];
extern unsigned short gDbColumnNameFramesPerThousandSeconds[];
extern unsigned short gDbColumnNameVideoFourCCCodec[];

void CrateFilesVideoSchma(void);
#endif
extern void CreateMusicSchma(void);
extern void CreateAlbumSchma(void);
extern void CreatePhotoSchma(void);
extern void CreateReferencesTableSchma(void);		// 060121woody_WarningRemoval
extern void CreateFilesObjectsSchma(void);

#endif
