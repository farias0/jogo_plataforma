#include "raylib.h"

#include "collision.h"
#include "entities/entity.h"
#include "global.h"


void CheckForCollisions() {
    if (PLAYER->hitbox.y > FLOOR_DEATH_HEIGHT) {
        STATE->isPlayerDead = true;
        STATE->isPaused = true;
        return;
    }

    Entity *enemy = ENTITIES;
    do {

        if (enemy->components & IsEnemy) {

            // Enemy offscreen
            if  (enemy->hitbox.x + enemy->hitbox.width < 0 ||
                    enemy->hitbox.y > FLOOR_DEATH_HEIGHT) {
                        
                ENTITIES = DestroyEntity(enemy); // TODO: How does this break the loop?
                break;
            }

            // Enemy hit player
            if (CheckCollisionRecs(enemy->hitbox, playersUpperbody)) {
                STATE->isPlayerDead = true;
                STATE->isPaused = true;
                break;
            }

            // Player hit enemy
            if (CheckCollisionRecs(enemy->hitbox, playersLowebody)) {
                ENTITIES = DestroyEntity(enemy); // TODO: How does this break the loop?
                break;
            }
        }

        enemy = enemy->next;
    } while (enemy != ENTITIES);
}