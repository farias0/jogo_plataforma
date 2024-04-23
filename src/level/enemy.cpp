#include <raylib.h>

#include "enemy.hpp"
#include "level.hpp"
#include "../debug.hpp"


#define ENEMY_SPEED_DEFAULT 4.0f
#define ENEMY_FALL_RATE 7.0f


void EnemyAdd(Vector2 origin) {

    Level::Entity *newEnemy = new Level::Entity();

    newEnemy->tags = Level::IS_ENEMY +
                            Level::IS_GROUND +
                            Level::IS_DANGER;
    newEnemy->origin = origin;
    newEnemy->sprite = &SPRITES->Enemy;
    newEnemy->hitbox = SpriteHitboxFromEdge(newEnemy->sprite, newEnemy->origin);
    newEnemy->isFacingRight = true;
    newEnemy->isFallingDown = true;

    LinkedList::AddNode(&Level::STATE->listHead, newEnemy);

    TraceLog(LOG_TRACE, "Added enemy to level (x=%.1f, y=%.1f)",
                newEnemy->hitbox.x, newEnemy->hitbox.y);
}

void EnemyCheckAndAdd(Vector2 origin) {    

    Rectangle hitbox = SpriteHitboxFromMiddle(&SPRITES->Enemy, origin);

    if (Level::CheckCollisionWithAnything(hitbox)) {
        TraceLog(LOG_DEBUG, "Couldn't add enemy to level, collision with entity.");
        return;
    }

    EnemyAdd({ hitbox.x, hitbox.y });
}

void EnemyTick(Level::Entity *enemy) {

    if (Level::STATE->concludedAgo >= 0) return;

    if (enemy->isDead) return;


    Level::Entity *groundBeneath = Level::GetGroundBeneath(enemy);

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

    LinkedList::Node *node = Level::STATE->listHead;
    while (node) {

        Level::Entity *entity = (Level::Entity *) node;

        if (entity == enemy) goto next_node;

        if (entity->tags & Level::IS_SCENARIO &&
            CheckCollisionRecs(entity->hitbox, enemy->hitbox)) {

                enemy->isFacingRight = !enemy->isFacingRight;

                return;
        }

next_node:
        node = node->next;
    }
}

void EnemyKill(Level::Entity *enemy) {

    enemy->isDead = true;

    DebugEntityStop(enemy);

    TraceLog(LOG_TRACE, "Enemy died.");
}
