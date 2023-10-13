#ifndef _ENTITY_H_INCLUDED_
#define _ENTITY_H_INCLUDED_


#include <raylib.h>

#include "../assets.h"


typedef enum Component {
    HasPosition             = 1,
    IsPlayer                = 2,
    HasSprite               = 4,
    DoesTick                = 8,
    IsEnemy                 = 16,
    IsLevelElement          = 32,
    IsCamera                = 64,
    IsOverworldElement      = 128,
    IsCursor                = 256,
} Component;

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

// Clears entity list and reloads basic entities
void ReloadEntityList();

// Adds an Entity to the end of the list and returns the head
Entity *AddToEntityList(Entity *head, Entity *toBeAdded);

void SetEntityPosition(Entity *entity, Vector2 pos);

void TickAllEntities(Entity *entity, Entity *player);

// Remove entity from linked list, destroy it and returns the list's head
Entity *DestroyEntity(Entity *entity);

// Destroys an entity whose hitbox contains pos and returns the list's head
Entity *DestroyEntityOn(Entity *head, Vector2 pos);

void DestroyAllEntities(Entity *head);

int CountEntities(Entity *listItem);

// The ground beneath the entity, or 0 if not on the ground
Entity *GetGroundBeneath(Entity *entity);


#endif // _ENTITY_H_INCLUDED_