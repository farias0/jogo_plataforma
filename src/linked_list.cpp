#include "raylib.h"

#include "linked_list.hpp"


namespace LinkedList {


Node *AddNode(Node **head, Node *node) {

    Node *lastItem = *head;

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

void DestroyNode(Node **head, Node *node) {

    RemoveNode(head, node);
    delete node;

    TraceLog(LOG_TRACE, "Destroyed node with item from linked list.");
}

void RemoveNode(Node **head, Node *node) {

    if (*head == node) {
        *head = node->next;
    }

    if (node->next) node->next->previous = node->previous;
    if (node->previous) node->previous->next = node->next;

    TraceLog(LOG_TRACE, "Removed node from linked list.");
}

void DestroyAll(Node **head) {

    while (*head) {
        DestroyNode(head, *head);
    }

    TraceLog(LOG_TRACE, "Destroyed all items from a linked list.");
}

void RemoveAll(Node **head) {

    Node *node = *head;
    while (node) {
        Node *next = node->next;
        node = next;
    }

    *head = 0;

    TraceLog(LOG_TRACE, "Removed all items from a linked list.");
}

int CountNodes(Node *head) {

    Node *node = head;
    int counter = 0;

    while (node != 0) {
        counter++;
        node = node->next;
    }

    return counter;
}


} // namespace
