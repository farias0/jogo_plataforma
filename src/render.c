#include <raylib.h>
#include <stdio.h>

#include "core.h"
#include "assets.h"
#include "input.h"
#include "level/level.h"
#include "overworld.h"
#include "camera.h"

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-result"
#define RAYGUI_IMPLEMENTATION
#include "../include/raygui.h"
#pragma GCC diagnostic pop


#define FIRST_LAYER 0
#define LAST_LAYER  1

#define SYS_MESSAGE_SECONDS 2

#define LEVEL_TRANSITION_SHADER_DURATION 1.3 // in seconds

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
        Vector2 levelBottomOnScreen = PosInSceneToScreen((Vector2){ 0, FLOOR_DEATH_HEIGHT });
        DrawRectangle(0, 0, SCREEN_WIDTH, levelBottomOnScreen.y, (Color){ 15, 15, 20, 255 });

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

static void renderSysMsgs() {

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
            LinkedListRemove(&SYS_MESSAGES_HEAD, node);
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

static void renderHUD() {

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
    }
skip_debug_grid:


    if (STATE->mode == MODE_IN_LEVEL) {

        if (STATE->isPaused && !STATE->isPlayerDead) DrawText("PAUSADO", 600, 360, 30, RAYWHITE);
        
        if (STATE->isPlayerDead) DrawText("VOCÊ MORREU", 450, 330, 60, RAYWHITE);
        
        if (STATE->loadedLevel[0] == '\0')
            DrawText("Arraste uma fase para cá", 400, 350, 40, RAYWHITE);

    }
    else if (STATE->mode == MODE_OVERWORLD) {

        OverworldEntity *tile = STATE->tileUnderCursor;

        if (tile->tileType == OW_LEVEL_DOT) {

            Vector2 pos = PosInSceneToScreen((Vector2){ tile->gridPos.x - 20,
                                tile->gridPos.y + (tile->sprite.sprite.height * tile->sprite.scale) });
            char levelName[LEVEL_NAME_BUFFER_SIZE];
            if (tile->levelName[0] != '\0') strcpy(levelName, tile->levelName);
            else strcpy(levelName, "[sem fase]");
            DrawText(levelName, pos.x, pos.y, 20, RAYWHITE);
        }

    }

    if (STATE->isEditorEnabled) {

        if (CameraIsPanned()) DrawText("Câmera deslocada",
                                    SCREEN_WIDTH - 300, SCREEN_HEIGHT - 45, 30, RAYWHITE);
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

    renderSysMsgs();
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

        bool isItemSelected = STATE->editorSelectedEntity == item;

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

        if (GuiButton(buttonRect, item->label)) {

            if (!item->handler) {
                TraceLog(LOG_WARNING, "No handler to editor control button #%d, '%s'.",
                            editorButtonsRendered, item->label);
                goto next_button;
            }

            item->handler();

            return;
        }

next_button:
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

static void renderLevelTransitionShader() {

    double elapsedTime = GetTime() - levelTransitionShaderControl.timer;

    if (elapsedTime >= LEVEL_TRANSITION_SHADER_DURATION) {
        TraceLog(LOG_TRACE, "ShaderLevelTransition finished.");
        levelTransitionShaderControl.timer = -1;
        return;
    }

    ShaderLevelTransitionSetUniforms(
        (Vector2){ GetScreenWidth(), GetScreenHeight() },
        levelTransitionShaderControl.focusPoint,
        LEVEL_TRANSITION_SHADER_DURATION,
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

    TraceLog(LOG_INFO, "Render initialized.");
}

void Render() {

    BeginDrawing();

        ClearBackground(BLACK);

        renderBackground();

        renderEntities();

        renderHUD();

        if (levelTransitionShaderControl.timer != -1) renderLevelTransitionShader();

        if (STATE->isEditorEnabled) renderEditor();

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
    
    ListNode *node = MemAlloc(sizeof(ListNode));
    node->item = newMsg;
    LinkedListAdd(&SYS_MESSAGES_HEAD, node);

    TraceLog(LOG_TRACE, "Added sys message to list: '%s'.", msg);
}

void RenderStartLevelTransitionShader(Vector2 focusPoint, bool isClose) {

    levelTransitionShaderControl.timer = GetTime();
    levelTransitionShaderControl.focusPoint = focusPoint;
    levelTransitionShaderControl.isClose = isClose;

    TraceLog(LOG_TRACE, "ShaderLevelTransition started.");
}
