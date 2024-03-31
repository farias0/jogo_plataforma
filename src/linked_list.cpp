#include "raylib.h"

#include "linked_list.hpp"


namespace LinkedList {


ListNode *Add(ListNode **head, NodeItem *item) {

    ListNode *node = (ListNode *) MemAlloc(sizeof(ListNode));

    node->item = item;

    ListNode *lastItem = *head;

    if (*head) {

        while (lastItem->next != 0) {
            lastItem = lastItem->next;
        }

        lastItem->next = node;

    }  else {
        *head = node;
    }

    node->previous = lastItem;
    node->next = 0;

    TraceLog(LOG_TRACE, "Added item to linked list.");

    return node;
}

void DestroyNode(ListNode **head, ListNode *node) {

    delete node->item;
    RemoveNode(head, node);

    TraceLog(LOG_TRACE, "Destroyed node with item from linked list.");
}

void RemoveNode(ListNode **head, ListNode *node) {

    if (*head == node) {
        *head = node->next;
    }

    if (node->next) node->next->previous = node->previous;
    if (node->previous) node->previous->next = node->next;
    MemFree(node);

    TraceLog(LOG_TRACE, "Destroyed node from linked list.");
}

ListNode *GetNode(ListNode *head, void *item) {

    while (head != 0 && head->item != item) {
        head = head->next;
    }

    return head;
}

void DestroyAll(ListNode **head) {

    while (*head) {
        DestroyNode(head, *head);
    }

    TraceLog(LOG_TRACE, "Destroyed a linked list.");
}

void RemoveAll(ListNode **head) {

    ListNode *node = *head;
    while (node) {
        ListNode *next = node->next;
        MemFree(node);
        node = next;
    }

    *head = 0;

    TraceLog(LOG_TRACE, "Removed all items from a linked list.");
}

int CountNodes(ListNode *head) {

    ListNode *node = head;
    int counter = 0;

    while (node != 0) {
        counter++;
        node = node->next;
    }

    return counter;
}


} // namespace
