#include <raylib.h>

#include "enemy.h"
#include "level.h"
#include "../render.h"

#define ENEMY_SPEED_DEFAULT 4.0f
#define ENEMY_FALL_RATE 7.0f


void EnemyAdd(Vector2 origin) {

    LevelEntity *newEnemy = MemAlloc(sizeof(LevelEntity));

    newEnemy->components = LEVEL_IS_ENEMY +
                            LEVEL_IS_GROUND +
                            LEVEL_IS_DANGER;
    newEnemy->origin = origin;
    newEnemy->hitbox = SpriteHitboxFromEdge(EnemySprite, origin);
    newEnemy->sprite = EnemySprite;
    newEnemy->isFacingRight = true;
    newEnemy->isFallingDown = true;

    LinkedListAdd(&LEVEL_STATE->listHead, newEnemy);

    TraceLog(LOG_TRACE, "Added enemy to level (x=%.1f, y=%.1f)",
                newEnemy->hitbox.x, newEnemy->hitbox.y);
}

void EnemyCheckAndAdd(Vector2 origin) {    

    Rectangle hitbox = SpriteHitboxFromMiddle(EnemySprite, origin);

    if (LevelCheckCollisionWithAnything(hitbox)) {
        TraceLog(LOG_DEBUG, "Couldn't add enemy to level, collision with entity.");
        return;
    }

    EnemyAdd((Vector2){ hitbox.x, hitbox.y });
}

void EnemyTick(ListNode *enemyNode) {

    if (LEVEL_STATE->concludedAgo >= 0) return;

    LevelEntity *enemy = (LevelEntity *)enemyNode->item;
    if (enemy->isDead) return;


    LevelEntity *groundBeneath = LevelGetGroundBeneath(enemy);

    if (!groundBeneath) enemy->isFallingDown = true;

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

        if  (enemy->hitbox.y + enemy->hitbox.height > FLOOR_DEATH_HEIGHT) {

            EnemyKill(enemy);
            return;
        }

        return;
    }

    if (!groundBeneath || 
            (enemy->isFacingRight &&
                ((enemy->hitbox.x + enemy->hitbox.width) > (groundBeneath->hitbox.x + groundBeneath->hitbox.width))) ||
            (!enemy->isFacingRight && (enemy->hitbox.x < groundBeneath->hitbox.x))
    ) {
        
        // Turn around
        enemy->isFacingRight = !(enemy->isFacingRight);
    }

    if (enemy->isFacingRight) enemy->hitbox.x += ENEMY_SPEED_DEFAULT;
    else enemy->hitbox.x -= ENEMY_SPEED_DEFAULT;

    ListNode *node = LEVEL_STATE->listHead;
    while (node) {

        LevelEntity *entity = (LevelEntity *) node->item;

        if (entity == enemy) goto next_node;

        if (entity->components & LEVEL_IS_SCENARIO &&
            CheckCollisionRecs(entity->hitbox, enemy->hitbox)) {

                enemy->isFacingRight = !enemy->isFacingRight;

                return;
        }

next_node:
        node = node->next;
    }
}

void EnemyKill(LevelEntity *entity) {

    entity->isDead = true;

    RenderDebugEntityStop(entity);

    TraceLog(LOG_TRACE, "Enemy died.");
}
