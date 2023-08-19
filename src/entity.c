#include <raylib.h>

#include "entity.h"
#include "player.h"

Entity *AddToEntityList(Entity *head, Entity *toBeAdded) {
    
    if (!head) return toBeAdded;

    Entity *last = head;

    while (last->next) {
        last = last->next;
    }

    toBeAdded->previous = last;
    last->next = toBeAdded;

    return head;
}

int RemoveFromEntityList(Entity *head, Entity *toBeRemoved) {
    if (toBeRemoved->previous) toBeRemoved->previous->next = toBeRemoved->next;
    if (toBeRemoved->next) toBeRemoved->next->previous = toBeRemoved->previous;
    
    if (head == toBeRemoved) head = head->next;

    MemFree(toBeRemoved);
}

void ClearEntityList(Entity *head) {
    
    Entity *current = head;

    while (current) {
        Entity *next = current->next;
        MemFree(current);
        current = next;
    } 
}

void SetEntityPosition(Entity *entity, float x, float y) {
    entity->hitbox.x = y;
    entity->hitbox.y = y;
}

void TickAllEntities(Entity *entity) {
    while (entity) {
        if (entity->components & IsPlayer != 0) PlayerTick(entity);

        entity = entity->next;
    }
}

void DrawAllEntities(Entity *entity) {
    while (entity) {
        if (entity->components & IsPlayer != 0) DrawPlayer(entity);

        entity = entity->next;
    }
}