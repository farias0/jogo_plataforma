#include <raylib.h>

#include "editor.hpp"
#include "core.hpp"
#include "level/level.hpp"
#include "level/enemy.hpp"
#include "level/powerups.hpp"
#include "level/block.hpp"
#include "level/checkpoint.hpp"
#include "level/textbox.hpp"
#include "level/moving_platform.hpp"
#include "level/npc/princess.hpp"
#include "level/coin.hpp"
#include "overworld.hpp"
#include "linked_list.hpp"
#include "camera.hpp"
#include "render.hpp"


#define EDITOR_BAR_WIDTH        200

// The proportion between the Entity button and the Control button spaces
#define EDITOR_BAR_DIVISION     ((float) 4 / (float) 5)


EditorState *EDITOR_STATE = 0;


static void editorStateReset() {

    EditorSelectionCancel();

    EditorEmpty();

    TraceLog(LOG_DEBUG, "Editor state reset.");
}

static void buttonsSelectDefault() {
    EDITOR_STATE->toggledEntityButton = EDITOR_STATE->defaultEntityButton;
}

static void editorUseEraserInLevel(Vector2 pos) {

    Level::Entity *foundEntity = Level::EntityGetRemoveableAt(pos);
    if (!foundEntity) return;

    auto foundEntityInSelection = std::find(EDITOR_STATE->selectedEntities.begin(),
                                                EDITOR_STATE->selectedEntities.end(),
                                                foundEntity);
    bool isPartOfSelection = foundEntityInSelection != EDITOR_STATE->selectedEntities.end();

    if (isPartOfSelection) {

        // Delete all selected entities
        for (auto selectedEntity = EDITOR_STATE->selectedEntities.begin();
                selectedEntity < EDITOR_STATE->selectedEntities.end();
                selectedEntity++) {

            Level::EntityDestroy((Level::Entity *)*selectedEntity);
        }

    }
    else {
        Level::EntityDestroy(foundEntity);
    }

}

static void editorUseEraser(Vector2 cursorPos, int interactionTags) {

    (void)interactionTags;


    switch (GAME_STATE->mode) {
        
    case MODE_IN_LEVEL:
        editorUseEraserInLevel(cursorPos);
        break;
    
    case MODE_OVERWORLD:
        OverworldTileRemoveAt(cursorPos);
        break;
    }


    EditorSelectionCancel();
}

static void editorAutoTileSelection(bool isSlope) {

    if (GAME_STATE->mode != MODE_IN_LEVEL) {
        TraceLog(LOG_ERROR, "editorAutoTileSelection isn't implemented for game mode %d.", GAME_STATE->mode);
        return;
    }

    for (auto e : EDITOR_STATE->selectedEntities) {
        
        auto entity = (Level::Entity *) e;
        if (entity->tags & Level::IS_TILE_BLOCK) {
            
            auto block = (Block *) entity;
            block->TileAutoAdjust(isSlope);
        }
    }
}

static void editorAutoTileSelectionSquare() {
    editorAutoTileSelection(false);
}

static void editorAutoTileSelectionSlope() {
    editorAutoTileSelection(true);
}

static EditorEntityButton *addEntityButton(
    EditorEntityType type, Sprite *sprite, void (*handler)(Vector2, int)) {

        EditorEntityButton *newButton = new EditorEntityButton();
        newButton->type = type;
        newButton->handler = handler;
        newButton->sprite = sprite;

        LinkedList::AddNode(&EDITOR_STATE->entitiesHead, newButton);

        return newButton;
}

EditorControlButton *addControlButton(EditorControlType type, char *label, void (*handler)()) {

    EditorControlButton *newButton = new EditorControlButton();
    newButton->type = type;
    newButton->handler = handler;
    newButton->label = label;

    LinkedList::AddNode(&EDITOR_STATE->controlHead, newButton);

    return newButton;
}

void loadInLevelEditor() {

    addEntityButton(EDITOR_ENTITY_ERASER, &SPRITES->Eraser, &editorUseEraser);
    addEntityButton(EDITOR_ENTITY_ENEMY, &SPRITES->Enemy, &Enemy::AddFromEditor);
    EDITOR_STATE->defaultEntityButton =
        addEntityButton(EDITOR_ENTITY_BLOCK, &SPRITES->Block4Sides, &Block::AddFromEditor);
    addEntityButton(EDITOR_ENTITY_ACID, &SPRITES->Acid, &AcidBlock::AddFromEditor);   
    addEntityButton(EDITOR_ENTITY_EXIT, &SPRITES->LevelEndOrb, &Level::ExitAddFromEditor);
    addEntityButton(EDITOR_ENTITY_GLIDE, &SPRITES->GlideItem, &GlideAddFromEditor);
    addEntityButton(EDITOR_ENTITY_TEXTBOX, &SPRITES->TextboxButton, &Textbox::AddFromEditor);
    addEntityButton(EDITOR_ENTITY_CHECKPOINT_PICKUP, &SPRITES->LevelCheckpointPickup1, &CheckpointPickup::AddFromEditor);
    addEntityButton(EDITOR_ENTITY_MOVING_PLATFORM, &SPRITES->MovingPlatform, &MovingPlatform::AddFromEditor);
    addEntityButton(EDITOR_ENTITY_ENEMY, &SPRITES->EnemyDummySpike, &EnemyDummySpike::AddFromEditor);
    addEntityButton(EDITOR_ENTITY_NPC, &SPRITES->PrincessEditorIcon, &INpc::AddFromEditor);
    addEntityButton(EDITOR_ENTITY_COIN, &SPRITES->Coin1, &Coin::AddFromEditor);

    addControlButton(EDITOR_CONTROL_SAVE, (char *) "Salvar fase", &Level::Save);
    addControlButton(EDITOR_CONTROL_NEW_LEVEL, (char *) "Nova fase", &Level::LoadNew);
    addControlButton(EDITOR_CONTROL_AUTO_TILE, (char *) "Auto ladrilho", &editorAutoTileSelectionSquare);
    addControlButton(EDITOR_CONTROL_AUTO_TILE_SLOPE, (char *) "Auto lad. rampa", &editorAutoTileSelectionSlope);

    TraceLog(LOG_TRACE, "Editor loaded in level itens.");
}

void loadOverworldEditor() {

    addEntityButton(EDITOR_ENTITY_ERASER, &SPRITES->Eraser, &editorUseEraser);
    EDITOR_STATE->defaultEntityButton =
        addEntityButton(EDITOR_ENTITY_LEVEL_DOT, &SPRITES->LevelDot, &OverworldTileAddOrInteract);
    addEntityButton(EDITOR_ENTITY_PATH_JOIN, &SPRITES->PathTileJoin, &OverworldTileAddOrInteract);
    addEntityButton(EDITOR_ENTITY_STRAIGHT, &SPRITES->PathTileStraight, &OverworldTileAddOrInteract);
    addEntityButton(EDITOR_ENTITY_PATH_IN_L, &SPRITES->PathTileInL, &OverworldTileAddOrInteract);

    addControlButton(EDITOR_CONTROL_SAVE, (char *) "Salvar mundo", &OverworldSave);
    addControlButton(EDITOR_CONTROL_NEW_LEVEL, (char *) "Nova fase", &Level::LoadNew);

    TraceLog(LOG_TRACE, "Editor loaded overworld itens.");
}

// Updates the entities part of the current loaded selection,
// based on its origin and current cursor pos
static void updateEntitySelectionList() {

    EDITOR_STATE->selectedEntities.clear();
    EDITOR_STATE->isSelectionGridlocked = false;

    const Rectangle selectionHitbox = EditorSelectionGetRect();

    for (LinkedList::Node *node = GetEntityListHead();
        node != 0;
        node = node->next) {

        if (GAME_STATE->mode == MODE_IN_LEVEL) {

            Level::Entity *entity = (Level::Entity *) node;
            
            if (entity->tags & Level::IS_PLAYER) continue;

            if (entity->tags & Level::IS_MOVING_PLATFORM) {

                // Only the moving platform's anchors are checked for collision and go into the entity selection
                auto p = (MovingPlatform *) entity;
                if (CheckCollisionRecs(selectionHitbox, p->startAnchor.hitbox))
                    EDITOR_STATE->selectedEntities.push_back(&p->startAnchor);
                if (CheckCollisionRecs(selectionHitbox, p->endAnchor.hitbox))
                    EDITOR_STATE->selectedEntities.push_back(&p->endAnchor);

                continue;
            }

            // generic entity
            else if (!entity->IsDisabled() && CheckCollisionRecs(selectionHitbox, entity->hitbox)) {
                EDITOR_STATE->selectedEntities.push_back(entity);
                if (entity->tags & Level::IS_GRIDLOCKED) EDITOR_STATE->isSelectionGridlocked = true;
                continue;
            }            

            // generic entity's origin ghost
            else if (CheckCollisionRecs(selectionHitbox, entity->GetOriginHitbox())) {
                EDITOR_STATE->selectedEntities.push_back(entity);
                if (entity->tags & Level::IS_GRIDLOCKED) EDITOR_STATE->isSelectionGridlocked = true;
                continue;
            }

        }

        else if (GAME_STATE->mode == MODE_OVERWORLD) {
            
            OverworldEntity *entity = (OverworldEntity *) node;
            
            if (entity->tags & OW_IS_CURSOR) continue;

            if (CheckCollisionRecs(selectionHitbox, OverworldEntitySquare(entity))) {
                EDITOR_STATE->selectedEntities.push_back(entity);
                EDITOR_STATE->isSelectionGridlocked = true;
                continue;
            }
        }
    }
}

void selectEntitiesApplyMove() {

    // Searches for collision 
    for (auto node = EDITOR_STATE->selectedEntities.begin(); node < EDITOR_STATE->selectedEntities.end(); node++) {

        switch (GAME_STATE->mode) {

        case MODE_IN_LEVEL: {
            Level::Entity *entity = (Level::Entity *) *node;
            Rectangle hitbox = entity->hitbox;
            Vector2 pos = EditorEntitySelectionCalcMove(RectangleGetPos(hitbox));
            RectangleSetPos(&hitbox, pos);
            if (Level::CheckCollisionWithAnythingElse(hitbox, EDITOR_STATE->selectedEntities))
                return;
            break;
        }

        case MODE_OVERWORLD: {
            OverworldEntity *entity = (OverworldEntity *) *node;
            Rectangle hitbox = OverworldEntitySquare(entity);
            Vector2 pos = EditorEntitySelectionCalcMove(RectangleGetPos(hitbox));
            RectangleSetPos(&hitbox, pos);
            if (OverworldCheckCollisionWithAnyTileExcept(hitbox, EDITOR_STATE->selectedEntities))
                return;
            break;
        }

        }
    }

    // Apply move
    for (auto node = EDITOR_STATE->selectedEntities.begin(); node < EDITOR_STATE->selectedEntities.end(); node++) {
        
        switch (GAME_STATE->mode) {

        case MODE_IN_LEVEL: {
            Level::Entity *entity = (Level::Entity *) *node;
            entity->SetHitboxPos(EditorEntitySelectionCalcMove(RectangleGetPos(entity->hitbox)));
            entity->SetOrigin(EditorEntitySelectionCalcMove(entity->origin));
            break;
        }

        case MODE_OVERWORLD: {
            OverworldEntity *entity = (OverworldEntity *) *node;
            entity->gridPos = EditorEntitySelectionCalcMove(entity->gridPos);
            break;
        }

        }
    }
    
    EDITOR_STATE->entitySelectionCoords.start =
        EditorEntitySelectionCalcMove(EDITOR_STATE->entitySelectionCoords.start);
    EDITOR_STATE->entitySelectionCoords.end =
        EditorEntitySelectionCalcMove(EDITOR_STATE->entitySelectionCoords.end);

    TraceLog(LOG_TRACE, "Editor applied selected entities displacement.");
}

void EditorInitialize() {

    EDITOR_STATE = (EditorState *) MemAlloc(sizeof(EditorState));
    EDITOR_STATE->selectedEntities = std::vector<LinkedList::Node *>();

    editorStateReset();

    TraceLog(LOG_INFO, "Editor initialized.");
}

void EditorSync() {
    
    editorStateReset();

    switch (GAME_STATE->mode) {
    
    case MODE_IN_LEVEL:
        loadInLevelEditor();
        break;

    case MODE_OVERWORLD:
        loadOverworldEditor();
        break;

    default:
        TraceLog(LOG_ERROR, "Could not find editor items list for game mode %d.", GAME_STATE->mode);
        return;
    }

    buttonsSelectDefault();
}

void EditorEmpty() {

    LinkedList::DestroyAll(&EDITOR_STATE->entitiesHead);
    LinkedList::DestroyAll(&EDITOR_STATE->controlHead);
    EDITOR_STATE->defaultEntityButton = 0;
    EDITOR_STATE->toggledEntityButton = 0;

    TraceLog(LOG_TRACE, "Editor emptied.");
}

void EditorEnable() {

    EDITOR_STATE->isEnabled = true;

    MouseCursorEnable();

    buttonsSelectDefault();

    TraceLog(LOG_TRACE, "Editor enabled.");
}

void EditorDisable() {

    EDITOR_STATE->isEnabled = false;

    MouseCursorDisable();

    CameraPanningReset();

    EditorSelectionCancel();

    TraceLog(LOG_TRACE, "Editor disabled.");
}

void EditorEnabledToggle() {

    if (EDITOR_STATE->isEnabled) EditorDisable();
    else EditorEnable();
}

Rectangle EditorBarGetRect() {
    return { (float) GetScreenWidth() - EDITOR_BAR_WIDTH,
                0,
                EDITOR_BAR_WIDTH,
                (float) GetScreenHeight() };   
}

float EditorBarGetDivisorY() {
    return EDITOR_BAR_DIVISION * GetScreenHeight();
}

void EditorTick() {

    EditorState *s = EDITOR_STATE;

    // Entity selection
    if (s->isSelectingEntities) {
        if (s->selectedEntitiesThisFrame)
            updateEntitySelectionList();
        else {
            s->isSelectingEntities = false;
            buttonsSelectDefault();
        }
    }
    s->selectedEntitiesThisFrame = false;

    // Moving selected entities
    if (s->isMovingSelectedEntities && !s->movedEntitiesThisFrame) {
        selectEntitiesApplyMove();
        s->isMovingSelectedEntities = false;
    }
    s->movedEntitiesThisFrame = false;
}

void EditorEntityButtonSelect(EditorEntityButton *item) {

    EDITOR_STATE->toggledEntityButton = item;
}

void EditorSelectEntities(Vector2 cursorPos) {

    EditorState *s = EDITOR_STATE;

    if (!s->isSelectingEntities) {
        s->entitySelectionCoords.start = cursorPos;
        s->isSelectionGridlocked = false;
    }
    
    s->entitySelectionCoords.end = cursorPos;
    s->isSelectingEntities = true;
    s->selectedEntitiesThisFrame = true;
}

void EditorSelectionCancel() {

    EDITOR_STATE->isSelectingEntities = false;
    EDITOR_STATE->selectedEntitiesThisFrame = false;
    EDITOR_STATE->isMovingSelectedEntities = false;
    EDITOR_STATE->isSelectionGridlocked = false;
    EDITOR_STATE->selectedEntities.clear();

    TraceLog(LOG_TRACE, "Editor's entity selection canceled.");
}

bool EditorSelectedEntitiesMove(Vector2 cursorPos) {

    EditorState *s = EDITOR_STATE;

    if (s->selectedEntities.empty()) {
        TraceLog(LOG_ERROR,
                    "Editor tried to check selection move, but there are no entities selected.");
        return false;
    }

    if (!s->isMovingSelectedEntities) {

        bool clickedOnASelectedEntity = false;

        for (auto e = s->selectedEntities.begin(); e != s->selectedEntities.end(); e++) {

            // Overworld

            if (GAME_STATE->mode == MODE_OVERWORLD) {

                if (CheckCollisionPointRec(cursorPos, OverworldEntitySquare((OverworldEntity *) *e))) {

                    clickedOnASelectedEntity = true; break;
                }

                continue;
            }

            // In level

            auto entity = (Level::Entity *) *e;

            if (entity->tags & Level::IS_MOVING_PLATFORM) {
                auto p = (MovingPlatform *) entity;

                if (CheckCollisionPointRec(cursorPos, p->startAnchor.hitbox) ||
                    CheckCollisionPointRec(cursorPos, p->endAnchor.hitbox)) {

                        clickedOnASelectedEntity = true; break;
                }
            }
            
            // Generic entity
            else if ((CheckCollisionPointRec(cursorPos, entity->hitbox) && !entity->IsDisabled()) ||
                    CheckCollisionPointRec(cursorPos, entity->GetOriginHitbox())) {
                        
                        clickedOnASelectedEntity = true; break;
            }
        }

        if (!clickedOnASelectedEntity) return false;

        s->selectedEntitiesMoveCoords.start = cursorPos;
    }

    s->selectedEntitiesMoveCoords.end = cursorPos;
    s->isMovingSelectedEntities = true;
    s->movedEntitiesThisFrame = true;

    return true;
}

Rectangle EditorEntityButtonRect(int buttonNumber) {

    const Rectangle drawSpace   = EditorBarGetRect();
    const int buttonSize        = ENTITY_BUTTON_SIZE;
    const int buttonSpacing     = ENTITY_BUTTON_SPACING;
    const int wallSpacing       = (drawSpace.width - (ENTITY_BUTTON_SIZE * 2) - ENTITY_BUTTON_SPACING) / 2;

    float itemX = drawSpace.x + wallSpacing;
    if (buttonNumber % 2) itemX += buttonSize + buttonSpacing;

    float itemY = drawSpace.y + wallSpacing;
    itemY += (buttonSize + buttonSpacing) * (buttonNumber / 2);

    return { itemX, itemY, buttonSize, buttonSize };
}

Rectangle EditorControlButtonRect(int buttonNumber) {

    Rectangle drawSpace         = EditorBarGetRect();
    drawSpace.y                 += EditorBarGetDivisorY();

    const int buttonWidth       = CONTROL_BUTTON_WIDTH;
    const int buttonHeight      = CONTROL_BUTTON_HEIGHT;
    const int buttonSpacing     = CONTROL_BUTTON_SPACING;
    const int wallSpacing       = (drawSpace.width - (CONTROL_BUTTON_WIDTH * 2) - CONTROL_BUTTON_SPACING) / 2;

    float itemX = drawSpace.x + wallSpacing;
    if (buttonNumber % 2) itemX += buttonWidth + buttonSpacing;

    float itemY = drawSpace.y + wallSpacing;
    itemY += (buttonHeight + buttonSpacing) * (buttonNumber / 2);

    return { itemX, itemY, buttonWidth, buttonHeight };
}

Rectangle EditorSelectionGetRect() {

    Trajectory t = EDITOR_STATE->entitySelectionCoords;

    float x         = t.start.x;
    float y         = t.start.y;
    float width     = t.end.x - t.start.x;
    float height    = t.end.y - t.start.y;

    if (t.end.x < t.start.x) {
        x       = t.end.x;
        width   = t.start.x - t.end.x;
    }

    if (t.end.y < t.start.y) {
        y       = t.end.y;
        height  = t.start.y - t.end.y;
    }

    return { x, y, width, height };
}

Vector2 EditorEntitySelectionCalcMove(Vector2 hitbox) {

    Trajectory t = EDITOR_STATE->selectedEntitiesMoveCoords;
    
    Vector2 delta = {
        t.end.x - t.start.x,
        t.end.y - t.start.y,
    };

    Vector2 pos = {
                    hitbox.x + delta.x,
                    hitbox.y + delta.y                            
                };


    if (EDITOR_STATE->isSelectionGridlocked) {

        Dimensions grid = LEVEL_GRID;
        if (GAME_STATE->mode == MODE_OVERWORLD) grid = OW_GRID;

        pos = SnapToGrid(pos, grid);
    }

    return pos;
}
