#ifndef __QUARTZ_DICTIONARY_H__
#define __QUARTZ_DICTIONARY_H__

#define QUARTZ_DICTIONARY_MAGIC (0xDDDDCCCC)

typedef struct DictionaryINFO{
    unsigned  long magic;
    int            sizeOfMemory;
    char*          pRootT;
} DICTIONARY_HEADER;

typedef struct TableNode{
    unsigned short offsetLeft;
    unsigned short offsetRight;
    signed   int   balanceFactor;   //balanceFactor
    signed   int   keyValue;

    unsigned short offsetName;
    unsigned short dataPage;
} TNode;


#define INIT_DICTIONARY_HEADER(a) do{                                           \
                                    (a).magic        = QUARTZ_DICTIONARY_MAGIC; \
                                    (a).sizeOfMemory = 0;                       \
                                    (a).pRootT       = Q_NULL;                  \
                                }while(0)

Q_RET_CODE   _CreateTable(DICTIONARY_HEADER* pDicHeader, char* t_name);
Q_RET_CODE   _DeleteTable(DICTIONARY_HEADER* pDicHeader, char* t_name);
char* __FindTable  (DICTIONARY_HEADER* pDicHeader, char* t_name);

Q_RET_CODE   _AddColume(char* t_name, char* c_name);
Q_RET_CODE   _RemoveAllColume(char* t_name);

#endif	//#ifndef __QUARTZ_DICTIONARY_H__