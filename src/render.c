#include <raylib.h>
#include <stdio.h>

#include "global.h"
#include "assets.h"
#include "input.h"
#include "inlevel/level.h"
#include "overworld.h"
#include "camera.h"

#define RAYGUI_IMPLEMENTATION
#include "../include/raygui.h"


#define FIRST_LAYER 0
#define LAST_LAYER  1

#define EDITOR_BUTTON_SIZE 80
#define EDITOR_BUTTON_SPACING 12
#define EDITOR_BUTTON_WALL_SPACING (EDITOR_BAR_WIDTH - (EDITOR_BUTTON_SIZE * 2) - EDITOR_BUTTON_SPACING) / 2


// How many editor buttons were rendered this frame.
int editorButtonsRendered = 0;

static void drawTexture(Sprite sprite, Vector2 pos, Color tint, bool flipHorizontally) {

    Dimensions dimensions = GetScaledDimensions(sprite);


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
static void drawInBackground(Sprite sprite, Vector2 pos, int layer) {

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

    pos.x = pos.x - (CAMERA->pos.x * parallaxSpeed);
    pos.y = pos.y - (CAMERA->pos.y * parallaxSpeed);

    DrawTextureEx(sprite.sprite, pos, 0, (scale * sprite.scale), tint);
}

static void renderBackground() {

    if (STATE->mode == MODE_OVERWORLD) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 39, 39, 54, 255 }); 
    }

    else if (STATE->mode == MODE_IN_LEVEL) {
        if (!STATE->showBackground) return; 

        drawInBackground(NightclubSprite,   (Vector2){ 1250, 250 },  -1);
        drawInBackground(BGHouseSprite,     (Vector2){ 600, 300 },  -2);
    }
}

static void renderOverworldEntity(OverworldEntity *entity) {

    Vector2 pos = PosInSceneToScreen((Vector2){
                                        entity->gridPos.x,
                                        entity->gridPos.y });

    drawTexture(entity->sprite, (Vector2){ pos.x, pos.y }, WHITE, false);
}

static void renderLevelEntity(LevelEntity *entity) {

    Vector2 pos = PosInSceneToScreen((Vector2){
                                        entity->hitbox.x,
                                        entity->hitbox.y });

    if (entity->components & LEVEL_IS_SCENARIO) {
        // How many tiles to be drawn in each axis
        int xTilesCount = entity->hitbox.width / entity->sprite.sprite.width;
        int yTilesCount = entity->hitbox.height / entity->sprite.sprite.width;

        for (int xCurrent = 0; xCurrent < xTilesCount; xCurrent++) {
            for (int yCurrent = 0; yCurrent < yTilesCount; yCurrent++) {
                DrawTextureEx(
                                entity->sprite.sprite,
                                (Vector2){pos.x + (xCurrent * entity->sprite.sprite.width),
                                            pos.y + (yCurrent * entity->sprite.sprite.height)},
                                0,
                                1,
                                WHITE
                            );
            }
        }

        return;
    }

    drawTexture(entity->sprite, (Vector2){ pos.x, pos.y }, WHITE, !entity->isFacingRight);
}

static void renderEntities() {

    for (int layer = FIRST_LAYER; layer <= LAST_LAYER; layer++) {

        ListNode *node = GetListHead();

        while (node != 0) {

            if (STATE->mode == MODE_IN_LEVEL) {
                LevelEntity *entity = (LevelEntity *) node->item;
                if (entity->layer != layer) goto next_entity;
                renderLevelEntity(entity);
            }

            else if (STATE->mode == MODE_OVERWORLD) {
                OverworldEntity *entity = (OverworldEntity *) node->item;
                if (entity->layer != layer) goto next_entity;
                renderOverworldEntity((OverworldEntity *) node->item);
            }

            else return;

next_entity:
            node = node->next;
        }
    }
}


static void renderHUD() {
    if (STATE->isPaused && !STATE->isPlayerDead) DrawText("PAUSE", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 30, RAYWHITE);
    if (STATE->isPlayerDead) DrawText("YOU DIED", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 60, RAYWHITE);


    if (STATE->showDebugGrid) {
        Dimensions gridSquareDim;
        if (STATE->mode == MODE_OVERWORLD) gridSquareDim = OW_GRID;
        else if (STATE->mode == MODE_IN_LEVEL) gridSquareDim = LEVEL_GRID;
        else {
            // maybe write something to the screen?
            goto skip_debug_grid;
        }

        Vector2 offset = (Vector2){
            PushOnGrid(CAMERA->pos.x, gridSquareDim.width),
            PushOnGrid(CAMERA->pos.y, gridSquareDim.height),
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

        ListNode *listHead = GetListHead();
        if (listHead) {
            char entity_count[50];
            sprintf(entity_count, "%d entities", LinkedListCountNodes(listHead));
            DrawText(entity_count, 10, 20, 20, WHITE);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            char mousePosTxt[50];
            sprintf(mousePosTxt, "Left click: x=%.0f, y=%.0f", mousePos.x, mousePos.y);
            DrawText(mousePosTxt, 600, 20, 20, WHITE);
        }
    }
}

static void renderButton(Rectangle editorWindow, EditorItem *item) {
    
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

static void renderEditor() {

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

    renderEntities();

    renderEditor();

    renderHUD();
}
