#include <raylib.h>

#include "editor.hpp"
#include "core.hpp"
#include "level/level.hpp"
#include "level/enemy.hpp"
#include "level/powerups.hpp"
#include "level/block.hpp"
#include "overworld.hpp"
#include "linked_list.hpp"
#include "camera.hpp"
#include "render.hpp"


EditorState *EDITOR_STATE = 0;


static void editorStateReset() {

    EditorSelectionCancel();

    EditorEmpty();

    TraceLog(LOG_DEBUG, "Editor state reset.");
}

static void buttonsSelectDefault() {
    EDITOR_STATE->toggledEntityButton = EDITOR_STATE->defaultEntityButton;
}

static void editorUseEraser(Vector2 cursorPos) {

    switch (GAME_STATE->mode) {
        
    case MODE_IN_LEVEL:
        LevelEntityRemoveAt(cursorPos);
        break;
    
    case MODE_OVERWORLD:
        OverworldTileRemoveAt(cursorPos);
        break;
    }


    EditorSelectionCancel();
}

static EditorEntityButton *addEntityButton(
    EditorEntityType type, Sprite sprite, void (*handler)(Vector2), EditorInteractionType interaction) {

        EditorEntityButton *newButton = (EditorEntityButton *) MemAlloc(sizeof(EditorEntityButton));
        newButton->type = type;
        newButton->handler = handler;
        newButton->sprite = sprite;
        newButton->interactionType = interaction;

        LinkedListAdd(&EDITOR_STATE->entitiesHead, newButton);

        return newButton;
}

EditorControlButton *addControlButton(EditorControlType type, char *label, void (*handler)()) {

    EditorControlButton *newButton = (EditorControlButton *) MemAlloc(sizeof(EditorControlButton));
    newButton->type = type;
    newButton->handler = handler;
    newButton->label = label;

    LinkedListAdd(&EDITOR_STATE->controlHead, newButton);

    return newButton;
}

void loadInLevelEditor() {

    addEntityButton(EDITOR_ENTITY_ERASER, SPRITES->Eraser, &editorUseEraser, EDITOR_INTERACTION_HOLD);
    addEntityButton(EDITOR_ENTITY_ENEMY, SPRITES->Enemy, &EnemyCheckAndAdd, EDITOR_INTERACTION_CLICK);
    EDITOR_STATE->defaultEntityButton =
        addEntityButton(EDITOR_ENTITY_BLOCK, SPRITES->Block, &BlockCheckAndAdd, EDITOR_INTERACTION_HOLD);
    addEntityButton(EDITOR_ENTITY_ACID, SPRITES->Acid, &AcidCheckAndAdd, EDITOR_INTERACTION_HOLD);   
    addEntityButton(EDITOR_ENTITY_EXIT, SPRITES->LevelEndOrb, &LevelExitCheckAndAdd, EDITOR_INTERACTION_CLICK);
    addEntityButton(EDITOR_ENTITY_GLIDE, SPRITES->GlideItem, &GlideCheckAndAdd, EDITOR_INTERACTION_CLICK);

    addControlButton(EDITOR_CONTROL_SAVE, (char *) "Salvar fase", &LevelSave);
    addControlButton(EDITOR_CONTROL_NEW_LEVEL, (char *) "Nova fase", &LevelLoadNew);

    TraceLog(LOG_TRACE, "Editor loaded in level itens.");
}

void loadOverworldEditor() {

    addEntityButton(EDITOR_ENTITY_ERASER, SPRITES->Eraser, &editorUseEraser, EDITOR_INTERACTION_HOLD);
    EDITOR_STATE->defaultEntityButton =
        addEntityButton(EDITOR_ENTITY_LEVEL_DOT, SPRITES->LevelDot, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);
    addEntityButton(EDITOR_ENTITY_PATH_JOIN, SPRITES->PathTileJoin, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);
    addEntityButton(EDITOR_ENTITY_STRAIGHT, SPRITES->PathTileStraight, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);
    addEntityButton(EDITOR_ENTITY_PATH_IN_L, SPRITES->PathTileInL, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);

    addControlButton(EDITOR_CONTROL_SAVE, (char *) "Salvar mundo", &OverworldSave);
    addControlButton(EDITOR_CONTROL_NEW_LEVEL, (char *) "Nova fase", &LevelLoadNew);

    TraceLog(LOG_TRACE, "Editor loaded overworld itens.");
}

// Updates the entities part of the current loaded selection,
// based on its origin and current cursor pos
static void updateEntitySelectionList() {

    LinkedListRemoveAll(&EDITOR_STATE->selectedEntities);

    Rectangle selectionHitbox = EditorSelectionGetRect();

    ListNode *node = GetEntityListHead();
    while (node != 0) {

        if (GAME_STATE->mode == MODE_IN_LEVEL) {
            LevelEntity *entity = (LevelEntity *) node->item;
            
            if (entity->components & LEVEL_IS_PLAYER) goto next_entity;

            bool collisionWithEntity = !entity->isDead && CheckCollisionRecs(selectionHitbox, entity->hitbox);
            bool collisionWithGhost = CheckCollisionRecs(selectionHitbox, LevelEntityOriginHitbox(entity));
            if (collisionWithEntity || collisionWithGhost) {
                
                LinkedListAdd(&EDITOR_STATE->selectedEntities, entity);
            }
        }
        else if (GAME_STATE->mode == MODE_OVERWORLD) {
            
            OverworldEntity *entity = (OverworldEntity *) node->item;
            
            if (entity->components & OW_IS_CURSOR) goto next_entity;

            if (CheckCollisionRecs(selectionHitbox, OverworldEntitySquare(entity))) {
                
                LinkedListAdd(&EDITOR_STATE->selectedEntities, entity);
            }
        }

next_entity:
        node = node->next;
    }
}

void selectEntitiesApplyMove() {

    // Searches for collision 
    ListNode *node = EDITOR_STATE->selectedEntities;
    while (node) {

        switch (GAME_STATE->mode) {

        case MODE_IN_LEVEL: {
            LevelEntity *entity = (LevelEntity *) node->item;
            Rectangle hitbox = entity->hitbox;
            Vector2 pos = EditorEntitySelectionCalcMove(RectangleGetPos(hitbox));
            RectangleSetPos(&hitbox, pos);
            if (LevelCheckCollisionWithAnythingElse(hitbox, EDITOR_STATE->selectedEntities))
                return;
            break;
        }

        case MODE_OVERWORLD: {
            OverworldEntity *entity = (OverworldEntity *) node->item;
            Rectangle hitbox = OverworldEntitySquare(entity);
            Vector2 pos = EditorEntitySelectionCalcMove(RectangleGetPos(hitbox));
            RectangleSetPos(&hitbox, pos);
            if (OverworldCheckCollisionWithAnyTileExcept(hitbox, EDITOR_STATE->selectedEntities))
                return;
            break;
        }

        }
        node = node->next;
    }

    // Apply move
    Vector2 newPos;
    node = EDITOR_STATE->selectedEntities;

    while (node) {
        
        switch (GAME_STATE->mode) {

        case MODE_IN_LEVEL: {
            LevelEntity *entity = (LevelEntity *) node->item;
            newPos = EditorEntitySelectionCalcMove(RectangleGetPos(entity->hitbox));
            RectangleSetPos(&entity->hitbox, newPos);
            entity->origin = EditorEntitySelectionCalcMove(entity->origin);
            break;
        }

        case MODE_OVERWORLD: {
            OverworldEntity *entity = (OverworldEntity *) node->item;
            entity->gridPos = EditorEntitySelectionCalcMove(entity->gridPos);
            break;
        }

        }
        node = node->next;
    }
    
    EDITOR_STATE->entitySelectionCoords.start =
        EditorEntitySelectionCalcMove(EDITOR_STATE->entitySelectionCoords.start);
    EDITOR_STATE->entitySelectionCoords.end =
        EditorEntitySelectionCalcMove(EDITOR_STATE->entitySelectionCoords.end);

    TraceLog(LOG_TRACE, "Editor applied selected entities displacement.");
}

void EditorInitialize() {

    EDITOR_STATE = (EditorState *) MemAlloc(sizeof(EditorState));

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

    LinkedListDestroyAll(&EDITOR_STATE->entitiesHead);
    LinkedListDestroyAll(&EDITOR_STATE->controlHead);
    EDITOR_STATE->defaultEntityButton = 0;
    EDITOR_STATE->toggledEntityButton = 0;

    TraceLog(LOG_TRACE, "Editor emptied.");
}

void EditorEnable() {

    EDITOR_STATE->isEnabled = true;

    RenderResizeWindow(SCREEN_WIDTH_W_EDITOR, SCREEN_HEIGHT);
    MouseCursorEnable();

    buttonsSelectDefault();

    TraceLog(LOG_TRACE, "Editor enabled.");
}

void EditorDisable() {

    EDITOR_STATE->isEnabled = false;

    RenderResizeWindow(SCREEN_WIDTH, SCREEN_HEIGHT);

    MouseCursorDisable();

    CameraPanningReset();

    EditorSelectionCancel();

    TraceLog(LOG_TRACE, "Editor disabled.");
}

void EditorEnabledToggle() {

    if (EDITOR_STATE->isEnabled) EditorDisable();
    else EditorEnable();
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
    }
    
    s->entitySelectionCoords.end = cursorPos;
    s->isSelectingEntities = true;
    s->selectedEntitiesThisFrame = true;
}

void EditorSelectionCancel() {

    EDITOR_STATE->isSelectingEntities = false;
    EDITOR_STATE->selectedEntitiesThisFrame = false;
    EDITOR_STATE->isMovingSelectedEntities = false;
    LinkedListRemoveAll(&EDITOR_STATE->selectedEntities);

    TraceLog(LOG_TRACE, "Editor's entity selection canceled.");
}

bool EditorSelectedEntitiesMove(Vector2 cursorPos) {

    EditorState *s = EDITOR_STATE;

    if (!s->selectedEntities) {
        TraceLog(LOG_ERROR,
                    "Editor tried to check selection move, but there are no entities selected.");
        return false;
    }

    if (!s->isMovingSelectedEntities) {

        // Should actually check for collision with each selected entity,
        // but this is good enough for now
        if (!CheckCollisionPointRec(cursorPos, EditorSelectionGetRect())) {
            return false;
        }

        s->selectedEntitiesMoveCoords.start = cursorPos;
    }

    s->selectedEntitiesMoveCoords.end = cursorPos;
    s->isMovingSelectedEntities = true;
    s->movedEntitiesThisFrame = true;

    return true;
}

Rectangle EditorEntityButtonRect(int buttonNumber) {

    const Vector2 panelOrigin   = RectangleGetPos(EDITOR_PANEL_RECT);
    const int buttonSize        = ENTITY_BUTTON_SIZE;
    const int buttonSpacing     = ENTITY_BUTTON_SPACING;
    const int wallSpacing       = ENTITY_BUTTON_WALL_SPACING;

    float itemX = panelOrigin.x + wallSpacing;
    if (buttonNumber % 2) itemX += buttonSize + buttonSpacing;

    float itemY = panelOrigin.y + wallSpacing;
    itemY += (buttonSize + buttonSpacing) * (buttonNumber / 2);

    return { itemX, itemY, buttonSize, buttonSize };
}

Rectangle EditorControlButtonRect(int buttonNumber) {

    Vector2 panelOrigin         = RectangleGetPos(EDITOR_PANEL_RECT);
    panelOrigin.y += EDITOR_CONTROL_PANEL_Y;

    const int buttonWidth       = CONTROL_BUTTON_WIDTH;
    const int buttonHeight      = CONTROL_BUTTON_HEIGHT;
    const int buttonSpacing     = CONTROL_BUTTON_SPACING;
    const int wallSpacing       = CONTROL_BUTTON_WALL_SPACING;

    float itemX = panelOrigin.x + wallSpacing;
    if (buttonNumber % 2) itemX += buttonWidth + buttonSpacing;

    float itemY = panelOrigin.y + wallSpacing;
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

    Dimensions grid = LEVEL_GRID;
    if (GAME_STATE->mode == MODE_OVERWORLD) grid = OW_GRID;
    
    Vector2 pos = SnapToGrid({
                                hitbox.x + delta.x,
                                hitbox.y + delta.y                            
                            }, grid);

    return pos;
}