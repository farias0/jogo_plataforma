#ifndef _ENTITY_H_INCLUDED_
#define _ENTITY_H_INCLUDED_


#include <raylib.h>

#include "../assets.h"


enum Components {
    HasPosition     = 1,
    IsPlayer        = 2,
    HasSprite       = 4,
    DoesTick        = 8,
    IsEnemy         = 16,
    IsLevelElement  = 32,
    IsCamera        = 64
};

typedef struct Entity {
    struct Entity *previous;
    struct Entity *next;

    unsigned long int components;
    
    Rectangle hitbox;
    Sprite sprite;

    bool isFacingRight;
    bool isFallingDown;
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
Entity *DestroyEntity(Entity *entity);
// Destroys an entity whose hitbox contains pos
Entity *DestroyEntityOn(Entity *listItem, Vector2 pos);
void DestroyAllEntities(Entity *listItem);
int CountEntities(Entity *listItem);
// The y on the ground the player is above, or -1 if not on the ground
float GetEntitiesGroundBeneath(Entity *entity);

#endif // _ENTITY_H_INCLUDED_