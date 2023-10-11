#include <raylib.h>

#include "enemy.h"
#include "entity.h"
#include "../global.h"
#include "level.h"
#include "../render.h"
#include "../assets.h"

#define ENEMY_SPEED_DEFAULT 4.0f
#define ENEMY_FALL_RATE 7.0f


Entity *InitializeEnemy(Entity *head, int x, int y) { 
    Entity *newEnemy = MemAlloc(sizeof(Entity));

    newEnemy->components = HasPosition +
                            HasSprite +
                            DoesTick +
                            IsEnemy +
                            IsLevelElement;
    newEnemy->hitbox = (Rectangle){ x - ((EnemySprite.sprite.width * EnemySprite.scale) / 2),
                                    y - (EnemySprite.sprite.height * EnemySprite.scale) + 1,
                                    EnemySprite.sprite.width * EnemySprite.scale,
                                    EnemySprite.sprite.height * EnemySprite.scale };
    newEnemy->sprite = EnemySprite;
    newEnemy->isFacingRight = false;
    newEnemy->isFallingDown = true;

    return AddToEntityList(head, newEnemy);
}

void EnemyTick(Entity *enemy, Entity *player) {
    int x_back = enemy->hitbox.x;
    Entity *groundBeneath = GetGroundBeneath(enemy);

    if (enemy->isFallingDown) {
        
        if (groundBeneath &&
            enemy->hitbox.y + enemy->hitbox.height + ENEMY_FALL_RATE >= groundBeneath->hitbox.y) {

            // Land
            enemy->hitbox.y = groundBeneath->hitbox.y - enemy->hitbox.height;
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
    
    // TODO use yGroundBeneath -- currently it breaks the AI when it goes to the edge of the ground
    if (!GetGroundBeneath(enemy)) {
        
        // Turn around
        enemy->hitbox.x = x_back;
        enemy->isFacingRight = !(enemy->isFacingRight);
    }


    // Collision checking
    {
        Entity *enemy = ENTITIES_HEAD;
        while (enemy != 0) {

            if (enemy->components & IsEnemy) {

                // Enemy offscreen
                if  (enemy->hitbox.x + enemy->hitbox.width < 0 ||
                        enemy->hitbox.y > FLOOR_DEATH_HEIGHT) {
                            
                    ENTITIES_HEAD = DestroyEntity(enemy); // TODO: How does this break the loop?
                    break;
                }
            }

            enemy = enemy->next;
        }
    }
}

Entity *AddEnemyToLevel(Entity *head, Vector2 pos) {    

    Rectangle hitbox = {
        // Considering the player is clicking in the middle of the sprite
        pos.x - ((EnemySprite.sprite.width * EnemySprite.scale) / 2),
        pos.y - ((EnemySprite.sprite.height * EnemySprite.scale) / 2),
        (EnemySprite.sprite.width * EnemySprite.scale),
        (EnemySprite.sprite.height * EnemySprite.scale)
    };

    Entity *currentItem = head;
    while (currentItem != 0) {
    
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

        currentItem = currentItem->next;

    }

    /*
        InitializeEnemy places the enemy _above_ this point,
        so we bring the y down to the feet of the sprite.
        
        TODO: Make InitializeAbove receive normal x and y.
        The level positions the enemy to fall to the floor.
    */
    float feet = pos.y + ((EnemySprite.sprite.height * EnemySprite.scale) / 2);


    return InitializeEnemy(head, pos.x, feet);
}
