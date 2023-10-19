#include <raylib.h>
#include <stdio.h>

#include "entities/entity.h"
#include "global.h"
#include "assets.h"
#include "input.h"
#include "entities/level.h"
#include "overworld.h"
#include "entities/camera.h"

#define RAYGUI_IMPLEMENTATION
#include "../include/raygui.h"


#define FIRST_LAYER 0
#define LAST_LAYER  1

#define EDITOR_BUTTON_SIZE 80
#define EDITOR_BUTTON_SPACING 12
#define EDITOR_BUTTON_WALL_SPACING (EDITOR_BAR_WIDTH - (EDITOR_BUTTON_SIZE * 2) - EDITOR_BUTTON_SPACING) / 2


// How many editor buttons were rendered this frame.
int editorButtonsRendered = 0;

void drawTexture(Sprite sprite, Vector2 pos, Color tint, bool flipHorizontally) {

    SpriteDimensions dimensions = GetScaledDimensions(sprite);


    // Raylib's draw function rotates the sprite around the origin, instead of its middle point.
    // Maybe this should be fixed in a way that works for any angle. 
    if (sprite.rotation == 90)          pos = (Vector2){ pos.x + dimensions.width, pos.y };
    else if (sprite.rotation == 180)    pos = (Vector2){ pos.x + dimensions.width,
                                                            pos.y + dimensions.height };
    else if (sprite.rotation == 270)    pos = (Vector2){ pos.x, pos.y + dimensions.height };



    if (!flipHorizontally) {
        DrawTextureEx(sprite.sprite,
                    pos,
                    sprite.rotation,
                    sprite.scale,
                    WHITE);    
        
        return;
    }

    Rectangle source = (Rectangle){
        0,
        0,
        -sprite.sprite.width,
        sprite.sprite.height
    };

    Rectangle destination = (Rectangle){
        pos.x,
        pos.y,
        dimensions.width,
        dimensions.height
    };

    DrawTexturePro(sprite.sprite,
                    source,
                    destination,
                    (Vector2){ 0, 0 },
                    sprite.rotation,
                    WHITE);    
}

// Draws sprite in the background, with effects applied.
void drawInBackground(Sprite sprite, Vector2 pos, int layer) {

    float scale = 1;
    Color tint = (Color){ 0xFF, 0xFF, 0xFF, 0xFF };
    float parallaxSpeed = 1;

    switch (layer) {

    case -1:
        scale = 0.7;
        tint = (Color){ 0xFF, 0xFF, 0xFF, 0x88 };
        parallaxSpeed = 0.4;
        break;

    case -2:
        scale = 0.3;
        tint = (Color){ 0xFF, 0xFF, 0xFF, 0x44 };
        parallaxSpeed = 0.25;
        break;

    default:
        TraceLog(LOG_ERROR, "No code found for drawing in the bg layer %d.", layer);
        return;
    }

    pos.x = pos.x * scale;
    pos.y = pos.y * scale;

    pos.x = pos.x - (CAMERA->hitbox.x * parallaxSpeed);
    pos.y = pos.y - (CAMERA->hitbox.y * parallaxSpeed);

    DrawTextureEx(sprite.sprite, pos, 0, (scale * sprite.scale), tint);
}

void renderBackground() {

    if (STATE->mode == Overworld) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 39, 39, 54, 255 }); 
    }

    else if (STATE->mode == InLevel) {
        if (!STATE->showBackground) return; 

        drawInBackground(NightclubSprite,   (Vector2){ 1250, 250 },  -1);
        drawInBackground(BGHouseSprite,     (Vector2){ 600, 300 },  -2);
    }
}

void renderEntitiesInLayer(int layer) {

    Entity *currentItem = ENTITIES_HEAD;

    while (currentItem != 0) {
        Vector2 pos = PosInSceneToScreen((Vector2){ currentItem->hitbox.x, currentItem->hitbox.y });

        if (currentItem->sprite.scale == 0 ||
            currentItem->layer != layer) {
                
                goto next_entity;
            }


        // Currently the only level element is a floor area to be tiled with a sprite
        bool isLevelBlock = (currentItem->components & IsLevelElement) &&
                            !(currentItem->components & IsEnemy);
        if (isLevelBlock) {
            // How many tiles to be drawn in each axis
            int xTilesCount = currentItem->hitbox.width / currentItem->sprite.sprite.width;
            int yTilesCount = currentItem->hitbox.height / currentItem->sprite.sprite.width;

            for (int xCurrent = 0; xCurrent < xTilesCount; xCurrent++) {
                for (int yCurrent = 0; yCurrent < yTilesCount; yCurrent++) {
                    DrawTextureEx(
                                    currentItem->sprite.sprite,
                                    (Vector2){pos.x + (xCurrent * currentItem->sprite.sprite.width),
                                                pos.y + (yCurrent * currentItem->sprite.sprite.height)},
                                    0,
                                    1,
                                    WHITE
                                );
                }
            }

            goto next_entity;
        }


        drawTexture(currentItem->sprite, (Vector2){ pos.x, pos.y }, WHITE, !currentItem->isFacingRight);

next_entity:
        currentItem = currentItem->next;
    }
}

void renderAllEntities() {

    for (int layer = FIRST_LAYER; layer <= LAST_LAYER; layer++) {
        
        renderEntitiesInLayer(layer);
    }
}


void renderHUD() {
    if (STATE->isPaused && !STATE->isPlayerDead) DrawText("PAUSE", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 30, RAYWHITE);
    if (STATE->isPlayerDead) DrawText("YOU DIED", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 60, RAYWHITE);


    if (STATE->showDebugGrid) {
        SpriteDimensions gridSquareDim;
        if (STATE->mode == Overworld) gridSquareDim = OverworldGridDimensions;
        else if (STATE->mode == InLevel) gridSquareDim = LevelGridDimensions;
        else {
            // maybe write something to the screen?
            goto skip_debug_grid;
        }

        Vector2 offset = (Vector2){
            PushOnGrid(CAMERA->hitbox.x, gridSquareDim.width),
            PushOnGrid(CAMERA->hitbox.y, gridSquareDim.height),
        };

        for (float lineX = offset.x; lineX <= SCREEN_WIDTH; lineX += gridSquareDim.width) {
            DrawLine(lineX, 0, lineX, SCREEN_HEIGHT, BLUE);
        }
        for (float lineY = offset.y; lineY <= SCREEN_HEIGHT; lineY += gridSquareDim.height) {
            DrawLine(0, lineY, SCREEN_WIDTH, lineY, BLUE);
        }

skip_debug_grid:
    }

    if (STATE->showDebugHUD) {
        char entity_count[50];
        sprintf(entity_count, "%d entities", CountEntities(ENTITIES_HEAD));
        DrawText(entity_count, 10, 20, 20, WHITE);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            char mousePosTxt[50];
            sprintf(mousePosTxt, "Left click: x=%.0f, y=%.0f", mousePos.x, mousePos.y);
            DrawText(mousePosTxt, 600, 20, 20, WHITE);
        }
    }
}

void renderButton(Rectangle editorWindow, EditorItem *item) {
    
    float itemX = editorWindow.x + EDITOR_BUTTON_WALL_SPACING;
    if (editorButtonsRendered % 2) itemX += EDITOR_BUTTON_SIZE + EDITOR_BUTTON_SPACING;

    float itemY = editorWindow.y + EDITOR_BUTTON_WALL_SPACING;
    itemY += (EDITOR_BUTTON_SIZE + EDITOR_BUTTON_SPACING) * (editorButtonsRendered / 2);
    
    bool isItemSelected = STATE->editorSelectedItem == item;

    GuiToggleSprite(
        (Rectangle){ itemX, itemY, EDITOR_BUTTON_SIZE, EDITOR_BUTTON_SIZE },
        item->sprite, 
        (Vector2){itemX, itemY},
        &isItemSelected
    );
    
    if (isItemSelected) ClickOnEditorItem(item);

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
    EditorItem *currentItem = EDITOR_ITEMS_HEAD;
    while (currentItem != 0) {
        renderButton(editorWindow, currentItem);
        currentItem = currentItem->next;
    }
}

void Render() {
    ClearBackground(BLACK);

    renderBackground();

    renderAllEntities();

    renderEditor();

    renderHUD();
}