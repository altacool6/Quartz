#ifndef _DBTASK_C		
#define _DBTASK_C
#include "config.h"
#ifdef CONFIG_AVLDB


#include "tfs4_global.h"
#include "tfs4_api.h"
#include "api_main.h"
#include "DBKeyFunc.h"
//#include "vbisam.h"
//#include "isinternal.h"
//#include "db_common_api.h"
//#include "ifp_local_function.h"
#include "FFileSys.h"
#include "safe_string.h"
#include "Uart_api.h"
//#include "photo.h"
//#include "browser.h"
#include "rtc.h"
#include "misclib.h"
#include "setup.h"
#include <ucos_ii.h>
#include "task.h"

//051229..chungwook
#ifdef CONFIG_FLASHLITE11
#include "bi_set.h"
#include "Bi_get.h"
#endif

#include "si_string.h"

#if UART_DISPLAY
#define	DbgPrintf(x)	UART_Print x
#else
#define	DbgPrintf(x)
#endif

static unsigned char nextTrackIsNotExsit = 0;
static unsigned char prevTrackIsNotExsit= 0;
static unsigned char changedQuickList = 0;
static unsigned char doNotCheckIndexNum = 0;
static unsigned char decleaseIndex = 0;
static unsigned char curIndexDoesNotExistInQuickList = 0;
static unsigned char doCheckForMyRating = 0;
static unsigned char makeIndexOne = 0;
static unsigned char makeIndexTwo = 0;
volatile unsigned char checkDBTask=1;
int startedPlayingIndexNum = 0;
unsigned char didDelInQuickList =0;
unsigned char didAllDelInQuickList =0;
unsigned char curTrackMyRatingVal = 0;
unsigned char gotoTitleListForMyRating = 1;
unsigned char gotoTitleListForQuick = 1;
unsigned char didAddInQuickList=0;
unsigned char haveToStopPlayBack = 0;
char tmpCBuff[MAX_UTF8_PATH_BYTE];
unsigned char changedMyRatingList = 0;
unsigned char useTmpPtr = 0;
unsigned char doUpdateRecPlayList = 1;
unsigned char doUpdateFavPlayList = 1;
volatile unsigned char dbUpdateTimeFlag = 0;
//unsigned short markedtbl[1000];
//volatile unsigned char checkFileExistence = 0;

short compare_quicklist (void)
{
	short retVal = 0;
	int i;

	retVal = 0;

	for(i=0;i<ulQuickListNum;i++)
	{
		if(pulDBQuickList[i]==prevTempPtr)
		{
			retVal = i+1;
			break;
		}
	}
	return retVal;
}

short compare_myrating (void)
{
	short retVal = 0;
	int i;

	retVal = 0;

	for(i=0; i < get_total_title_num(); i++)
	{
		if(pulPlayPtr[i] == prevTempPtr)
		{
			retVal = i+1;
			break;
		}
	}
	return retVal;
}

void save_cur_moving_path(void)
{		
	switch(curTitlePath)
	{
		case 1:
			curMovingPath = g_artistStataus + g_artistAlbumStataus;
			break;
		case 2:
			curMovingPath = g_albumStataus;
			break;
		case 3:
			curMovingPath = g_genreStataus + g_genreArtistStataus + g_genreArtistAlbumStataus;			
			break;
		case 4:
			break;
		case 5:
			curMovingPath = g_MyRatingStataus;
			break;
		case 8:
			curMovingPath = g_myPlaylistStataus;
			break;			
	}
	
}

unsigned char check_moving_path(unsigned short curIndex)
{
	unsigned char retVal;

	retVal = 1;

	if (curTitlePath == MYRATING_TITLE && prevTitlePath == MYRATING_TITLE)
	{
		if (curIndex == get_myrating_cur_list_num())		
			retVal = 1;
		else
			retVal = 2;
	}
	
	switch(curTitlePath)
	{
		case 1:
			if (curMovingPath != g_artistStataus + g_artistAlbumStataus)  retVal = 2;
			break;
		case 2:
			if (curMovingPath != g_albumStataus) retVal = 2;
			break;
		case 3:
			if (curMovingPath != g_genreStataus + g_genreArtistStataus + g_genreArtistAlbumStataus) retVal = 2;			
			break;
		case 4:
			break;
		case 5:
			if (curMovingPath != g_MyRatingStataus) retVal = 2;
			break;
		case 8:
			if (curMovingPath != g_myPlaylistStataus) retVal = 2;
			break;			
	}

	if (curTitlePath == MY_QUICKLISTS_TITLE)
	{
//		if (CheckQuickList(curIndex - 1) == 1)	janged
		if (CheckQuickList(curIndex) == 1)
		{
			retVal =2;
		}	
		else
		{			
			if (wasInAllDelQuickList || pulDBQuickList[curIndex-1] != pulPlayPtr[curIndex-1])
			{
				wasInAllDelQuickList = 0;
				retVal = 2;
			}
		}	
	}
	else if (curTitlePath != MYRATING_TITLE && curTitlePath != MY_QUICKLISTS_TITLE && mustDoPlayAgain)
	{
		mustDoPlayAgain = 0;
		retVal = 2;
	}

	return retVal;	
}

unsigned char db_compare_index (unsigned short curIndex, unsigned char playMode)
{
	short i;
	
	if (curTitlePath != prevTitlePath) 
	{
		return 2;
	}
	else
	{
		i = db_get_index_for_ui();
		
		if (prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath == MY_QUICKLISTS_TITLE)
		{
			i = compare_quicklist();
			if (i == 0)
			{
				return 2;
			}
		}	
		else if (prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE )
		{
			if (doCheckForMyRating)  
			{
				doCheckForMyRating = 0;
				return 2;	
			}
		}		
		
		if (i != curIndex)
		{
			return 2;
		}
		else
		{
			return check_moving_path(curIndex);
		}
	}
}

unsigned char check_prev_music_exist (unsigned char playMode)
{
	if (prevTrackIsNotExsit || get_total_title_num() <= 0)
	{
		return 2;
	}	
	else		
	{
		return 1;
	}	
}

unsigned char check_next_music_exist (unsigned char playMode)
{
	unsigned char fileNotExist = 1;
	unsigned short checkPlayTrakNum = 1;
	unsigned short shufflePlayTrackNum = 1;
	unsigned char retVal=0;
	unsigned char lastTrack = 0;
	short loopCount = 3;

	if(((playMode == playmode_none) || (playMode == playmode_repeat1)) 
		&&  (get_music_playing_index() >= get_total_title_num()))
	{
		nextTrackIsNotExsit = 1;
	}

	if (nextTrackIsNotExsit || get_total_title_num() <= 0 )
	{
		return 2;
	}
	else		
	{
		retVal=1;
		
		if (playMode == playmode_none || playMode == playmode_shuffle)
		{
			checkPlayTrakNum = curPlayingIndexNum +1;

			//if (checkPlayTrakNum > get_total_title_num()) return 2;
		
			if (playMode == playmode_none || playMode == playmode_repeat || playMode == playmode_repeat1) 
			{
				fileNotExist = db_check_filename_exist(checkPlayTrakNum, 0);
				if (fileNotExist == 2 && checkPlayTrakNum+1 == get_total_title_num()) 
				{
					retVal=2;
					goto NOW_EXIT;
				}	
			}	
			else	
			{
				shufflePlayTrackNum = get_shuffle_index(checkPlayTrakNum-1);
				fileNotExist = db_check_filename_exist(shufflePlayTrackNum, 0);
				if (fileNotExist == 2 && checkPlayTrakNum == get_total_title_num()) 
				{
					retVal=2;
					goto NOW_EXIT;
				}	
			}
			
			if (fileNotExist == 2 && (playMode == playmode_none || playMode == playmode_repeat || playMode == playmode_repeat1))
			{
				while (loopCount)
				{
					checkPlayTrakNum ++;
					fileNotExist = db_check_filename_exist(checkPlayTrakNum, 0);
					if (fileNotExist == 1)
					{
						break;
					}
					else if (checkPlayTrakNum > get_total_title_num())
					{
						lastTrack= 1;
						break;						
					}
					loopCount --;
				}		
				if (fileNotExist == 2 || lastTrack || loopCount <= 0) 
					retVal = 2;
				else
					retVal = 1;
			}
			else if (fileNotExist == 2 && (playMode == playmode_shuffle || playMode == playmode_shuffle_repeat))
			{
				while (loopCount)
				{
					checkPlayTrakNum ++;					
					shufflePlayTrackNum = get_shuffle_index(checkPlayTrakNum-1);
					fileNotExist = db_check_filename_exist(shufflePlayTrackNum, 0);
					if (fileNotExist == 1)
					{
						break;
					}
					else if (fileNotExist == 2 && checkPlayTrakNum + 2 >= get_total_title_num())
					{
						while(loopCount)
						{
							checkPlayTrakNum ++;					
							shufflePlayTrackNum = get_shuffle_index(checkPlayTrakNum-1);
							fileNotExist = db_check_filename_exist(shufflePlayTrackNum, 0);
							if (fileNotExist == 1)
							{
								break;
							}	
	
							loopCount --;
						}					
					}				
					else if (checkPlayTrakNum > get_total_title_num()) 
					{
						lastTrack= 1;
						break;
					}	
					loopCount --;
				}		
				if (fileNotExist == 2 || lastTrack || loopCount <= 0) 
					retVal = 2;
				else
					retVal = 1;
			}
		}

NOW_EXIT:
	
		return retVal;

	}

}

void fill_randomnumber_for_db(unsigned char used)
{
    int i;

     for(i=START_SHUFLE_LIST; i<MAX_FILE_COUNT; i++)
    {
		 randomtbl[i] = 0;
    }
}

unsigned short GetRawRandomNum(unsigned short usNum)
{
	unsigned short usRandomSeedLocal;
             
	usRandomSeedLocal = random(usNum);

    	return(usRandomSeedLocal);      
}

unsigned char CheckRandomUsed(unsigned short usSongNumber)
{
	unsigned short i, j;
	unsigned long k;

	i = usSongNumber / 16;
	j = usSongNumber % 16;
	k = 0x0001 << j;

	if (randomtbl[i] & k)
	{
	    return(1); //����
	}
	else
		return(0);
}

void MarkRandomUsed (unsigned short usSongNumber)
{

	if(usSongNumber >= MAX_FILE_COUNT) return;

	randomtbl[usSongNumber/16] |= 0x0001<<(usSongNumber%16);
}

void db_make_shuffle_list (unsigned char startFirst, unsigned short curPlayingIndex, unsigned char shuffle_mode)
{
	unsigned long i;
	unsigned short usRandomNumber, usRawRandomNum;
	unsigned short usTemp = START_SHUFLE_LIST;
	static unsigned short usTotalShuffleNum = 0;
	unsigned long tmpIndex = 0;
	
	usTotalShuffleNum = get_total_title_num();

	if (usTotalShuffleNum == 0) return;
	

   	fill_randomnumber_for_db(0);
	
	if (startFirst)
	{
		MarkRandomUsed(curPlayingIndex);
		ShuffleList[0] = curPlayingIndex;		
		
		makeShuffleFirst = 1;
		
	}
	else
	{
		MarkRandomUsed(curPlayingIndex);
		
		if (doMakeShuffleAgain)
		{			
			ShuffleList[usTotalShuffleNum - 1] = curPlayingIndex;			
		}
		else
		{
			ShuffleList[0] = curPlayingIndex;
		}

	}

	for(i = START_SHUFLE_LIST; i < usTotalShuffleNum;  ) 
	{		
		usTemp++;

		usRawRandomNum = GetRawRandomNum(usTotalShuffleNum);

		usRandomNumber = (usRawRandomNum) % (usTotalShuffleNum);

		if( ((i + 2) >= usTotalShuffleNum) || (usTemp > 500) ) 
		{			
			unsigned short usNum;

			if(doMakeShuffleAgain)
			{
				for(usNum = START_SHUFLE_LIST; usNum < usTotalShuffleNum; usNum++) 
				{
					if(!CheckRandomUsed(usNum)) 
					{					
						ShuffleList[i] = usNum;
		
	            		MarkRandomUsed(usNum);

						i++;	
						usTemp = START_SHUFLE_LIST;								
					}	
				}
				if (usTemp > START_SHUFLE_LIST) 
					break;
			}	
			else
			{
				for(usNum = START_SHUFLE_LIST; usNum < usTotalShuffleNum; usNum++) 
				{
					if(!CheckRandomUsed(usNum)) 
					{					
							ShuffleList[i+1] = usNum;

		            		MarkRandomUsed(usNum);
						
						i++;
						usTemp = START_SHUFLE_LIST;
					}	
				}
				if(usTemp > START_SHUFLE_LIST) 					
					break;
			}				
		} 
		else 
		{
			if(!CheckRandomUsed(usRandomNumber)) 
			{	

				if(doMakeShuffleAgain)// && !startFirst)
				{
					ShuffleList[i] = usRandomNumber;
				}
				else
				{
					ShuffleList[i+1] = usRandomNumber;
			
				}	
	            		MarkRandomUsed(usRandomNumber);
				
				i++;				
				usTemp = START_SHUFLE_LIST;
			}		
		}	
	}	
#if 0 //ori - angelo chk (getSmartKeyShufflePlayStatus
	if(GetPlayAllStatus() == 1 && usTotalShuffleNum > 1 && startFirst == 1)
#else
	if((GetPlayAllStatus() == 1 ||getSmartKeyShufflePlayStatus() ==1) && usTotalShuffleNum > 1 && startFirst == 1)
#endif
	{
		tmpIndex = ShuffleList[usTotalShuffleNum-1];
		ShuffleList[usTotalShuffleNum -1] = curPlayingIndex;
		ShuffleList[0] = tmpIndex;
		
	}

	doMakeShuffleAgain = 0;
}

unsigned short get_shuffle_index (unsigned short listnum)
{
	return (ShuffleList[listnum]);
}
unsigned short get_cur_playing_index_num (unsigned char playMode, unsigned char playStatus)
{
	int totalListNum;
	unsigned char changeIndexLater = 0;

	changeIndexLater = 0;
	totalListNum = get_total_title_num();

	
	if ((prevTitlePath == MY_QUICKLISTS_TITLE || prevTitlePath == MYRATING_TITLE) && doNotCheckIndexNum && curPlayingIndexNum == 1) 
	{	
		changeIndexLater = 1;
	}
	else if ((prevTitlePath == MY_QUICKLISTS_TITLE || prevTitlePath == MYRATING_TITLE) && decleaseIndex&& (curPlayingIndexNum != 1 && totalListNum != 1))
	{
		curPlayingIndexNum --;
	}	
	
	//prevPlayModeStatus = playMode;
	
	if (!changeIndexLater)
	{
		if (curPlayingIndexNum == totalListNum) //������ ��
		{
			debug("\nplaying_index_num1 : curPlayingIndexNum(%d), totalListNum(%d)\n", curPlayingIndexNum, totalListNum);
			switch(playMode)
			{
				case playmode_none:
					if (playStatus == DB_PLAY_PREV)
					{
						if (GoToPrevSong) curPlayingIndexNum --;
					}	
					else if (playStatus == DB_PLAY_NEXT)
					{
						curPlayingIndexNum = 0;	
					}					
					break;
				case playmode_repeat:
					if (playStatus == DB_PLAY_PREV)
					{
						if (curPlayingIndexNum == 1)
						{
							if (GoToPrevSong) curPlayingIndexNum = totalListNum;
						}
						else
						{
							if (GoToPrevSong) curPlayingIndexNum --;
						}
					}	
					else if (playStatus == DB_PLAY_NEXT)
					{
						curPlayingIndexNum = 1;
					}				
					break;
				case playmode_repeat1:
					if (playStatus == DB_PLAY_PREV)
					{
						if (curPlayingIndexNum == 1)
						{
							if (GoToPrevSong) curPlayingIndexNum = totalListNum;
						}
						else
						{
							if (GoToPrevSong) curPlayingIndexNum --;
						}
					}	
					else if (playStatus == DB_PLAY_NEXT)
					{
						curPlayingIndexNum = 1;
					}			
					break;	
					
				case playmode_shuffle:
					if (playStatus == DB_PLAY_PREV)
					{
						if (GoToPrevSong) curPlayingIndexNum --;
					}	
					else if (playStatus == DB_PLAY_NEXT)
					{
						curPlayingIndexNum = 0;	
					}			
					break;	
					
				case playmode_shuffle_repeat:
					if (playStatus == DB_PLAY_PREV)
					{
						if (curPlayingIndexNum == 1)
						{
							if (GoToPrevSong) curPlayingIndexNum = totalListNum;
						}
						else
						{
							if (GoToPrevSong) curPlayingIndexNum --;
						}
					}	
					else if (playStatus == DB_PLAY_NEXT)
					{
						doMakeShuffleAgain = 1;
#if 0 // angelo  						
						db_make_shuffle_list(0, db_get_cur_index_for_display(), playMode);
#endif
						curPlayingIndexNum = 1;
						doMakeShuffleAgain = 0;
					}			
					break;						

			}			
		}	
		else if (curPlayingIndexNum == 1) //ù ��
		{
			debug("\nplaying_index_num2 : curPlayingIndexNum(%d), totalListNum(%d)\n", curPlayingIndexNum, totalListNum);
			switch(playMode)
			{
				case playmode_none:
					if (playStatus == DB_PLAY_PREV)
					{
						if (GoToPrevSong) curPlayingIndexNum = 0;
					}	
					else if (playStatus == DB_PLAY_NEXT)
					{
						curPlayingIndexNum ++;
					}						
					break;
				case playmode_repeat:
					if (playStatus == DB_PLAY_PREV)
					{
						if (GoToPrevSong) curPlayingIndexNum = totalListNum;
					}	
					else if (playStatus == DB_PLAY_NEXT)
					{
						curPlayingIndexNum ++;
					}	
					break;
				case playmode_repeat1:
					if (playStatus == DB_PLAY_PREV)
					{
						if (GoToPrevSong) curPlayingIndexNum = totalListNum;
					}	
					else if (playStatus == DB_PLAY_NEXT)
					{
						curPlayingIndexNum ++;
					}					
					break;	
				case playmode_shuffle:
					if (playStatus == DB_PLAY_PREV)
					{
						if (GoToPrevSong) curPlayingIndexNum = 0;
					}	
					else if (playStatus == DB_PLAY_NEXT)
					{
						curPlayingIndexNum ++;
					}					
					break;	
				case playmode_shuffle_repeat:
					if (playStatus == DB_PLAY_PREV)
					{
						if (GoToPrevSong) 
						{
							curPlayingIndexNum = totalListNum;
						}
					}	
					else if (playStatus == DB_PLAY_NEXT)
					{
						curPlayingIndexNum ++;
					}	
					break;								
					
			}				
		}
		else
		{		
			debug("\nplaying_index_num3 : curPlayingIndexNum(%d), totalListNum(%d), playStatus (%d)\n", curPlayingIndexNum, totalListNum, playStatus);
			switch(playStatus)
			{
				case DB_PLAY_PREV:
					if (GoToPrevSong) curPlayingIndexNum --;
					break;
				case DB_PLAY_NEXT:
					curPlayingIndexNum ++;
					break;
			}				
		}
	}
	else
	{
		changeIndexLater = 0;
	}
	
	doNotCheckIndexNum= FALSE;
	changedQuickList = FALSE;
	changedMyRatingList = FALSE;
	decleaseIndex = FALSE;
	gotoTitleListForMyRating = TRUE;	
	gotoTitleListForQuick = TRUE;
	doCpmparePrevPtr = FALSE;


	if (playMode == playmode_shuffle || playMode == playmode_shuffle_repeat)
	{		
		if ((makeIndexOne || makeIndexTwo) && (prevTitlePath == MYRATING_TITLE || prevTitlePath == MY_QUICKLISTS_TITLE))				
		{
			if (makeIndexOne)
				curPlayingIndexNum = 1;
			else if (makeIndexTwo)
				curPlayingIndexNum = 2;
		}	
	}
	
	makeIndexOne = FALSE;
	makeIndexTwo = FALSE;
	
	debug("get_cur_playing_index_num : return curPlayingIndexNum(%d)\n", curPlayingIndexNum);
	debug("\n################################################################################\n");

	prevPlayModeStatus = playMode;
	return (curPlayingIndexNum);
}


#ifdef CONFIG_FWUI
unsigned int GetPrevOrNextPlaybackIndex(unsigned char playMode, unsigned short currentIndex, unsigned char playStatus)
{
	int totalListNum;

	totalListNum = get_total_title_num();

	if ((prevTitlePath == MY_QUICKLISTS_TITLE || prevTitlePath == MYRATING_TITLE) && decleaseIndex&& (currentIndex != 1 && totalListNum != 1))
	{
		currentIndex --;
	}
	
	if (currentIndex == totalListNum) //������ ��
	{
		switch(playMode)
		{
			case playmode_none:
				if (playStatus == DB_PLAY_PREV)
				{
					currentIndex --;
				}	
				else if (playStatus == DB_PLAY_NEXT)
				{
					currentIndex = 0;	
				}					
				break;
			case playmode_repeat:
				if (playStatus == DB_PLAY_PREV)
				{
					if (currentIndex == 1)
					{
						currentIndex = totalListNum;
					}
					else
					{
						currentIndex --;
					}
				}	
				else if (playStatus == DB_PLAY_NEXT)
				{
					currentIndex = 1;
				}				
				break;
			case playmode_repeat1:
				if (playStatus == DB_PLAY_PREV)
				{
					if (currentIndex == 1)
					{
						currentIndex = totalListNum;
					}
					else
					{
						currentIndex --;
					}
				}	
				else if (playStatus == DB_PLAY_NEXT)
				{
					currentIndex = 1;
				}			
				break;	
				
			case playmode_shuffle:
				if (playStatus == DB_PLAY_PREV)
				{
					currentIndex --;
				}	
				else if (playStatus == DB_PLAY_NEXT)
				{
					currentIndex = 0;	
				}			
				break;	
				
			case playmode_shuffle_repeat:
				if (playStatus == DB_PLAY_PREV)
				{
					if (currentIndex == 1)
					{
						currentIndex = totalListNum;
					}
					else
					{
						currentIndex --;
					}
				}	
				else if (playStatus == DB_PLAY_NEXT)
				{
					currentIndex = 1;
				}			
				break;						

		}			
	}	
	else if (currentIndex == 1) //ù ��
	{
		switch(playMode)
		{
			case playmode_none:
				if (playStatus == DB_PLAY_PREV)
				{
					currentIndex = 0;
				}	
				else if (playStatus == DB_PLAY_NEXT)
				{
					currentIndex ++;
				}						
				break;
			case playmode_repeat:
				if (playStatus == DB_PLAY_PREV)
				{
					currentIndex = totalListNum;
				}	
				else if (playStatus == DB_PLAY_NEXT)
				{
					currentIndex ++;
				}	
				break;
			case playmode_repeat1:
				if (playStatus == DB_PLAY_PREV)
				{
					currentIndex = totalListNum;
				}	
				else if (playStatus == DB_PLAY_NEXT)
				{
					currentIndex ++;
				}					
				break;	
			case playmode_shuffle:
				if (playStatus == DB_PLAY_PREV)
				{
					currentIndex = 0;
				}	
				else if (playStatus == DB_PLAY_NEXT)
				{
					currentIndex ++;
				}					
				break;	
			case playmode_shuffle_repeat:
				if (playStatus == DB_PLAY_PREV)
				{
					currentIndex = totalListNum;
				}	
				else if (playStatus == DB_PLAY_NEXT)
				{
					currentIndex ++;
				}	
				break;								
				
		}				
	}
	else
	{		
		switch(playStatus)
		{
			case DB_PLAY_PREV:
				currentIndex --;
				break;
			case DB_PLAY_NEXT:
				currentIndex ++;
				break;
		}				
	}
	
	return (currentIndex);
}
#endif


void clear_setted_param (unsigned char userMode)
{
	doNotCheckIndexNum = FALSE;
	decleaseIndex = FALSE;
	changedQuickList = FALSE;
	changedMyRatingList = FALSE;
	doCpmparePrevPtr = FALSE;
	haveToStopPlayBack = FALSE;
	doCheckForMyRating = FALSE;

	gotoTitleListForMyRating = TRUE;	
	gotoTitleListForQuick = TRUE;
	changedTrackMyRatingVal = 0;
	makeIndexOne = FALSE;
	makeIndexTwo = FALSE;
	
	if (!userMode)
	{
		nextTrackIsNotExsit = TRUE;
		prevTrackIsNotExsit = TRUE;	
	}	
}

void clear_recently_played_time ()
{
	remainRectlyTime = 2; 
	curRectlyPlayedTime	 = 0;
}

void compare_setted_val (void)
{
	if (doNotCheckIndexNum) doNotCheckIndexNum = 0;
	if (decleaseIndex) decleaseIndex= 0;
	if (haveToStopPlayBack) haveToStopPlayBack = 0;
}

static void check_next_prev_available_for_myrating (int curTotalListNum, unsigned char curPlayMode)
{
	curPlayMode = PSound_Type->Play_Mode_SET;

	//curTotalListNum �� �̹� ������ �� �� ����	
	if ((curTotalListNum == curPlayingIndexNum) && curTotalListNum > 1) //�� ������ 2�� �̻��̰� ���� ����ϴ� ���� ������ ��
	{
		if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle) //Shuffle ���
		{
			if (curPlayMode == playmode_shuffle)
				prevTrackIsNotExsit = TRUE;
			else
				prevTrackIsNotExsit = FALSE;
			
			nextTrackIsNotExsit = FALSE;
		}
		else if (curPlayMode == playmode_none)//General ���
		{
			nextTrackIsNotExsit = FALSE;
			prevTrackIsNotExsit = FALSE;

			decleaseIndex = 1;
			debug("\n curPlayMode == playmode_none \n\n");
		}
		else if (curPlayMode == playmode_repeat || curPlayMode == playmode_repeat1) 
		{
			nextTrackIsNotExsit = FALSE;
			prevTrackIsNotExsit = FALSE;

			decleaseIndex = 1;
		}
	}
	// �Ѱ��� ���� ����
	else if (curPlayingIndexNum == 1 && curTotalListNum == 1 && curPlayingIndexNum <= curTotalListNum)
	{
		nextTrackIsNotExsit = FALSE;
		prevTrackIsNotExsit = TRUE;

		doNotCheckIndexNum = TRUE;
		if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_repeat) 
		{						
			prevTrackIsNotExsit = FALSE;	
			nextTrackIsNotExsit = FALSE;
		}			
	}
	else if (curPlayingIndexNum > curTotalListNum && curTotalListNum == 1) //������ ��
	{
		nextTrackIsNotExsit = FALSE;
		prevTrackIsNotExsit = TRUE;
		
		if (curPlayMode == playmode_none)
		{				
			nextTrackIsNotExsit = TRUE;
			prevTrackIsNotExsit = FALSE;
		}
	}		
	else if (curPlayingIndexNum > curTotalListNum && curTotalListNum > 1) //������ ��
	{
		nextTrackIsNotExsit = FALSE;
		prevTrackIsNotExsit = FALSE;
		
		if (curPlayMode == playmode_none)
		{		
			nextTrackIsNotExsit = TRUE;
		}
		
		if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle)
		{
			if (curPlayMode == playmode_shuffle) prevTrackIsNotExsit = TRUE;
			makeIndexOne = 1;
		}
	}				
	// ù ���ε� �ڽ��� ���� �� �� ������ 2�� �̻��� ����
	else if (curPlayingIndexNum == 1 && curTotalListNum > 1)  
	{
		nextTrackIsNotExsit = FALSE;
		prevTrackIsNotExsit = TRUE;

		doNotCheckIndexNum = TRUE;
		if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_repeat) 
		{
			//doNotCheckIndexNum = FALSE;
			if (curPlayMode == playmode_shuffle_repeat) prevTrackIsNotExsit = FALSE;
		}	
		if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_repeat || curPlayMode == playmode_repeat1) prevTrackIsNotExsit = FALSE;	
	}		
	else
	{
		nextTrackIsNotExsit = FALSE;
		prevTrackIsNotExsit = FALSE;
		
		decleaseIndex = 1;
		
		if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle)
		{
			if (curPlayMode == playmode_shuffle) prevTrackIsNotExsit = TRUE;
		}
	}			

	if (curPlayMode == playmode_repeat1) 
	{
		haveToStopPlayBack = TRUE;
		nextTrackIsNotExsit = TRUE;
		prevTrackIsNotExsit = TRUE;
	}	
}


static void check_next_prev_available_for_quicklist (int curTotalListNum, unsigned char curPlayMode)
{
	short curPlayTrackIndex = 0;
	
	curPlayTrackIndex = compare_quicklist();

	//���� ������� �ʴ� ���� ���� �� ���.
	if (curPlayTrackIndex != 0)
	{
		//curTotalListNum �� �̹� ������ �� �� ����	
		if (curTotalListNum == curPlayTrackIndex && curTotalListNum > 1) //�� ������ 2�� �̻��̰� ���� ����ϴ� ���� ������ ��
		{			
			if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle) //Shuffle ���
			{				
				nextTrackIsNotExsit = FALSE;
				if (curPlayMode == playmode_shuffle)
					prevTrackIsNotExsit = TRUE;	
				else
					prevTrackIsNotExsit = FALSE;
				
			}
			else if (curPlayMode == playmode_none)//General ��
			{
				nextTrackIsNotExsit = TRUE;
				prevTrackIsNotExsit = FALSE;
				debug("\n curPlayMode == playmode_none \n\n");
			}
			else if (curPlayMode == playmode_repeat || curPlayMode == playmode_repeat1)
			{
				nextTrackIsNotExsit = FALSE;
				prevTrackIsNotExsit = FALSE;
			}
		}
		else if (curPlayTrackIndex == 1 && curTotalListNum == 1) 
		{
			nextTrackIsNotExsit = TRUE;
			prevTrackIsNotExsit = TRUE;
			
			if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_repeat || curPlayMode == playmode_repeat1) //Shuffle ���
			{						
				prevTrackIsNotExsit = FALSE;	
				nextTrackIsNotExsit = FALSE;
			}
		}
		else
		{
			nextTrackIsNotExsit = FALSE;
			prevTrackIsNotExsit = FALSE;
			
			if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle)
			{				
				if (curPlayMode == playmode_shuffle) prevTrackIsNotExsit = TRUE;
			}
		}

		curPlayingIndexNum = curPlayTrackIndex;
	}
	//���� ����ϴ� ���� ���� �� ���
	else 
	{
		curIndexDoesNotExistInQuickList = TRUE;
		//debug("\n 8888888888888888888888888888888888 \n");
		//curTotalListNum �� �̹� ������ �� �� ����	
		if ((curTotalListNum == curPlayingIndexNum) && curTotalListNum > 1) //�� ������ 2�� �̻��̰� ���� ����ϴ� ���� ������ ��
		{
			if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle) //Shuffle ���
			{
				if (curPlayMode == playmode_shuffle)
					prevTrackIsNotExsit = TRUE;
				else
					prevTrackIsNotExsit = FALSE;
				
				nextTrackIsNotExsit = FALSE;
			}
			else if (curPlayMode == playmode_none)//General ���
			{
				nextTrackIsNotExsit = FALSE;
				prevTrackIsNotExsit = FALSE;

				decleaseIndex = 1;
				debug("\n curPlayMode == playmode_none \n\n");
			}
			else if (curPlayMode == playmode_repeat || curPlayMode == playmode_repeat1) 
			{
				nextTrackIsNotExsit = FALSE;
				prevTrackIsNotExsit = FALSE;
				decleaseIndex = 1;
			}
		}
		// �Ѱ��� ���� ����
		else if (curPlayingIndexNum == 1 && curTotalListNum == 1 && curPlayingIndexNum <= curTotalListNum)
		{
			nextTrackIsNotExsit = FALSE;
			prevTrackIsNotExsit = TRUE;

			doNotCheckIndexNum = TRUE;
			if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_repeat)// || curPlayMode == playmode_repeat1) 
			{						
				prevTrackIsNotExsit = FALSE;	
				nextTrackIsNotExsit = FALSE;
			}			
			else if (curPlayMode == playmode_repeat1) 
			{
				haveToStopPlayBack = TRUE;
				nextTrackIsNotExsit = TRUE;
			}				
		}
		else if (curPlayingIndexNum > curTotalListNum && curTotalListNum == 1) //������ ��
		{
			nextTrackIsNotExsit = FALSE;
			prevTrackIsNotExsit = TRUE;
			
			if (curPlayMode == playmode_none)
			{				
				nextTrackIsNotExsit = TRUE;
				prevTrackIsNotExsit = FALSE;
			}
			else if (curPlayMode == playmode_repeat1)
			{
				haveToStopPlayBack = 1;	
				nextTrackIsNotExsit = TRUE;
			}
			
			//curPlayingIndexNum = curTotalListNum;
		}		
		else if (curPlayingIndexNum > curTotalListNum && curTotalListNum > 1) //������ ��
		{
			nextTrackIsNotExsit = FALSE;
			prevTrackIsNotExsit = FALSE;
			
			if (curPlayMode == playmode_none)
			{		
				nextTrackIsNotExsit = TRUE;
			}
			else if (curPlayMode == playmode_repeat1)
			{
				haveToStopPlayBack = 1;			
			}
			
			if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle)
			{
				if (curPlayMode == playmode_shuffle) prevTrackIsNotExsit = TRUE;
				makeIndexOne = 1;
			}
			//curPlayingIndexNum = curTotalListNum;
		}				
		// ù ���ε� �ڽ��� ���� �� �� ������ 2�� �̻��� ����
		else if (curPlayingIndexNum == 1 && curTotalListNum > 1)  
		{
			nextTrackIsNotExsit = FALSE;
			prevTrackIsNotExsit = TRUE;

			doNotCheckIndexNum = TRUE;
			if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_repeat) 
			{
				//doNotCheckIndexNum = FALSE;
				if (curPlayMode == playmode_shuffle_repeat) prevTrackIsNotExsit = FALSE;
			}	
			if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_repeat || curPlayMode == playmode_repeat1) prevTrackIsNotExsit = FALSE;	
		}		
		else
		{
			nextTrackIsNotExsit = FALSE;
			prevTrackIsNotExsit = FALSE;
			
			decleaseIndex = 1;
			
			if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle)
			{
				if (curPlayMode == playmode_shuffle) prevTrackIsNotExsit = TRUE;
			}
		}			

		if (curPlayMode == playmode_repeat1) 
		{
			haveToStopPlayBack = TRUE;
			nextTrackIsNotExsit = TRUE;
			prevTrackIsNotExsit = TRUE;			
		}	
	}
}


static unsigned char calc_update_time (void)
{
	int remainTime = 0;
	
	remainTime = curTrackTotalTime/1.1;
	if (remainTime <= Clk.Play_msec/1000)
		return 1;
	else
		return 0;	
}

extern volatile int ui_ready;
unsigned char makeShuffleList = 0;
unsigned char saveQuickListTitle = 0;
void db_task(void *p_arg)
{
	int i;
	int totalListNum;
	unsigned char curPlayMode;
	static unsigned char quickListDelStatus = 0;
		
	OSTaskSuspend(DB_TASK_PRIO);

	for(i=0;i<3;i++){
		if(ui_ready){
			break;
		}
		OSTimeDly(100);
	}
	
	while(1)
	{
		if (get_cur_music_play_position() == MUSIC_MODE)
		{
			curPlayMode = PSound_Type->Play_Mode_SET;

			// ���� ����Ʈ �����.
			if (makeShuffleList && (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle))
			{
				if ((prevPlayModeStatus != playmode_shuffle_repeat && prevPlayModeStatus != playmode_shuffle) || changedMyRatingList || changedQuickList)
				{
					totalListNum = get_total_title_num();
					if (totalListNum > 0)	
					{
						if (changedMyRatingList && prevTitlePath == MYRATING_TITLE)
						{
							//������ ���� ���� ���� ��Ų ���
							if (curPlayingIndexNum > totalListNum)
							{
								db_make_shuffle_list(1, totalListNum - 1, curPlayMode);	
							}
							//������ ���� ���� ��Ų���� ���� ���ؽ��� ���ø���Ʈ ó���� �ְ� �ε����� �����Ű�� �ʴ´�.
							else
							{
								db_make_shuffle_list(1, curPlayingIndexNum - 1, curPlayMode);
								makeIndexOne = 1;
							}
						}
						else if (changedQuickList && prevTitlePath == MY_QUICKLISTS_TITLE)
						{
							if (quickListDelStatus)
							{
								// ������Ʈ���� ������ ���� ���� ��Ų ���
								if (curPlayingIndexNum > totalListNum)
								{
									db_make_shuffle_list(1, totalListNum - 1, curPlayMode);	
								}
								//������ ���� ���� ��Ų���� ���� ���ؽ��� ���ø���Ʈ ó���� �ְ� �ε����� �����Ű�� �ʴ´�.
								else
								{
									db_make_shuffle_list(1, curPlayingIndexNum - 1, curPlayMode);
									makeIndexOne = 1;
								}
								quickListDelStatus = 0;
							}
							// �� ����Ʈ�� ���� �߰� �� ���
							else 
							{
								db_make_shuffle_list(1, curPlayingIndexNum, curPlayMode);
								makeIndexOne = 1;							
							}
						}
						else	
						{
							db_make_shuffle_list(1, db_get_cur_index_for_display(), curPlayMode);	
						}
					}
					prevPlayModeStatus = curPlayMode;
					makeShuffleList = FALSE;
					if (!changedMyRatingList && !changedQuickList) curPlayingIndexNum = 1;
					if (reStartPlayback)  reStartPlayback = 0;
				}
			}	
				
			if (prevTitlePath == MYRATING_TITLE || prevTitlePath == MY_QUICKLISTS_TITLE)
			{
				if (prevTitlePath == MYRATING_TITLE && curTitlePath == MYRATING_TITLE && changedMyRatingVal) 
				{	
					if (curTrackMyRatingVal == changedTrackMyRatingVal)
					{
						clear_setted_param(1);		
						if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle)
						{
							db_make_shuffle_list(1, curPlayingIndexNum - 1, curPlayMode);
							makeIndexTwo = TRUE;
						}
					}
					else
					{
						changedMyRatingVal = FALSE;
						changedMyRatingList = TRUE;
						doCheckForMyRating = TRUE;	
						useTmpPtr = TRUE;
						if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle) makeShuffleList = TRUE;
					}
					changedMyRatingVal = FALSE;
				}
				 //Quicklist �󿡼��� Del �� AllDel
				else if (prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath == MY_QUICKLISTS_TITLE && didSomthingInQuickList)
				{
					didSomthingInQuickList = FALSE;
					changedQuickList= TRUE;
					doCpmparePrevPtr = TRUE;	
					quickListDelStatus = 1;
				}
				
				if (prevTitlePath == MY_QUICKLISTS_TITLE && curTitlePath != MY_QUICKLISTS_TITLE && didAddInQuickList)
				{
					//debug("\n==================== QUCIK (%d)========================", didAddInQuickList);

					//
					//�������� db_get_list_data()���� navibuf�� ���ڿ��� ���� �����ؼ� flash�� �Ѱ���µ� 
					//����� "name"���� ptr�� ������ ���� �Ѱ��ش�. �׷��Ƿ� db_get_list_data2()���� 
					//���� ���� �۾��� �� �ʿ䰡 ����. 
					//
#ifdef  NEW_PLAYALL_SEARCH			
					db_get_list_data2(DB_PLAYLISTS, 5, 0, NULL, 0);
#else 
					db_get_list_data2(DB_PLAYLISTS, 5, 0);
#endif
					//debug("\n==================== end QUCIK ========================\n");
					//db_save_title_list();
					didAddInQuickList = 0;
					saveQuickListTitle = TRUE;
					changedQuickList= TRUE;
					
					quickListDelStatus = 0;
				}
			}		
			
			if (checkDBTask && !changedQuickList && !haveToStopPlayBack && !changedMyRatingList)
			{				
				totalListNum = get_total_title_num();

				//shuffle -> normal
				if ((curPlayMode != playmode_shuffle_repeat && curPlayMode != playmode_shuffle) && (prevPlayModeStatus == playmode_shuffle || prevPlayModeStatus == playmode_shuffle_repeat))
				{				
					curPlayingIndexNum = db_get_index_for_ui();
				}

				if (totalListNum == curPlayingIndexNum && totalListNum > 1) //������ ��
				{
					if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle) //Shuffle ���
					{						
						prevTrackIsNotExsit = FALSE;
						
						if (prevPlayModeStatus == playmode_shuffle && curPlayMode == playmode_shuffle_repeat)						
						{						
							nextTrackIsNotExsit = FALSE;						
						}
						else if (prevPlayModeStatus == playmode_shuffle_repeat && curPlayMode == playmode_shuffle) 
						{							
							nextTrackIsNotExsit = TRUE;
						}	
						else if (prevPlayModeStatus != playmode_shuffle && prevPlayModeStatus != playmode_shuffle_repeat) //������ ���̶� �� ���� shuffle list �ٽ� ����
						{
							nextTrackIsNotExsit = FALSE;
							prevTrackIsNotExsit = TRUE;
						}
						else if (prevPlayModeStatus == playmode_shuffle && curPlayMode == playmode_shuffle)
						{
							nextTrackIsNotExsit = TRUE;						
						}
						else if (prevPlayModeStatus == playmode_shuffle_repeat && curPlayMode == playmode_shuffle_repeat)
						{
							doMakeShuffleAgain = TRUE;
							nextTrackIsNotExsit = FALSE;	
						}
						else if ((prevPlayModeStatus == playmode_shuffle_repeat || prevPlayModeStatus == playmode_shuffle) && curPlayMode == playmode_shuffle)
						{
							nextTrackIsNotExsit = TRUE;	
						}								
						
					}
					else if (curPlayMode == playmode_none)//General ��
					{
						nextTrackIsNotExsit = TRUE;
						prevTrackIsNotExsit = FALSE;
						//debug("\n curPlayMode == playmode_none \n\n");
					}
					else if (curPlayMode == playmode_repeat || curPlayMode == playmode_repeat1)
					{
						nextTrackIsNotExsit = FALSE;
						prevTrackIsNotExsit = FALSE;
					}
				}
				else if (curPlayingIndexNum == 1 && totalListNum > 1) //ù ��
				{
					nextTrackIsNotExsit = FALSE;
					prevTrackIsNotExsit = TRUE;
					
					if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle) //Shuffle ���
					{						
						if (prevPlayModeStatus != playmode_shuffle && prevPlayModeStatus != playmode_shuffle_repeat) //������ ���̶� �� ���� shuffle list �ٽ� ����
						{
							prevTrackIsNotExsit = FALSE;
						}
						else if (curPlayMode == playmode_shuffle_repeat)
						{
							prevTrackIsNotExsit = FALSE;
						}			
					}
					else if (curPlayMode == playmode_repeat || curPlayMode == playmode_repeat1)
					{
						prevTrackIsNotExsit = FALSE;
					}					
				}
				else if (curPlayingIndexNum == 1 && totalListNum == 1) //ù ��
				{
					nextTrackIsNotExsit = TRUE;
					prevTrackIsNotExsit = TRUE;
					
					if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_repeat || curPlayMode == playmode_repeat1) //Shuffle ���
					{						
						prevTrackIsNotExsit = FALSE;
						nextTrackIsNotExsit = FALSE;			
					}
				}	
				else
				{
					nextTrackIsNotExsit = FALSE;
					prevTrackIsNotExsit = FALSE;
				}

				//���� �� ������ 0�̸� ������ ����.
				if (totalListNum <= 0)				
				{
					nextTrackIsNotExsit = TRUE;
					prevTrackIsNotExsit = TRUE;
				}
				
				checkDBTask = 0;
			
			}	
			else if (checkDBTask && changedQuickList)
			{				
//				db_save_title_list();   //angelo - ���� ���� pulPlayPtr�� ������ �ʿ� ����. 	
				totalListNum = get_total_title_num();
				// Quicklist���� ���� ���� or �߰� �� ���	
				if (prevTitlePath == MY_QUICKLISTS_TITLE)
				{
					if (curPlayMode == playmode_shuffle_repeat || curPlayMode == playmode_shuffle) 
					{
						makeShuffleList = 1;						
					}	
					//shuffle -> normal
					else if ((curPlayMode != playmode_shuffle_repeat && curPlayMode != playmode_shuffle) && (prevPlayModeStatus == playmode_shuffle || prevPlayModeStatus == playmode_shuffle_repeat))
					{				
						curPlayingIndexNum = db_get_index_for_ui();
					}
					
					compare_setted_val();
					check_next_prev_available_for_quicklist(totalListNum, curPlayMode);						
					didDelInQuickList = 0;				
				}	
				
				//���� �� ������ 0�̸� ������ ����.
				if (totalListNum <= 0)				
				{
					nextTrackIsNotExsit = TRUE;
					prevTrackIsNotExsit = TRUE;
				}
				
				checkDBTask = 0;				
			}
			else if (checkDBTask && changedMyRatingList)
			{
				totalListNum = get_total_title_num();
				// MyRating���� ������� ���� �� ���	
				if (prevTitlePath == MYRATING_TITLE)
				{	//shuffle -> normal
					if ((curPlayMode != playmode_shuffle_repeat && curPlayMode != playmode_shuffle) && (prevPlayModeStatus == playmode_shuffle || prevPlayModeStatus == playmode_shuffle_repeat))
					{				
						curPlayingIndexNum = db_get_index_for_ui();
					}					
					compare_setted_val();
					check_next_prev_available_for_myrating(totalListNum, curPlayMode);														
				}	
				
				//���� �� ������ 0�̸� ������ ����.
				if (totalListNum <= 0)				
				{
					nextTrackIsNotExsit = TRUE;
					prevTrackIsNotExsit = TRUE;
				}
				
				checkDBTask = 0;							
			}
			//else
			//{	//���� �� ������ 0�̸� ������ ����.
			if (get_total_title_num() <= 0)				
			{
				nextTrackIsNotExsit = TRUE;
				prevTrackIsNotExsit = TRUE;
			}			
			//}
			
			//playcount & lastedplay ���� 
			if ( calc_update_time() && (startedPlayingIndexNum > 0) && dbUpdateTimeFlag)
			{
//				db_update_playlists_val();
#if 0  // ori
				fwui_write_playCount_value(gTempPlayingIndexNum+1);
#else  //angelo
				send_ui_eventw(stringPlayCountUpdate);
				IncreaseExecuteEvent();
#endif
				clear_recently_played_time();
				dbUpdateTimeFlag = 0;
			}
		}
		OSTimeDly(10);
	}	
}



void fwui_PlayCountUpdate(void)
{
	fwui_write_playCount_value(gTempPlayingIndexNum+1);
}





/////////////////////////////////////////////////////////////////////////////////////////
//
//
//								new fwui functions
//	
//
/////////////////////////////////////////////////////////////////////////////////////////
#ifdef CONFIG_FWUI

void fwui_makeShufflePlay(unsigned char startFirst, 
							unsigned short curPlayingIndex, 
							unsigned char shuffle_mode,
							unsigned short total)
{
	unsigned long i;
	unsigned short usRandomNumber, usRawRandomNum;
	unsigned short usTemp = START_SHUFLE_LIST;
	static unsigned short usTotalShuffleNum = 0;

	usTotalShuffleNum = total;

	if (usTotalShuffleNum == 0) return;
	

   	fill_randomnumber_for_db(0);
	
	if (startFirst)
	{
		MarkRandomUsed(curPlayingIndex);
		ShuffleList[0] = curPlayingIndex;		
		
		makeShuffleFirst = 1;
		
	}
	else
	{
		MarkRandomUsed(curPlayingIndex);
		
		if (doMakeShuffleAgain)
		{			
			ShuffleList[usTotalShuffleNum - 1] = curPlayingIndex;			
		}
		else
		{
			ShuffleList[0] = curPlayingIndex;
		}

	}

	for(i = START_SHUFLE_LIST; i < usTotalShuffleNum;  ) 
	{		
		usTemp++;

		usRawRandomNum = GetRawRandomNum(usTotalShuffleNum);

		usRandomNumber = (usRawRandomNum) % (usTotalShuffleNum);

		if( ((i + 2) >= usTotalShuffleNum) || (usTemp > 500) ) 
		{			
			unsigned short usNum;

			if(doMakeShuffleAgain)
			{
				for(usNum = START_SHUFLE_LIST; usNum < usTotalShuffleNum; usNum++) 
				{
					if(!CheckRandomUsed(usNum)) 
					{					
						ShuffleList[i] = usNum;
		
	            		MarkRandomUsed(usNum);

						i++;	
						usTemp = START_SHUFLE_LIST;								
					}	
				}
				if (usTemp > START_SHUFLE_LIST) 
					break;
			}	
			else
			{
				for(usNum = START_SHUFLE_LIST; usNum < usTotalShuffleNum; usNum++) 
				{
					if(!CheckRandomUsed(usNum)) 
					{					
							ShuffleList[i+1] = usNum;

		            		MarkRandomUsed(usNum);
						
						i++;
						usTemp = START_SHUFLE_LIST;
					}	
				}
				if(usTemp > START_SHUFLE_LIST) 					
					break;
			}				
		} 
		else 
		{
			if(!CheckRandomUsed(usRandomNumber)) 
			{	

				if(doMakeShuffleAgain)// && !startFirst)
				{
					ShuffleList[i] = usRandomNumber;
				}
				else
				{
					ShuffleList[i+1] = usRandomNumber;
			
				}	

           		MarkRandomUsed(usRandomNumber);
				
				i++;				
				usTemp = START_SHUFLE_LIST;
			}		
		}	
	}		
	doMakeShuffleAgain = 0;
}


unsigned char fwui_get_NextTrack(void)
{
	return isNextSong;
}

void fwui_set_NextTrack(unsigned char value)
{
	isNextSong = value;
}


unsigned char fwui_get_NextTrackIsNotExist(void)
{
	return nextTrackIsNotExsit;
}



#endif	//#ifdef CONFIG_FWUI

#endif //#ifdef CONFIG_AVLDB

#endif //_DBTASK_C	
