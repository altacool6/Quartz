#ifndef _DBKEYFUNC_H		// 050602billy
#define _DBKEYFUNC_H

#include "avldbupgrade.h"

#include "stringsys.h"

/* ******************************************************************************
	File			: DBkeyFunc.h
	Author		: woong & billy (Wibro team)
	Description 	: function using db engine
******************************************************************************* */
/////////////////////////////////////////////////////////////////////////////////////////////////
//	DEFINE																				
/////////////////////////////////////////////////////////////////////////////////////////////////

enum PLP_ITEM_TYPE{
	PLP_ITEM_LIST = 0,
	PLP_ITEM_MENU
};
#ifdef NEW_MUSIC_PLAYLIST_MENU
#define MAX_PLP_HEAD_MENU 2
#else
#define MAX_PLP_HEAD_MENU 0
#endif

#define PLPTOTALNUM 	(101 + MAX_PLP_HEAD_MENU)	//plp file 가능한 개수..
#define TOTAL_TITLE_NUM 3000
#define KEY_TYPE_SIZE		2	//마지막은 freelinked list 
#define DISPLAY_LINE_COUNT 7
#ifndef CHANGE_FULLPATH_260CHAR
#define FIELD_TITLE_SIZE 256		//chungwook..20050508..각 포지션의 소제목을 display할때 필요
#endif
#define SHUFFLE_NOT_REPEAT 4
#define SHUFFLE_REPEAT 5
#define QUICKLIST_MAXNUM	200 //ori - 200 /  angelo test중  
#define MAX_FILE_COUNT TOTAL_TITLE_NUM + 1
#define START_SHUFLE_LIST 0

//Field Name    
#define FIELD_0			0		//record number                                    
#define FIELD_1			1		//File Name index 								   
#define FIELD_2			2		//Title index                                      
#define FIELD_3			3		//Artist index                                     
#define FIELD_4			4		//Album index                                      
#define FIELD_5			5		//Genre index	                                   
#define FIELD_6			6		//My Rating                                     
#define FIELD_7			7		//Play Count                                         
#define FIELD_8			8		//Last Played                                      
#define FIELD_9			9		//File Format                                        
#define FIELD_10			10		//track number	                          
#define FIELD_11			11		//DRM                                          
#define FIELD_12			12		//Release year                                 
#define FIELD_13			13		//File Size                                    
#define FIELD_14			14		//Total Time                                   
#define FIELD_15			15		//Sample rate                                  
#define FIELD_16			16		//Bitrate    
#define FIELD_17			17		//My rating time
#define FIELD_18			18		//lyrics
#define FIELD_19			19		//Audible Product id                           
#define FIELD_20			20		//Audible bookmark                             
#define FIELD_21			21		//Audible Playback position                    
#define FIELD_22			22		//Audible codec type/id                        
#define FIELD_23			23		//path length                                  
#define FIELD_24			24		//상위 2 byte- variable data에서의 위치, 하위 2 byte- File Name Length                             
#define FIELD_25			25		//상위 2 byte- variable data에서의 위치, 하위 2 byte-Title Lengh                              
#define FIELD_26			26		//상위 2 byte- variable data에서의 위치, 하위 2 byte-Artist length                                
#define FIELD_27			27		//상위 2 byte- variable data에서의 위치, 하위 2 byte-Album Length                                 
#define FIELD_28			28		//상위 2 byte- variable data에서의 위치, 하위 2 byte-Genre Length                                 
#define FIELD_29			29		//File Path                                    
#define FIELD_30			30		//File Name                                    
#define FIELD_31			31		//variable 영역의 title                        
#define FIELD_32			32		//variable 영역의 artist                       
#define FIELD_33			33		//variable 영역의 album                        
#define FIELD_34			34		//variable 영역의 genre                        
#define FIELD_35			35	//                                                 
                                                                                   
////Field name friendly name                                                       
#define RECORD_NUM_FLD			FIELD_0			//record number                
#define FILENAME_IDX_FLD		FIELD_1			//File Name index 					
#define TITLE_IDX_FLD			FIELD_2			//Title index                      
#define ARTIST_IDX_FLD			FIELD_3			//Artist index                     
#define ALBUM_IDX_FLD			FIELD_4			//Album index                  
#define GENRE_IDX_FLD			FIELD_5			//Genre index	                   
#define MYRATING_FLD			FIELD_6			//My Rating                      
#define PLAYCOUNT_FLD			FIELD_7			//Play Count                  
#define LASTPLAYED_FLD			FIELD_8			//Last Played                     
#define FILEFORMAT_FLD			FIELD_9			//File Format                     
#define TRACK_NUM_FLD			FIELD_10			//track number
#define DRM_FLD					FIELD_11			//DRM                      
#define RELEASEDYEAR_FLD		FIELD_12			//Release year                 
#define FILESIZE_FLD				FIELD_13			//File Size                
#define TOTALTIME_FLD			FIELD_14			//Total Time                   
#define SAMPLERATE_FLD			FIELD_15			//Sample rate 
#define BITRATE_FLD				FIELD_16			//Bitrate                  
#define MYRATINGTIME_FLD		FIELD_17			//MYRATINGTIME
#define LYRICS_FLD				FIELD_18			//LYRICS
#define ADBPROID_FLD			FIELD_19			//Audible Product id           
#define ADBBOOKMARK_FLD		FIELD_20			//Audible bookmark             
#define ADBPLAYBACKPOS_FLD	FIELD_21			//Audible Playback position    
#define ADBCODECTYPE_FLD		FIELD_22			//Audible codec type/id        
#define FILEPATHLENGTH_FLD		FIELD_23			//path length              
#define FILENAMELENGTH_FLD		FIELD_24			//상위 2 byte- variable data에서의 위치, 하위 2 byte-File Name Length         
#define TITLELENGTH_FLD			FIELD_25			//상위 2 byte- variable data에서의 위치, 하위 2 byte-Title Lenght             
#define ARTISTLENGTH_FLD		FIELD_26			//상위 2 byte- variable data에서의 위치, 하위 2 byte-Artist length                
#define ALBUMLENGTH_FLD		FIELD_27			//상위 2 byte- variable data에서의 위치, 하위 2 byte-Album Length                 
#define GENRELENGTH_FLD		FIELD_28			//상위 2 byte- variable data에서의 위치, 하위 2 byte-Genre Length                 
#define FILEPATH_FLD			FIELD_29			//File Path                    
#define FILENAME_FLD			FIELD_30			//File Name                    
#define TITLE_FLD				FIELD_31			//variable 영역의 title        
#define ARTIST_FLD				FIELD_32			//variable 영역의 artist   
#define ALBUM_FLD				FIELD_33			//variable 영역의 album        
#define GENRE_FLD				FIELD_34			//variable 영역의 genre   

//검색키 type
#define KEY_TYPE0			0x00	//PRIMARY KEY              
#define KEY_TYPE1 			0x01	//FILE NAME                
#define KEY_TYPE2 			0x02	//TITLE                    
#define KEY_TYPE3			0x03	//ARTIST                   
#define KEY_TYPE4			0x04	//ALBUM                    
#define KEY_TYPE5			0x05	//Genre                    	T
#define KEY_TYPE6			0x06	//Genre_Artist             
#define KEY_TYPE7			0x07	//Genre_album				
#define KEY_TYPE8			0x09	//Genre_Artist_Album       
#define KEY_TYPE9			0x0A	//Artist_Album				
#define KEY_TYPE10			0x0B	 //My rating               
#define KEY_TYPE11			0x0C	//Play count          
#define KEY_TYPE12			0x0D       //Last played         
#define KEY_TYPE13			0x0E       //File format    

//검색키 type friendly name
#define PREKEY_KEYWD 		KEY_TYPE0				//PRIMARY KEY       
#define FILENAME_KEYWD 		KEY_TYPE1 				//FILE NAME         
#define TITLE_KEYWD			KEY_TYPE2 				//TITLE             
#define ARTIST_KEYWD		KEY_TYPE3				//ARTIST            
#define ALBUM_KEYWD			KEY_TYPE4				//ALBUM             
#define GENRE_KEYWD			KEY_TYPE5				//Genre             
#define G_ARTIST_KEYWD		KEY_TYPE6				//Genre_Artist      
#define G_ALBUM_KEYWD		KEY_TYPE7				//Genre_album		
#define GA_ALBUM_KEYWD		KEY_TYPE8				//Genre_Artist_Album
#define A_ALBUM_KEYWD		KEY_TYPE9				//Artist_Album		
#define MYRATING_KEYWD		KEY_TYPE10				 //My rating        
#define PLAYCOUNT_KEYWD		KEY_TYPE11				//Play count        
#define LASTPLAYED_KEYWD	KEY_TYPE12			       //Last played    
#define FILEFORMAT_KEYWD	KEY_TYPE13			       //File format    

#define DB_ARTISTS 									1
#define DB_ARTISTS_ALBUMS 							11
#define DB_ARTISTS_ALBUMS_TITLES 					111
#define DB_ALBUMS 									2
#define DB_ALBUMS_TITLES 							22
#define DB_GENRES 									3
#define DB_GENRES_ARTISTS 							33
#define DB_GENRES_ARTISTS_ALBUMS 					333
#define DB_GENRES_ARTISTS_ALBUMS_TITLES 			3333
#define DB_TITLES 									4
#define DB_PLAYLISTS 								5
#define DB_PLAYLISTS_MYRATING_SELECT 				55
#define DB_PLAYLISTS_MYRATING_SELECT_TITLE 			555 
#define DB_PLAYLISTS_COUNTS_TITLE 					56
#define DB_PLAYLISTS_LASTPLAYED_TITLE				57
#define DB_PLAYLISTS_PLAYLISTS_TITLE 				58
#define DB_PLAYLISTS_QUICKLISTS_TITLE 				59
#define DB_MUSIC_PLAYBACK 							6
#if defined(CONFIG_AUDIBLE)
#define DB_AUDIBLE 7
#endif

#define DEBUG_BI_GET_MUSIC 1
#define DO_DELETE 1
#define DO_NOT_DELETE 2

#define ARTIST_ALBUM_TITLE 1
#define ALBUMS_TITLE 2
#define GENRES_A_A_TITLE 3
#define PURE_TITLE 4
#define MYRATING_TITLE 5
#define MY_FAVERITS_TITLE 6
#define RECENTLY_PLAYED_TITLE 7
#define MY_PLAYLISTS_TITLE 8
#define MY_QUICKLISTS_TITLE 9

#if defined(CONFIG_AUDIBLE)
#define AUDIBLE_TITLE 10
#endif

#define MY_PLAYLISTS_LISTS 20
#define MY_PLAYLISTS_RATE 30
#define MY_PLAYLISTS_LISTS_1 40
#define ARTIST_LISTS 50
#define ALBUM_LISTS 60
#define GENRES_LISTS 70

#define PlayAll_TITLE 80
#define ARTIST_ALBUM 90
#define GENRES_ARTIST 100
#define NOWPLAYING_TITLE 110
#define SHUFFLE_PLAY 120

#define DB_PLAY_PREV 1
#define DB_PLAY_NEXT 2
#define IS_EXIST 1
#define IS_NOT_EXIST 2


//=========heechul추가(begin)=========//

enum
{
	MUSIC_MODE,
	BROWSER_MODE,
	SPECIAL_FMREC,
	SPECIAL_REC
};

//=========heechul추가(end)=========//



/***********************************************************************************************/
/////////////////////////////////////////////////////////////////////////////////////////////////
//	STRUCT																			
/////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct 
{
	unsigned long pulResultLinkedListBuffer[TOTAL_TITLE_NUM];
	unsigned long ulTotalLinkNumber;
	unsigned char ref_memory;
}DB_DISPLAY;

/***********************************************************************************************/
//heechul (begin) Objects Table관련
void DataBaseFileSafeSave(void);
int RecordingFileInsertDB(unsigned short* pFullPathAndFileName, unsigned long vFileSize);
int RecordingFileDeleteDB(unsigned short* pFilePath, unsigned short* pFileName);
int CheckObjectUid(unsigned long uId, unsigned long * rowId);
int FillObjectPath(unsigned long uId, unsigned short* targetBuffer, unsigned long pathOnlyOrFullPath);
int FillObjectName(unsigned long uId, unsigned short* targetBuffer);
//heechul (end)
/////////////////////////////////////////////////////////////////////////////////////////////////
//	EXTERN																			
/////////////////////////////////////////////////////////////////////////////////////////////////
extern unsigned int recievedCurPos;
extern unsigned int g_artistStataus;
extern unsigned int g_artistAlbumStataus;
extern unsigned int g_albumStataus;
extern unsigned int g_genreStataus;
extern unsigned int g_genreArtistStataus;
extern unsigned int g_genreArtistAlbumStataus;
extern unsigned int g_myPlaylistStataus;
extern unsigned int g_prevArtistStataus;
extern unsigned int g_prevArtistAlbumStataus;
extern unsigned int g_prevGenreStataus;
extern unsigned int g_prevGenreArtistStataus;
extern unsigned int g_prevGenreArtistAlbumStataus;
extern int usTotalTitleResultLinkNum ;
extern unsigned short firstPlayFileDepth;
extern unsigned short SecondPlayFileDepth;
extern unsigned char makeShuffleFirst;
extern unsigned char reStartPlayback;
extern unsigned char curTitlePath;
extern unsigned char prevTitlePath;
extern unsigned char startShuffleMode;
extern unsigned char GoToPrevSong;
extern unsigned char checkExsit;
extern unsigned char curMovingPath;
extern int tmpQuicListTotalNum;
extern unsigned char notFinishedPlay;
extern unsigned int g_MyRatingStataus;
extern unsigned int curPlayingIndexNum;
extern unsigned char GoToPrevSong;
extern unsigned char curMusicPlayPos;	 //0: Music Mode, 1: Browser Mode
extern unsigned char doEndDbFunc;
extern unsigned char startGenreFirst;
extern unsigned char startArtistFirst;
extern unsigned char mustDoPlayAgain;
extern unsigned char didSomthingInQuickList;
extern unsigned char changedMyRatingVal;
extern unsigned char prevMyRatingPos;
extern unsigned char prevPlayModeStatus;
extern unsigned char doToMakeShuffle;
extern volatile unsigned char checkDBTask;
extern int startedPlayingIndexNum;
extern unsigned char didDelInQuickList;
extern unsigned char didAllDelInQuickList;
extern unsigned char gotoTitleListForMyRating;
extern unsigned char gotoTitleListForQuick;
extern unsigned char curTrackMyRatingVal;
extern unsigned char didAddInQuickList;
extern unsigned char haveToStopPlayBack;
extern unsigned char waitForMyRating;
extern unsigned long prevTempPtr; 
extern unsigned char wasInAllDelQuickList;
extern volatile unsigned long pulDBQuickList[QUICKLIST_MAXNUM];
extern unsigned long *pulPlayPtr; 
extern unsigned char mustDoPlayAgain;
extern unsigned short ShuffleList[MAX_FILE_COUNT];
extern unsigned short randomtbl[MAX_FILE_COUNT];
extern unsigned char makeShuffleFirst ;
extern unsigned char doMakeShuffleAgain;
extern unsigned char didSomthingInQuickList;
extern unsigned char changedMyRatingVal;
extern unsigned char prevMyRatingPos;
extern volatile unsigned char ulQuickListNum;
extern unsigned char changedMyRatingList;
extern unsigned char changedTrackMyRatingVal;
extern unsigned char useTmpPtr;
extern unsigned char doCpmparePrevPtr;
extern int curTrackTotalTime;
extern int curRectlyPlayedTime;
extern int remainRectlyTime;
extern volatile unsigned char dbUpdateTimeFlag;
extern int rectlyTotalTitleNum ;
extern int favoritTotalTitleNum ;
extern volatile unsigned char checkFileExistence;
extern unsigned char curListsPath;
/***********************************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////
//	ETC																			
/////////////////////////////////////////////////////////////////////////////////////////////////
/*
extern void Save_Memory_Quicklist(void);
extern int db_check_db_file_exist (void);
extern int db_check_generate_db (void);
extern void InitGetPlpPlaylist(void);


extern void db_get_list_data2(int curPos, int listnum);
extern void db_get_list_total_num(int curPos, int curPlaylistsPos);
extern void db_save_title_list (void);
extern unsigned char db_get_full_path_name (int listnum, unsigned char curPlayStatus, unsigned char playmode);
extern int db_check_music_file_exist (char *fullFileName);
extern unsigned char db_compare_index (unsigned short curIndex, unsigned char playMode);
extern unsigned int db_get_cur_index_for_display(void);
extern unsigned int db_get_index_for_ui(void);
extern unsigned char db_get_cur_myrating_val(unsigned short usIndexLocal);
extern unsigned short CheckQuickList(unsigned long ulCurPos);
extern unsigned short SaveQuickPtr(unsigned long ulCurPos);
extern unsigned short DelCurQuickListFile(unsigned long ulCurPos);
extern unsigned short InitQuickPtr(void);
extern void set_my_rating_value (int curIndex, int rating_val);
extern int get_total_title_num(void);

extern unsigned short ComparePlaylistFileName(unsigned char *CompareFileName);
//extern t_char db_get_cur_filename(int index);
extern void save_cur_moving_path(void);
extern void db_save_title_list_for_exist_check (void);
extern unsigned char get_cur_music_play_position (void);
extern unsigned short get_music_playing_index (void);
extern unsigned char check_prev_music_exist (unsigned char playMode);
extern unsigned char check_next_music_exist (unsigned char playMode);
extern unsigned short get_cur_playing_index_num (unsigned char playMode, unsigned char playStatus);
extern unsigned short get_cur_playing_index_num_for_shuffle (unsigned char playMode, unsigned char playStatus);

extern void db_task(void *p_arg);
extern void clear_setted_param (unsigned char userMode);
extern void db_update_playlists_val(void);
extern short compare_quicklist (void);
extern short get_myrating_cur_list_num(void);
extern unsigned char db_check_filename_exist (int listnum, unsigned char checkMode);
extern short compare_myrating (void);
extern unsigned short get_shuffle_index (unsigned short listnum);
extern void db_make_shuffle_list (unsigned char startFirst, unsigned short curPlayingIndex, unsigned char shuffle_mode);
extern unsigned int save_rectly_fav_index_for_resume (unsigned char listPos);*/
/***********************************************************************************************/
//===========희철이가 추가한 부분(begin)================
/*
unsigned long GetRowidOfEachTableByUid(unsigned short* pTableName, unsigned long vUid);
unsigned long GetRowidOfMusicTableByUid(unsigned long vUid);
unsigned long GetRowidOfAlbumTableByUid(unsigned long vUid);
unsigned long GetRowidOfPhotoTableByUid(unsigned long vUid);
unsigned long GetRowidOfRefferenceTableByUid(unsigned long vUid);
unsigned long GetRowidOfObjectsTableByUid(unsigned long vUid);
void DeleteRecordUidInAllTable(unsigned long vUid);
*/
#ifdef DB_SEARCH_CHANGED
int GetPictureFolders(unsigned long *pSearchArray, unsigned long vSearchArraySize, unsigned long vIndex, unsigned long *pTotal);
int GetPictureFolderContents(unsigned long *pSearchArray, unsigned long vSearchArraySize, unsigned long vIndex, unsigned long *pTotal);
int GetThumbnailPositionOfUid(unsigned long *pSearchArray, unsigned long vSearchArraySize, unsigned long vIndex, unsigned long *pPosition);
void GetMusicPlayListsFileNameContents(unsigned long *pSearchArray, unsigned long vSearchArraySize, unsigned long vIndex, unsigned long *pTotal);
void GetPhotoPlayListsFileNameContents(unsigned long *pSearchArray, unsigned long vSearchArraySize, unsigned long vIndex, unsigned long *pTotal);
#else
int GetPictureFolders(unsigned long *pSearchArray, unsigned long vIndex, unsigned long *pTotal);
int GetPictureFolderContents(unsigned long *pSearchArray, unsigned long vIndex, unsigned long *pTotal);
int GetThumbnailPositionOfUid(unsigned long *pSearchArray, unsigned long vIndex, unsigned long *pPosition);
void GetMusicPlayListsFileNameContents(unsigned long *pSearchArray, unsigned long vIndex, unsigned long *pTotal);
void GetPhotoPlayListsFileNameContents(unsigned long *pSearchArray, unsigned long vIndex, unsigned long *pTotal);
#endif

void PlayListOrAlbumFileDeleteProcess(unsigned long vDataRowid);
int GetPictureFoldersName(unsigned long *pSearchArray, unsigned long vIndex, unsigned short *pNameBuffer);

int GetPictureFolderContentName(unsigned long *pSearchArray, unsigned long vIndex, unsigned short *pNameBuffer);
int GetThumbnailPositionOfUidUseObjectTable(unsigned long *pSearchArray, unsigned long vIndex, unsigned long *pPosition);
int GetThumbnailPositionOfUid2(unsigned long vUid, unsigned long *pPosition);
//============================================
void GetPlayListsFullPathFileName(unsigned long *pSearchArray, unsigned long vIndex, unsigned short *pFullPathFileNameBuffer);

void GetPlayListContentsName(unsigned long *pSearchArray, unsigned long vIndex, unsigned short *pFileNameBuffer);
int CompareUnicodeString(unsigned short* targetBuffer, unsigned short* fileFullPathName);
int DeleteDeviceFileToDB(unsigned short* pFilePath, unsigned short* pFileName, unsigned long *pUid);
//int FindUid(unsigned short* fileFullPathName, unsigned long* uId);
//int FillObjectPath(unsigned long uId, unsigned short* targetBuffer, unsigned long pathOnlyOrFullPath);
//int FillObjectName(unsigned long uId, unsigned short* targetBuffer);
//int FindRating(int ratingValue, unsigned long targetBuffer[], unsigned long *searchTotalNumber);
//===========희철이가 추가한 부분( e n d )================
int GetFileTypeFromDb(unsigned short* pFullPathName, unsigned short *pFileType);
void Save_Memory_Quicklist(void);
int db_check_db_file_exist (void);
int db_check_generate_db (void);
void InitGetPlpPlaylist(void);
void Check_Valid_QuickList(void);
void Check_Valid_QuickList_Entry(void);
	
#ifdef ALARM_MUSIC_NEW_LIST
int check_generate_alarm_music_db(void);
int make_empty_alarm_music_db(void);
#endif

#ifdef NEW_PLAYALL_SEARCH	
void db_get_list_data2(int curPos, int listnum, int curPlaylistsPos, unsigned short* dbKeyOffset, int filetype);
#else
void db_get_list_data2(int curPos, int listnum, int curPlaylistsPos);
#endif
void db_get_list_total_num(int curPos, int curPlaylistsPos);

int db_get_list_dataNtotalnum(int listnum); //angelo - chk 

void db_save_title_list (void);
unsigned char db_get_full_path_name (int listnum, unsigned char curPlayStatus, unsigned char playmode);
//int db_check_music_file_exist (char *fullFileName);
int db_check_music_file_exist (unsigned short *fullFileName);
unsigned char db_compare_index (unsigned short curIndex, unsigned char playMode);
unsigned int db_get_cur_index_for_display(void);
unsigned int db_get_index_for_ui(void);
unsigned char db_get_cur_myrating_val(unsigned short usIndexLocal);
unsigned short CheckQuickList(unsigned long ulCurPos);
unsigned short SaveQuickPtr(unsigned long ulCurPos);
unsigned short DelCurQuickListFile(unsigned long ulCurPos);
unsigned short InitQuickPtr(void);
void set_my_rating_value (int curIndex, int rating_val);
int get_total_title_num(void);

#if 0
unsigned short ComparePlaylistFileName(unsigned char *CompareFileName);
#endif
//t_char db_get_cur_filename(int index);
void save_cur_moving_path(void);
void db_save_title_list_for_exist_check (void);
unsigned char get_cur_music_play_position (void);
void set_cur_music_play_opsition(unsigned char value);
unsigned short get_music_playing_index (void);
unsigned char check_prev_music_exist (unsigned char playMode);
unsigned char check_next_music_exist (unsigned char playMode);
unsigned short get_cur_playing_index_num (unsigned char playMode, unsigned char playStatus);
unsigned short get_cur_playing_index_num_for_shuffle (unsigned char playMode, unsigned char playStatus);

void db_task(void *p_arg);
void clear_setted_param (unsigned char userMode);
void db_update_playlists_val(void);
short compare_quicklist (void);
short get_myrating_cur_list_num(void);
unsigned char db_check_filename_exist (int listnum, unsigned char checkMode);
short compare_myrating (void);
unsigned short get_shuffle_index (unsigned short listnum);
void db_make_shuffle_list (unsigned char startFirst, unsigned short curPlayingIndex, unsigned char shuffle_mode);
unsigned int save_rectly_fav_index_for_resume (unsigned char listPos);



/////////////////////////////////////////////////////////////////
void fwui_search_artist(void);
void fwui_search_albums(void);
void fwui_search_genres(void);
void fwui_search_songs(void);
void fwui_Init_fwuiDbData(void);
void fwui_Generate_DB(void);
void fwui_search_playlist(unsigned short indexNum);
unsigned short fwui_total_playlistfile(void);
//void fwui_search_Quicklist(void);
int fwui_search_Quicklist(void);

unsigned shortfwui_total_quicklist(void);
void fwui_quilist_test(void);
int fwui_add_quicklist(int selectIdx);
int fwui_delcur_quicklist(int selectIdx);
void fwui_delall_quicklist(void);
void fwui_search_MyRating(unsigned short starNum);
void fwui_set_myrating_value(unsigned short value, unsigned short curIndex, char mode);
void fwui_Create_fwuiDbData(void);
void fwui_write_myrating_value(int curIndex, int rating_val, char mode);
unsigned long fwui_get_cur_myrating_val(unsigned short usIndexLocal);
unsigned short fwui_addGroupQuicklist(unsigned short total);
void fwui_addGroupQueueToPlayPtrList(unsigned short total);
unsigned short fwui_total_quicklist(void);
void fwui_Create_fwui_AlarmMusic(void);

//	alarm functions
#if 0
void fwui_alarm_set_music(unsigned short value);
void fwui_alarm_set_playlists(unsigned short value);
void fwui_alarm_set_playlists_rating(unsigned short value);
void fwui_alarm_set_playlists_myplaylist(unsigned short value);
void fwui_alarm_set_playlists_quicklist(unsigned short value);
void fwui_alarm_set_artist(unsigned short value);
void fwui_alarm_set_album(unsigned short value);
void fwui_alarm_set_genre(unsigned short value);
void fwui_alarm_set_title(unsigned short value);
void fwui_alarm_set_type(unsigned short value);
unsigned short fwui_alarm_get_music(void);
unsigned short fwui_alarm_get_playlists(void);
unsigned short fwui_alarm_get_playlists_rating(void);
unsigned short fwui_alarm_get_playlists_myplaylist(void);
unsigned short fwui_alarm_get_playlists_quicklist(void);
unsigned short fwui_alarm_get_artist(void);
unsigned short fwui_alarm_get_album(void);
unsigned short fwui_alarm_get_genre(void);
unsigned short fwui_alarm_get_title(void);
#endif 

void fwui_initPlayingListBuff(void);
unsigned char fwui_quicklist_filename_exist(int listnum);
void fwui_write_buy_value(int curIndex, char mode);

// playlist mtp
#if 1
int fwui_playlist_getMusicRowId(unsigned long *playlistArray, unsigned long currentIndex);
#else
unsigned int fwui_playlist_plaFileGetStartCluster( unsigned short *CreateFileDirName );
unsigned int fwui_playlist_musicFileGetCluster( unsigned int StartCluster );
#endif
unsigned long fwui_playlist_DBSearchClusRowid(unsigned short* mtpTableName,
						      	  				unsigned long   Cluster);

void fwui_PlayCountUpdate(void);
void fwui_makeShufflePlay(unsigned char startFirst, 
							unsigned short curPlayingIndex, 
							unsigned char shuffle_mode,
							unsigned short total);

char fwui_getNextMusicFileName(unsigned short currentIndex, unsigned short* fileName);
void fwui_Init_Struct_db_display(void);
unsigned short fwui_totalNum_Search(void);
void fwui_write_playCount_value(int curIndex);
void fwui_add_queue(int curIndex);
unsigned short fwui_checkPlayingListBuff(void);

unsigned char fwui_get_NextTrack(void);
void fwui_set_NextTrack(unsigned char value);
unsigned char fwui_get_NextTrackIsNotExist(void);

void fwui_make_empty_queue(void);

#ifdef NEW_PLAYALL_SEARCH
struct FWUI_DbData
{
	unsigned short genreNum;	//장르에서의 index
	unsigned short artistNum;	//아티에서의 index
	unsigned short albumNum;	//앨범에서의 index

	unsigned short genreDBOffset[MAX_UNICODE_PATH_BYTE];
	unsigned short artistDBOffset[MAX_UNICODE_PATH_BYTE];
	unsigned short albumDBOffset[MAX_UNICODE_PATH_BYTE];		
};
#else
struct FWUI_DbData
{
	unsigned short genreNum;	//장르에서의 index
	unsigned short artistNum;	//아티에서의 index
	unsigned short albumNum;	//앨범에서의 index	
};
#endif

#if 0  //ori
struct FWUI_MusicAlarmDbData
{
	unsigned short  music;	//index
		unsigned short  playlists;	//아티에서의 index
			unsigned short playlists_rating;	//앨범에서의 index
			unsigned short playlists_myplaylist;	//장르에서의 index
			unsigned short playlists_quicklist;	//아티에서의 index
				unsigned short playlists_title;

		unsigned short artist;	//앨범에서의 index
		unsigned short album;	//앨범에서의 index
		unsigned short genre;	//앨범에서의 index
		unsigned short title;	//앨범에서의 index

		unsigned long artistDBOffset;
		unsigned long albumDBOffset;	
		unsigned long genreDBOffset;
		
		unsigned long uid; // 해당 곡의 uid
};
#else

enum
{
	PLAYLIST_MODE = 0,
	RATED_MODE,
	QUICK_MODE,
	ARTIST_MODE,
	SONG_MODE,
	ALBUM_MODE,
	GENRE_MODE
};


struct FWUI_MusicAlarmDbData
{
	unsigned short music; //menu (rating, quick, myplaylist, artist, song, album, genre)
		unsigned short playlists_rating;	//rating서의 index
		unsigned short playlists_quicklist;	//quicklist의 index
		unsigned short playlists_myplaylist;	//playlist?index

		unsigned short artist;	//artist에서의 index
		unsigned short album;	//의 index
		unsigned short genre;	//의 index

		unsigned short artistDBOffset[MAX_UNICODE_PATH_BYTE];
		unsigned short albumDBOffset[MAX_UNICODE_PATH_BYTE];	
		unsigned short genreDBOffset[MAX_UNICODE_PATH_BYTE];

			unsigned short title;	//앨범에서의 index
			unsigned long uid; // 해당 곡의 uid
};


#endif

extern unsigned char playReturnPos;
extern unsigned char isNextSong;
extern unsigned int gTempPlayingIndexNum;



//struct FWUI_DbData fwuiDbData;


/////////////////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_FWUI
unsigned int GetPrevOrNextPlaybackIndex(unsigned char playMode, unsigned short currentIndex, unsigned char playStatus);
void GetPlaylistName(int currentIndex, unsigned short *currentPlaylist);



extern void fwui_alarm_set_music(unsigned short value);
extern void fwui_alarm_set_playlists(unsigned short value);
extern void fwui_alarm_set_playlists_rating(unsigned short value);
extern void fwui_alarm_set_playlists_myplaylist(unsigned short value);
extern void fwui_alarm_set_playlists_quicklist(unsigned short value);
extern void fwui_alarm_set_artist(unsigned short value);
extern void fwui_alarm_set_album(unsigned short value);
extern void fwui_alarm_set_genre(unsigned short value);
extern void fwui_alarm_set_title(unsigned short value);
extern void fwui_alarm_set_music_uid(unsigned long value);
extern unsigned short fwui_alarm_get_music(void);
extern unsigned short fwui_alarm_get_playlists(void);
extern unsigned short fwui_alarm_get_playlists_rating(void);
extern unsigned short fwui_alarm_get_playlists_myplaylist(void);
extern unsigned short fwui_alarm_get_playlists_quicklist(void);
extern unsigned short fwui_alarm_get_artist(void);
extern unsigned short* fwui_alarm_get_artist_DBOffset(void);
extern unsigned short fwui_alarm_get_album(void);
extern unsigned short* fwui_alarm_get_album_DBOffset(void);
extern unsigned short fwui_alarm_get_genre(void);
extern unsigned short* fwui_alarm_get_genre_DBOffset(void);
extern unsigned short fwui_alarm_get_title(void);
extern unsigned long  fwui_alarm_get_music_uid(void);
extern void fwui_alarm_set_artist_DBOffset(unsigned short value);
extern void fwui_alarm_set_album_DBOffset(unsigned short value);
extern void fwui_alarm_set_genre_DBOffset(unsigned short value);
#endif


int GetPlpHeadMenuCount(void);
int CheckStringSize(unsigned short *path, unsigned short *filename);



#endif // _DBKEYFUNC_H		// 050602billy
