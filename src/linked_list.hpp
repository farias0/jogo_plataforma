#ifndef _LINKED_LIST_H_INCLUDED_
#define _LINKED_LIST_H_INCLUDED_


namespace LinkedList {


class NodeItem {

public:
    virtual ~NodeItem() = default; // so objects from derived classes can be deleted from a NodeItem reference
};

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *previous;

    NodeItem *item;
} ListNode;


// Adds new node containing 'item' to a linked list. Returns the new node.
ListNode *Add(ListNode **head, NodeItem *item);

// Returns list node containing item, or 0 if not found.
ListNode *GetNode(ListNode *head, void *item);

// Removes and destroys node, with its item, from a linked list.
void DestroyNode(ListNode **head, ListNode *node);

// Removes and destroys all nodes, with its items, from a linked list.
void DestroyAll(ListNode **head);

// Removes and destroys a node from a linked list, but doesn't destroy its item.
void RemoveNode(ListNode **head, ListNode *node);

// Removes all nodes from a linked list, but doesn't destroy the items.
void RemoveAll(ListNode **head);

// Counts how many nodes there are in a linked list.
int CountNodes(ListNode *head);


} // namespace


#endif // _LINKED_LIST_H_INCLUDED_
