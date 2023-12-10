#include <raylib.h>
#include <stdio.h>

#include "core.hpp"
#include "assets.hpp"
#include "level/level.hpp"
#include "level/player.hpp"
#include "overworld.hpp"
#include "camera.hpp"
#include "editor.hpp"
#include "debug.hpp"

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wenum-compare"
#define RAYGUI_IMPLEMENTATION
#include "../include/raygui.h"
#pragma GCC diagnostic pop


#define FIRST_LAYER -1
#define LAST_LAYER  1

#define SYS_MESSAGE_SECONDS 2


typedef struct SysMessage {
    char *msg;
    float secondsUntilDisappear;
} SysMessage;

typedef struct LevelTransitionShaderControl {
    double timer;
    bool isClose;
    Vector2 focusPoint;
} LevelTransitionShaderControl;


ListNode *SYS_MESSAGES_HEAD = 0;


// Texture covering the whole screen, used to render shaders
static RenderTexture2D shaderRenderTexture;
static LevelTransitionShaderControl levelTransitionShaderControl;


// Returns the given color, with the given transparency level. 
static Color getColorTransparency(Color color, int transparency) {

    return { color.r, color.g, color.b,
                (unsigned char) transparency };
}

// Draws rectangle with scene-based position
static void drawSceneRectangle(Rectangle rect, Color color) {

    Vector2 scenePos = PosInSceneToScreen({ rect.x, rect.y });
    Rectangle screenRect = { scenePos.x, scenePos.y, rect.width, rect.height };
    DrawRectangleRec(screenRect, color);
}

static void drawTexture(Sprite sprite, Vector2 pos, Color tint, bool flipHorizontally) {

    Dimensions dimensions = SpriteScaledDimensions(sprite);


    // Raylib's draw function rotates the sprite around the origin, instead of its middle point.
    // Maybe this should be fixed in a way that works for any angle. 
    if (sprite.rotation == 90)          pos = { pos.x + dimensions.width, pos.y };
    else if (sprite.rotation == 180)    pos = { pos.x + dimensions.width,
                                                            pos.y + dimensions.height };
    else if (sprite.rotation == 270)    pos = { pos.x, pos.y + dimensions.height };



    if (!flipHorizontally) {
        DrawTextureEx(sprite.sprite,
                    pos,
                    sprite.rotation,
                    sprite.scale,
                    tint);    
        
        return;
    }

    Rectangle source = {
        0.0,
        0.0,
        (float) -sprite.sprite.width,
        (float) sprite.sprite.height
    };

    Rectangle destination = {
        pos.x,
        pos.y,
        dimensions.width,
        dimensions.height
    };

    DrawTexturePro(sprite.sprite,
                    source,
                    destination,
                    { 0, 0 },
                    sprite.rotation,
                    tint);    
}

// Draws sprite in the background, with effects applied.
static void drawSpriteInBackground(Sprite sprite, Vector2 pos, int layer) {

    float scale = 1;
    Color tint = { 0xFF, 0xFF, 0xFF, 0xFF };
    float parallaxSpeed = 1;

    switch (layer) {

    case -1:
        scale = 0.7;
        tint = { 0xFF, 0xFF, 0xFF, 0x88 };
        parallaxSpeed = 0.4;
        break;

    case -2:
        scale = 0.3;
        tint = { 0xFF, 0xFF, 0xFF, 0x44 };
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

static void drawBackground() {

    if (GAME_STATE->mode == MODE_OVERWORLD) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, { 39, 39, 54, 255 }); 
    }


    else if (GAME_STATE->mode == MODE_IN_LEVEL) {
        if (LEVEL_STATE->levelName[0] == '\0') {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
            return;
        }

        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, { 15, 15, 20, 255 });

        Vector2 levelBottomOnScreen = PosInSceneToScreen({ 0, FLOOR_DEATH_HEIGHT });
        DrawRectangle(0, levelBottomOnScreen.y, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);

        if (!GAME_STATE->showBackground) return; 
        drawSpriteInBackground(SPRITES->Nightclub,   { 1250, 250 },  -1);
        drawSpriteInBackground(SPRITES->BGHouse,     { 600, 300 },  -2);
    }
}

static void drawOverworldEntity(OverworldEntity *entity) {

    Vector2 pos = PosInSceneToScreen({
                                        entity->gridPos.x,
                                        entity->gridPos.y });

    drawTexture(entity->sprite, { pos.x, pos.y }, WHITE, false);
}

// Draws the ghost of an editor's selected entity being moved
static void drawOverworldEntityMoveGhost(OverworldEntity *entity) {

    Vector2 pos = EditorEntitySelectionCalcMove({
                                                    entity->gridPos.x,
                                                    entity->gridPos.y });

    pos = PosInSceneToScreen(pos);

    Color color =  { WHITE.r, WHITE.g, WHITE.b,
                            EDITOR_SELECTION_MOVE_TRANSPARENCY };

    drawTexture(entity->sprite, pos, color, false);
}

static void drawLevelEntity(LevelEntity *entity) {

    Vector2 pos = PosInSceneToScreen({
                                        entity->hitbox.x,
                                        entity->hitbox.y });

    drawTexture(entity->sprite, { pos.x, pos.y }, WHITE, !entity->isFacingRight);
}

// Draws the ghost of an editor's selected entity being moved
static void drawLevelEntityMoveGhost(LevelEntity *entity) {

    Vector2 pos = EditorEntitySelectionCalcMove({
                                                    entity->hitbox.x,
                                                    entity->hitbox.y });

    pos = PosInSceneToScreen(pos);

    Color color =  { WHITE.r, WHITE.g, WHITE.b,
                            EDITOR_SELECTION_MOVE_TRANSPARENCY };

    drawTexture(entity->sprite, pos, color, !entity->isFacingRight);
}

static void drawLevelEntityOrigin(LevelEntity *entity) {

    Vector2 pos = PosInSceneToScreen({
                                        entity->origin.x,
                                        entity->origin.y });

    drawTexture(entity->sprite, { pos.x, pos.y },
                 { WHITE.r, WHITE.g, WHITE.b, 30 }, false);
}

static void drawEntities() {

    for (int layer = FIRST_LAYER; layer <= LAST_LAYER; layer++) {

        ListNode *node = GetEntityListHead();

        while (node != 0) {

            if (GAME_STATE->mode == MODE_IN_LEVEL) {
                LevelEntity *entity = (LevelEntity *) node->item;
                if (entity->layer != layer) goto next_entity;
                
                if (!(entity->components & LEVEL_IS_ENEMY) ||
                    !entity->isDead)
                        drawLevelEntity(entity);

                if (EDITOR_STATE->isEnabled)
                        drawLevelEntityOrigin(entity);
            }

            else if (GAME_STATE->mode == MODE_OVERWORLD) {
                OverworldEntity *entity = (OverworldEntity *) node->item;
                if (entity->layer != layer) goto next_entity;
                drawOverworldEntity((OverworldEntity *) node->item);
            }

            else return;

next_entity:
            node = node->next;
        }
    }
}

static void drawSysMessages() {

    ListNode *node = SYS_MESSAGES_HEAD;
    ListNode *nextNode;
    SysMessage *msg;
    size_t currentMsg = 1;
    int x, y;

    while (node) {

        nextNode = node->next;

        msg = (SysMessage *) node->item;

        if (msg->secondsUntilDisappear <= 0) {
            MemFree(msg->msg);
            LinkedListDestroyNode(&SYS_MESSAGES_HEAD, node);
            goto next_node;
        }        

        x = 15;
        y = SCREEN_HEIGHT - 15 - (30 * currentMsg);

        DrawText(msg->msg, x, y, 30, RAYWHITE);
        
        currentMsg++;
        msg->secondsUntilDisappear -= GetFrameTime();

next_node:
        node = nextNode;
    } 
}

static void drawDebugGrid() {

    Dimensions grid;
    if (GAME_STATE->mode == MODE_OVERWORLD) grid = OW_GRID;
    else if (GAME_STATE->mode == MODE_IN_LEVEL) grid = LEVEL_GRID;
    else return;

    Vector2 offset = DistanceFromGrid(CAMERA->pos, grid);

    for (float lineX = offset.x; lineX <= SCREEN_WIDTH; lineX += grid.width) {
        DrawLine(lineX, 0, lineX, SCREEN_HEIGHT, BLUE);
    }

    for (float lineY = offset.y; lineY <= SCREEN_HEIGHT; lineY += grid.height) {
        DrawLine(0, lineY, SCREEN_WIDTH, lineY, BLUE);
    }
}

static void drawLevelHud() {

    if (EDITOR_STATE->isEnabled) return;

    if (LEVEL_STATE->isPaused && PLAYER_ENTITY && !PLAYER_ENTITY->isDead)
        DrawText("PAUSADO", 600, 360, 30, RAYWHITE);
        
    if (PLAYER_ENTITY && PLAYER_ENTITY->isDead)
        DrawText("VOCÊ MORREU", 450, 330, 60, RAYWHITE);
    
    if (LEVEL_STATE->levelName[0] == '\0')
        DrawText("Arraste uma fase para cá", 400, 350, 40, RAYWHITE);
}

static void drawOverworldHud() {

    OverworldEntity *tile = OW_STATE->tileUnderCursor;

    if (tile->tileType == OW_LEVEL_DOT) {

        // Draw level name

        Vector2 pos = PosInSceneToScreen({ tile->gridPos.x - 20,
                            tile->gridPos.y + (tile->sprite.sprite.height * tile->sprite.scale) });

        char levelName[LEVEL_NAME_BUFFER_SIZE];

        if (!tile->levelName)
            TraceLog(LOG_ERROR, "Overworld level dot to be rendered has no levelName referenced.");

        if (tile->levelName[0] != '\0') strcpy(levelName, tile->levelName);

        else strcpy(levelName, "[sem fase]");

        DrawText(levelName, pos.x, pos.y, 20, RAYWHITE);
    }
}

static void drawDebugHud() {

    if (CameraIsPanned()) DrawText("Câmera deslocada",
                                    SCREEN_WIDTH - 300, SCREEN_HEIGHT - 45, 30, RAYWHITE);

    ListNode *listHead = GetEntityListHead();
    if (listHead) {
        char buffer[50];
        sprintf(buffer, "Debug ligado\n%d entidades", LinkedListCountNodes(listHead));
        DrawText(buffer, 10, 20, 20, WHITE);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        char buffer[50];
        sprintf(buffer, "Botão esquerdo: x=%.0f, y=%.0f", mousePos.x, mousePos.y);
        DrawText(buffer, 600, 20, 20, WHITE);
    }

    ListNode *node = DEBUG_ENTITY_INFO_HEAD;
    while (node != 0) {
        void *entity = node->item;
        Rectangle hitbox;
        Vector2 screenPos;
        ListNode *nextNode = node->next;

        if (!entity) { // Destroyed
            LinkedListRemoveNode(&DEBUG_ENTITY_INFO_HEAD, node);
            TraceLog(LOG_TRACE, "Debug entity info stopped showing entity.");
            goto next_node;
        }

        switch (GAME_STATE->mode) {
        case MODE_IN_LEVEL:
            hitbox = ((LevelEntity *) entity)->hitbox;
            break;

        case MODE_OVERWORLD:
            RectangleSetPos(&hitbox, ((OverworldEntity *) entity)->gridPos);
            RectangleSetDimensions(&hitbox, OW_GRID);
            break;

        default:
            return;
        }

        screenPos = PosInSceneToScreen(RectangleGetPos(hitbox));

        DrawRectangle(screenPos.x, screenPos.y,
                hitbox.width, hitbox.height, { GREEN.r, GREEN.g, GREEN.b, 128 });
        DrawRectangleLines(screenPos.x, screenPos.y,
                hitbox.width, hitbox.height, GREEN);

        char buffer[500];
        sprintf(buffer, "X=%.1f\nY=%.1f",
                    hitbox.x,
                    hitbox.y);
        DrawText(buffer, screenPos.x, screenPos.y, 25, WHITE);

next_node:
        node = nextNode;
    }
}

// Render editor buttons of game entities
static void drawEditorEntityButtons() {

    int renderedCount = 0;
    ListNode *node = EDITOR_STATE->entitiesHead;

    while (node != 0) {

        EditorEntityButton *item = (EditorEntityButton *) node->item;

        bool isItemSelected = EDITOR_STATE->toggledEntityButton == item;

        Rectangle buttonRect = EditorEntityButtonRect(renderedCount);

        GuiToggleSprite(
            buttonRect,
            item->sprite, 
            { buttonRect.x, buttonRect.y },
            &isItemSelected
        );

        if (isItemSelected) EditorEntityButtonSelect(item);

        renderedCount++;

        node = node->next;
    }
}

// Render editor buttons related to control functions
static void drawEditorControlButtons() {

    int renderedCount = 0;
    ListNode *node = EDITOR_STATE->controlHead;

    while (node != 0) {

        EditorControlButton *item = (EditorControlButton *) node->item;

        Rectangle buttonRect = EditorControlButtonRect(renderedCount);

        if (GuiButton(buttonRect, item->label)) {

            if (!item->handler) {
                TraceLog(LOG_WARNING, "No handler to editor control button #%d, '%s'.",
                            renderedCount, item->label);
                goto next_button;
            }

            item->handler();

            return;
        }

next_button:
        renderedCount++;

        node = node->next;
    }
}

static void drawEditorEntitySelection() {

    if (EDITOR_STATE->isSelectingEntities)
        drawSceneRectangle(EditorSelectionGetRect(), EDITOR_SELECTION_RECT_COLOR);

    ListNode *node = EDITOR_STATE->selectedEntities;
    while (node != 0) {

        const Color color = EDITOR_SELECTION_ENTITY_COLOR;

        if (GAME_STATE->mode == MODE_IN_LEVEL) {

            LevelEntity *entity = (LevelEntity *) node->item;

            if (EDITOR_STATE->isMovingSelectedEntities) {
                drawLevelEntityMoveGhost(entity);
            } else {
                if (!entity->isDead) drawSceneRectangle(entity->hitbox, color);
                drawSceneRectangle(LevelEntityOriginHitbox(entity), color);
            }

        }
        else if (GAME_STATE->mode == MODE_OVERWORLD) {

            OverworldEntity *entity = (OverworldEntity *) node->item;

            if (EDITOR_STATE->isMovingSelectedEntities) {
                drawOverworldEntityMoveGhost(entity);
            } else {
                drawSceneRectangle(OverworldEntitySquare(entity), color);
            }
            
        }

        node = node->next;
    }
}

static void drawEditorCursor() {

    EditorEntityButton* b = EDITOR_STATE->toggledEntityButton;
    if (!b) return;

    Vector2 m = GetMousePosition();
    if (!IsInPlayArea(m)) return;

    DrawTexture(b->sprite.sprite, m.x, m.y, getColorTransparency(WHITE, 96));
}

static void drawEditor() {

    drawEditorEntitySelection();


    DrawLine(SCREEN_WIDTH,
                0,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                RAYWHITE);

    DrawRectangle(EDITOR_PANEL_RECT.x,
                    EDITOR_PANEL_RECT.y,
                    EDITOR_PANEL_RECT.width,
                    EDITOR_PANEL_RECT.height,
                    EDITOR_BG_COLOR);

    drawEditorEntityButtons();

    DrawLine(SCREEN_WIDTH,
                EDITOR_CONTROL_PANEL_Y,
                SCREEN_WIDTH + EDITOR_PANEL_RECT.width,
                EDITOR_CONTROL_PANEL_Y,
                RAYWHITE);

    drawEditorControlButtons();

    drawEditorCursor();
}

static void drawLevelTransitionShader() {

    double elapsedTime = GetTime() - levelTransitionShaderControl.timer;

                                                            // small buffer 
    if (elapsedTime >= LEVEL_TRANSITION_ANIMATION_DURATION + GetFrameTime()) {
        TraceLog(LOG_TRACE, "ShaderLevelTransition finished.");
        levelTransitionShaderControl.timer = -1;
        return;
    }

    ShaderLevelTransitionSetUniforms(
        { (float) GetScreenWidth(), (float) GetScreenHeight() },
        levelTransitionShaderControl.focusPoint,
        LEVEL_TRANSITION_ANIMATION_DURATION,
        elapsedTime,
        (int) levelTransitionShaderControl.isClose
    );

    BeginShaderMode(ShaderLevelTransition);
        DrawTextureRec(shaderRenderTexture.texture,
                         { 0, 0, (float)shaderRenderTexture.texture.width, (float)-shaderRenderTexture.texture.height },
                         { 0, 0 },
                        WHITE );
    EndShaderMode();
}

void RenderInitialize() {

    shaderRenderTexture = LoadRenderTexture(SCREEN_WIDTH_W_EDITOR, SCREEN_HEIGHT);

    levelTransitionShaderControl.timer = -1;

    // Line spacing of DrawText() 's containing line break
    SetTextLineSpacing(30);

    TraceLog(LOG_INFO, "Render initialized.");
}

void Render() {

    BeginDrawing();

        ClearBackground(BLACK);

        drawBackground();

        drawEntities();

        if      (GAME_STATE->mode == MODE_IN_LEVEL)          drawLevelHud();
        else if (GAME_STATE->mode == MODE_OVERWORLD)         drawOverworldHud();

        if      (GAME_STATE->showDebugGrid)                  drawDebugGrid();

        if (levelTransitionShaderControl.timer != -1)   drawLevelTransitionShader();

        if      (GAME_STATE->showDebugHUD)                   drawDebugHud();

        drawSysMessages();

        if (EDITOR_STATE->isEnabled) drawEditor();

    EndDrawing();
}

void RenderResizeWindow(int width, int height) {

    SetWindowSize(width, height);
}

void RenderPrintSysMessage(char *msg) {

    char *msgCopy = (char *) MemAlloc(sizeof(char) * SYS_MSG_BUFFER_SIZE);
    strcpy(msgCopy, msg); 

    SysMessage *newMsg = (SysMessage *) MemAlloc(sizeof(SysMessage));
    newMsg->msg = msgCopy;
    newMsg->secondsUntilDisappear = SYS_MESSAGE_SECONDS;
    
    LinkedListAdd(&SYS_MESSAGES_HEAD, newMsg);

    TraceLog(LOG_TRACE, "Added sys message to list: '%s'.", msg);
}

void RenderLevelTransitionEffectStart(Vector2 sceneFocusPoint, bool isClose) {

    levelTransitionShaderControl.timer = GetTime();
    levelTransitionShaderControl.focusPoint = PosInSceneToScreen(sceneFocusPoint);
    levelTransitionShaderControl.isClose = isClose;

    TraceLog(LOG_TRACE, "ShaderLevelTransition started from x=%.1f, y=%.1f.",
        levelTransitionShaderControl.focusPoint.x, levelTransitionShaderControl.focusPoint.y);

    // Fix for how GLSL works
    levelTransitionShaderControl.focusPoint.y = GetScreenHeight() - levelTransitionShaderControl.focusPoint.y;
}