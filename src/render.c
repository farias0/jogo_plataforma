#include <raylib.h>
#include <stdio.h>

#include "core.h"
#include "assets.h"
#include "level/level.h"
#include "level/player.h"
#include "overworld.h"
#include "camera.h"
#include "editor.h"

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-result"
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

ListNode *DEBUG_ENTITY_INFO_HEAD = 0;


// Texture covering the whole screen, used to render shaders
static RenderTexture2D shaderRenderTexture;
static LevelTransitionShaderControl levelTransitionShaderControl;

// Draws rectangle with scene-based position
static void drawSceneRectangle(Rectangle rect, Color color) {

    Vector2 scenePos = PosInSceneToScreen((Vector2){ rect.x, rect.y });
    Rectangle screenRect = (Rectangle){ scenePos.x, scenePos.y, rect.width, rect.height };
    DrawRectangleRec(screenRect, color);
}

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
static void drawSpriteInBackground(Sprite sprite, Vector2 pos, int layer) {

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

static void drawBackground() {

    if (STATE->mode == MODE_OVERWORLD) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 39, 39, 54, 255 }); 
    }


    else if (STATE->mode == MODE_IN_LEVEL) {
        if (STATE->loadedLevel[0] == '\0') {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
            return;
        }

        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 15, 15, 20, 255 });

        Vector2 levelBottomOnScreen = PosInSceneToScreen((Vector2){ 0, FLOOR_DEATH_HEIGHT });
        DrawRectangle(0, levelBottomOnScreen.y, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);

        if (!STATE->showBackground) return; 
        drawSpriteInBackground(NightclubSprite,   (Vector2){ 1250, 250 },  -1);
        drawSpriteInBackground(BGHouseSprite,     (Vector2){ 600, 300 },  -2);
    }
}

static void drawOverworldEntity(OverworldEntity *entity) {

    Vector2 pos = PosInSceneToScreen((Vector2){
                                        entity->gridPos.x,
                                        entity->gridPos.y });

    drawTexture(entity->sprite, (Vector2){ pos.x, pos.y }, WHITE, false);
}

// Draws the ghost of an editor's selected entity being moved
static void drawOverworldEntityMoveGhost(OverworldEntity *entity) {

    Vector2 pos = EditorEntitySelectionCalcMove((Vector2){
                                                    entity->gridPos.x,
                                                    entity->gridPos.y });

    pos = PosInSceneToScreen(pos);

    Color color = (Color) { WHITE.r, WHITE.g, WHITE.b,
                            EDITOR_SELECTION_MOVE_TRANSPARENCY };

    drawTexture(entity->sprite, pos, color, false);
}

static void drawLevelEntity(LevelEntity *entity) {

    Vector2 pos = PosInSceneToScreen((Vector2){
                                        entity->hitbox.x,
                                        entity->hitbox.y });

    drawTexture(entity->sprite, (Vector2){ pos.x, pos.y }, WHITE, !entity->isFacingRight);
}

// Draws the ghost of an editor's selected entity being moved
static void drawLevelEntityMoveGhost(LevelEntity *entity) {

    Vector2 pos = EditorEntitySelectionCalcMove((Vector2){
                                                    entity->hitbox.x,
                                                    entity->hitbox.y });

    pos = PosInSceneToScreen(pos);

    Color color = (Color) { WHITE.r, WHITE.g, WHITE.b,
                            EDITOR_SELECTION_MOVE_TRANSPARENCY };

    drawTexture(entity->sprite, pos, color, !entity->isFacingRight);
}

static void drawLevelEntityOrigin(LevelEntity *entity) {

    Vector2 pos = PosInSceneToScreen((Vector2){
                                        entity->origin.x,
                                        entity->origin.y });

    drawTexture(entity->sprite, (Vector2){ pos.x, pos.y },
                (Color) { WHITE.r, WHITE.g, WHITE.b, 30 }, false);
}

static void drawEntities() {

    for (int layer = FIRST_LAYER; layer <= LAST_LAYER; layer++) {

        ListNode *node = GetEntityListHead();

        while (node != 0) {

            if (STATE->mode == MODE_IN_LEVEL) {
                LevelEntity *entity = (LevelEntity *) node->item;
                if (entity->layer != layer) goto next_entity;
                
                if (!(entity->components & LEVEL_IS_ENEMY) ||
                    !entity->isDead)
                        drawLevelEntity(entity);

                if (STATE->isEditorEnabled)
                        drawLevelEntityOrigin(entity);
            }

            else if (STATE->mode == MODE_OVERWORLD) {
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
    if (STATE->mode == MODE_OVERWORLD) grid = OW_GRID;
    else if (STATE->mode == MODE_IN_LEVEL) grid = LEVEL_GRID;
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

    if (STATE->isEditorEnabled) return;

    if (STATE->isPaused && LEVEL_PLAYER && !LEVEL_PLAYER->isDead)
        DrawText("PAUSADO", 600, 360, 30, RAYWHITE);
        
    if (LEVEL_PLAYER && LEVEL_PLAYER->isDead)
        DrawText("VOCÊ MORREU", 450, 330, 60, RAYWHITE);
    
    if (STATE->loadedLevel[0] == '\0')
        DrawText("Arraste uma fase para cá", 400, 350, 40, RAYWHITE);
}

static void drawOverworldHud() {

    OverworldEntity *tile = STATE->tileUnderCursor;

    if (tile->tileType == OW_LEVEL_DOT) {

        // Draw level name

        Vector2 pos = PosInSceneToScreen((Vector2){ tile->gridPos.x - 20,
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
        LevelEntity *entity = (LevelEntity *) node->item;
        ListNode *nextNode = node->next;

        if (!entity) { // Destroyed
            LinkedListRemoveNode(&DEBUG_ENTITY_INFO_HEAD, node);
            TraceLog(LOG_TRACE, "Debug entity info stopped showing entity.");
            goto next_node;
        }

        Vector2 pos = PosInSceneToScreen((Vector2) {
                                            entity->hitbox.x,
                                            entity->hitbox.y
                                        });

        DrawRectangle(pos.x, pos.y,
                entity->hitbox.width, entity->hitbox.height, (Color){ GREEN.r, GREEN.g, GREEN.b, 128 });
        DrawRectangleLines(pos.x, pos.y,
                entity->hitbox.width, entity->hitbox.height, GREEN);

        char buffer[500];
        sprintf(buffer, "X=%.1f\nY=%.1f",
                    entity->hitbox.x,
                    entity->hitbox.y);
        DrawText(buffer, pos.x, pos.y, 25, WHITE);

next_node:
        node = nextNode;
    }
}

// Render editor buttons of game entities
static void drawEditorEntityButtons() {

    DrawRectangle(EDITOR_ENTITIES_AREA.x,
                    EDITOR_ENTITIES_AREA.y,
                    EDITOR_ENTITIES_AREA.width,
                    EDITOR_ENTITIES_AREA.height,
                    EDITOR_BG_COLOR);

    int renderedCount = 0;
    ListNode *node = EDITOR_ENTITIES_HEAD;

    while (node != 0) {

        EditorEntityButton *item = (EditorEntityButton *) node->item;

        bool isItemSelected = STATE->editorButtonToggled == item;

        Rectangle buttonRect = EditorEntityButtonRect(renderedCount);

        GuiToggleSprite(
            buttonRect,
            item->sprite, 
            (Vector2){ buttonRect.x, buttonRect.y },
            &isItemSelected
        );

        if (isItemSelected) EditorEntityButtonSelect(item);

        renderedCount++;

        node = node->next;
    }
}

// Render editor buttons related to control functions
static void drawEditorControlButtons() {

    DrawRectangle(EDITOR_CONTROL_AREA.x,
                    EDITOR_CONTROL_AREA.y,
                    EDITOR_CONTROL_AREA.width,
                    EDITOR_CONTROL_AREA.height,
                    EDITOR_BG_COLOR);

    int renderedCount = 0;
    ListNode *node = EDITOR_CONTROL_HEAD;

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

        if (STATE->mode == MODE_IN_LEVEL) {

            LevelEntity *entity = (LevelEntity *) node->item;

            if (EDITOR_STATE->isMovingSelectedEntities) {
                drawLevelEntityMoveGhost(entity);
            } else {
                if (!entity->isDead) drawSceneRectangle(entity->hitbox, color);
                drawSceneRectangle(LevelEntityOriginHitbox(entity), color);
            }

        }
        else if (STATE->mode == MODE_OVERWORLD) {

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

static void drawEditor() {

    drawEditorEntitySelection();


    DrawLine(SCREEN_WIDTH,
                0,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                RAYWHITE);

    drawEditorEntityButtons();

    DrawLine(SCREEN_WIDTH,
                EDITOR_ENTITIES_AREA.height,
                SCREEN_WIDTH + EDITOR_BAR_WIDTH,
                EDITOR_ENTITIES_AREA.height,
                RAYWHITE); // Separator

    drawEditorControlButtons();
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
        (Vector2){ GetScreenWidth(), GetScreenHeight() },
        levelTransitionShaderControl.focusPoint,
        LEVEL_TRANSITION_ANIMATION_DURATION,
        elapsedTime,
        (int) levelTransitionShaderControl.isClose
    );

    BeginShaderMode(ShaderLevelTransition);
        DrawTextureRec(shaderRenderTexture.texture,
                        (Rectangle) { 0, 0, (float)shaderRenderTexture.texture.width, (float)-shaderRenderTexture.texture.height },
                        (Vector2) { 0, 0 },
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

        if      (STATE->mode == MODE_IN_LEVEL)          drawLevelHud();
        else if (STATE->mode == MODE_OVERWORLD)         drawOverworldHud();

        if      (STATE->showDebugGrid)                  drawDebugGrid();

        if (levelTransitionShaderControl.timer != -1)   drawLevelTransitionShader();

        if      (STATE->showDebugHUD)                   drawDebugHud();

        drawSysMessages();

        if (STATE->isEditorEnabled) drawEditor();

    EndDrawing();
}

void RenderResizeWindow(int width, int height) {

    SetWindowSize(width, height);
}

void RenderPrintSysMessage(char *msg) {

    char *msgCopy = MemAlloc(sizeof(char) * SYS_MSG_BUFFER_SIZE);
    strcpy(msgCopy, msg); 

    SysMessage *newMsg = MemAlloc(sizeof(SysMessage));
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

void RenderDebugEntityToggle(LevelEntity *entity) {
    
    ListNode *entitysNode = LinkedListGetNode(DEBUG_ENTITY_INFO_HEAD, entity);

    if (entitysNode) {
        LinkedListRemoveNode(&DEBUG_ENTITY_INFO_HEAD, entitysNode);
        TraceLog(LOG_TRACE, "Debug entity info removed entity;");
    } else {
        LinkedListAdd(&DEBUG_ENTITY_INFO_HEAD, entity);
        TraceLog(LOG_TRACE, "Debug entity info added entity;");
    }
}

void RenderDebugEntityStop(LevelEntity *entity) {
    ListNode *entitysNode = LinkedListGetNode(DEBUG_ENTITY_INFO_HEAD, entity);
    if (entitysNode) {
        LinkedListRemoveNode(&DEBUG_ENTITY_INFO_HEAD, entitysNode);
        TraceLog(LOG_TRACE, "Debug entity info stopped showing entity.");
    }
}

void RenderDebugEntityStopAll() {
    LinkedListRemoveAll(&DEBUG_ENTITY_INFO_HEAD);
    TraceLog(LOG_TRACE, "Debug entity info stopped showing all entities.");
}
