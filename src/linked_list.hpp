#ifndef _LINKED_LIST_H_INCLUDED_
#define _LINKED_LIST_H_INCLUDED_


namespace LinkedList {


class Node {

public:
    Node *next;
    Node *previous;

    virtual ~Node() = default; // so objects from derived classes can be deleted from a Node pointer
};


// Adds new node containing 'item' to a linked list. Returns the new node.
Node *AddNode(Node **head, Node *node);

// Removes Node from a linked list and destroys it.
void DestroyNode(Node **head, Node *node);

// Removes all Nodes from a linked list and destroys them.
void DestroyAll(Node **head);

// Removes Node from a linked list, but doesn't destroy it.
void RemoveNode(Node **head, Node *node);

// Removes all Nodes from a linked list, but doesn't destroy them.
void RemoveAll(Node **head);

// Counts how many nodes there are in a linked list.
int CountNodes(Node *head);


} // namespace


#endif // _LINKED_LIST_H_INCLUDED_
