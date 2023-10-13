#include <raylib.h>
#include <stdlib.h>

#include "entity.h"
#include "player.h"
#include "enemy.h"
#include "level.h"
#include "camera.h"
#include "../global.h"

#define ON_THE_GROUND_Y_TOLERANCE 5 // The difference between the y of the hitbox and the ground to be considered "on the ground"


void ReloadEntityList() {

    DestroyAllEntities(ENTITIES_HEAD);
    ENTITIES_HEAD = InitializeCamera(0, &CAMERA);
}

Entity *AddToEntityList(Entity *head, Entity *toBeAdded) { 

    Entity *lastItem = head;

    if (head) {
        while (lastItem->next != 0) {
            lastItem = lastItem->next;
        }

        lastItem->next = toBeAdded;
    }  else {
        head = toBeAdded;
    }

    toBeAdded->previous = lastItem;
    toBeAdded->next = 0;

    TraceLog(LOG_DEBUG, "Entity of component=%lu, x=%.1f, y=%.1f added to list.",
                toBeAdded->components, toBeAdded->hitbox.x, toBeAdded->hitbox.y);

    return head;
}

void SetEntityPosition(Entity *entity, Vector2 pos) {
    entity->hitbox.x = pos.x;
    entity->hitbox.y = pos.y;
}

void TickAllEntities(Entity *listItem, Entity *player) {
    Entity *currentItem = listItem;

    while (currentItem != 0) {
        // IMPORTANT: Enemy must tick before player or collision check between
        // the two _might_ break
        if (currentItem->components & IsEnemy) EnemyTick(currentItem, player);
        else if (currentItem->components & IsPlayer) PlayerTick(currentItem);
        else if (currentItem->components & IsCamera) CameraTick();

        /*
            TODO !! EXTREMELY IMPORTANT !!
            If currentItem was Destroyed during this iteration, then the statement below will break.
            Fix this.
        */
        currentItem = currentItem->next;
    }
}

Entity *DestroyEntity(Entity *entity) {
    Entity *listHead = ENTITIES_HEAD; 
    if (entity == ENTITIES_HEAD) {
        listHead = entity->next;
        ENTITIES_HEAD = listHead;
    }

    if (entity->next) entity->next->previous = entity->previous;
    if (entity->previous) entity->previous->next = entity->next;

    Entity copy = *entity;

    MemFree(entity);

    TraceLog(LOG_DEBUG, "Entity of component=%lu destroyed.", copy.components);

    return listHead;
}

void DestroyAllEntities(Entity *head) {
    while (head) {
        head = DestroyEntity(head);
    }
}

Entity *DestroyEntityOn(Entity *head, Vector2 pos) {
    
    Entity *currentItem = head;

    while (currentItem != 0) {

        if (currentItem->components & HasPosition &&
            !(currentItem->components & IsPlayer) &&
            CheckCollisionPointRec(pos, currentItem->hitbox)) {

                return DestroyEntity(currentItem);
            }

        currentItem = currentItem->next;
        
    };

    return head;
}

int CountEntities(Entity *listItem) {

    Entity *currentItem = listItem;
    int counter = 0;

    while (currentItem != 0) {
        counter++;
        currentItem = currentItem->next;
    }

    // At least Camera and Player must be present
    if (counter < 2) {
        TraceLog(LOG_ERROR, "Entity counter at %d!", counter);
    }

    return counter;
}

Entity *GetGroundBeneath(Entity *entity) {

    int entitysFoot = entity->hitbox.y + entity->hitbox.height;
    Entity *possibleGround = ENTITIES_HEAD;

    while (possibleGround != 0) {

        if (possibleGround->components & IsLevelElement &&

            possibleGround != entity &&

            // TODO increase x tolerance

            // If x is within the possible ground
            possibleGround->hitbox.x < (entity->hitbox.x + entity->hitbox.width) &&
            entity->hitbox.x < (possibleGround->hitbox.x + possibleGround->hitbox.width) &&

            // If y is RIGHT above the possible ground
            abs(possibleGround->hitbox.y - entitysFoot) <= ON_THE_GROUND_Y_TOLERANCE) {
                
                return possibleGround;
            } 

        possibleGround = possibleGround->next;
    }

    return 0;    
}