#include <raylib.h>
#include <stdio.h>

#include "core.h"
#include "assets.h"
#include "input.h"
#include "level/level.h"
#include "overworld.h"
#include "camera.h"

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wunused-parameter" // Ignore warnings
#define RAYGUI_IMPLEMENTATION
#include "../include/raygui.h"
#pragma GCC diagnostic pop


#define FIRST_LAYER 0
#define LAST_LAYER  1


static void drawTexture(Sprite sprite, Vector2 pos, Color tint, bool flipHorizontally) {

    Dimensions dimensions = SpriteScaledDimensions(sprite);


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
                    tint);    
        
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
                    tint);    
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

        ListNode *node = GetEntityListHead();

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

        ListNode *listHead = GetEntityListHead();
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

static void renderEditorEntities() {

    DrawRectangle(EDITOR_ENTITIES_AREA.x,
                    EDITOR_ENTITIES_AREA.y,
                    EDITOR_ENTITIES_AREA.width,
                    EDITOR_ENTITIES_AREA.height,
                    EDITOR_BG_COLOR);

    int editorButtonsRendered = 0;
    ListNode *node = EDITOR_ENTITIES_HEAD;

    while (node != 0) {

        EditorEntityItem *item = (EditorEntityItem *) node->item;

        bool isItemSelected = STATE->editorSelectedItem == item;

        Rectangle buttonRect = EditorEntityButtonRect(editorButtonsRendered);

        GuiToggleSprite(
            buttonRect,
            item->sprite, 
            (Vector2){ buttonRect.x, buttonRect.y },
            &isItemSelected
        );

        if (isItemSelected) InputEditorEntitySelect(item);

        editorButtonsRendered++;

        node = node->next;
    }
}

static void renderEditorControl() {

    DrawRectangle(EDITOR_CONTROL_AREA.x,
                    EDITOR_CONTROL_AREA.y,
                    EDITOR_CONTROL_AREA.width,
                    EDITOR_CONTROL_AREA.height,
                    EDITOR_BG_COLOR);

    int editorButtonsRendered = 0;
    ListNode *node = EDITOR_CONTROL_HEAD;

    while (node != 0) {

        EditorControlItem *item = (EditorControlItem *) node->item;

        Rectangle buttonRect = EditorControlButtonRect(editorButtonsRendered);

        GuiButton(buttonRect, item->label);

        editorButtonsRendered++;

        node = node->next;
    }
}

static void renderEditor() {

    DrawLine(SCREEN_WIDTH,
                0,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                RAYWHITE);

    renderEditorEntities();

    DrawLine(SCREEN_WIDTH,
                EDITOR_ENTITIES_AREA.height,
                SCREEN_WIDTH + EDITOR_BAR_WIDTH,
                EDITOR_ENTITIES_AREA.height,
                RAYWHITE); // Separator

    renderEditorControl();
}

void Render() {
    ClearBackground(BLACK);

    renderBackground();

    renderEntities();

    if (STATE->isEditorEnabled)
        renderEditor();

    renderHUD();
}
