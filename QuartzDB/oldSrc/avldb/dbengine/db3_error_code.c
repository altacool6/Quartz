#include "db3_error_code.h"

unsigned long 		gDbErrorCode;
char				*gDbErrorMessage;
char				*gDbErrorFunction;

void dbShowErrorCode()
{
	switch(gDbErrorCode)
	{
		case DB_ERROR_NO_ERROR:
			gDbErrorMessage = "에러가 발생하지 않았습니다.";
			break;
			
		case DB_ERROR_DATABASE_DO_NOT_RUNNING:
			gDbErrorMessage = "Database가 구동중이 아닙니다.";
			break;
			
		case DB_ERROR_PARAMETER_INCORRECT:
			gDbErrorMessage = "전달된 인자값이 하나 이상 이상을 발생 했습니다.";
			break;
			
		case DB_ERROR_DBAPIALLOCKEY_FUNCTION_DBFILETYPE_PARAMETER_INCORRECT:
			gDbErrorMessage = "File Type으로 전달 받은 인자값이 옳지 않습니다.";
			break;
			
		case DB_ERROR_DBAPISEARCHMS_FUNCTION_CONDITION_PARAMETER_INCORRECT:
			gDbErrorMessage = "인덱스로 전달받은 컬럼의 갯수보다 조건으로 받은 값이 더 많습니다.";
			break;
			
		case DB_ERROR_DBAPISEARCHMS_FUNCTION_TABLE_NOT_FOUND:
			gDbErrorMessage = "dbApiSearchMS함수에서 테이블을 찾지 못했습니다.";
			break;
			
		case DB_ERROR_TABLE_NAME_ALREADY_EXIST:
			gDbErrorMessage = "생성하려는 Table이 DB상에 이미 존재합니다.";
			break;
			
		case DB_ERROR_TABLE_ALLOC_PAGE_NONE:
			gDbErrorMessage = "Table에 할당된 Page가 존재하지 않습니다.";
			break;
			
		case DB_ERROR_COLUMN_NAME_ALREADY_EXIST:
			gDbErrorMessage = "생성하려는 Column Name이 Table상에 이미 존재합니다.";
			break;
			
		case DB_ERROR_TABLE_NOT_FOUND:
			gDbErrorMessage = "해당 Table을 찾을 수 없습니다.";
			break;
			
		case DB_ERROR_COLUMN_NOT_FOUND:
			gDbErrorMessage = "해당 Column을 찾을 수 없습니다.";
			break;
			
		case DB_ERROR_INDIAN_INCORRECT:
			gDbErrorMessage = "전달 받은 인자값중 Indian값이 옮지 않은 값입니다.";
			break;
			
		case DB_ERROR_ALREADY_RUNNING:
			gDbErrorMessage = "디비가 이미 구동중입니다.";
			break;
			
		case DB_ERROR_DICTIONARY_FILE_CREATE_FAILED:
			gDbErrorMessage = "Dictionary File을 생성하는 과정에서 Create Error가 발생했습니다.";
			break;
			
		case DB_ERROR_DATA_FILE_CREATE_FAILED:
			gDbErrorMessage = "Data File을 생성하는 과정에서 Create Error가 발생했습니다.";
			break;
			
		case DB_ERROR_INDEX_FILE_CREATE_FAILED:
			gDbErrorMessage = "Index File을 생성하는 과정에서 Create Error가 발생했습니다.";
			break;
			
		case DB_ERROR_DICTIONARY_FILE_TRUNCATE_FAILED:
			gDbErrorMessage = "Dictionary File을 생성하는 과정에서 Truncate Error가 발생했습니다.";
			break;
			
		case DB_ERROR_DATA_FILE_TRUNCATE_FAILED:
			gDbErrorMessage = "Data File을 생성하는 과정에서 Truncate Error가 발생했습니다.";
			break;
			
		case DB_ERROR_INDEX_FILE_TRUNCATE_FAILED:
			gDbErrorMessage = "Index File을 생성하는 과정에서 Truncate Error가 발생했습니다.";
			break;
			
		case DB_ERROR_DICTIONARY_FILE_CRASH:
			gDbErrorMessage = "Dictionary File이 정상상태가 아닙니다.";
			break;
			
		case DB_ERROR_DICTIONARY_FILE_CRASH_DICTIONARY_HEADER_SIZE_VERY_SMALL:
			gDbErrorMessage = "Dictionary File이 정상상태가 아닙니다. Dictionary Header의 값중 Dictionary File의 Size가 너무 작습니다.";
			break;
			
		case DB_ERROR_DBAPISEARCHMS_FUNCTION_COLUMN_NOT_FOUND:
			gDbErrorMessage = "해당 컬럼이 시스템에 존재하지 않습니다.";
			break;
			
		case DB_ERROR_DBAPISEARCHMS_COMPLEXINDEX_NOT_FOUND:
			gDbErrorMessage = "해당 복합인덱스가 시스템에 존재하지 않습니다..";
			break;

		case DB_ERROR_DBAPISEARCHMS_FUNCTION_ORDER_INVALID:
			gDbErrorMessage = "전달받은 정렬 값이 옳지 않습니다.";
			break;

		case DB_ERROR_INVALID_ROWID:
			gDbErrorMessage = "접근하려는 Rowid가 옳지 않습니다.";
			break;

		case DB_ERROR_RECORD_COPY_ERROR:
			gDbErrorMessage = "Record를 복사하는 과정에서 Error가 발생했습니다.";
			break;

		case DB_ERROR_DICTIONARY_FILE_OPEN_FAIL_AT_DBLOADDATABASEFILES_FUNTION:
		  gDbErrorMessage = "dbLoadDatabaseFiles함수에서 Dictionary File을 Open할수 없어서 함수가 실패되었습니다.";
			break;
			
		case DB_ERROR_DICTIONARY_FILE_READING_FAIL_AT_DBLOADDATABASEFILES_FUNTION:
		  gDbErrorMessage = "dbLoadDatabaseFiles함수에서 Dictionary File을 Reading하는 과정에서 실패되었습니다.";
			break;
		case DB_ERROR_DATA_FILE_OPEN_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileLoadMemorypage함수에서 Data File Open에 실패 하였습니다. 이후에 오동작이 발생할 수 있습니다.";
		  break;
		case DB_ERROR_DATA_FILE_READ_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileLoadMemorypage함수에서 Data File Read에 실패 하였습니다. 이후에 오동작이 발생할 수 있습니다.";
		  break;
		case DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileLoadMemorypage함수에서 Index File Open에 실패 하였습니다. 이후에 오동작이 발생할 수 있습니다.";
		  break;
		case DB_ERROR_INDEX_FILE_READ_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileLoadMemorypage함수에서 Index File Read에 실패 하였습니다. 이후에 오동작이 발생할 수 있습니다.";
		  break;
		case DB_ERROR_DATA_FILE_OPEN_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileStoreMemorypage함수에서 Data File Open에 실패 하였습니다. 이후에 오동작이 발생할 수 있습니다.";
		  break;
		case DB_ERROR_DATA_FILE_WRITE_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileStoreMemorypage함수에서 Data File Write에 실패 하였습니다. 이후에 오동작이 발생할 수 있습니다.";
		  break;
		case DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileStoreMemorypage함수에서 Index File Open에 실패 하였습니다. 이후에 오동작이 발생할 수 있습니다.";
		  break;
		case DB_ERROR_INDEX_FILE_WRITE_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileStoreMemorypage함수에서 Index File Write에 실패 하였습니다. 이후에 오동작이 발생할 수 있습니다.";
		  break;
		case DB_ERROR_Dictionary_FILE_OPEN_FAIL_AT_DBSTOREDICTIONARYFILE_FUNTION:
		  gDbErrorMessage = "dbStoreDictionaryfile함수에서 Dictionary File Open에 실패 하였습니다.";
		  break;
		case DB_ERROR_Dictionary_FILE_WRITE_FAIL_AT_DBSTOREDICTIONARYFILE_FUNTION:
		  gDbErrorMessage = "dbStoreDictionaryfile함수에서 Dictionary File WRITE에 실패 하였습니다.";
		  break;
		case DB_ERROR_DATA_FILE_OPEN_FAIL_AT_DBSTOREDATAFILE_FUNTION:
		  gDbErrorMessage = "dbStoreDatafile함수에서 Data File Open에 실패 하였습니다.";
		  break;
		case DB_ERROR_DATA_FILE_WRITE_FAIL_AT_DBSTOREDATAFILE_FUNTION:
		  gDbErrorMessage = "dbStoreDatafile함수에서 Data File WRITE에 실패 하였습니다.";
		  break;
		case DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSTOREINDEXFILE_FUNTION:
		  gDbErrorMessage = "dbStoreIndexfile함수에서 Index File Open에 실패 하였습니다.";
		  break;
		case DB_ERROR_INDEX_FILE_WRITE_FAIL_AT_DBSTOREINDEXFILE_FUNTION:
		  gDbErrorMessage = "dbStoreIndexfile함수에서 Index File Write에 실패 하였습니다.";
		  break;
		
		
		
		default:
			;
	}
}
