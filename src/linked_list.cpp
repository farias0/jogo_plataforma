#include "raylib.h"

#include "linked_list.hpp"


ListNode *LinkedListAdd(ListNode **head, void *item) {

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

void LinkedListDestroyNode(ListNode **head, ListNode *node) {

    MemFree(node->item);
    LinkedListRemoveNode(head, node);

    TraceLog(LOG_TRACE, "Destroyed node with item from linked list.");
}

void LinkedListRemoveNode(ListNode **head, ListNode *node) {

    if (*head == node) {
        *head = node->next;
    }

    if (node->next) node->next->previous = node->previous;
    if (node->previous) node->previous->next = node->next;
    MemFree(node);

    TraceLog(LOG_TRACE, "Destroyed node from linked list.");
}

ListNode *LinkedListGetNode(ListNode *head, void *item) {

    while (head != 0 && head->item != item) {
        head = head->next;
    }

    return head;
}

void LinkedListDestroyAll(ListNode **head) {

    while (*head) {
        LinkedListDestroyNode(head, *head);
    }

    TraceLog(LOG_TRACE, "Destroyed a linked list.");
}

void LinkedListRemoveAll(ListNode **head) {

    ListNode *node = *head;
    while (node) {
        ListNode *next = node->next;
        MemFree(node);
        node = next;
    }

    *head = 0;

    TraceLog(LOG_TRACE, "Removed all items from a linked list.");
}

int LinkedListCountNodes(ListNode *head) {

    ListNode *node = head;
    int counter = 0;

    while (node != 0) {
        counter++;
        node = node->next;
    }

    return counter;
}
