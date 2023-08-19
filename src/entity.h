#ifndef _ENTITY_H_INCLUDED_
#define _ENTITY_H_INCLUDED_


#include <raylib.h>

enum Components {
    HasPosition     = 0x1,
    IsPlayer        = 0x2,
    HasSprite       = 0x4,
    DoesTick        = 0x8
};

typedef struct Entity {
    struct Entity *previous;
    struct Entity *next;

    unsigned long int components;
    
    Rectangle hitbox;
    Texture2D sprite;
} Entity;

/*
    TODO: Entity system implemented as a linked list.
    Should reimplement it as a data structure that uses contiguous memory.
*/
Entity *AddToEntityList(Entity *head, Entity *toBeAdded);
int RemoveFromEntityList(Entity *head, Entity *toBeRemoved);
void ClearEntityList(Entity *head);

// Common operations
void SetEntityPosition(Entity *entity, float x, float y); // Sets the position of the hitbox
void TickAllEntities(Entity *entity);
void DrawAllEntities(Entity *entity);

#endif // _ENTITY_H_INCLUDED_