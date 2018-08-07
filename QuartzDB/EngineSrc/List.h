#ifndef __LIST_H__
#define __LIST_H__

#include "TypeDef.h"

typedef struct ListNode {
    struct ListNode* prev;
    struct ListNode* next;
}LIST_NODE;

#define INIT_LIST_NODE(x)   do{                \
                               (x).prev = &(x);\
                               (x).next = &(x);\
                            } while(0) 

#define LIST_FIRST(x)  ((x).next)
#define LIST_LAST(x)   ((x).prev)

void   LIST_InitNode (LIST_NODE *node);
Q_BOOL LIST_IsEmpty  (LIST_NODE *head);
void   LIST_AddFirst (LIST_NODE *head, LIST_NODE *link);
void   LIST_AddLast  (LIST_NODE *head, LIST_NODE *link);
void   LIST_Remove   (LIST_NODE *link);

#endif  // #ifndef __LIST_H__