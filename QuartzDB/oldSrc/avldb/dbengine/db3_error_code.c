#include "db3_error_code.h"

unsigned long 		gDbErrorCode;
char				*gDbErrorMessage;
char				*gDbErrorFunction;

void dbShowErrorCode()
{
	switch(gDbErrorCode)
	{
		case DB_ERROR_NO_ERROR:
			gDbErrorMessage = "������ �߻����� �ʾҽ��ϴ�.";
			break;
			
		case DB_ERROR_DATABASE_DO_NOT_RUNNING:
			gDbErrorMessage = "Database�� �������� �ƴմϴ�.";
			break;
			
		case DB_ERROR_PARAMETER_INCORRECT:
			gDbErrorMessage = "���޵� ���ڰ��� �ϳ� �̻� �̻��� �߻� �߽��ϴ�.";
			break;
			
		case DB_ERROR_DBAPIALLOCKEY_FUNCTION_DBFILETYPE_PARAMETER_INCORRECT:
			gDbErrorMessage = "File Type���� ���� ���� ���ڰ��� ���� �ʽ��ϴ�.";
			break;
			
		case DB_ERROR_DBAPISEARCHMS_FUNCTION_CONDITION_PARAMETER_INCORRECT:
			gDbErrorMessage = "�ε����� ���޹��� �÷��� �������� �������� ���� ���� �� �����ϴ�.";
			break;
			
		case DB_ERROR_DBAPISEARCHMS_FUNCTION_TABLE_NOT_FOUND:
			gDbErrorMessage = "dbApiSearchMS�Լ����� ���̺��� ã�� ���߽��ϴ�.";
			break;
			
		case DB_ERROR_TABLE_NAME_ALREADY_EXIST:
			gDbErrorMessage = "�����Ϸ��� Table�� DB�� �̹� �����մϴ�.";
			break;
			
		case DB_ERROR_TABLE_ALLOC_PAGE_NONE:
			gDbErrorMessage = "Table�� �Ҵ�� Page�� �������� �ʽ��ϴ�.";
			break;
			
		case DB_ERROR_COLUMN_NAME_ALREADY_EXIST:
			gDbErrorMessage = "�����Ϸ��� Column Name�� Table�� �̹� �����մϴ�.";
			break;
			
		case DB_ERROR_TABLE_NOT_FOUND:
			gDbErrorMessage = "�ش� Table�� ã�� �� �����ϴ�.";
			break;
			
		case DB_ERROR_COLUMN_NOT_FOUND:
			gDbErrorMessage = "�ش� Column�� ã�� �� �����ϴ�.";
			break;
			
		case DB_ERROR_INDIAN_INCORRECT:
			gDbErrorMessage = "���� ���� ���ڰ��� Indian���� ���� ���� ���Դϴ�.";
			break;
			
		case DB_ERROR_ALREADY_RUNNING:
			gDbErrorMessage = "��� �̹� �������Դϴ�.";
			break;
			
		case DB_ERROR_DICTIONARY_FILE_CREATE_FAILED:
			gDbErrorMessage = "Dictionary File�� �����ϴ� �������� Create Error�� �߻��߽��ϴ�.";
			break;
			
		case DB_ERROR_DATA_FILE_CREATE_FAILED:
			gDbErrorMessage = "Data File�� �����ϴ� �������� Create Error�� �߻��߽��ϴ�.";
			break;
			
		case DB_ERROR_INDEX_FILE_CREATE_FAILED:
			gDbErrorMessage = "Index File�� �����ϴ� �������� Create Error�� �߻��߽��ϴ�.";
			break;
			
		case DB_ERROR_DICTIONARY_FILE_TRUNCATE_FAILED:
			gDbErrorMessage = "Dictionary File�� �����ϴ� �������� Truncate Error�� �߻��߽��ϴ�.";
			break;
			
		case DB_ERROR_DATA_FILE_TRUNCATE_FAILED:
			gDbErrorMessage = "Data File�� �����ϴ� �������� Truncate Error�� �߻��߽��ϴ�.";
			break;
			
		case DB_ERROR_INDEX_FILE_TRUNCATE_FAILED:
			gDbErrorMessage = "Index File�� �����ϴ� �������� Truncate Error�� �߻��߽��ϴ�.";
			break;
			
		case DB_ERROR_DICTIONARY_FILE_CRASH:
			gDbErrorMessage = "Dictionary File�� ������°� �ƴմϴ�.";
			break;
			
		case DB_ERROR_DICTIONARY_FILE_CRASH_DICTIONARY_HEADER_SIZE_VERY_SMALL:
			gDbErrorMessage = "Dictionary File�� ������°� �ƴմϴ�. Dictionary Header�� ���� Dictionary File�� Size�� �ʹ� �۽��ϴ�.";
			break;
			
		case DB_ERROR_DBAPISEARCHMS_FUNCTION_COLUMN_NOT_FOUND:
			gDbErrorMessage = "�ش� �÷��� �ý��ۿ� �������� �ʽ��ϴ�.";
			break;
			
		case DB_ERROR_DBAPISEARCHMS_COMPLEXINDEX_NOT_FOUND:
			gDbErrorMessage = "�ش� �����ε����� �ý��ۿ� �������� �ʽ��ϴ�..";
			break;

		case DB_ERROR_DBAPISEARCHMS_FUNCTION_ORDER_INVALID:
			gDbErrorMessage = "���޹��� ���� ���� ���� �ʽ��ϴ�.";
			break;

		case DB_ERROR_INVALID_ROWID:
			gDbErrorMessage = "�����Ϸ��� Rowid�� ���� �ʽ��ϴ�.";
			break;

		case DB_ERROR_RECORD_COPY_ERROR:
			gDbErrorMessage = "Record�� �����ϴ� �������� Error�� �߻��߽��ϴ�.";
			break;

		case DB_ERROR_DICTIONARY_FILE_OPEN_FAIL_AT_DBLOADDATABASEFILES_FUNTION:
		  gDbErrorMessage = "dbLoadDatabaseFiles�Լ����� Dictionary File�� Open�Ҽ� ��� �Լ��� ���еǾ����ϴ�.";
			break;
			
		case DB_ERROR_DICTIONARY_FILE_READING_FAIL_AT_DBLOADDATABASEFILES_FUNTION:
		  gDbErrorMessage = "dbLoadDatabaseFiles�Լ����� Dictionary File�� Reading�ϴ� �������� ���еǾ����ϴ�.";
			break;
		case DB_ERROR_DATA_FILE_OPEN_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileLoadMemorypage�Լ����� Data File Open�� ���� �Ͽ����ϴ�. ���Ŀ� �������� �߻��� �� �ֽ��ϴ�.";
		  break;
		case DB_ERROR_DATA_FILE_READ_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileLoadMemorypage�Լ����� Data File Read�� ���� �Ͽ����ϴ�. ���Ŀ� �������� �߻��� �� �ֽ��ϴ�.";
		  break;
		case DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileLoadMemorypage�Լ����� Index File Open�� ���� �Ͽ����ϴ�. ���Ŀ� �������� �߻��� �� �ֽ��ϴ�.";
		  break;
		case DB_ERROR_INDEX_FILE_READ_FAIL_AT_DBSETFILELOADMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileLoadMemorypage�Լ����� Index File Read�� ���� �Ͽ����ϴ�. ���Ŀ� �������� �߻��� �� �ֽ��ϴ�.";
		  break;
		case DB_ERROR_DATA_FILE_OPEN_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileStoreMemorypage�Լ����� Data File Open�� ���� �Ͽ����ϴ�. ���Ŀ� �������� �߻��� �� �ֽ��ϴ�.";
		  break;
		case DB_ERROR_DATA_FILE_WRITE_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileStoreMemorypage�Լ����� Data File Write�� ���� �Ͽ����ϴ�. ���Ŀ� �������� �߻��� �� �ֽ��ϴ�.";
		  break;
		case DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileStoreMemorypage�Լ����� Index File Open�� ���� �Ͽ����ϴ�. ���Ŀ� �������� �߻��� �� �ֽ��ϴ�.";
		  break;
		case DB_ERROR_INDEX_FILE_WRITE_FAIL_AT_DBSETFILESTOREMEMORYPAGE_FUNTION:
		  gDbErrorMessage = "dbSetFileStoreMemorypage�Լ����� Index File Write�� ���� �Ͽ����ϴ�. ���Ŀ� �������� �߻��� �� �ֽ��ϴ�.";
		  break;
		case DB_ERROR_Dictionary_FILE_OPEN_FAIL_AT_DBSTOREDICTIONARYFILE_FUNTION:
		  gDbErrorMessage = "dbStoreDictionaryfile�Լ����� Dictionary File Open�� ���� �Ͽ����ϴ�.";
		  break;
		case DB_ERROR_Dictionary_FILE_WRITE_FAIL_AT_DBSTOREDICTIONARYFILE_FUNTION:
		  gDbErrorMessage = "dbStoreDictionaryfile�Լ����� Dictionary File WRITE�� ���� �Ͽ����ϴ�.";
		  break;
		case DB_ERROR_DATA_FILE_OPEN_FAIL_AT_DBSTOREDATAFILE_FUNTION:
		  gDbErrorMessage = "dbStoreDatafile�Լ����� Data File Open�� ���� �Ͽ����ϴ�.";
		  break;
		case DB_ERROR_DATA_FILE_WRITE_FAIL_AT_DBSTOREDATAFILE_FUNTION:
		  gDbErrorMessage = "dbStoreDatafile�Լ����� Data File WRITE�� ���� �Ͽ����ϴ�.";
		  break;
		case DB_ERROR_INDEX_FILE_OPEN_FAIL_AT_DBSTOREINDEXFILE_FUNTION:
		  gDbErrorMessage = "dbStoreIndexfile�Լ����� Index File Open�� ���� �Ͽ����ϴ�.";
		  break;
		case DB_ERROR_INDEX_FILE_WRITE_FAIL_AT_DBSTOREINDEXFILE_FUNTION:
		  gDbErrorMessage = "dbStoreIndexfile�Լ����� Index File Write�� ���� �Ͽ����ϴ�.";
		  break;
		
		
		
		default:
			;
	}
}
