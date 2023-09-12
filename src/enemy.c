#include <raylib.h>

#include "enemy.h"
#include "entity.h"
#include "global.h"
#include "level.h"


#define ENEMY_SPRITE_SCALE 2

// sprite is 32x32 pixels
#define ENEMY_WIDTH (float)(ENEMY_SPRITE_SCALE * 32)
#define ENEMY_HEIGHT (float)(ENEMY_SPRITE_SCALE * 32)

#define ENEMY_SPEED_DEFAULT 4.0f


Entity *InitializeEnemy(Entity *listItem, int x, int y) { 
    Entity *newEnemy = MemAlloc(sizeof(Entity));

    newEnemy->components = HasPosition +
                            HasSprite +
                            DoesTick +
                            IsEnemy +
                            IsLevelElement;
    newEnemy->hitbox = (Rectangle){ x - (ENEMY_WIDTH/2), y - ENEMY_HEIGHT, ENEMY_WIDTH, ENEMY_HEIGHT };
    // TODO load assets only once reference them from the sprite property. An asset manager, basically.
    newEnemy->sprite = LoadTexture("../assets/enemy_default_1.png");
    newEnemy->spriteScale = ENEMY_SPRITE_SCALE;
    newEnemy->isFacingRight = false;

    AddToEntityList(listItem, newEnemy);

    return newEnemy;
}

void EnemyTick(Entity *enemy, Entity *player) {
    int x_back = enemy->hitbox.x;
    
    if (enemy->isFacingRight) enemy->hitbox.x -= ENEMY_SPEED_DEFAULT;
    else enemy->hitbox.x += ENEMY_SPEED_DEFAULT;
    
    if (GetEntitiesGroundBeneath(enemy) == -1) {
        enemy->hitbox.x = x_back;
        enemy->isFacingRight = !(enemy->isFacingRight);
    }
}

bool AddEnemyToLevel(Entity *listItem, Vector2 pos) {
    for (Entity *currentItem = listItem->next; currentItem != listItem; currentItem = currentItem->next) {
        if ((currentItem->components & IsLevelElement ||
                currentItem->components & IsEnemy) &&
            CheckCollisionPointRec(pos, currentItem->hitbox)) return true;
    }

    InitializeEnemy(listItem, pos.x, pos.y);
    return true;
}
