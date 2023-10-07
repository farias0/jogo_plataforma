#include <raylib.h>
#include <stdio.h>

#include "entities/entity.h"
#include "global.h"
#include "assets.h"

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
                DrawTextureEx(currentItem->sprite.sprite, (Vector2){inSceneX, inSceneY}, 0, currentItem->sprite.scale, WHITE);
            else {
                Rectangle source = (Rectangle){
                    0,
                    0,
                    -currentItem->sprite.sprite.width,
                    currentItem->sprite.sprite.height
                };
                Rectangle destination = (Rectangle){
                    inSceneX,
                    inSceneY,
                    currentItem->sprite.sprite.width * currentItem->sprite.scale,
                    currentItem->sprite.sprite.height * currentItem->sprite.scale
                };
                DrawTexturePro(currentItem->sprite.sprite, source, destination, (Vector2){ 0, 0 }, 0, WHITE);
            }

        else if (currentItem->components & IsLevelElement) {
            
            // Currently the only level element is a floor area to be tiled with a sprite

            // How many tiles to be drawn in each axis
            int xTilesCount = currentItem->hitbox.width / currentItem->sprite.sprite.width;
            int yTilesCount = currentItem->hitbox.height / currentItem->sprite.sprite.width;

            for (int xCurrent = 0; xCurrent < xTilesCount; xCurrent++) {
                for (int yCurrent = 0; yCurrent < yTilesCount; yCurrent++) {
                    DrawTextureEx(
                                    currentItem->sprite.sprite,
                                    (Vector2){inSceneX + (xCurrent * currentItem->sprite.sprite.width),
                                                inSceneY + (yCurrent * currentItem->sprite.sprite.height)},
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

    // Debug
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        char mousePosTxt[50];
        sprintf(mousePosTxt, "Left click: x=%.0f, y=%.0f", mousePos.x, mousePos.y);
        DrawText(mousePosTxt, 600, 20, 20, WHITE);
    }

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

    DrawRectangle( editorWindow.x, editorWindow.y, editorWindow.width, editorWindow.height, backgroundColor );
    GuiGroupBox(editorWindow, "Editor");


    float buttonSize = 80;
    float buttonSpacing = 12;
    float buttonWallSpacing = (EDITOR_BAR_WIDTH - (buttonSize * 2) - buttonSpacing) / 2;

    bool isItemSelected = false;
    float itemX;
    float itemY;

    isItemSelected = STATE->editorSelectedItem == Block;
    itemX = editorWindow.x + buttonWallSpacing;
    itemY = editorWindow.y + buttonWallSpacing;
    GuiToggleSprite((Rectangle){ itemX, itemY, buttonSize, buttonSize }, BlockSprite, (Vector2){itemX, itemY}, &isItemSelected);
    //GuiToggle((Rectangle){ itemX, itemY, buttonSize, buttonSize }, "Bloco", &isItemSelected);
    EditorSetSelectedItem(Block, isItemSelected);

    isItemSelected = STATE->editorSelectedItem == Enemy;
    itemX = editorWindow.x + buttonWallSpacing + buttonSize + buttonSpacing;
    itemY = editorWindow.y + buttonWallSpacing;
    GuiToggleSprite((Rectangle){ itemX, itemY, buttonSize, buttonSize }, EnemySprite, (Vector2){itemX, itemY}, &isItemSelected);
    EditorSetSelectedItem(Enemy, isItemSelected);

    isItemSelected = STATE->editorSelectedItem == Eraser;
    itemX = editorWindow.x + buttonWallSpacing;
    itemY = editorWindow.y + buttonWallSpacing + buttonSize + buttonSpacing;
    //GuiToggleSprite((Rectangle){ itemX, itemY, buttonSize, buttonSize }, BlockSprite, (Vector2){itemX, itemY}, &isItemSelected);
    GuiToggle((Rectangle){ itemX, itemY, buttonSize, buttonSize }, "Borracha", &isItemSelected);
    EditorSetSelectedItem(Eraser, isItemSelected);
}