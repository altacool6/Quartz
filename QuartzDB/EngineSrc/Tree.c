#include "Tree.h"

static void __RotationLR();
static void __RotationLL();
static void __RotationRR();
static void __RotationRL();


/******************************************************************************
* Function name	: __CompareKey(ptr, req)
* Return         	:  0 : ret is same with ptr
                       1 : req is bigger than ptr
                      -1 : req is smaller than ptr
* Description    	: 
* Programmer		:  endy@gmail.com
* Revision       	:  v1.0
******************************************************************************/
static void __CompareKey(void* ptr, TREE_NODE_ADD_REQ* req)
{

}

void _PrintTree(int *pParentOffset, TREE_KEY_TYPE type)
{
    
}

/******************************************************************************
* Function name	: _AddTreeNode(ptr, req)
* Return        :  0 : ret is same with ptr
                       1 : req is bigger than ptr
                      -1 : req is smaller than ptr
* Description   : �� �Լ��� Recusive Call�� �����Ǿ� ������
                  Call In ���¿��� ����
                  Call Out ���¿��� Node�� Height ��� �� Rebalancing �۾��ϸ�
                  Tree�� Balancing�� �����ϴ� ������ �Ѵ�.
* Programmer    : endy@gmail.com
* Revision      : v1.0
******************************************************************************/
TREE_NODE* _AddTreeNode(int offsetOfParentOffset, TREE_NODE_ADD_REQ* req)
{
    TREE_NODE*  pTreeNode;
    QPAGE_INFO* pPageInfo;
    int         parentOffset

    pPageInfo    = MMU_TOUCH(pParentOffset);
    parentOffset = pPageInfo->memory + 

    if (*pParentOffset == 0) {
        /* Make TREE_NODE */
        Q
    }
    else {

    }
    treeNode = OFFSET_2_TREE_NODE(*pParentOffset)
}