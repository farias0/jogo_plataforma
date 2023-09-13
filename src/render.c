#include <raylib.h>
#include <stdio.h>

#include "entities/entity.h"
#include "global.h"


void RenderAllEntities() {

    Entity *currentItem = ENTITIES;

    do {
        float inSceneX = currentItem->hitbox.x - CAMERA->hitbox.x;
        float inSceneY = currentItem->hitbox.y - CAMERA->hitbox.y;

        if (currentItem->components & IsPlayer ||
            currentItem->components & IsEnemy)
            if (currentItem->isFacingRight)
                DrawTextureEx(currentItem->sprite, (Vector2){inSceneX, inSceneY}, 0, currentItem->spriteScale, WHITE);
            else {
                Rectangle source = (Rectangle){
                    0,
                    0,
                    -currentItem->sprite.width,
                    currentItem->sprite.height
                };
                Rectangle destination = (Rectangle){
                    inSceneX,
                    inSceneY,
                    currentItem->sprite.width * currentItem->spriteScale,
                    currentItem->sprite.height * currentItem->spriteScale
                };
                DrawTexturePro(currentItem->sprite, source, destination, (Vector2){ 0, 0 }, 0, WHITE);
            }

        else if (currentItem->components & IsLevelElement) {
            
            // Currently the only level element is a floor area to be tiled with a sprite

            // How many tiles to be drawn in each axis
            int xTilesCount = currentItem->hitbox.width / currentItem->spriteScale;
            int yTilesCount = currentItem->hitbox.height / currentItem->spriteScale;

            for (int xCurrent = 0; xCurrent < xTilesCount; xCurrent++) {
                for (int yCurrent = 0; yCurrent < yTilesCount; yCurrent++) {
                    DrawTextureEx(
                                    currentItem->sprite,
                                    (Vector2){inSceneX + (xCurrent * currentItem->spriteScale),
                                                inSceneY + (yCurrent * currentItem->spriteScale)},
                                    0,
                                    1,
                                    WHITE
                                );
                }
            }
        }

        currentItem = currentItem->next;
    } while (currentItem != ENTITIES);
}

void RenderHUD() {
    if (STATE->isPaused && !STATE->isPlayerDead) DrawText("PAUSE", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 30, RAYWHITE);
    if (STATE->isPlayerDead) DrawText("YOU DIED", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 60, RAYWHITE);


    // Debug
    char entity_count[50];
    sprintf(entity_count, "%d entities", CountEntities(ENTITIES));
    DrawText(entity_count, 10, 20, 20, WHITE);


    // Gamepad debug
    if (IsGamepadAvailable(STATE->gamepadIdx)) {
        char gpad[27];
        sprintf(gpad, "Gamepad index: %i", STATE->gamepadIdx);
        DrawText(gpad, 10, 900, 20, WHITE);
        DrawText(GetGamepadName(STATE->gamepadIdx), 10, 920, 20, WHITE);
    }

}