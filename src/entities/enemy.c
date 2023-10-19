#include <raylib.h>

#include "enemy.h"
#include "../linked_list.h"
#include "../global.h"
#include "level.h"
#include "../render.h"
#include "../assets.h"

#define ENEMY_SPEED_DEFAULT 4.0f
#define ENEMY_FALL_RATE 7.0f


void LevelEnemyAdd(int x, int y) {

    LevelEntity *newEnemy = MemAlloc(sizeof(LevelEntity));

    newEnemy->components = LEVEL_IS_ENEMY;
    newEnemy->hitbox = (Rectangle){ x - ((EnemySprite.sprite.width * EnemySprite.scale) / 2),
                                    y - (EnemySprite.sprite.height * EnemySprite.scale) + 1,
                                    EnemySprite.sprite.width * EnemySprite.scale,
                                    EnemySprite.sprite.height * EnemySprite.scale };
    newEnemy->sprite = EnemySprite;
    newEnemy->isFacingRight = false;
    newEnemy->isFallingDown = true;

    ListNode *node = MemAlloc(sizeof(ListNode));
    node->item = newEnemy;
    LinkedListAdd(&LEVEL_LIST_HEAD, node);

    TraceLog(LOG_INFO, "Added enemy to level (x=%.1f, y=%.1f)",
                newEnemy->hitbox.x, newEnemy->hitbox.y);
}

void LevelEnemyCheckAndAdd(Vector2 pos) {    

    Rectangle hitbox = GetSpritesHitboxFromMiddle(EnemySprite, pos);

    ListNode *node = LEVEL_LIST_HEAD;

    while (node != 0) {
    
        LevelEntity *entity = (LevelEntity *) node->item;

        if (CheckCollisionRecs(hitbox, entity->hitbox)) {

            /*
                TODO: Click closer to the ground and still place the enemy.
                
                If the collision is with a block (create a block component btw),
                but the y of the block is below the mouse, change the hitbox
                to be right above the ground.

                It will need to check for collisions again, and be careful about
                not entering infinite loops.
            */

            TraceLog(LOG_DEBUG, "Couldn't add enemy to level, collision with entity on x=%.1f, y=%.1f.",
                entity->hitbox.x, entity->hitbox.y);
            return;
        }

        node = node->next;

    }

    /*
        LevelEnemyAdd places the enemy _above_ this point,
        so we bring the y down to the feet of the sprite.
        
        TODO: Make InitializeAbove receive normal x and y.
        The level positions the enemy to fall to the floor.
    */
    float feet = pos.y + ((EnemySprite.sprite.height * EnemySprite.scale) / 2);


    return LevelEnemyAdd(pos.x, feet);
}

void LevelEnemyTick(ListNode *enemyNode) {

    LevelEntity *enemy = (LevelEntity *)enemyNode->item;

    LevelEntity *groundBeneath = LevelGetGroundBeneath(enemy);

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

    int x_back = enemy->hitbox.x;
    if (enemy->isFacingRight) enemy->hitbox.x -= ENEMY_SPEED_DEFAULT;
    else enemy->hitbox.x += ENEMY_SPEED_DEFAULT;
    
    // TODO use groundBeneath -- currently it breaks the AI when it goes to the edge of the ground
    if (!LevelGetGroundBeneath(enemy)) {
        
        // Turn around
        enemy->hitbox.x = x_back;
        enemy->isFacingRight = !(enemy->isFacingRight);
    }


    // Enemy fell off level
    if  (enemy->hitbox.y > FLOOR_DEATH_HEIGHT) {
                
        LinkedListRemove(&LEVEL_LIST_HEAD, enemyNode);
        return;
    }
}
