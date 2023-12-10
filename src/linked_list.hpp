#ifndef _LINKED_LIST_H_INCLUDED_
#define _LINKED_LIST_H_INCLUDED_


typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *previous;

    void *item;
} ListNode;


// Adds new node containing 'item' to a linked list. Returns the new node.
ListNode *LinkedListAdd(ListNode **head, void *item);

// Returns list node containing item, or 0 if not found.
ListNode *LinkedListGetNode(ListNode *head, void *item);

// Removes and destroys node, with its item, from a linked list.
void LinkedListDestroyNode(ListNode **head, ListNode *node);

// Removes and destroys all nodes, with its items, from a linked list.
void LinkedListDestroyAll(ListNode **head);

// Removes and destroys a node from a linked list, but doesn't destroy its item.
void LinkedListRemoveNode(ListNode **head, ListNode *node);

// Removes all nodes from a linked list, but doesn't destroy the items.
void LinkedListRemoveAll(ListNode **head);

// Counts how many nodes there are in a linked list.
int LinkedListCountNodes(ListNode *head);


#endif // _LINKED_LIST_H_INCLUDED_
