#include "avldb.h"
#include "db_design.h"
#include "db_design_delete.h"

//�� ���Ͽ��� db_design.c�� ���ǵǾ� �ִ� ���̺��� Record�� ������ ó���ϴ� �Լ��̴�.

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

