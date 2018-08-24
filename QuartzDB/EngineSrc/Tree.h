#ifndef __TREE_H__
#define __TREE_H__

typedef char TREE_KEY_TYPE;
enum {
    TREE_KEY_U8,
    TREE_KEY_U16,
    TREE_KEY_U32,
    TREE_KEY_VCHAR,
    TREE_KEY_VCHAR2
};/* TREE_KEY_TYPE */

typedef struct TreeNodeAddReq{
    TREE_KEY_TYPE   type;
    void*           pData;
    int             sizeOfData;
} TREE_NODE_ADD_REQ;

typedef struct TreeNode{
    int  offsetL;
    int  offsetR;
    char height;
} TREE_NODE;

#define OFFSET_2_TREE_NODE(offset) (TREE_NODE*)(offset)

void _AddTreeNode(int *pParentOffset, TREE_NODE_ADD_REQ* req);

#endif //#ifndef __TREE_H__
