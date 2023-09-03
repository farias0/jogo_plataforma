#include <raylib.h>

#include "enemy.h"
#include "entity.h"
#include "global.h"


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
    newEnemy->isFacingRight = false;

    AddToEntityList(listItem, newEnemy);

    return newEnemy;
}

void EnemyTick(Entity *enemy, Entity *player) {
    int x_back = enemy->hitbox.x;
    if (enemy->isFacingRight) enemy->hitbox.x -= ENEMY_SPEED_DEFAULT;
    else enemy->hitbox.x += ENEMY_SPEED_DEFAULT;
    if (!IsOnTheGround(enemy)) {
        enemy->hitbox.x = x_back;
        enemy->isFacingRight = !(enemy->isFacingRight);
    }
}

void DrawEnemy(Entity *enemy) {
    DrawTextureEx(enemy->sprite, (Vector2){enemy->hitbox.x, enemy->hitbox.y}, 0, ENEMY_SPRITE_SCALE, WHITE);
}