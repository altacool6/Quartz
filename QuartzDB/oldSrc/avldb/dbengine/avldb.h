#ifndef __AVLDB_H__
#define __AVLDB_H__
#include "db3_basictype_define.h"

#define DB_SEARCH_CHANGED

int dbApiSearchMSRemainRecords(unsigned long* pRemainRecordsNumber);
int dbApiGetFileTotalSize(unsigned long *pFileSize);
int dbApiGetDictionaryFileSize(unsigned long *pFileSize);
int dbApiGetDataFileSize(unsigned long *pFileSize);
int dbApiGetIndexFileSize(unsigned long *pFileSize);

void dbInitialSetting(void);
//unsigned long dbRecordGetSizeResultContents(unsigned long	startColumnOffset, 
//							  					void*		contents[]);
unsigned long  dbApiStartUp(char *dbDictionaryFileName,
							char *dbDataFileName,
							char *dbIndexFileName,
							char *dbAreaDictionaryAreaStart,
							char *dbAreaDataAreaStart,
							char *dbAreaIndexAreaStart,
							char *dbMmuAreaDataMmuStart,
							char *dbMmuAreaIndexMmuStart,
							unsigned long	dbAreaSizeDictionaryAreaSize,
							unsigned long	dbAreaSizeDataAreaSize,
							unsigned long	dbAreaSizeIndexAreaSize,
							unsigned long	dbPageSizeDataFilePageSize,
							unsigned long	dbPageSizeIndexFilePageSize,
							unsigned char	dbSameIndian,
							unsigned long vDictionaryLogicalSize);
pDictionaryHeader dbGetDictionaryHeader(void);
//void dbLoadDatabaseFiles(char *dbFileNameDictionaryFileName,
//						 char *dbFileNameDataFileName,
//						 char *dbFileNameIndexFileName);
int dbLoadDatabaseFiles(char *dbFileNameDictionaryFileName,
						 char *dbFileNameDataFileName,
						 char *dbFileNameIndexFileName,
						 unsigned long vDictionaryFileSize);

void dbInitMemorySetting(char	**dbAreaDictionaryAreaStart,
						 char	**dbAreaDataAreaStart,
						 char	**dbAreaIndexAreaStart,
						 char	**dbMmuAreaDataMmuStart,
						 char	**dbMmuAreaIndexMmuStart);
void dbMmuCreate(pMMNODE *dbMmuDataPage,
				 pMMNODE *dbMmuIndexPage,
				 char *dbMmuAreaDataMmuStart,
				 char *dbMmuAreaIndexMmuStart,
				 char *dbAreaDataAreaStart,
				 char *dbAreaIndexAreaStart);
unsigned char dbGetLruListPage(pMMNODE **start, unsigned long	pageNumber);
void dbMaintainLruList(pMMNODE *start, pMMNODE *lastAccessPage);
void dbTouch(unsigned long offset, unsigned long filetype);
void dbSetFileLoadMemorypage(unsigned long filePageNumber, char *targetMemoryAddress, unsigned long fileType);
int dbSetFileStoreMemorypage(unsigned long	filePageNumber, char *targetMemoryAddress, unsigned long filetype);
void dbStoreDictionaryfile(void);
void dbStoreDatafile(void);
void dbStoreIndexfile(void);
void dbApiShutDown(void);
void dbMemcpyFreeIndian(void *source, void *target, unsigned long unitSize, unsigned long repeat);
void dbMemcpyOrderIndian(void			*pSource, 
							   void			*pTarget, 
							   unsigned long	dbUnitSize, 
							   unsigned long	dbRepeat);
void dbMemcpySameIndian(void		     	*pSource, 
							   void			*pTarget, 
							   unsigned long	dbUnitSize, 
							   unsigned long	dbRepeat);

void dbApiCreateTable(DB_VARCHAR *tablename);
void dbTableSetNode(char *target, DB_VARCHAR	*tablename);
unsigned long dbStringCopy(DB_VARCHAR* target, DB_VARCHAR* string);
unsigned long dbStringGetSize(DB_VARCHAR* string);
unsigned long dbStringGetSize_AREA(DB_VARCHAR* string);
void dbTableSetCreateTab(unsigned long *tableOffset, DB_VARCHAR *tableName);
unsigned long dbTableGetHeight(unsigned long tableOffset);
signed long dbTableGetBalanceFactor(unsigned long tableOffset);
signed char dbStringCompareAreaVsStack(DB_VARCHAR *dbAreaMemory, DB_VARCHAR *dbStackMemory);
signed char dbStringCompareAreaVsArea(DB_VARCHAR *dbAreaMemory1, DB_VARCHAR *dbAreaMemory2);
unsigned short* dbStringRTrimStack(DB_VARCHAR *dbStackMemoryString);
unsigned short* dbStringRTrimArea(DB_VARCHAR *dbAreaMemoryString);
unsigned short* dbStringWithoutArticleArea(DB_VARCHAR	*dbAreaMemoryString);
unsigned short* dbStringWithoutArticleStack(DB_VARCHAR *dbStackMemoryString);
signed char dbStringCompareAreaVsAreaMS(DB_VARCHAR *dbAreaMemory1, DB_VARCHAR *dbAreaMemory2);
signed char dbStringCompareAreaVsStackMS(DB_VARCHAR *dbAreaMemory, DB_VARCHAR *dbStackMemory);
signed char dbStringCompare_AREAVs_STACK(DB_VARCHAR *areaMemory, DB_VARCHAR *stackMemory);
void dbTableSetHeight(pTableNode ptable);
void dbTableRotationLR(unsigned long *source);
signed char dbIndexGetBalanceFactor(unsigned long leftSubOffset, unsigned long rightSubOffset);
unsigned long dbIndexGetHeight(unsigned long indexOffset);
unsigned long dbIndexGetCalculateHeight(unsigned long leftSubOffset, unsigned long rightSubOffset);
void dbIndexRotationLR(unsigned long *l1, unsigned long *l2, unsigned long *l3);
void dbIndexRotationLL(unsigned long *l1, unsigned long *l2, unsigned long *l3);
void dbIndexRotationRR(unsigned long *l1, unsigned long *l2, unsigned long *l3);
void dbIndexRotationRL(unsigned long *l1, unsigned long *l2, unsigned long *l3);
void dbTableRotationLL(unsigned long *source);
void dbTableRotationLR(unsigned long *source);
void dbTableRotationRR(unsigned long *source);
void dbTableRotationRL(unsigned long *source);
unsigned long dbApiTableColumnNumber(DB_VARCHAR	*dbTableName);
void dbApiCreateColumn(DB_VARCHAR	*tableName, DB_VARCHAR *columnName, DB_COLUMN_TYPE type);
void dbColumnSetNode(char *target, DB_VARCHAR *columnName, DB_COLUMN_TYPE	type);
unsigned char dbTableFindColumnName(unsigned long	**source, DB_VARCHAR *columnName);
unsigned char dbTableFindTableName(unsigned long **source,  DB_VARCHAR *tableName);
void dbApiCreateIndexSingle(DB_VARCHAR *tableName, DB_VARCHAR *columnName);
unsigned long dbIndexGetCnode(DB_VARCHAR* columnName[], unsigned long columnNumber);
void dbApiCreateIndexComplex(DB_VARCHAR* tableName, DB_VARCHAR*	columnNames[], unsigned long	columnNumber);
void dbDataSetSlot(unsigned long slotNumber, unsigned long recordRowid);
unsigned long dbRecordGetSize(unsigned long	startColumnOffset, void* contents[]);
unsigned long dbApiRecordInsertMS(DB_VARCHAR *dbTableName,
								  void*	dbContents[],
								  unsigned char dbUsingInUpdate);
unsigned long dbDataSetRecordMS(unsigned long	dbStartColumnOffset, void* dbContents[], unsigned long dbRequirementSize);
void dbDataSetRecordReal(char *pRecord, unsigned long dbColumnStartAddress, void* dbContents[]);
char* dbIndexGetPageHaveRequirementSizeInDeleteNodes(unsigned long requirementSize);
char* dbDataGetPageHaveRequirementSizeInDeleteNodes(unsigned long requirementSize);
char* dbIndexGetPageHaveRequirementSizeInFreespace(unsigned long requirementSize);
pLinkNode dbIndexGetLink(unsigned long indexLinkOffset);
pIndexNode dbIndexGetNode(unsigned long dbIndexOffset);
unsigned long dbIndexSetNode(pIndexNode pInode, void* keyValue, unsigned long	keyDataType);
signed char dbIndexCompareKeyAreaVsAreaMS(void* dbAreaKeyValue1, void* dbAreaKeyValue2, unsigned long dbKeyDataType);
signed char dbIndexCompareKeyAreaVsStackMS(void* dbAreaKeyValue, void* dbStackKeyValue, unsigned long dbKeyDataType);
signed char dbIndexCompareKey_AREAVs_STACK(void* dbAreaKeyValue, void* dbStackKeyValue, unsigned long	dbKeyDataType);
void dbIndexLinkNodeAppend(unsigned long indexNodeOffset, unsigned long recordRowid);
pLinkNode	dbIndexGetPageHaveRequirementSizeInDeleteLinks(void);
unsigned long dbIndexSingleUnitInsertMS(unsigned long	dbIndexNodeRootOffset, void* dbKeyValue, unsigned long dbKeyDataType, unsigned long dbRecordOffset);
unsigned long dbIndexSingleUnitInsert(unsigned long	indexNodeRootOffset, void* keyValue, unsigned long keyDataType, unsigned long recordOffset);
signed char dbStringCompare_AREAVs_AREA(DB_VARCHAR *areaMemory1, DB_VARCHAR *areaMemory2);
unsigned long dbIndexComplexUnitInsertPostProcessMS(unsigned long	dbIndexRootOffset, void* dbKeyValues[], unsigned long	dbKeyDataTypes[], unsigned long	dbColumnNumber, unsigned long	dbRecordRowid);
//unsigned long dbIndexComplexUnitInsertPostProcess(unsigned long	rootOffset, void* keyValues[], unsigned long	keyDataTypes[], unsigned long	columnNumber, unsigned long	recordRowid);
unsigned long dbFindColumnName(unsigned long columnStartOffset, pColumnNode *columnAddress, DB_VARCHAR *columnName, unsigned char is_AREA_AREA);
void dbIndexComplexUnitInsertPreProcessMS(pComplexIndexNode dbComplexIndex, unsigned long dbColumnStart, void* dbContents[], unsigned long dbRecordRowid);
//void dbIndexComplexUnitInsertPreProcess(pComplexIndexNode cInode, unsigned long columnStartOffset, void* contents[], unsigned long recordRowid);
void dbIndexComplexAllInsertMS(unsigned long dbTableOffset, void* dbContents[], unsigned long dbRecordRowid);
void dbIndexSingleAllInsertMS(unsigned long	dbStartColumnOffset, void* dbContents[], unsigned long	dbRecordRowid);
void dbIndexTreeTraverseAsc(unsigned long rootOffset, unsigned long traverseSubLink);
void dbIndexTreeTraverseDesc(unsigned long rootOffset, unsigned long traverseSubLink);
void dbIndexTreeTraverse(unsigned long rootOffset, unsigned char isAsc, unsigned char traverseSubLink);
unsigned char dbIndexCheckName(DB_VARCHAR* areaAddress, DB_VARCHAR*	indexColumnName[], unsigned long	indexColumnNumber);
//unsigned long dbApiSearch(DB_VARCHAR* tableName, DB_VARCHAR* indexColumnName[], unsigned long	indexColumnNumber, void* conditionSet[], unsigned long	conditionColumnNumber, unsigned char	traverseOrder, unsigned long	resultArray[], unsigned char	*refMemory);
unsigned char dbDataGetRecord(unsigned long columnStartOffset, unsigned long recordRowid, void* resultArray[]);
void dbDataRecordSetColumn(void* target, void* source, unsigned long columnType);
//void dbApiGetRecord(DB_VARCHAR *tableName, unsigned long recordRowid, void* columnsValue[]);
//void dbApiRecordUpdateWithoutIndexKey(DB_VARCHAR *tableName, unsigned long	recordRowid, DB_VARCHAR *columnName,	void *updateColumnValue);
//void dbApiRecordUpdateWithSingleIndexKey(DB_VARCHAR *tableName,	unsigned long	recordRowid, DB_VARCHAR *columnName, void *updateColumnValue);
unsigned long dbColumnTotalCount(unsigned long startColumnOffset);
void dbDataSetRecordDelete(unsigned long	startColumnOffset, void* columnsValue[], unsigned long recordRowid);
//void dbIndexDeleteAllComplex(unsigned long startColumnOffset, unsigned long startComplexIndexOffset, void* columnsValue[], unsigned long recordRowid);
void dbIndexDeleteAllComplexMS(unsigned long	dbStartColumnOffset, unsigned long dbStartComplexIndexOffset, void* dbColumnsValues[], unsigned long dbRecordRowid);
int dbApiRecordDeleteMS(DB_VARCHAR *dbTableName, unsigned long dbRecordRowid);
//void dbApiRecordDelete(DB_VARCHAR *tableName, unsigned long recordRowid);
signed char dbIndexCompareKey_AREAVs_AREA(void* areaKeyValue1, void* areaKeyValue2, unsigned long keyDataType);
unsigned long dbIndexLinkNodeDelete(unsigned long linkOffset, unsigned long recordRowid);
unsigned long dbIndexDeleteNodeMakeTreeUpMinNode(unsigned long rootNode, unsigned long *minNodeOffset);
unsigned long dbIndexDeleteNodeMakeTreeUpMaxNode(unsigned long rootNode, unsigned long *maxNodeOffset);
unsigned long dbIndexDeleteUnitSingleMS(unsigned long	dbRootIndexOffset, unsigned long dbDataType, void* dbKeyValue,	unsigned long	dbRecordRowid);
//unsigned long dbIndexDeleteUnitSingle(unsigned long	rootIndexOffset, unsigned long dataType, void* keyValue, unsigned long recordRowid);
//dbIndexDeleteUnitSingleunsigned long dbIndexDeleteUnitComplex(unsigned long rootIndexOffset, unsigned long dataTypes[], void* keyValues[], unsigned long recordRowid, unsigned long indexLevel);
unsigned long dbIndexDeleteUnitComplexMS(unsigned long	dbRootIndexOffset, unsigned long dbDataTypes[], void* dbKeyValues[], unsigned long dbRecordRowid, unsigned long dbIndexLevel);
void dbIndexDeleteIndexNode(unsigned long deleteIndexOffset, unsigned long dataType);
//void dbIndexDeleteAllSingle(unsigned long	columnStartOffset, void* columnsValue[],	unsigned long	recordRowid);
void dbIndexDeleteAllSingleMS(unsigned long	dbColumnStartOffset, void* dbColumnsValues[], unsigned long	dbRecordRowid);
char* dbApiAllocKey(unsigned long dbKeyValueOffset, 
					   unsigned char dbFileType);
//void dbRecordUpdate(DB_VARCHAR *tableName, unsigned long recordRowid, DB_VARCHAR *columnName, void *updateColumnValue, unsigned char state);
void dbIndexLinkTraverse(unsigned long dbLinkOffset);
unsigned long dbApiSearchWithout(DB_VARCHAR* tableName, DB_VARCHAR* indexColumnName, void* columnValues[],	unsigned long	columnCount, unsigned char	traverseOrder, unsigned long	resultArray[], unsigned char	*refMemory);
void dbIndexTreeTraverseConditonWithout(unsigned long	rootOffset, unsigned long dataType, void* keyValues[], unsigned long keyNumber, unsigned char isAsc);
void dbIndexTreeTraverseAscConditionWithout(unsigned long rootOffset, unsigned long dataType, void* withoutKeys[], unsigned long keyNumber);
void dbIndexTreeTraverseDescConditionWithout(unsigned long rootOffset, unsigned long dataType, void*	withoutKeys[], unsigned long keyNumber);


int dbApiGetTotalRecordNum(DB_VARCHAR	*tableName, unsigned long *pTotal);
void          dbApiAllSave(void);
#if 0
unsigned long dbApiGetTotalRecordNum(DB_VARCHAR	*tableName);


unsigned long dbApiSearch(DB_VARCHAR* tableName, DB_VARCHAR* indexColumnName[], unsigned long indexColumnNumber, void* conditionSet[], unsigned long conditionColumnNumber, unsigned char traverseOrder, unsigned long resultArray[], unsigned char *refMemory);
#endif
unsigned char dbApiGetRecordCopy(DB_VARCHAR * tableName, unsigned long recordRowid, void * columnsValue [ ]);
unsigned char dbDataGetRecordCopy(unsigned long columnStartOffset, unsigned long recordRowid, void* resultArray[]);
//unsigned long dbApiSearch(DB_VARCHAR* tableName, DB_VARCHAR* indexColumnName[], unsigned long indexColumnNumber, void* conditionSet[], unsigned long conditionColumnNumber, unsigned char traverseOrder, unsigned long resultArray[], unsigned char *refMemory);
unsigned long dbIndexSearchGetInodeMS(unsigned long	rootOffset,
										unsigned long	dataType,
										void*			keyValue);
int dbApiResultArraySorting(	unsigned short	*pTableName, 
								unsigned short	*pColumnName, 
								int				vAscOrDesc,
								unsigned long		*pResultArray, 
								unsigned long		vResultArryTotal);
unsigned long dbApiSearchMS(DB_VARCHAR*		dbTableName,
						    DB_VARCHAR*		indexColumnName[],
							unsigned long	indexColumnNumber,
							void*			conditionSet[],
							unsigned long	conditionColumnNumber,
							unsigned char	traverseOrder,
							unsigned long	resultArray[],
							#ifdef DB_SEARCH_CHANGED
							unsigned long	resultArraySize,
							#endif
							unsigned char	*refMemory);
void dbCloseDatabaseFiles(void);
void dbApiFileFullMemoryLoad(unsigned long vDataPageTotalNum, unsigned long vIndexPageTotalNum);
unsigned char dbApiDataCheckRecordValidRowID(unsigned long dbRecordRowid);
unsigned char dbDataCheckRecordValidRowID(unsigned long dbRecordRowid);
void dbIndexTreeTraverseAllDesc(unsigned long rootOffset,
								unsigned char vDepth);
void dbIndexTreeTraverseAllAsc(unsigned long rootOffset,
									unsigned char vDepth);
void dbIndexTreeTraverseAll(unsigned long rootOffset, 
							unsigned char isAsc, 
							unsigned char vDepth);
unsigned long dbApiSearchMSAllRecords(DB_VARCHAR*	dbTableName,
										DB_VARCHAR*	indexColumnName[],
										unsigned long		indexColumnNumber,
										void*			conditionSet[],
										unsigned long		conditionColumnNumber,
										unsigned char		traverseOrder,
										unsigned long		resultArray[]);

#endif
