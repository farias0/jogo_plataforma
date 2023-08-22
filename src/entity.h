#ifndef _ENTITY_H_INCLUDED_
#define _ENTITY_H_INCLUDED_


#include <raylib.h>

enum Components {
    HasPosition     = 1,
    IsPlayer        = 2,
    HasSprite       = 4,
    DoesTick        = 8,
    IsEnemy         = 16,
    IsLevelElement  = 32
};

typedef struct Entity {
    struct Entity *previous;
    struct Entity *next;

    unsigned long int components;
    
    Rectangle hitbox;
    Texture2D sprite;   // TODO be a pointer
} Entity;

/*
    TODO: Entity system implemented as a linked list.
    Should reimplement it as a data structure that uses contiguous memory.
*/
void AddToEntityList(Entity *listItem, Entity *toBeAdded);
int RemoveFromEntityList(Entity *head, Entity *toBeRemoved);
void ClearEntityList(Entity *head);

// Common operations
void SetEntityPosition(Entity *entity, float x, float y); // Sets the position of the hitbox
void TickAllEntities(Entity *entity, Entity *player);
void DrawAllEntities(Entity *entity);
Entity *DestroyEntity(Entity *entity);
void DestroyAllEntities(Entity *listItem);
int CountEntities(Entity *listItem);
bool IsOnTheGround(Entity *entity); // If the entity has a level tile immediately bellow it (is "on the ground")

#endif // _ENTITY_H_INCLUDED_