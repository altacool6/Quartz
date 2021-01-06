//////////////////////////////////////////////////////////////////////////////////
//
//
//			file system 관련 function
//
//
//////////////////////////////////////////////////////////////////////////////////
#include "tfs4_types.h"
//20051124...FS함수 작업

#define NUM_FILESYSTEM_FUNC 15

#define FS_OPEN					0
#define FS_READ 				1
#define FS_WRITE 				2
#define FS_CLOSE 				3
#define FS_SEEK 				4
#define FS_LENGTH 				5
#define FS_TELL 				6
#define FS_APPEND				7
#define FS_WRITEMODIFY 			8
#define FS_DELETE 				9
#define FS_OPENDIR 				10
#define FS_CLOSEDIR 			11
	

#define DB_FS_OPEN_ERROR	 										0
#define DB_FS_READ_ERROR											0
#define DB_FS_WRITE_ERROR 											0
#define DB_FS_CLOSE_ERROR											0
#define DB_FS_SEEK_ERROR											0
#define DB_FS_LENGTH_ERROR 											0
#define DB_FS_TELL_ERROR 											0
#define DB_FS_WRITEMODIFY_ERROR 									0
#define DB_FS_DELETE_ERROR											0
#define DB_FS_OPENDIR_ERROR											0
#define DB_FS_CLOSEDIR_ERROR										0


unsigned long FS_DBFile_Open( unsigned long* pFile, 
							unsigned long param2, unsigned long *param3, 
							unsigned long param4, unsigned long param5);	

unsigned long FS_DBFile_Read( unsigned long* pFile, 
							unsigned long param2, unsigned long *param3, 
							unsigned long param4, unsigned long param5);	

unsigned long FS_DBFile_Write( unsigned long* pFile, 
							unsigned long param2, unsigned long *param3, 
							unsigned long param4, unsigned long param5);	

unsigned long FS_DBFile_Close( unsigned long* pFile, 
							unsigned long param2, unsigned long *param3, 
							unsigned long param4, unsigned long param5);	

unsigned long FS_DBFile_Seek( unsigned long* pFile, 
							unsigned long param2, unsigned long *param3, 
							unsigned long param4, unsigned long param5);	
							
unsigned long FS_DBFile_Length( unsigned long* pFile, 
							unsigned long param2, unsigned long *param3, 
							unsigned long param4, unsigned long param5);	

unsigned long FS_DBFile_Tell( unsigned long* pFile, 
							unsigned long param2, unsigned long *param3, 
							unsigned long param4, unsigned long param5);	
						
unsigned long FS_DBFile_Append( unsigned long* pFile, 
							unsigned long param2, unsigned long *param3, 
							unsigned long param4, unsigned long param5);				
						
unsigned long FS_DBFile_Writemodify( unsigned long* pFile, 
									unsigned long param2, unsigned long *param3, 
									unsigned long param4, unsigned long param5);				

unsigned long FS_DBFile_Delete( unsigned long* pFile, 
								unsigned long param2, unsigned long *param3, 
								unsigned long param4, unsigned long param5);				

unsigned long FS_DBFile_OpenDir( unsigned long* pFile, 
								unsigned long param2, unsigned long *param3, 
								unsigned long param4, unsigned long param5);

unsigned long FS_DBFile_CloseDir( unsigned long* pFile, 
								unsigned long param2, unsigned long *param3, 
								unsigned long param4, unsigned long param5);



static unsigned long (*const pfn_FS_FUNC[NUM_FILESYSTEM_FUNC])( unsigned long* pFile, 
																	unsigned long param2, unsigned long *param3, 
																	unsigned long param4, unsigned long param5) = 
{
	FS_DBFile_Open,
	FS_DBFile_Read,
	FS_DBFile_Write,
	FS_DBFile_Close,
	FS_DBFile_Seek,
	FS_DBFile_Length,	
	FS_DBFile_Tell,
	FS_DBFile_Append,
	FS_DBFile_Writemodify,
	FS_DBFile_Delete,
	FS_DBFile_OpenDir,
	FS_DBFile_CloseDir
};						


															
//	file open function 
//	param1 - file index 
//	param2 - file name pointer(unsigned short*)
//	param3 - open flag
//	param4 - Not use
//	param5 - not use
//
unsigned long FS_DBFile_Open( unsigned long* pFile, 
								unsigned long param2, unsigned long *param3, 
								unsigned long param4, unsigned long param5)
{

	t_int32 dwFD;

    dwFD = tfs4_open((t_char*)param3, O_RDWR|O_TFS4_DIRECTIO);
    if(dwFD < 0)
    {
		return DB_FS_OPEN_ERROR;	
    }
    else
        return 1;  

}


//	param2 - file descriptor 
//	param3 - buffer pointer (char *)
//	param4 - Read size

unsigned long FS_DBFile_Read( unsigned long* pFile, 
								unsigned long param2, unsigned long* param3, 
								unsigned long param4, unsigned long param5)
{
	if(0 != (param2%512))
	{
		return DB_FS_READ_ERROR;
	}

	return (tfs4_read(param2, (void*) param3, param4));
}


//	param2 - file descriptor 
//	param3 - buffer pointer (char *)
//	param4 - Write size

unsigned long FS_DBFile_Write( unsigned long* pFile, 
								unsigned long param2, unsigned long* param3, 
								unsigned long param4, unsigned long param5)
{
	
	if(0 != (param2 % 512))
	{
		return DB_FS_WRITE_ERROR;
	}

	return (tfs4_write(param2, (void*) param3, param4));
}

//
//	param1 - file index 
//	param2 - Not use
//	param3 - Not use
//	param4 - Not use
//	param5 - not use
//

unsigned long FS_DBFile_Close( unsigned long* pFile, 
								unsigned long param2, unsigned long *param3, 
								unsigned long param4, unsigned long param5)
{
	t_int32 dwRET;

    dwRET = tfs4_close(param2);
    if(dwRET < 0)
    {
		return DB_FS_CLOSE_ERROR;	
    }
    else
        return 1;  


}

//	param2 - file descriptor 
//	param3 - offset
//	param4 - option
unsigned long FS_DBFile_Seek( unsigned long* pFile, 
								unsigned long param2, unsigned long *param3, 
								unsigned long param4, unsigned long param5)
{
	if(0 != (param4 % 512))
	{
		return DB_FS_SEEK_ERROR;
	}
	
	return (tfs4_lseek(param2, param4, param5));
}

//
//	param1 - file index 
//	param2 - Not use
//	param3 - Not use
//	param4 - Not use
//	param5 - not use
//

unsigned long FS_DBFile_Length( unsigned long* pFile, 
									unsigned long param2, unsigned long *param3, 
									unsigned long param4, unsigned long param5)
{
	return 1;		//필요시 작업 
}

//
//	param1 - file index 
//	param2 - Not use
//	param3 - Not use
//	param4 - Not use
//	param5 - not use
//
unsigned long FS_DBFile_Tell( unsigned long* pFile, 
								unsigned long param2, unsigned long *param3, 
								unsigned long param4, unsigned long param5)
{
	return (tfs4_ftell(param2));
}

unsigned long FS_DBFile_Append( unsigned long* pFile, 
									unsigned long param2, unsigned long *param3, 
									unsigned long param4, unsigned long param5)
{
	return 1;		//필요시 작업 
}


unsigned long FS_DBFile_Writemodify( unsigned long* pFile, 
										unsigned long param2, unsigned long *param3, 
										unsigned long param4, unsigned long param5)
{
	return 1;		//필요시 작업
}

unsigned long FS_DBFile_Delete( unsigned long* pFile, 
								unsigned long param2, unsigned long *param3, 
								unsigned long param4, unsigned long param5)
{
	return 1;		//필요시 작업
}

unsigned long FS_DBFile_OpenDir( unsigned long* pFile, 
								unsigned long param2, unsigned long *param3, 
								unsigned long param4, unsigned long param5)
{
	t_DIR *pdir;

//	pdir = tfs4_opendir((int*)param3);
	if(pdir == NULL )
	{
		return DB_FS_OPENDIR_ERROR;
	}
	//UPrint(("tfs4_opendir() Success. \n"));

	return TRUE;
}


unsigned long FS_DBFile_CloseDir( unsigned long* pFile, 
									unsigned long param2, unsigned long *param3, 
									unsigned long param4, unsigned long param5)
{
	return (tfs4_closedir(param2));
}
