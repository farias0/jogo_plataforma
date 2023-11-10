#ifndef _LINKED_LIST_H_INCLUDED_
#define _LINKED_LIST_H_INCLUDED_


typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *previous;

    void *item;
} ListNode;


// Adds new node containing 'item' to a linked list. Returns the new node.
ListNode *LinkedListAdd(ListNode **head, void *item);

// Removes and destroys node from a linked list.
void LinkedListRemove(ListNode **head, ListNode *node);

// Removes and destroys all nodes from a linked list.
void LinkedListRemoveAll(ListNode **head);

// Counts how many nodes there are in a linked list.
int LinkedListCountNodes(ListNode *head);


#endif // _LINKED_LIST_H_INCLUDED_
