#include "TypeDef.h"
#include "List.h"

void LIST_InitNode(LIST_NODE *node)
{
	node->next = node;
	node->prev = node;
}


Q_BOOL LIST_IsEmpty(LIST_NODE *head)
{
    if (head == head->next && head == head->prev)
        return Q_TRUE;
    else
        return Q_FALSE;
}

void LIST_AddFirst(LIST_NODE *head, 
                   LIST_NODE *link)
{
    link->next = head->next;
    link->prev = head;

    head->next->prev = link;
	head->next       = link;
}

void LIST_AddLast(LIST_NODE *head, 
                  LIST_NODE *link)
{
    link->next = head;
    link->prev = head->prev;

    head->prev->next = link;
    head->prev       = link;
}


void LIST_Remove(LIST_NODE *link)
{
    link->next->prev = link->prev;
    link->prev->next = link->next;

   LIST_InitNode(link);
}
