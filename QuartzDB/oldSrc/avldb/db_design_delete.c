#include "avldb.h"
#include "db_design.h"
#include "db_design_delete.h"

//이 파일에는 db_design.c에 정의되어 있는 테이블의 Record를 삭제을 처리하는 함수이다.

int DeleteObjectTableRecord(unsigned long vRowid)
{
	if(dbApiRecordDeleteMS(gDbTableNameObjects, vRowid)==FUNCTION_FAILED)
	{
		return FUNCTION_FAILED;
	}
	else
	{
		return FUNCTION_SUCCESS;
	}
}

int DeleteReferenceTableRecord(unsigned long vRowid)
{
	if(dbApiRecordDeleteMS(gDbTableNameReferences, vRowid)==FUNCTION_FAILED)
	{
		return FUNCTION_FAILED;
	}
	else
	{
		return FUNCTION_SUCCESS;
	}
}

int DeletePhotoTableRecord(unsigned long vRowid)
{	
	if (dbApiRecordDeleteMS(gDbTableNamePhoto, vRowid)==FUNCTION_FAILED)
	{
		return FUNCTION_FAILED;
	}
	else
	{
		return FUNCTION_SUCCESS;
	}
}

int DeleteAlbumTableRecord(unsigned long vRowid)
{	

	if (dbApiRecordDeleteMS(gDbTableNameAlbum, vRowid)==FUNCTION_FAILED)
	{
		return FUNCTION_FAILED;
	}
	else
	{
		return FUNCTION_SUCCESS;
	}
}

int DeleteMusicTableRecord(unsigned long vRowid)
{	
	if (dbApiRecordDeleteMS(gDbTableNameMusic, vRowid)==FUNCTION_FAILED)
	{
		return FUNCTION_FAILED;
	}
	else
	{
		return FUNCTION_SUCCESS;
	}
}

