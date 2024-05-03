#include <raylib.h>
#include <stdio.h>
#include <string>

#include "core.hpp"
#include "assets.hpp"
#include "level/level.hpp"
#include "level/player.hpp"
#include "level/textbox.hpp"
#include "overworld.hpp"
#include "camera.hpp"
#include "editor.hpp"
#include "debug.hpp"
#include "input.hpp"

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


namespace Render {


class SysMessage : public LinkedList::Node {

public:
    char *msg;
    float secondsUntilDisappear;
};

typedef struct LevelTransitionShaderControl {
    double timer;
    bool isClose;
    Vector2 focusPoint;
} LevelTransitionShaderControl;


LinkedList::Node *SYS_MESSAGES_HEAD = 0;


// Texture covering the whole screen, used to render shaders
RenderTexture2D shaderRenderTexture;
LevelTransitionShaderControl levelTransitionShaderControl;

// Returns the given color, with the given transparency level. 
Color getColorTransparency(Color color, int transparency) {

    return { color.r, color.g, color.b,
                (unsigned char) transparency };
}

// Draws rectangle with scene-based position
void drawSceneRectangle(Rectangle rect, Color color) {

    Vector2 scenePos = PosInSceneToScreen({ rect.x, rect.y });
    Dimensions sceneDim = DimensionsInSceneToScreen({ rect.width, rect.height });
    Rectangle screenRect = { scenePos.x, scenePos.y, sceneDim.width, sceneDim.height };
    DrawRectangleRec(screenRect, color);
}

void drawTexture(Sprite *sprite, Vector2 pos, Color tint, int rotation, bool flipHorizontally) {

    Dimensions dimensions = DimensionsInSceneToScreen(
                                SpriteScaledDimensions(sprite));


    // Raylib's draw function rotates the sprite around the origin, instead of its middle point.
    // Maybe this should be fixed in a way that works for any angle. 
    if (rotation == 90)          pos = { pos.x + dimensions.width, pos.y };
    else if (rotation == 180)    pos = { pos.x + dimensions.width,
                                                            pos.y + dimensions.height };
    else if (rotation == 270)    pos = { pos.x, pos.y + dimensions.height };



    if (!flipHorizontally) {
        DrawTextureEx(sprite->sprite,
                    pos,
                    rotation,
                    ScaleInSceneToScreen(sprite->scale),
                    tint);    
        
        return;
    }

    Rectangle source = {
        0.0,
        0.0,
        (float) -sprite->sprite.width,
        (float) sprite->sprite.height
    };

    Rectangle destination = {
        pos.x,
        pos.y,
        dimensions.width,
        dimensions.height
    };

    DrawTexturePro(sprite->sprite,
                    source,
                    destination,
                    { 0, 0 },
                    rotation,
                    tint);    
}

// Draws sprite in the background, with effects applied.
void drawSpriteInBackground(Sprite *sprite, Vector2 pos, int layer) {

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

    DrawTextureEx(sprite->sprite, pos, 0, (scale * sprite->scale), tint);
}

void drawBackground() {

    if (GAME_STATE->mode == MODE_OVERWORLD) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, { 39, 39, 54, 255 }); 
    }


    else if (GAME_STATE->mode == MODE_IN_LEVEL) {
        if (Level::STATE->levelName[0] == '\0') {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
            return;
        }

        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, { 25, 25, 35, 255 });

        Vector2 levelBottomOnScreen = PosInSceneToScreen({ 0, FLOOR_DEATH_HEIGHT });
        DrawRectangle(0, levelBottomOnScreen.y, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);

        if (!GAME_STATE->showBackground) return; 
        drawSpriteInBackground(&SPRITES->Nightclub,   { 1250, 250 },  -1);
        drawSpriteInBackground(&SPRITES->BGHouse,     { 600, 300 },  -2);
    }
}

void drawOverworldEntity(OverworldEntity *entity) {

    Vector2 pos = PosInSceneToScreen({
                                        entity->gridPos.x,
                                        entity->gridPos.y });

    drawTexture(entity->sprite, { pos.x, pos.y }, WHITE, entity->rotation, false);
}

// Draws the ghost of an editor's selected entity being moved
void drawOverworldEntityMoveGhost(OverworldEntity *entity) {

    Vector2 pos = EditorEntitySelectionCalcMove({
                                                    entity->gridPos.x,
                                                    entity->gridPos.y });

    pos = PosInSceneToScreen(pos);

    Color color =  { WHITE.r, WHITE.g, WHITE.b,
                            EDITOR_SELECTION_MOVE_TRANSPARENCY };

    drawTexture(entity->sprite, pos, color, entity->rotation, false);
}

void drawEntities() {

    for (int layer = FIRST_LAYER; layer <= LAST_LAYER; layer++) {

        LinkedList::Node *node = GetEntityListHead();

        while (node != 0) {

            if (GAME_STATE->mode == MODE_IN_LEVEL) {

                Level::Entity *entity = (Level::Entity *) node;
                if (entity->layer != layer) goto next_entity;                
                entity->Draw();
            }

            else if (GAME_STATE->mode == MODE_OVERWORLD) {

                OverworldEntity *entity = (OverworldEntity *) node;
                if (entity->layer != layer) goto next_entity;
                drawOverworldEntity(entity);
            }

            else return;

next_entity:
            node = node->next;
        }
    }
}

void drawSysMessages() {

    SysMessage *msg = (SysMessage *) SYS_MESSAGES_HEAD;
    SysMessage *nextMsg;
    size_t currentMsg = 1;
    int x, y;

    while (msg) {

        nextMsg = (SysMessage *) msg->next;

        if (msg->secondsUntilDisappear <= 0) {
            MemFree(msg->msg);
            LinkedList::DestroyNode(&SYS_MESSAGES_HEAD, msg);
            goto next_node;
        }        

        x = 15;
        y = SCREEN_HEIGHT - 15 - (30 * currentMsg);

        DrawText(msg->msg, x, y, 30, RAYWHITE);
        
        currentMsg++;
        msg->secondsUntilDisappear -= GetFrameTime();

next_node:
        msg = nextMsg;
    } 
}

void drawDebugGrid() {

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

void drawLevelHud() {

    if (EDITOR_STATE->isEnabled) return;

    DrawTextureEx(SPRITES->LevelCheckpointFlag.sprite,
                    { SCREEN_WIDTH-149, SCREEN_HEIGHT-65 }, 0, SPRITES->LevelCheckpointFlag.scale/1.7, WHITE);
    DrawText(std::string("x " + std::to_string(Level::STATE->checkpointsLeft)).c_str(),
                SCREEN_WIDTH-100, SCREEN_HEIGHT-56, 30, RAYWHITE);

    if (Level::STATE->isPaused && PLAYER && !PLAYER->isDead)
        DrawText("PAUSADO", 600, 360, 30, RAYWHITE);
        
    if (PLAYER && PLAYER->isDead)
        DrawText("VOCÊ MORREU", 450, 330, 60, RAYWHITE);
    
    if (Level::STATE->levelName[0] == '\0')
        DrawText("Arraste uma fase para cá", 400, 350, 40, RAYWHITE);
}

void drawOverworldHud() {

    OverworldEntity *tile = OW_STATE->tileUnderCursor;

    if (tile->tileType == OW_LEVEL_DOT) {

        // Draw level name

        Vector2 pos = PosInSceneToScreen({ tile->gridPos.x - 20,
                            tile->gridPos.y + (tile->sprite->sprite.height * tile->sprite->scale) });

        char levelName[LEVEL_NAME_BUFFER_SIZE];

        if (!tile->levelName)
            TraceLog(LOG_ERROR, "Overworld level dot to be rendered has no levelName referenced.");

        if (tile->levelName[0] != '\0') strcpy(levelName, tile->levelName);

        else strcpy(levelName, "[sem fase]");

        DrawText(levelName, pos.x, pos.y, 20, RAYWHITE);
    }
}

void drawDebugEntityInfo(LinkedList::Node *entity) {

    Rectangle hitbox;
    Vector2 screenPos;
    Dimensions screenDim;
    std::string str;

    switch (GAME_STATE->mode) {
    case MODE_IN_LEVEL:
        hitbox = ((Level::Entity *) entity)->hitbox;
        str = ((Level::Entity *) entity)->GetEntityDebugString();
        break;

    case MODE_OVERWORLD:
        RectangleSetPos(&hitbox, ((OverworldEntity *) entity)->gridPos);
        RectangleSetDimensions(&hitbox, OW_GRID);
        str = std::string("x=" + std::to_string((int) hitbox.x) +
                            "\ny=" + std::to_string((int) hitbox.y)); // TODO create Overworld Entity
        break;

    default:
        return;
    }

    screenPos = PosInSceneToScreen(RectangleGetPos(hitbox));
    screenDim = DimensionsInSceneToScreen({ hitbox.width, hitbox.height });

    DrawRectangle(screenPos.x, screenPos.y,
            screenDim.width, screenDim.height, { GREEN.r, GREEN.g, GREEN.b, 128 });
    DrawRectangleLines(screenPos.x, screenPos.y,
            screenDim.width, screenDim.height, GREEN);

    DrawText(str.c_str(), screenPos.x, screenPos.y, 20, WHITE);
}

void drawDebugHud() {

    if (CameraIsPanned()) DrawText("Câmera deslocada",
                                    SCREEN_WIDTH - 300, SCREEN_HEIGHT - 45, 30, RAYWHITE);

    LinkedList::Node *listHead = GetEntityListHead();
    if (listHead) {
        char buffer[50];
        sprintf(buffer, "%d entidades", LinkedList::CountNodes(listHead));
        DrawText(buffer, 10, 20, 20, WHITE);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        Vector2 mousePosScene = PosInScreenToScene(mousePos); 
        char buffer[50];
        sprintf(buffer, "Tela: x=%.0f, y=%.0f; Cena: x=%.0f, y=%.0f", mousePos.x, mousePos.y, mousePosScene.x, mousePosScene.y);
        DrawText(buffer, 600, 20, 20, WHITE);
    }

    for (auto e = DEBUG_ENTITY_INFO_HEAD.begin(); e < DEBUG_ENTITY_INFO_HEAD.end(); e++) {
        drawDebugEntityInfo(*e);
    }
}

// Render editor buttons of game entities
void drawEditorEntityButtons() {

    int renderedCount = 0;
    EditorEntityButton *button = (EditorEntityButton *) EDITOR_STATE->entitiesHead;

    while (button != 0) {

        bool isItemSelected = EDITOR_STATE->toggledEntityButton == button;

        Rectangle buttonRect = EditorEntityButtonRect(renderedCount);

        GuiToggleSprite(
            buttonRect,
            button->sprite, 
            { buttonRect.x, buttonRect.y },
            &isItemSelected
        );

        if (isItemSelected) EditorEntityButtonSelect(button);

        renderedCount++;

        button = (EditorEntityButton *) button->next;
    }
}

// Render editor buttons related to control functions
void drawEditorControlButtons() {

    int renderedCount = 0;
    EditorControlButton *button = (EditorControlButton *) EDITOR_STATE->controlHead;

    while (button != 0) {

        Rectangle buttonRect = EditorControlButtonRect(renderedCount);

        if (GuiButton(buttonRect, button->label)) {

            if (!button->handler) {
                TraceLog(LOG_WARNING, "No handler to editor control button #%d, '%s'.",
                            renderedCount, button->label);
                goto next_button;
            }

            button->handler();

            return;
        }

next_button:
        renderedCount++;

        button = (EditorControlButton *) button->next;
    }
}

void drawEditorEntitySelection() {

    if (EDITOR_STATE->isSelectingEntities)
        drawSceneRectangle(EditorSelectionGetRect(), EDITOR_SELECTION_RECT_COLOR);

    for (auto e = EDITOR_STATE->selectedEntities.begin(); e < EDITOR_STATE->selectedEntities.end(); e++) {

        const Color color = EDITOR_SELECTION_ENTITY_COLOR;

        if (GAME_STATE->mode == MODE_IN_LEVEL) {

            Level::Entity *entity = (Level::Entity *) *e;

            if (EDITOR_STATE->isMovingSelectedEntities) {
                entity->DrawMoveGhost();
            } else {
                if (!entity->IsADeadEnemy()) drawSceneRectangle(entity->hitbox, color);
                drawSceneRectangle(entity->GetOriginHitbox(), color);
            }

        }
        else if (GAME_STATE->mode == MODE_OVERWORLD) {

            OverworldEntity *entity = (OverworldEntity *) *e;

            if (EDITOR_STATE->isMovingSelectedEntities) {
                drawOverworldEntityMoveGhost(entity);
            } else {
                drawSceneRectangle(OverworldEntitySquare(entity), color);
            }
            
        }
    }
}

void drawEditorCursor() {

    EditorEntityButton* b = EDITOR_STATE->toggledEntityButton;
    if (!b) return;

    Vector2 m = GetMousePosition();
    if (!IsInPlayArea(m)) return;

    DrawTexture(b->sprite->sprite, m.x, m.y, getColorTransparency(WHITE, 96));
}

void drawEditor() {

    Rectangle rect = EditorBarGetRect();
    float divisorY = EditorBarGetDivisorY();


    drawEditorEntitySelection();

    DrawLine(rect.x,
                rect.y,
                rect.x,
                rect.y + rect.height,
                RAYWHITE);

    DrawRectangle(rect.x,
                    rect.y,
                    rect.width,
                    rect.height,
                    EDITOR_BG_COLOR);

    drawEditorEntityButtons();

    DrawLine(rect.x,
                divisorY,
                rect.x + rect.width,
                divisorY,
                RAYWHITE);

    drawEditorControlButtons();

    drawEditorCursor();
}

void drawLevelTransitionShader() {

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

void drawTextInput() {

    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, { 0x00, 0x00, 0x00, 0xaa });

    // TODO wrap text
    DrawText(std::string("> " + Input::STATE.textInputed).c_str(), 120, 100, 60, RAYWHITE);
}

void Initialize() {

    //shaderRenderTexture = LoadRenderTexture(SCREEN_WIDTH_W_EDITOR, SCREEN_HEIGHT);
    shaderRenderTexture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    levelTransitionShaderControl.timer = -1;

    // Line spacing of DrawText() 's containing line break
    SetTextLineSpacing(35);

    TraceLog(LOG_INFO, "Render initialized.");
}

void Render() {

    BeginDrawing();

        ClearBackground(BLACK);

        drawBackground();

        drawEntities();

        if      (GAME_STATE->mode == MODE_IN_LEVEL)         drawLevelHud();
        else if (GAME_STATE->mode == MODE_OVERWORLD)        drawOverworldHud();

        if      (GAME_STATE->showDebugGrid)                 drawDebugGrid();

        if      (levelTransitionShaderControl.timer != -1)  drawLevelTransitionShader();

        if      (GAME_STATE->showDebugHUD)                  drawDebugHud();

        if      (GAME_STATE->waitingForTextInput)           drawTextInput();

        drawSysMessages();

        if (EDITOR_STATE->isEnabled) drawEditor();

    EndDrawing();
}

void ResizeWindow(int width, int height) {

    SetWindowSize(width, height);
}

void DrawLevelEntity(Level::Entity *entity) {

    Vector2 pos = PosInSceneToScreen({
                                        entity->hitbox.x,
                                        entity->hitbox.y });

    drawTexture(entity->sprite, { pos.x, pos.y }, WHITE, 0, !entity->isFacingRight);
}

void DrawLevelEntityOriginGhost(Level::Entity *entity) {

    Vector2 pos = PosInSceneToScreen({
                                        entity->origin.x,
                                        entity->origin.y });

    drawTexture(entity->sprite, { pos.x, pos.y },
                 { WHITE.r, WHITE.g, WHITE.b, ORIGIN_GHOST_TRANSPARENCY }, 0, false);
}

void DrawLevelEntityMoveGhost(Level::Entity *entity) {

    Vector2 pos = EditorEntitySelectionCalcMove({
                                                    entity->hitbox.x,
                                                    entity->hitbox.y });

    pos = PosInSceneToScreen(pos);

    Color color =  { WHITE.r, WHITE.g, WHITE.b,
                            EDITOR_SELECTION_MOVE_TRANSPARENCY };

    drawTexture(entity->sprite, pos, color, 0, !entity->isFacingRight);
}

void PrintSysMessage(const std::string &msg) {

    // TODO move the whole SysMessage system to C++ strings

    char *msgCopy = (char *) MemAlloc(sizeof(char) * SYS_MSG_BUFFER_SIZE);
    strcpy(msgCopy, msg.c_str()); 

    SysMessage *newMsg = new SysMessage();
    newMsg->msg = msgCopy;
    newMsg->secondsUntilDisappear = SYS_MESSAGE_SECONDS;
    
    LinkedList::AddNode(&SYS_MESSAGES_HEAD, newMsg);

    TraceLog(LOG_TRACE, "Added sys message to list: '%s'.", msg);
}

void LevelTransitionEffectStart(Vector2 sceneFocusPoint, bool isClose) {

    levelTransitionShaderControl.timer = GetTime();
    levelTransitionShaderControl.focusPoint = PosInSceneToScreen(sceneFocusPoint);
    levelTransitionShaderControl.isClose = isClose;

    TraceLog(LOG_TRACE, "ShaderLevelTransition started from x=%.1f, y=%.1f.",
        levelTransitionShaderControl.focusPoint.x, levelTransitionShaderControl.focusPoint.y);

    // Fix for how GLSL works
    levelTransitionShaderControl.focusPoint.y = GetScreenHeight() - levelTransitionShaderControl.focusPoint.y;
}

void FullscreenToggle() {

    ToggleBorderlessWindowed();
}


} // namespace
