#include <raylib.h>

#include "enemy.hpp"
#include "level.hpp"
#include "moving_platform.hpp"
#include "../debug.hpp"
#include "../editor.hpp"


#define ENEMY_SPEED_DEFAULT 4.0f
#define ENEMY_FALL_RATE 7.0f


Enemy *Enemy::Add() {
    return Add({ 0,0 });
}

Enemy *Enemy::Add(Vector2 origin) {

    Enemy *newEnemy = new Enemy();

    newEnemy->tags = Level::IS_ENEMY +
                            Level::IS_GROUND +
                            Level::IS_COLLIDE_DANGER +
                            Level::IS_PERSISTABLE;
    newEnemy->origin = origin;
    newEnemy->sprite = &SPRITES->Enemy;
    newEnemy->hitbox = SpriteHitboxFromEdge(newEnemy->sprite, newEnemy->origin);
    newEnemy->isFacingRight = true;
    newEnemy->isFallingDown = true;

    LinkedList::AddNode(&Level::STATE->listHead, newEnemy);

    TraceLog(LOG_TRACE, "Added enemy to level (x=%.1f, y=%.1f)",
                newEnemy->hitbox.x, newEnemy->hitbox.y);

    return newEnemy;
}

void Enemy::CheckAndAdd(Vector2 origin) {    

    Rectangle hitbox = SpriteHitboxFromMiddle(&SPRITES->Enemy, origin);

    if (Level::CheckCollisionWithAnything(hitbox)) {
        TraceLog(LOG_DEBUG, "Couldn't add enemy to level, collision with entity.");
        return;
    }

    Add({ hitbox.x, hitbox.y });
}

void Enemy::Tick() {

    if (Level::STATE->concludedAgo >= 0) return;

    if (isDead) return;


    Level::Entity *groundBeneath = Level::GetGroundBeneath(this);

    if (!groundBeneath) isFallingDown = true;

    if (isFallingDown) {
        
        if (groundBeneath &&
            hitbox.y + hitbox.height + ENEMY_FALL_RATE >= groundBeneath->hitbox.y) {

            // Land
            hitbox.y = groundBeneath->hitbox.y - hitbox.height;
            isFallingDown = false;
        }

        else {

            // Fall
            hitbox.y += ENEMY_FALL_RATE;
        }

        if  (hitbox.y + hitbox.height > FLOOR_DEATH_HEIGHT) {

            Kill();
            return;
        }

        return;
    }

    if (!groundBeneath || 
            (isFacingRight &&
                ((hitbox.x + hitbox.width) > (groundBeneath->hitbox.x + groundBeneath->hitbox.width))) ||
            (!isFacingRight && (hitbox.x < groundBeneath->hitbox.x))
    ) {
        
        // Turn around
        isFacingRight = !(isFacingRight);
    }

    if (isFacingRight) hitbox.x += ENEMY_SPEED_DEFAULT;
    else hitbox.x -= ENEMY_SPEED_DEFAULT;

    // Moving platform
    if (groundBeneath && groundBeneath->tags & Level::IS_MOVING_PLATFORM) {
        Vector2 trajectory = ((MovingPlatform *)groundBeneath)->lastFrameTrajectory;
        SetHitboxPos({
            hitbox.x += trajectory.x,
            hitbox.y += trajectory.y
        }); 
    }


    LinkedList::Node *node = Level::STATE->listHead;
    while (node) {

        Level::Entity *entity = (Level::Entity *) node;

        if (entity == this) goto next_node;

        if (entity->tags & Level::IS_COLLIDE_WALL &&
            CheckCollisionRecs(entity->hitbox, hitbox)) {

                isFacingRight = !isFacingRight;

                return;
        }

next_node:
        node = node->next;
    }
}

void Enemy::Kill() {

    isDead = true;

    DebugEntityStop(this);

    TraceLog(LOG_TRACE, "Enemy died.");
}

void Enemy::Draw() {

    if (!isDead)
        Render::DrawLevelEntity(this);

    if (EDITOR_STATE->isEnabled)
        Render::DrawLevelEntityOriginGhost(this);
}
