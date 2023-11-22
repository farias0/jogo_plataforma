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
EditorSelection *EDITOR_ENTITY_SELECTION = 0;

static bool selectedEntitiesThisFrame = false;


EditorEntityItem *loadEditorEntityItem(
    EditorEntityType type, Sprite sprite, void (*handler), EditorItemInteraction interaction) {

        EditorEntityItem *newItem = MemAlloc(sizeof(EditorEntityItem));
        newItem->type = type;
        newItem->handler = handler;
        newItem->sprite = sprite;
        newItem->interaction = interaction;

        LinkedListAdd(&EDITOR_ENTITIES_HEAD, newItem);

        return newItem;
}

EditorControlItem *loadEditorControlItem(EditorControlType type, char *label, void (*handler)) {

    EditorControlItem *newItem = MemAlloc(sizeof(EditorControlItem));
    newItem->type = type;
    newItem->handler = handler;
    newItem->label = label;

    LinkedListAdd(&EDITOR_CONTROL_HEAD, newItem);

    return newItem;
}

void loadInLevelEditor() {

    loadEditorEntityItem(EDITOR_ENTITY_ERASER, EraserSprite, &LevelEntityRemoveAt, EDITOR_INTERACTION_HOLD);
    loadEditorEntityItem(EDITOR_ENTITY_ENEMY, EnemySprite, &LevelEnemyCheckAndAdd, EDITOR_INTERACTION_CLICK);
    STATE->editorSelectedEntity =
        loadEditorEntityItem(EDITOR_ENTITY_BLOCK, BlockSprite, &LevelBlockCheckAndAdd, EDITOR_INTERACTION_HOLD);
    loadEditorEntityItem(EDITOR_ENTITY_ACID, AcidSprite, &LevelAcidCheckAndAdd, EDITOR_INTERACTION_HOLD);   
    loadEditorEntityItem(EDITOR_ENTITY_EXIT, LevelEndOrbSprite, &LevelExitCheckAndAdd, EDITOR_INTERACTION_CLICK);

    loadEditorControlItem(EDITOR_CONTROL_SAVE, "Salvar fase", &LevelSave);
    loadEditorControlItem(EDITOR_CONTROL_NEW_LEVEL, "Nova fase", &LevelLoadNew);

    TraceLog(LOG_TRACE, "Editor loaded in level itens.");
}

void loadOverworldEditor() {

    loadEditorEntityItem(EDITOR_ENTITY_ERASER, EraserSprite, &OverworldTileRemoveAt, EDITOR_INTERACTION_HOLD);
    STATE->editorSelectedEntity =
        loadEditorEntityItem(EDITOR_ENTITY_LEVEL_DOT, LevelDotSprite, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);
    loadEditorEntityItem(EDITOR_ENTITY_PATH_JOIN, PathTileJoinSprite, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);
    loadEditorEntityItem(EDITOR_ENTITY_STRAIGHT, PathTileStraightSprite, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);
    loadEditorEntityItem(EDITOR_ENTITY_PATH_IN_L, PathTileInLSprite, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);

    loadEditorControlItem(EDITOR_CONTROL_SAVE, "Salvar mundo", &OverworldSave);
    loadEditorControlItem(EDITOR_CONTROL_NEW_LEVEL, "Nova fase", &LevelLoadNew);

    TraceLog(LOG_TRACE, "Editor loaded overworld itens.");
}

static void updateEntitySelectionList() {

    if (!EDITOR_ENTITY_SELECTION) {
        TraceLog(LOG_ERROR, "Entity selection list tried to update, but there's no reference to selection.");
        return;
    }

    LinkedListRemoveAll(&EDITOR_ENTITY_SELECTION->entitiesHead);

    Rectangle selectionHitbox = EditorSelectionGetRect();

    ListNode *node = GetEntityListHead();
    while (node != 0) {

        if (STATE->mode == MODE_IN_LEVEL) {
            LevelEntity *entity = (LevelEntity *) node->item;
            if ((!entity->isDead && CheckCollisionRecs(selectionHitbox, entity->hitbox)) ||
                CheckCollisionRecs(selectionHitbox, LevelEntityOriginHitbox(entity))) {
                
                    LinkedListAdd(&EDITOR_ENTITY_SELECTION->entitiesHead, entity);
            }
        }
        else if (STATE->mode == MODE_OVERWORLD) {
            OverworldEntity *entity = (OverworldEntity *) node->item;
            if (CheckCollisionRecs(selectionHitbox, OverworldEntitySquare(entity))) {
                
                LinkedListAdd(&EDITOR_ENTITY_SELECTION->entitiesHead, entity);
            }
        }

        node = node->next;
    }
}

void EditorSync() {

    LinkedListDestroyAll(&EDITOR_ENTITIES_HEAD);
    LinkedListDestroyAll(&EDITOR_CONTROL_HEAD);
    
    EditorSelectionCancel();

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

    STATE->editorSelectedEntity = 0;

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

    // Entity selection
    if (EDITOR_ENTITY_SELECTION) {
        if (selectedEntitiesThisFrame)
            updateEntitySelectionList();
        else
            EDITOR_ENTITY_SELECTION->isSelecting = false;
    }
    selectedEntitiesThisFrame = false;
}

void EditorSelectEntities(Vector2 cursorPos) {

    EditorSelection *s = EDITOR_ENTITY_SELECTION;

    if (!s) {
        EDITOR_ENTITY_SELECTION = MemAlloc(sizeof(EditorSelection));
        s = EDITOR_ENTITY_SELECTION;
        s->origin = cursorPos;
    }

    if (!s->isSelecting) {
        s->origin = cursorPos;
    }
    
    s->current = cursorPos;
    s->isSelecting = true;
    selectedEntitiesThisFrame = true;
}

void EditorSelectionCancel() {
    
    if (EDITOR_ENTITY_SELECTION) {
        LinkedListRemoveAll(&EDITOR_ENTITY_SELECTION->entitiesHead);
        MemFree(EDITOR_ENTITY_SELECTION);
        EDITOR_ENTITY_SELECTION = 0;
    }

    TraceLog(LOG_TRACE, "Editor's entity selection canceled.");
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

    EditorSelection *s = EDITOR_ENTITY_SELECTION;

    float x         = s->origin.x;
    float y         = s->origin.y;
    float width     = s->current.x - s->origin.x;
    float height    = s->current.y - s->origin.y;

    if (s->current.x < s->origin.x) {
        x       = s->current.x;
        width   = s->origin.x - s->current.x;
    }

    if (s->current.y < s->origin.y) {
        y       = s->current.y;
        height  = s->origin.y - s->current.y;
    }

    return (Rectangle) { x, y, width, height };
}
