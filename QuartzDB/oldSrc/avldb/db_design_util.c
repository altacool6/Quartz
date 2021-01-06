#include "avldb.h"
#include "db_design.h"
#include "db_design_util.h"

//=============================================================//
//			�� ���̺��� Uid�� ������ ���ڵ��� Rowid�� �޾Ƴ��� �Լ��̴�.(Begin)				//
//=============================================================//
unsigned long GetRowidOfMusicTableByUid(unsigned long vUid)
{
	//�Լ� ���� : Music Table���� Uid���� ������ Record�� Rowid�� ���ϴ� �Լ�
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned long		vRowids[10];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	vIndexColumn[0]	= gDbColumnNameUid;		//�ε����� �����Ѵ�.
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
	//�Լ� ���� : Album Table���� Uid���� ������ Record�� Rowid�� ���ϴ� �Լ�
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned long		vRowids[10];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	vIndexColumn[0]	= gDbColumnNameUid;		//�ε����� �����Ѵ�.
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
	//�Լ� ���� : Photo Table���� Uid���� ������ Record�� Rowid�� ���ϴ� �Լ�
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned long		vRowids[10];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	vIndexColumn[0]	= gDbColumnNameUid;		//�ε����� �����Ѵ�.
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
	//�Լ� ���� : Photo Table���� Uid���� ������ Record�� Rowid�� ���ϴ� �Լ�
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned long		vRowids[10];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	vIndexColumn[0]	= gDbColumnNameUid;		//�ε����� �����Ѵ�.
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
	//�Լ� ���� : Refferance Table���� Uid���� ������ Record�� Rowid�� ���ϴ� �Լ�
	//                ���ϰ��� Refferance���� ParentUid�� ���ڰ����� ���� vUid�� ������ Record�� ������ �����Ѵ�.
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	vIndexColumn[0]	= gDbColumnNameParentCluster;		//�ε����� �����Ѵ�.
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
	//�Լ� ���� : Refferance Table���� Uid���� ������ Record�� Rowid�� ���ϴ� �Լ�
	//                ���ϰ��� Refferance���� ChildUid�� ���ڰ����� ���� vUid�� ������ Record�� ������ �����Ѵ�.
	unsigned short*	vIndexColumn[1];
	void*			vConditions[1];
	unsigned char		vRefMemory;
	unsigned long		vSearchTotal;

	vIndexColumn[0]	= gDbColumnNameChildCluster;		//�ε����� �����Ѵ�.
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
//			�� ���̺��� Uid�� ������ ���ڵ��� Rowid�� �޾Ƴ��� �Լ��̴�.(E n d)				//
//=============================================================//


//=============================================================//
//			Object Table�� Uid�� ������ ���ڵ��� Rowid�� �޾Ƴ��� �Լ��̴�.(Begin)			//
//=============================================================//
int CheckObjectUid(	unsigned long uId, 
						unsigned long * rowId)
{
	//�� �Լ��� ���� :  ���� ��� ������ uid�� �˻��� ����� 10�� �̻� �˻��� ���
	//				�ý����� ���ϼ� �ִ�.
	//				�׷��� ������uid�� �˻��Ȱ��� �װ� ��ü�� ������.
	
	unsigned short* 	indexColumnNames[1];	//�ε����� ������ ����
	void* 				conditionSet[1];			//������ ������ ����	
	unsigned long		resultArray[10];			//�˻��� ����� ������ ����
	unsigned long		totalSearchCount;		//�˻��� ������ ������ ����
	unsigned char		refMemory;

	indexColumnNames[0]	= gDbColumnNameUid;	//�ε����� �����Ѵ�.
	conditionSet[0] 		= &uId;					//������ �����Ѵ�.

	
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
		//�ش� Uid�� Object�� �ϳ� ã�� ���
		*rowId = resultArray[0];
		return FUNCTION_SUCCESS;
	}

	else if (totalSearchCount == 0)
	{
		//�ش� Uid�� Object�� �������� �ʴ� ���
		return FUNCTION_FAILED;
	}
	else
	{
		//�ش� Uid�� Object�� �ϳ��̻��� ���
		if (totalSearchCount>10)
		{
			;//�̶� �ý����� ������ �ִ�.
		}
		else
		{
			;//�̰�쿡�� �ý����� �������� �ʴ´�.
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

	//���ڿ��� ������ �����Ѵ�.
	pStartPosition = fileFullPathName;

	//���ڿ��� �������� ã�´�.
	pEndPosition = fileFullPathName;
	while(*pEndPosition)
	{
		pEndPosition++;
	}

	//���ڿ� �˻��� ���� ������ ���ڿ� ���������� �����Ѵ�.
	pCurrentPosition = pEndPosition;

	//���� �˻��� ���ڿ��� ��ġ�� ���� ���� �ǵ��� �̵��Ѵ�.
	while(1)
	{
		if (pCurrentPosition == fileFullPathName)
		{
			//���� ���� ���ڿ��� ���� ���� ���
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
	
	//��񿡼� �˻��� Uid���� ������ �����̴�.
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
		//�ش������� DB�� ����Ǿ� ���� �ʴ�.
		free(rowIds);
		return FUNCTION_FAILED;		
	}
	else if (totalSearchCount == 1)
	{
		//���� �˻��� Record�� Uid�� ������ �ִ�.
		void* columnValues[DB_OBJECT_TABLE_COUNT];

		//stack�� �Ҵ�޾ұ� ������ �ʱ�ȭ �Ѵ�.
		memset(columnValues, 0, sizeof(columnValues));

		//�˻��� ���ϴ� �÷��� Uid�� �ش� �ּҸ� �����Ѵ�.
		columnValues[OBJECT_TABLE_INDEX_UID] = uId;
		
		//�ش� Rowid�� ���� Record�� parentUId�� ä���´�.
 		dbApiGetRecordCopy(gDbTableNameObjects, *rowIds, columnValues);
		//���� �Լ��� ����ǰ� ���� uId���� �ش� ���ڵ��� uId���� ä������.

		//���޸𸮸� �����Ѵ�.
 		free(rowIds);
		
		return FUNCTION_SUCCESS;
	}
	else
	{
		//���� �˻��� Record�� Uid�� ������ �ִ�.
		void* columnValues[DB_OBJECT_TABLE_COUNT];
		unsigned short* targetBuffer;

		targetBuffer = (unsigned short*)malloc(514);
		
		//stack�� �Ҵ�޾ұ� ������ �ʱ�ȭ �Ѵ�.
		memset(columnValues, 0, sizeof(columnValues));
		
		//�˻��� ���ϴ� �÷��� Uid�� �ش� �ּҸ� �����Ѵ�.
		columnValues[OBJECT_TABLE_INDEX_UID] = uId;

		
		while(totalSearchCount)
		{
			totalSearchCount--;
	 		dbApiGetRecordCopy(gDbTableNameObjects, *(rowIds + totalSearchCount), columnValues);
			FillObjectPath(*uId, targetBuffer, FILE_PATH_AND_FILE_NAME);
			
			if (CompareUnicodeString(targetBuffer, fileFullPathName)==1)
			{
				//���޸𸮸� �����Ѵ�.
		 		free(rowIds);
				free(targetBuffer);
				return FUNCTION_SUCCESS;
			}
		}
		
		//���޸𸮸� �����Ѵ�.
 		free(rowIds);
		free(targetBuffer);
		return FUNCTION_FAILED;
	}
}

//unsigned short	debuggingTest[256];

int FillObjectPath(unsigned long uId, unsigned short* targetBuffer, unsigned long pathOnlyOrFullPath)
{
	unsigned long rowId;						//Objects Table���� Rowid�� ���� ������ ����
	unsigned long parentUId;					
	unsigned long depthReverseCounter = 0;	//directory�� Depth�� ������ ī��Ʈ �� ����
	void*		columnValues[DB_OBJECT_TABLE_COUNT];
	unsigned long objectsRowIdArray[DEVICE_DIRECTORY_DEPTH + 2];


	memset(columnValues, 0, sizeof(columnValues));
	columnValues[OBJECT_TABLE_INDEX_PARENTUID] = &parentUId;	//Parent Rowid���� �޾ƿð��̴�.
//	columnValues[OBJECT_TABLE_INDEX_NAME] = debuggingTest;
	
	while(uId)	//root�� uid�� ���� 0�̹Ƿ� root �� ������ Ż���Ѵ�.
	{
		//uId�� Object�� �ִ����� �˻��Ѵ�.
		if (CheckObjectUid(uId, &rowId) == FUNCTION_FAILED)
		{
			return FUNCTION_FAILED;
		}
		//���� if���� �������Դٸ� rowid������ �ش� Rowid�� �˻��Ǿ� �ִ�.

		//�ش� Object Record�� Object Rowid�� ã�´�.
		objectsRowIdArray[depthReverseCounter] = rowId;
		
		depthReverseCounter++;

		//�ش� Rowid�� ���� Record�� parentUId�� ä���´�.
 		dbApiGetRecordCopy(gDbTableNameObjects, rowId, columnValues);	//Parent�� Uid���� ä���´�.

		//�̹��� �˻��� record�� uid�� Parent uid�̴�.
		uId = parentUId;	

		if (uId==0)
		{
			break;
			/*
			//root�� Rowid�� ã�Ƴ���.
			if (CheckObjectUid(uId, &rowId) == FUNCTION_FAILED)
			{
				//root�� ã�� �� ���� ���
				return FUNCTION_FAILED;
			}
			objectsRowIdArray[depthReverseCounter] = rowId;			
			*/
		}
	}
	//���� while���� ���� ������ Object���� Rowid���� objectsRowIdArray�� ��� �ִ�.

	//=======uid���� 0�ϰ�� Error ó���߰�(begin)=====//
	if (depthReverseCounter == 0)			//error check
	{									//error check
		return FUNCTION_FAILED;			//error check
	}									//error check
	//=======uid���� 0�ϰ�� Error ó���߰�(e n d)=====//
	
	//====�ʱ�ȭ====//
	memset(columnValues, 0, sizeof(columnValues));
	*targetBuffer = 0;
	depthReverseCounter--;
	//====�ʱ�ȭ====//
	
	//path�� �����Ѵ�.
	while(depthReverseCounter)
	{
		
		columnValues[OBJECT_TABLE_INDEX_NAME] = targetBuffer;	//objectname�� ������ �ּҰ��� mapping
		
	 	dbApiGetRecordCopy(gDbTableNameObjects, objectsRowIdArray[depthReverseCounter], columnValues);	//Parent�� Uid���� ä���´�.

		//targetBuffer��  ���ڿ��� ���� ����Ű���� �Ѵ�. (begin)
		while(*targetBuffer)
		{
			targetBuffer++;
		}
		//targetBuffer��  ���ڿ��� ���� ����Ű���� �Ѵ�. (end)

	 	depthReverseCounter--;		
	}
	//������� File Path�� �����Դ�.

	if (pathOnlyOrFullPath == FILE_PATH_AND_FILE_NAME)
	{		
		//���ϸ���� ���Ҷ�
		columnValues[OBJECT_TABLE_INDEX_NAME] = targetBuffer;	//objectname�� ������ �ּҰ��� mapping
		dbApiGetRecordCopy(gDbTableNameObjects, objectsRowIdArray[depthReverseCounter], columnValues);	//Parent�� Uid���� ä���´�.			
	}
	else if(pathOnlyOrFullPath == FILE_PATH)
	{
		//Path�� �������� ��� '/'�� ���� �ؾ� �Ѵ�.
		/*
		while(*targetBuffer)
		{
			targetBuffer++;
		}
		//targetBuffer�� NULL�ΰ����� ���� ������.
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
		//�̿��� ���� ������� �ʴ´�.
		return FUNCTION_FAILED;
	}

	return FUNCTION_SUCCESS;
}

int FillObjectName(unsigned long uId, unsigned short* targetBuffer)
{
	unsigned long rowId;									//Objects Table���� Rowid�� ���� ������ ����
//	unsigned short*	CharPos;
	void*		columnValues[DB_OBJECT_TABLE_COUNT];

	//uId�� Object�� �ִ����� �˻��Ѵ�.
	if (CheckObjectUid(uId, &rowId) == FUNCTION_FAILED)
	{
		return FUNCTION_FAILED;
	}
	//���� if���� �������Դٸ� rowid������ �ش� Rowid�� �˻��Ǿ� �ִ�.


	//�÷��� ���� �޾ƿ� �ּҰ����� ����� �迭�� �ʱ�ȭ �Ѵ�.
	memset(columnValues, 0, sizeof(columnValues));
	columnValues[OBJECT_TABLE_INDEX_NAME] = targetBuffer;		//Object Name�� �޾ƿ� ���̱� ������ �ش� �ּҸ� ��ũ�Ѵ�.
	
	//�ش� Object�� Name�� �޾ƿ´�.
	dbApiGetRecordCopy(gDbTableNameObjects,
						rowId, columnValues);

//	CharPos = (unsigned short*)wcsrchr( targetBuffer, '/' );
//	if( *CharPos == '/' )	*CharPos = 0;

	return FUNCTION_SUCCESS;
}
//=============================================================//
//			Object Table�� Uid�� ������ ���ڵ��� Rowid�� �޾Ƴ��� �Լ��̴�.(E n d)			//
//=============================================================//


