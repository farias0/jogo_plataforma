#include <raylib.h>
#include <stdlib.h>

#include "entity.h"
#include "player.h"
#include "enemy.h"
#include "level.h"
#include "camera.h"

#define ON_THE_GROUND_Y_TOLERANCE 5 // The difference between the y of the hitbox and the ground to be considered "on the ground"


    /*
        Adds a new entity right after listItem.
    */
void AddToEntityList(Entity *listItem, Entity *toBeAdded) {
    
    if (listItem) {
        Entity *nextItem = listItem->next;

        nextItem->previous = toBeAdded;
        toBeAdded->previous = listItem;

        listItem->next = toBeAdded;
        toBeAdded->next = nextItem;
    } else {
        toBeAdded->next = toBeAdded;
        toBeAdded->previous = toBeAdded;
    }
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
    entity->hitbox.x = x;
    entity->hitbox.y = y;
}

void TickAllEntities(Entity *listItem, Entity *player) {
    Entity *currentItem = listItem;

    do {
        // IMPORTANT: Enemy must tick before player or collision check between
        // the two _might_ break
        if (currentItem->components & IsEnemy) EnemyTick(currentItem, player);
        else if (currentItem->components & IsPlayer) PlayerTick(currentItem);
        else if (currentItem->components & IsCamera) CameraTick();

        currentItem = currentItem->next;
    } while (currentItem != listItem);
}

Entity *DestroyEntity(Entity *entity) {
    Entity *listReference = entity;
    if (entity == entity->next) listReference = 0;
    else listReference = entity->next;

    entity->next->previous = entity->previous;
    entity->previous->next = entity->next;

    UnloadTexture(entity->sprite);

    MemFree(entity);

    return listReference;
}

void DestroyAllEntities(Entity *listItem) {
    while (listItem) {
        listItem = DestroyEntity(listItem);
    }
}

int CountEntities(Entity *listItem) {
    if (listItem == 0) return 0;

    Entity *currentItem = listItem;
    int counter = 0;

    do {
        counter++;
        currentItem = currentItem->next;
    } while (currentItem != listItem);

    return counter;
}

float GetEntitiesGroundBeneath(Entity *entity) {

    int entitysFoot = entity->hitbox.y + entity->hitbox.height;
    Entity *possibleGround = entity->next;

    while (possibleGround != entity) {

        if (possibleGround->components & IsLevelElement &&


            // TODO increase x tolerance

            // If x is within the possible ground, with a fraction of the player's hitbox as "coyote time"
            possibleGround->hitbox.x < (entity->hitbox.x + ((4*entity->hitbox.width)/5)) &&
            entity->hitbox.x + (entity->hitbox.width/5) < (possibleGround->hitbox.x + possibleGround->hitbox.width) &&

            // If y is RIGHT above the possible ground
            abs(possibleGround->hitbox.y - entitysFoot) <= ON_THE_GROUND_Y_TOLERANCE) {
                
                return possibleGround->hitbox.y;
            } 

        possibleGround = possibleGround->next;
    }

    return -1;    
}