#include "TypeDef.h"
#include "_QuartzDictionary.h"

static char* __FindTNode  (DICTIONARY_HEADER* pDicHeader, char* t_name);
static char* __ADDTNode  (DICTIONARY_HEADER* pDicHeader, char* t_name);


/******************************************************************************
* Function name	: _CreateTable()
* Return         	:  0 : Table Create Success;
                      -1 : Table Create Failure
* Description    	: 
* Programmer		:  endy@gmail.com
* Revision       	:  v1.0
******************************************************************************/
Q_RET_CODE _CreateTable(DICTIONARY_HEADER* pDicHeader, char* t_name)
{
    Q_RET_CODE ret     = Q_RET_FAILURE;
    char*      t_node  = Q_NULL;

    t_node = __FindTable(pDicHeader, t_name);

    if (t_node == Q_NULL)
    {

    }

    return 0;
}

Q_RET_CODE _DeleteTable(DICTIONARY_HEADER* pDicHeader, char* t_name)
{
    return 0;
}

Q_RET_CODE _AddColume(char* t_name, char* c_name)
{
    return 0;
}

Q_RET_CODE _RemoveAllColume(char* t_name)
{
    return 0;
}

/******************************************************************************
* Function name	: __FindTNode()
* Return         	:  0 : Table Find Success;
                      -1 : Table Find Failure
* Description    	:  
* Programmer		:  endy@gmail.com
* Revision       	:  v1.0
******************************************************************************/
static char* __FindTNode(DICTIONARY_HEADER* pDicHeader, char* t_name)
{
    char* t_node = Q_NULL;

    return t_node;
}

/******************************************************************************
* Function name	: __AddTNode()
* Return         	:  0 : Table Add Success;
                      -1 : Table Add Failure
* Description    	:  
* Programmer		:  endy@gmail.com
* Revision       	:  v1.0
******************************************************************************/
static char* __AddTNode(DICTIONARY_HEADER* pDicHeader, char* t_name)
{
    char* t_node = Q_NULL;

    return t_node;
}


/******************************************************************************
* Function name	: __AddTNode()
* Return         	:  0 : Table Add Success;
                      -1 : Table Add Failure
* Description    	:  
* Programmer		:  endy@gmail.com
* Revision       	:  v1.0
******************************************************************************/
static char* __AllocTNode(DICTIONARY_HEADER* pDicHeader, char* t_name)
{
    char* t_node = Q_NULL;

    return t_node;
}