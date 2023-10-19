#include "raylib.h"

#include "linked_list.h"


void LinkedListAdd(ListNode **head, ListNode *node) {

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
}

void LinkedListRemove(ListNode **head, ListNode *node) {

    if (*head == node) {
        *head = node->next;
    }

    if (node->next) node->next->previous = node->previous;
    if (node->previous) node->previous->next = node->next;

    if (node->item) MemFree(node->item);
    MemFree(node);

    TraceLog(LOG_TRACE, "Removed item from linked list and destroyed it.");
}

void LinkedListRemoveAll(ListNode **head) {

    while (*head) {
        LinkedListRemove(head, *head);
    }

    TraceLog(LOG_TRACE, "Removed all items from linked list and destroyed them.");
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