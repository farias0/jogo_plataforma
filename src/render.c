#include <raylib.h>
#include <stdio.h>

#include "entities/entity.h"
#include "global.h"
#include "assets.h"

#define RAYGUI_IMPLEMENTATION
#include "../include/raygui.h"


#define EDITOR_BUTTON_SIZE 80
#define EDITOR_BUTTON_SPACING 12
#define EDITOR_BUTTON_WALL_SPACING (EDITOR_BAR_WIDTH - (EDITOR_BUTTON_SIZE * 2) - EDITOR_BUTTON_SPACING) / 2


// How many editor buttons were rendered this frame.
int editorButtonsRendered = 0;


void renderBackground() {
    float floor = 530;
    // DrawRectangle(0, 0, SCREEN_WIDTH, floor, GRAY); //(Color){ 0x32, 0x29, 0x47, 0xFF }); //(Color){ 0xEB, 0x56, 0x4B, 0xFF });
    // DrawRectangle(0, floor, SCREEN_WIDTH, SCREEN_HEIGHT - floor, (Color){ 0x32, 0x29, 0x47, 0xFF }); 
}

void renderAllEntities() {

    Entity *currentItem = ENTITIES_HEAD;

    while (currentItem != 0) {
        float inSceneX = currentItem->hitbox.x - CAMERA->hitbox.x;
        float inSceneY = currentItem->hitbox.y - CAMERA->hitbox.y;

        // TODO Restructure this 

        if (currentItem->components & IsPlayer ||
            currentItem->components & IsEnemy ||
            currentItem->components & IsOverworldElement)
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
    }
}


void renderHUD() {
    if (STATE->isPaused && !STATE->isPlayerDead) DrawText("PAUSE", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 30, RAYWHITE);
    if (STATE->isPlayerDead) DrawText("YOU DIED", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 60, RAYWHITE);


    // Debug
    char entity_count[50];
    sprintf(entity_count, "%d entities", CountEntities(ENTITIES_HEAD));
    DrawText(entity_count, 10, 20, 20, WHITE);

    // Debug
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        char mousePosTxt[50];
        sprintf(mousePosTxt, "Left click: x=%.0f, y=%.0f", mousePos.x, mousePos.y);
        DrawText(mousePosTxt, 600, 20, 20, WHITE);
    }
}

void renderButton(Rectangle editorWindow, EditorItem item, Sprite sprite) {
    
    float itemX = editorWindow.x + EDITOR_BUTTON_WALL_SPACING;
    if (editorButtonsRendered % 2) itemX += EDITOR_BUTTON_SIZE + EDITOR_BUTTON_SPACING;

    float itemY = editorWindow.y + EDITOR_BUTTON_WALL_SPACING;
    itemY += (EDITOR_BUTTON_SIZE + EDITOR_BUTTON_SPACING) * (editorButtonsRendered / 2);
    
    bool isItemSelected = STATE->editorSelectedItem == item;
    GuiToggleSprite((Rectangle){ itemX, itemY, EDITOR_BUTTON_SIZE, EDITOR_BUTTON_SIZE }, sprite, (Vector2){itemX, itemY}, &isItemSelected);
    EditorSetSelectedItem(item, isItemSelected);

    editorButtonsRendered++;
}

void renderEditor() {

    Rectangle editorWindow = { SCREEN_WIDTH, 5, EDITOR_BAR_WIDTH, SCREEN_HEIGHT };
    // Currently the color is transparent because it's fun,
    // but in the future for game design reasons it will have to be solid.
    Color backgroundColor = (Color){ 150, 150, 150, 40 };

    DrawRectangle( editorWindow.x, editorWindow.y, editorWindow.width, editorWindow.height, backgroundColor );
    GuiGroupBox(editorWindow, "Editor");

    editorButtonsRendered = 0;
    renderButton(editorWindow, Eraser, EraserSprite);
    renderButton(editorWindow, Block, BlockSprite);
    renderButton(editorWindow, Enemy, EnemySprite);
}

void Render() {
    ClearBackground(BLACK);

    renderBackground();

    renderAllEntities();

    renderEditor();

    renderHUD();
}