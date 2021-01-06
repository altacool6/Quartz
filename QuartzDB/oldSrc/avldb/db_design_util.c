#include "avldb.h"
#include "db_design.h"
#include "db_design_util.h"

//=============================================================//
//			각 테이블의 Uid를 가지는 레코드의 Rowid를 받아내는 함수이다.(Begin)				//
//=============================================================//
unsigned long GetRowidOfMusicTableByUid(unsigned long vUid)
{
	//함수 설명 : Music Table에서 Uid값을 가지는 Record의 Rowid를 구하는 함수
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned long		vRowids[10];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	vIndexColumn[0]	= gDbColumnNameUid;		//인덱스를 지정한다.
	vConditions[0]	= &vUid;
	
	vSearchTotal = dbApiSearchMS(gDbTableNameMusic, 
				 			  	vIndexColumn,	1, 
							      	vConditions,		1,
							      	DB_ASC, 
							      	vRowids, 
							      	#ifdef DB_SEARCH_CHANGED
								10,
								#endif
							      	&vRefMemory);
	if (vSearchTotal == 1)
	{
		return vRowids[0];
	}
	else
	{
		return 0;
	}
}

unsigned long GetRowidOfAlbumTableByUid(unsigned long vUid)
{
	//함수 설명 : Album Table에서 Uid값을 가지는 Record의 Rowid를 구하는 함수
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned long		vRowids[10];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	vIndexColumn[0]	= gDbColumnNameUid;		//인덱스를 지정한다.
	vConditions[0]	= &vUid;
	
	vSearchTotal = dbApiSearchMS(gDbTableNameAlbum, 
				 			  	vIndexColumn,	1, 
							      	vConditions,		1,
							      	DB_ASC, 
							      	vRowids, 
							      	#ifdef DB_SEARCH_CHANGED
								10,
								#endif
							      	&vRefMemory);
	if (vSearchTotal == 1)
	{
		return vRowids[0];
	}
	else
	{
		return 0;
	}
}

unsigned long GetRowidOfPhotoTableByUid(unsigned long vUid)
{
	//함수 설명 : Photo Table에서 Uid값을 가지는 Record의 Rowid를 구하는 함수
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned long		vRowids[10];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	vIndexColumn[0]	= gDbColumnNameUid;		//인덱스를 지정한다.
	vConditions[0]	= &vUid;
	
	vSearchTotal = dbApiSearchMS(gDbTableNamePhoto, 
				 			  	vIndexColumn,	1, 
							      	vConditions,		1,
							      	DB_ASC, 
							      	vRowids, 
							      	#ifdef DB_SEARCH_CHANGED
								10,
								#endif
							      	&vRefMemory);
	if (vSearchTotal == 1)
	{
		return vRowids[0];
	}
	else
	{
		return 0;
	}
}

unsigned long GetRowidOfObjectsTableByUid(unsigned long vUid)
{
	//함수 설명 : Photo Table에서 Uid값을 가지는 Record의 Rowid를 구하는 함수
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned long		vRowids[10];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	vIndexColumn[0]	= gDbColumnNameUid;		//인덱스를 지정한다.
	vConditions[0]	= &vUid;
	
	vSearchTotal = dbApiSearchMS(gDbTableNameObjects, 
				 			  	vIndexColumn,	1, 
							      	vConditions,		1,
							      	DB_ASC, 
							      	vRowids, 
							      	#ifdef DB_SEARCH_CHANGED
								10,
								#endif
							      	&vRefMemory);
	if (vSearchTotal == 1)
	{
		return vRowids[0];
	}
	else
	{
		return 0;
	}
}

unsigned long GetRowidOfRefferanceTableByParentUid(	unsigned long vUid, 
												   			unsigned long *vRowids
															#ifdef DB_SEARCH_CHANGED
															,unsigned long vRowidsArraySize
															#endif
														)
{
	//함수 설명 : Refferance Table에서 Uid값을 가지는 Record의 Rowid를 구하는 함수
	//                리턴값은 Refferance에서 ParentUid가 인자값으로 받은 vUid를 가지는 Record의 갯수를 리턴한다.
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	vIndexColumn[0]	= gDbColumnNameParentCluster;		//인덱스를 지정한다.
	vConditions[0]	= &vUid;
	
	vSearchTotal = dbApiSearchMS(gDbTableNameReferences, 
				 			  	vIndexColumn,	1, 
							      	vConditions,		1,
							      	DB_ASC, 
							      	vRowids, 
							      	#ifdef DB_SEARCH_CHANGED
								vRowidsArraySize,
								#endif
							      	&vRefMemory);
	return vSearchTotal;
}

unsigned long GetRowidOfRefferanceTableByChildUid(unsigned long vUid, 
														unsigned long *vRowids
														#ifdef DB_SEARCH_CHANGED
														,unsigned long vRowidsArraySize
														#endif
														)
{
	//함수 설명 : Refferance Table에서 Uid값을 가지는 Record의 Rowid를 구하는 함수
	//                리턴값은 Refferance에서 ChildUid가 인자값으로 받은 vUid를 가지는 Record의 갯수를 리턴한다.
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	vIndexColumn[0]	= gDbColumnNameChildCluster;		//인덱스를 지정한다.
	vConditions[0]	= &vUid;
	
	vSearchTotal = dbApiSearchMS(gDbTableNameReferences, 
				 			  	vIndexColumn,	1, 
							      	vConditions,		1,
							      	DB_ASC, 
							      	vRowids, 
							      	#ifdef DB_SEARCH_CHANGED
								vRowidsArraySize,
								#endif
							      	&vRefMemory);
	return vSearchTotal;
}
//=============================================================//
//			각 테이블의 Uid를 가지는 레코드의 Rowid를 받아내는 함수이다.(E n d)				//
//=============================================================//


//=============================================================//
//			Object Table의 Uid를 가지는 레코드의 Rowid를 받아내는 함수이다.(Begin)			//
//=============================================================//
int CheckObjectUid(	unsigned long uId, 
						unsigned long * rowId)
{
	//이 함수의 맹점 :  만약 디비에 동일한 uid로 검색된 결과가 10개 이상 검색될 경우
	//				시스템을 죽일수 있다.
	//				그런데 동일한uid가 검색된것은 그것 자체가 문제다.
	
	unsigned short* 	indexColumnNames[1];	//인덱스를 저장할 변수
	void* 				conditionSet[1];			//조건을 저장할 변수	
	unsigned long		resultArray[10];			//검색된 결과를 저장할 변수
	unsigned long		totalSearchCount;		//검색된 갯수를 저장할 변수
	unsigned char		refMemory;

	indexColumnNames[0]	= gDbColumnNameUid;	//인덱스를 지정한다.
	conditionSet[0] 		= &uId;					//조건을 지정한다.

	
	totalSearchCount = dbApiSearchMS(gDbTableNameObjects, 
				     				  indexColumnNames, 1, 
								      conditionSet, 1,
								      DB_ASC, 
								      resultArray,
								      #ifdef DB_SEARCH_CHANGED
									10,
									#endif
								      &refMemory);

	if (totalSearchCount==1)
	{
		//해당 Uid의 Object를 하나 찾은 경우
		*rowId = resultArray[0];
		return FUNCTION_SUCCESS;
	}

	else if (totalSearchCount == 0)
	{
		//해당 Uid의 Object가 존재하지 않는 경우
		return FUNCTION_FAILED;
	}
	else
	{
		//해당 Uid의 Object가 하나이상이 경우
		if (totalSearchCount>10)
		{
			;//이때 시스템이 죽을수 있다.
		}
		else
		{
			;//이경우에는 시스템을 죽이지는 않는다.
		}
		return FUNCTION_FAILED;
	}
	
}
int FindUid(unsigned short* fileFullPathName, unsigned long* uId)
{
	unsigned long*	rowIds;
	unsigned long 		totalSearchCount;
	unsigned short*	indexColumnNames[1];
	unsigned char		refMemory;
	void*			conditionSet[1];
	unsigned short*	pCurrentPosition;
	unsigned short*	pEndPosition;
	unsigned short*	pStartPosition;

	//문자열의 시작을 저장한다.
	pStartPosition = fileFullPathName;

	//문자열을 끝지점을 찾는다.
	pEndPosition = fileFullPathName;
	while(*pEndPosition)
	{
		pEndPosition++;
	}

	//문자열 검색의 시작 지점을 문자열 끝지점으로 설정한다.
	pCurrentPosition = pEndPosition;

	//지금 검색할 문자열의 위치가 파일 명이 되도록 이동한다.
	while(1)
	{
		if (pCurrentPosition == fileFullPathName)
		{
			//전달 받은 문자열이 옳지 않은 경우
			return FUNCTION_FAILED;
		}
		if ((*pCurrentPosition)=='/')
		{
			break;
		}
		pCurrentPosition--;
	}

	indexColumnNames[0]	= gDbColumnNameObjectName;
	conditionSet[0] 		= (pCurrentPosition+1);
	
	//디비에서 검색된 Uid들을 저장할 공간이다.
	rowIds = (unsigned long*)malloc(sizeof(unsigned long)*MAX_OBJECT_COUNT);

	totalSearchCount = dbApiSearchMS(gDbTableNameObjects, 
				     				      indexColumnNames, 1, 
								      conditionSet, 1,
								      DB_ASC, 
								      rowIds, 
								      #ifdef DB_SEARCH_CHANGED
									MAX_OBJECT_COUNT,
									#endif
								      &refMemory);
	if (totalSearchCount == 0)
	{
		//해당파일을 DB에 저장되어 있지 않다.
		free(rowIds);
		return FUNCTION_FAILED;		
	}
	else if (totalSearchCount == 1)
	{
		//지금 검색된 Record가 Uid를 가지고 있다.
		void* columnValues[DB_OBJECT_TABLE_COUNT];

		//stack을 할당받았기 때문에 초기화 한다.
		memset(columnValues, 0, sizeof(columnValues));

		//검색을 원하는 컬럼인 Uid에 해당 주소를 연결한다.
		columnValues[OBJECT_TABLE_INDEX_UID] = uId;
		
		//해당 Rowid를 가진 Record의 parentUId를 채워온다.
 		dbApiGetRecordCopy(gDbTableNameObjects, *rowIds, columnValues);
		//위의 함수가 실행되고 나면 uId에는 해당 레코드의 uId값이 채워진다.

		//힙메모리를 제거한다.
 		free(rowIds);
		
		return FUNCTION_SUCCESS;
	}
	else
	{
		//지금 검색된 Record가 Uid를 가지고 있다.
		void* columnValues[DB_OBJECT_TABLE_COUNT];
		unsigned short* targetBuffer;

		targetBuffer = (unsigned short*)malloc(514);
		
		//stack을 할당받았기 때문에 초기화 한다.
		memset(columnValues, 0, sizeof(columnValues));
		
		//검색을 원하는 컬럼인 Uid에 해당 주소를 연결한다.
		columnValues[OBJECT_TABLE_INDEX_UID] = uId;

		
		while(totalSearchCount)
		{
			totalSearchCount--;
	 		dbApiGetRecordCopy(gDbTableNameObjects, *(rowIds + totalSearchCount), columnValues);
			FillObjectPath(*uId, targetBuffer, FILE_PATH_AND_FILE_NAME);
			
			if (CompareUnicodeString(targetBuffer, fileFullPathName)==1)
			{
				//힙메모리를 제거한다.
		 		free(rowIds);
				free(targetBuffer);
				return FUNCTION_SUCCESS;
			}
		}
		
		//힙메모리를 제거한다.
 		free(rowIds);
		free(targetBuffer);
		return FUNCTION_FAILED;
	}
}

//unsigned short	debuggingTest[256];

int FillObjectPath(unsigned long uId, unsigned short* targetBuffer, unsigned long pathOnlyOrFullPath)
{
	unsigned long rowId;						//Objects Table에서 Rowid의 값을 저장할 변수
	unsigned long parentUId;					
	unsigned long depthReverseCounter = 0;	//directory의 Depth를 역으로 카운트 할 변수
	void*		columnValues[DB_OBJECT_TABLE_COUNT];
	unsigned long objectsRowIdArray[DEVICE_DIRECTORY_DEPTH + 2];


	memset(columnValues, 0, sizeof(columnValues));
	columnValues[OBJECT_TABLE_INDEX_PARENTUID] = &parentUId;	//Parent Rowid만을 받아올것이다.
//	columnValues[OBJECT_TABLE_INDEX_NAME] = debuggingTest;
	
	while(uId)	//root의 uid의 값은 0이므로 root 를 만나면 탈출한다.
	{
		//uId의 Object가 있는지를 검사한다.
		if (CheckObjectUid(uId, &rowId) == FUNCTION_FAILED)
		{
			return FUNCTION_FAILED;
		}
		//위의 if문을 빠져나왔다면 rowid변수에 해당 Rowid가 검색되어 있다.

		//해당 Object Record의 Object Rowid를 찾는다.
		objectsRowIdArray[depthReverseCounter] = rowId;
		
		depthReverseCounter++;

		//해당 Rowid를 가진 Record의 parentUId를 채워온다.
 		dbApiGetRecordCopy(gDbTableNameObjects, rowId, columnValues);	//Parent의 Uid만을 채워온다.

		//이번에 검색할 record의 uid는 Parent uid이다.
		uId = parentUId;	

		if (uId==0)
		{
			break;
			/*
			//root의 Rowid를 찾아낸다.
			if (CheckObjectUid(uId, &rowId) == FUNCTION_FAILED)
			{
				//root를 찾을 수 없는 경우
				return FUNCTION_FAILED;
			}
			objectsRowIdArray[depthReverseCounter] = rowId;			
			*/
		}
	}
	//위의 while문을 빠져 나오면 Object들의 Rowid들이 objectsRowIdArray에 들어 있다.

	//=======uid값이 0일경우 Error 처리추가(begin)=====//
	if (depthReverseCounter == 0)			//error check
	{									//error check
		return FUNCTION_FAILED;			//error check
	}									//error check
	//=======uid값이 0일경우 Error 처리추가(e n d)=====//
	
	//====초기화====//
	memset(columnValues, 0, sizeof(columnValues));
	*targetBuffer = 0;
	depthReverseCounter--;
	//====초기화====//
	
	//path를 복사한다.
	while(depthReverseCounter)
	{
		
		columnValues[OBJECT_TABLE_INDEX_NAME] = targetBuffer;	//objectname을 복사할 주소공간 mapping
		
	 	dbApiGetRecordCopy(gDbTableNameObjects, objectsRowIdArray[depthReverseCounter], columnValues);	//Parent의 Uid만을 채워온다.

		//targetBuffer가  문자열의 끝을 가르키도록 한다. (begin)
		while(*targetBuffer)
		{
			targetBuffer++;
		}
		//targetBuffer가  문자열의 끝을 가르키도록 한다. (end)

	 	depthReverseCounter--;		
	}
	//여기까지 File Path는 가져왔다.

	if (pathOnlyOrFullPath == FILE_PATH_AND_FILE_NAME)
	{		
		//파일명까지 원할때
		columnValues[OBJECT_TABLE_INDEX_NAME] = targetBuffer;	//objectname을 복사할 주소공간 mapping
		dbApiGetRecordCopy(gDbTableNameObjects, objectsRowIdArray[depthReverseCounter], columnValues);	//Parent의 Uid만을 채워온다.			
	}
	else if(pathOnlyOrFullPath == FILE_PATH)
	{
		//Path만 가져가는 경우 '/'를 제거 해야 한다.
		/*
		while(*targetBuffer)
		{
			targetBuffer++;
		}
		//targetBuffer가 NULL인곳에서 빠져 나간다.
		if ((*(targetBuffer - 1)=='/')||(*(targetBuffer - 1)=='\\'))
		{
			*(targetBuffer - 1) = 0;
		}
		else
		{
			return FUNCTION_FAILED;
		}
		*/
	}
	else
	{
		//이외의 값은 취급하지 않는다.
		return FUNCTION_FAILED;
	}

	return FUNCTION_SUCCESS;
}

int FillObjectName(unsigned long uId, unsigned short* targetBuffer)
{
	unsigned long rowId;									//Objects Table에서 Rowid의 값을 저장할 변수
//	unsigned short*	CharPos;
	void*		columnValues[DB_OBJECT_TABLE_COUNT];

	//uId의 Object가 있는지를 검사한다.
	if (CheckObjectUid(uId, &rowId) == FUNCTION_FAILED)
	{
		return FUNCTION_FAILED;
	}
	//위의 if문을 빠져나왔다면 rowid변수에 해당 Rowid가 검색되어 있다.


	//컬럼의 값을 받아올 주소공간이 저장된 배열을 초기화 한다.
	memset(columnValues, 0, sizeof(columnValues));
	columnValues[OBJECT_TABLE_INDEX_NAME] = targetBuffer;		//Object Name은 받아올 것이기 때문에 해당 주소를 링크한다.
	
	//해당 Object의 Name을 받아온다.
	dbApiGetRecordCopy(gDbTableNameObjects,
						rowId, columnValues);

//	CharPos = (unsigned short*)wcsrchr( targetBuffer, '/' );
//	if( *CharPos == '/' )	*CharPos = 0;

	return FUNCTION_SUCCESS;
}
//=============================================================//
//			Object Table의 Uid를 가지는 레코드의 Rowid를 받아내는 함수이다.(E n d)			//
//=============================================================//


