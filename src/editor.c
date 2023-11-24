#include <raylib.h>

#include "editor.h"
#include "core.h"
#include "level/level.h"
#include "overworld.h"
#include "linked_list.h"
#include "camera.h"
#include "render.h"


ListNode *EDITOR_ENTITIES_HEAD = 0;
ListNode *EDITOR_CONTROL_HEAD = 0;
EditorState *EDITOR_STATE = 0;


static void editorUseEraser(Vector2 cursorPos) {

    switch (STATE->mode) {
        
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
    EditorEntityType type, Sprite sprite, void (*handler), EditorInteractionType interaction) {

        EditorEntityButton *newButton = MemAlloc(sizeof(EditorEntityButton));
        newButton->type = type;
        newButton->handler = handler;
        newButton->sprite = sprite;
        newButton->interactionType = interaction;

        LinkedListAdd(&EDITOR_ENTITIES_HEAD, newButton);

        return newButton;
}

EditorControlButton *addControlButton(EditorControlType type, char *label, void (*handler)) {

    EditorControlButton *newButton = MemAlloc(sizeof(EditorControlButton));
    newButton->type = type;
    newButton->handler = handler;
    newButton->label = label;

    LinkedListAdd(&EDITOR_CONTROL_HEAD, newButton);

    return newButton;
}

void loadInLevelEditor() {

    addEntityButton(EDITOR_ENTITY_ERASER, EraserSprite, &editorUseEraser, EDITOR_INTERACTION_HOLD);
    addEntityButton(EDITOR_ENTITY_ENEMY, EnemySprite, &LevelEnemyCheckAndAdd, EDITOR_INTERACTION_CLICK);
    STATE->editorButtonToggled =
        addEntityButton(EDITOR_ENTITY_BLOCK, BlockSprite, &LevelBlockCheckAndAdd, EDITOR_INTERACTION_HOLD);
    addEntityButton(EDITOR_ENTITY_ACID, AcidSprite, &LevelAcidCheckAndAdd, EDITOR_INTERACTION_HOLD);   
    addEntityButton(EDITOR_ENTITY_EXIT, LevelEndOrbSprite, &LevelExitCheckAndAdd, EDITOR_INTERACTION_CLICK);

    addControlButton(EDITOR_CONTROL_SAVE, "Salvar fase", &LevelSave);
    addControlButton(EDITOR_CONTROL_NEW_LEVEL, "Nova fase", &LevelLoadNew);

    TraceLog(LOG_TRACE, "Editor loaded in level itens.");
}

void loadOverworldEditor() {

    addEntityButton(EDITOR_ENTITY_ERASER, EraserSprite, &editorUseEraser, EDITOR_INTERACTION_HOLD);
    STATE->editorButtonToggled =
        addEntityButton(EDITOR_ENTITY_LEVEL_DOT, LevelDotSprite, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);
    addEntityButton(EDITOR_ENTITY_PATH_JOIN, PathTileJoinSprite, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);
    addEntityButton(EDITOR_ENTITY_STRAIGHT, PathTileStraightSprite, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);
    addEntityButton(EDITOR_ENTITY_PATH_IN_L, PathTileInLSprite, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);

    addControlButton(EDITOR_CONTROL_SAVE, "Salvar mundo", &OverworldSave);
    addControlButton(EDITOR_CONTROL_NEW_LEVEL, "Nova fase", &LevelLoadNew);

    TraceLog(LOG_TRACE, "Editor loaded overworld itens.");
}

// Updates the entities part of the current loaded selection,
// based on its origin and current cursor pos
static void updateEntitySelectionList() {

    LinkedListRemoveAll(&EDITOR_STATE->selectedEntities);

    Rectangle selectionHitbox = EditorSelectionGetRect();

    ListNode *node = GetEntityListHead();
    while (node != 0) {

        if (STATE->mode == MODE_IN_LEVEL) {
            LevelEntity *entity = (LevelEntity *) node->item;
            bool collisionWithEntity = !entity->isDead && CheckCollisionRecs(selectionHitbox, entity->hitbox);
            bool collisionWithGhost = CheckCollisionRecs(selectionHitbox, LevelEntityOriginHitbox(entity));
            if (collisionWithEntity || collisionWithGhost) {
                
                LinkedListAdd(&EDITOR_STATE->selectedEntities, entity);
            }
        }
        else if (STATE->mode == MODE_OVERWORLD) {
            OverworldEntity *entity = (OverworldEntity *) node->item;
            if (CheckCollisionRecs(selectionHitbox, OverworldEntitySquare(entity))) {
                
                LinkedListAdd(&EDITOR_STATE->selectedEntities, entity);
            }
        }

        node = node->next;
    }
}

void EditorInitialize() {

    EDITOR_STATE = MemAlloc(sizeof(EditorState));

    EditorStateReset();

    TraceLog(LOG_INFO, "Editor initialized.");
}

void EditorStateReset() {

    EditorSelectionCancel();

    LinkedListDestroyAll(&EDITOR_ENTITIES_HEAD);
    LinkedListDestroyAll(&EDITOR_CONTROL_HEAD);

    TraceLog(LOG_DEBUG, "Editor state reset.");
}

void EditorSync() {
    
    EditorStateReset();

    switch (STATE->mode) {
    
    case MODE_IN_LEVEL:
        loadInLevelEditor();
        break;

    case MODE_OVERWORLD:
        loadOverworldEditor();
        break;

    default:
        TraceLog(LOG_ERROR, "Could not find editor items list for game mode %d.", STATE->mode);
    }
}

void EditorEmpty() {

    LinkedListDestroyAll(&EDITOR_ENTITIES_HEAD);
    LinkedListDestroyAll(&EDITOR_CONTROL_HEAD);

    STATE->editorButtonToggled = 0;

    TraceLog(LOG_TRACE, "Editor emptied.");
}

void EditorEnable() {

    STATE->isEditorEnabled = true;

    RenderResizeWindow(SCREEN_WIDTH_W_EDITOR, SCREEN_HEIGHT);
    MouseCursorEnable();

    TraceLog(LOG_TRACE, "Editor enabled.");
}

void EditorDisable() {

    STATE->isEditorEnabled = false;

    RenderResizeWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
    MouseCursorDisable();

    CameraPanningReset();

    EditorSelectionCancel();

    TraceLog(LOG_TRACE, "Editor disabled.");
}

void EditorEnabledToggle() {

    if (STATE->isEditorEnabled) EditorDisable();
    else EditorEnable();
}

void EditorTick() {

    EditorState *s = EDITOR_STATE;

    // Entity selection
    if (s->isSelectingEntities) {
        if (s->selectedEntitiesThisFrame)
            updateEntitySelectionList();
        else
            s->isSelectingEntities = false;
    }
    s->selectedEntitiesThisFrame = false;

    // Moving selected entities
    if (s->isMovingSelectedEntities && !s->movedEntitiesThisFrame) {
        // TODO apply displacement to entities
        TraceLog(LOG_INFO, "test -- stopped moving entity"); // TODO remove it
        s->isMovingSelectedEntities = false;
    }
    s->movedEntitiesThisFrame = false;
}

void EditorEntityButtonSelect(EditorEntityButton *item) {

    STATE->editorButtonToggled = item;
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

    if (EDITOR_STATE) {
        
        EDITOR_STATE->isSelectingEntities = false;
        EDITOR_STATE->selectedEntitiesThisFrame = false;
        EDITOR_STATE->isMovingSelectedEntities = false;
        LinkedListRemoveAll(&EDITOR_STATE->selectedEntities);
    }

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

        s->selectedEntitiesDisplacement.start = cursorPos;
    }

    s->selectedEntitiesDisplacement.end = cursorPos;
    s->isMovingSelectedEntities = true;
    s->movedEntitiesThisFrame = true;

    return true;
}

Rectangle EditorEntityButtonRect(int buttonNumber) {

    const Rectangle area        = EDITOR_ENTITIES_AREA;
    const int buttonSize        = ENTITY_BUTTON_SIZE;
    const int buttonSpacing     = ENTITY_BUTTON_SPACING;
    const int wallSpacing       = ENTITY_BUTTON_WALL_SPACING;

    float itemX = area.x + wallSpacing;
    if (buttonNumber % 2) itemX += buttonSize + buttonSpacing;

    float itemY = area.y + wallSpacing;
    itemY += (buttonSize + buttonSpacing) * (buttonNumber / 2);

    return (Rectangle){ itemX, itemY, buttonSize, buttonSize };
}

Rectangle EditorControlButtonRect(int buttonNumber) {

    const Rectangle area        = EDITOR_CONTROL_AREA;
    const int buttonWidth       = CONTROL_BUTTON_WIDTH;
    const int buttonHeight      = CONTROL_BUTTON_HEIGHT;
    const int buttonSpacing     = CONTROL_BUTTON_SPACING;
    const int wallSpacing       = CONTROL_BUTTON_WALL_SPACING;

    float itemX = area.x + wallSpacing;
    if (buttonNumber % 2) itemX += buttonWidth + buttonSpacing;

    float itemY = area.y + wallSpacing;
    itemY += (buttonHeight + buttonSpacing) * (buttonNumber / 2);

    return (Rectangle){ itemX, itemY, buttonWidth, buttonHeight };
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

    return (Rectangle) { x, y, width, height };
}
