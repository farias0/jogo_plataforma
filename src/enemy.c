#include <raylib.h>

#include "enemy.h"
#include "entity.h"
#include "global.h"


#define ENEMY_SPRITE_SCALE 2

// sprite is 32x32 pixels
#define ENEMY_WIDTH (float)(ENEMY_SPRITE_SCALE * 32)
#define ENEMY_HEIGHT (float)(ENEMY_SPRITE_SCALE * 32)

#define ENEMY_SPAWN_X SCREEN_WIDTH * 3/4

#define ENEMY_SPEED_DEFAULT 8.0f


Entity *InitializeEnemy(Entity *listItem) { 
    Entity *newEnemy = MemAlloc(sizeof(Entity));

    newEnemy->components = HasPosition +
                            HasSprite +
                            DoesTick +
                            IsEnemy;
    newEnemy->hitbox = (Rectangle){ ENEMY_SPAWN_X, FLOOR_HEIGHT-ENEMY_HEIGHT, ENEMY_WIDTH, ENEMY_HEIGHT };
    // TODO load assets only once reference them from the sprite property. An asset manager, basically.
    newEnemy->sprite = LoadTexture("../assets/enemy_default_1.png");

    AddToEntityList(listItem, newEnemy);

    return newEnemy;
}

void EnemyTick(Entity *enemy, Entity *player) {
    enemy->hitbox.x -= ENEMY_SPEED_DEFAULT;
}

void DrawEnemy(Entity *enemy) {
    DrawTextureEx(enemy->sprite, (Vector2){enemy->hitbox.x, enemy->hitbox.y}, 0, ENEMY_SPRITE_SCALE, WHITE);
}