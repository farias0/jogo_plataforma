#include <raylib.h>

#include "enemy.h"
#include "entity.h"
#include "../global.h"
#include "level.h"
#include "../render.h"
#include "../assets.h"

#define ENEMY_SPEED_DEFAULT 4.0f
#define ENEMY_FALL_RATE 7.0f


Entity *InitializeEnemy(Entity *listItem, int x, int y) { 
    Entity *newEnemy = MemAlloc(sizeof(Entity));

    newEnemy->components = HasPosition +
                            HasSprite +
                            DoesTick +
                            IsEnemy +
                            IsLevelElement;
    newEnemy->hitbox = (Rectangle){ x - (EnemySprite.sprite.width/2), y - EnemySprite.sprite.height + 1, EnemySprite.sprite.width, EnemySprite.sprite.height };
    newEnemy->sprite = EnemySprite;
    newEnemy->isFacingRight = false;
    newEnemy->isFallingDown = true;

    AddToEntityList(listItem, newEnemy);

    return newEnemy;
}

void EnemyTick(Entity *enemy, Entity *player) {
    int x_back = enemy->hitbox.x;
    float yGroundBeneath = GetEntitiesGroundBeneath(enemy);

    if (enemy->isFallingDown) {
        
        if (yGroundBeneath >= 0 && enemy->hitbox.y + ENEMY_FALL_RATE + EnemySprite.sprite.height >= yGroundBeneath) {

            // Land
            enemy->hitbox.y = yGroundBeneath - EnemySprite.sprite.height;
            enemy->isFallingDown = false;
        }

        else {

            // Fall
            enemy->hitbox.y += ENEMY_FALL_RATE;
        }

        return;
    }


    if (enemy->isFacingRight) enemy->hitbox.x -= ENEMY_SPEED_DEFAULT;
    else enemy->hitbox.x += ENEMY_SPEED_DEFAULT;
    
    // TODO use yGroundBeneath -- currently it breaks the AI
    if (GetEntitiesGroundBeneath(enemy) == -1) {
        
        // Turn around
        enemy->hitbox.x = x_back;
        enemy->isFacingRight = !(enemy->isFacingRight);
    }


    // Collision checking
    {
        Entity *enemy = ENTITIES;
        do {

            if (enemy->components & IsEnemy) {

                // Enemy offscreen
                if  (enemy->hitbox.x + enemy->hitbox.width < 0 ||
                        enemy->hitbox.y > FLOOR_DEATH_HEIGHT) {
                            
                    ENTITIES = DestroyEntity(enemy); // TODO: How does this break the loop?
                    break;
                }
            }

            enemy = enemy->next;
        } while (enemy != ENTITIES);
    }
}

bool AddEnemyToLevel(Entity *listItem, Vector2 pos) {    

    Rectangle hitbox = {
        // Considering the player is clicking in the middle of the sprite
        pos.x - (EnemySprite.sprite.width / 2), pos.y - (EnemySprite.sprite.height / 2),
        EnemySprite.sprite.width, EnemySprite.sprite.height
    };

    for (Entity *currentItem = listItem->next; currentItem != listItem; currentItem = currentItem->next) {
        
        if ((currentItem->components & IsLevelElement || currentItem->components & IsEnemy) &&
                CheckCollisionRecs(hitbox, currentItem->hitbox)) {

                /*
                    TODO: Click closer to the ground and still place the enemy.
                    
                        If the collision is with a block (create a block component btw),
                        but the y of the block is below the mouse, change the hitbox
                        to be right above the ground.

                        It will need to check for collisions again, and be careful about
                        not entering infinite loops.
                */

                return false;
            }
    }

    /*
        InitializeEnemy places the enemy _above_ this point,
        so we bring the y down to the feet of the sprite.
        
        TODO: Make InitializeAbove receive normal x and y.
        The level positions the enemy to fall to the floor.
    */
    float feet = pos.y + (EnemySprite.sprite.height / 2);

    InitializeEnemy(listItem, pos.x, feet);
    return true;
}
