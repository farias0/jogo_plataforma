#include <raylib.h>
#include <stdio.h>

#include "entities/entity.h"
#include "global.h"

#define RAYGUI_IMPLEMENTATION
#include "../include/raygui.h"


void RenderBackground() {
    float floor = 530;
    // DrawRectangle(0, 0, SCREEN_WIDTH, floor, GRAY); //(Color){ 0x32, 0x29, 0x47, 0xFF }); //(Color){ 0xEB, 0x56, 0x4B, 0xFF });
    // DrawRectangle(0, floor, SCREEN_WIDTH, SCREEN_HEIGHT - floor, (Color){ 0x32, 0x29, 0x47, 0xFF }); 
}

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

void RenderEditor() {

    Rectangle editorWindow = { SCREEN_WIDTH, 5, EDITOR_BAR_WIDTH, SCREEN_HEIGHT };

    // Currently the color is transparent because it's fun,
    // but in the future for game design reasons it will have to be solid.
    Color backgroundColor = (Color){ 150, 150, 150, 40 };

    float buttonSize = 80;
    float buttonSpacing = 12;
    float buttonWallSpacing = (EDITOR_BAR_WIDTH - (buttonSize * 2) - buttonSpacing) / 2;


    DrawRectangle( editorWindow.x, editorWindow.y, editorWindow.width, editorWindow.height, backgroundColor );

    GuiGroupBox(editorWindow, "Editor");


    bool isItemSelected = false;

    isItemSelected = STATE->editorSelectedItem == Block;
    GuiToggle((Rectangle){ editorWindow.x + buttonWallSpacing,
                                        editorWindow.y + buttonWallSpacing,
                                        buttonSize,
                                        buttonSize }, "Bloco", &isItemSelected);
    EditorSetSelectedItem(Block, isItemSelected);

    isItemSelected = STATE->editorSelectedItem == Enemy;
    GuiToggle((Rectangle){ editorWindow.x + buttonWallSpacing + buttonSize + buttonSpacing,
                                        editorWindow.y + buttonWallSpacing,
                                        buttonSize,
                                        buttonSize }, "Inimigo", &isItemSelected);
    EditorSetSelectedItem(Enemy, isItemSelected);
}