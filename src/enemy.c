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
    newEnemy->hitbox = (Rectangle){ x - (ENEMY_WIDTH/2), y - ENEMY_HEIGHT + 1, ENEMY_WIDTH, ENEMY_HEIGHT };
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

    Rectangle hitbox = {
        // Considering the player is clicking in the middle of the sprite
        pos.x - (ENEMY_WIDTH / 2), pos.y - (ENEMY_HEIGHT / 2),
        ENEMY_WIDTH, ENEMY_HEIGHT
    };

    for (Entity *currentItem = listItem->next; currentItem != listItem; currentItem = currentItem->next) {
        
        if ((currentItem->components & IsLevelElement || currentItem->components & IsEnemy) &&
                CheckCollisionRecs(hitbox, currentItem->hitbox)) {

                return false;
            }
    }

    /*
        InitializeEnemy places the enemy _above_ this point,
        so we bring the y down to the feet of the sprite.
        
        TODO: Clarify this through different functions,
        like InitializeEnemyAbove() 
    */
    float feet = pos.y + (ENEMY_HEIGHT / 2);

    InitializeEnemy(listItem, pos.x, feet);
    return true;
}
