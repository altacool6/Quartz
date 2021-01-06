#ifndef __DB3_BASICTYPE_DEFINE_H__

#define __DB3_BASICTYPE_DEFINE_H__

#define ANSI_BASE_STAND_IO 0
#define TFS4_BASE_STAND_IO 1

#define		DB_MAX_COMPLEX_INDEX_COLUMN_NUMBER	4
#define		DB_TABLE_MAX_COLUMN_NUMBER			40
//===========사용자 정의 define (end)===========//



//===========관리자 정의 define (begin)===========//
enum INDIAN
{
	DB_ORDER_INDIAN,
	DB_SAME_INDIAN
};

enum INSERT_OPTION
{
	DB_USING_IN_UPDATE,
	DB_NOT_USING_IN_UPDATE
};

enum 
{
	DB_FUNCTION_FAIL,
	DB_FUNCTION_SUCCESS
};

enum
{
	DB_INVALID_ROWID,
	DB_INVALID_ROWID_ZERO,
	DB_INVALID_ROWID_TOO_VERY_LARGE,
	DB_VALID_ROWID
};


#define		DB_UPDATE_WITHOUT_INDEX			1
#define		DB_UPDATE_WITH_SINGLE_INDEX		2
#define		DB_UPDATE_WITH_COMPLEX_INDEX	3
#define		DB_AREA_VS_AREA					1
#define		DB_AREA_VS_STACK					2
#define		DB_UNCHANGED						0
#define		DB_CHANGED						1
#define		DB_NULL							0
#define		DB_END_OF_STRING					0
#define		DB_DATA_FILE_LOAD					1
#define		DB_INDEX_FILE_LOAD				2

#define		DB_VARCHAR_TYPE					1
#define		DB_UINT8_TYPE						2
#define		DB_UINT16_TYPE					3
#define		DB_UINT32_TYPE					4

#define		DB_VARCHAR			unsigned short
#define		DB_UINT8				unsigned char
#define		DB_UINT16				unsigned short
#define		DB_UINT32				unsigned long
#define		DB_VARCHAR_SIZE		sizeof(DB_VARCHAR)
#define		DB_UINT8_SIZE			sizeof(DB_UINT8)
#define		DB_UINT16_SIZE			sizeof(DB_UINT16)
#define		DB_UINT32_SIZE			sizeof(DB_UINT32)
#define		DB_COLUMN_TYPE		unsigned long

#define		DB_ASC					1
#define		DB_DESC				2

#define		DB_SLOT_SIZE		sizeof(unsigned long)

enum
{
	DB_DO_NOT_RUNNING,
	DB_DO_RUNNING
};

//===========관리자 정의 define ( end )===========//



struct tagMMNODE{
	unsigned long		mmuPageNumber;
	unsigned char		mmuChanged;
	char				*mmuMemoryAddress;
	struct tagMMNODE	*mmuNext;
};

typedef struct tagMMNODE *pMMNODE;
typedef struct tagMMNODE MMNODE;

typedef struct tagDictinaryHeader{
	unsigned long dictionaryHeaderSizeOfDictionary;
	unsigned long dictionaryHeaderNumberOfDataFileEndPage;
	unsigned long dictionaryHeaderNumberOfIndexFileEndPage;
	unsigned long dictionaryHeaderDeleteNode;
	unsigned long dictionaryHeaderDeleteStartPage;
	unsigned long dictionaryHeaderOffsetOfRootTable;
}DictionaryHeader, *pDictionaryHeader;

typedef struct tagDatafilePageheader{
	unsigned long dataHeaderEndOfRecords;
	unsigned long dataHeaderNumberOfSlots;
	unsigned long dataHeaderDeleteRecordOffset;
	unsigned long dataHeaderNextSameTablePage;
}DataFilePageHeader, *pDataFilePageHeader;

typedef struct tagIndexfilePageheader{
	unsigned long indexHeaderEndOfDatas;
	unsigned long indexHeaderDeleteNodeOffset;
	unsigned long indexHeaderDeleteLinkOffset;
}IndexFilePageHeader, *pIndexFilePageHeader;

typedef struct tagTableNode{	//Table에서 사용할 구조
	unsigned long	tableNodeLeft;
	unsigned long	tableNodeRight;
	unsigned long	tableNodeHeight;
	unsigned long	tableNodeStartPage;
	unsigned long	tableNodeColumnOffset;//subOffset;
	unsigned long	tableNodeComplexIndexOffset;
}TableNode, *pTableNode;


typedef struct tagColumnNode{
	unsigned long columnNodeNextColumnOffset;
	unsigned long columnNodeDataType;
	unsigned long columnNodeIndexRootOffset;
}ColumnNode,*pColumnNode;

typedef struct tagComplexIndexNode{
	unsigned long complexIndexNodeNext;
	unsigned long complexIndexNodeColumnNumber;
	unsigned long complexIndexNodeIndexRootOffset;
}ComplexIndexNode, *pComplexIndexNode;

typedef struct tagIndexNode{
	unsigned long	indexNodeLeft;
	unsigned long	indexNodeRight;
	unsigned long	indexNodeHeight;
	unsigned long	indexNodeLinkOffset;
}IndexNode, *pIndexNode;

typedef struct tagLinkNode{
	unsigned long	linkNodeRecordRowid;
	unsigned long	linkNodeNext;
}LinkNode, *pLinkNode;

typedef struct tagDeleteNode{
	unsigned long	deleteNodeSize;
	unsigned long	deleteNodeNext;
}DeleteNode, *pDeleteNode;

#endif
