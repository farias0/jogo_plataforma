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
    STATE->editorSelectedEntity =
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

void EditorSync() {

    LinkedListDestroyAll(&EDITOR_ENTITIES_HEAD);
    LinkedListDestroyAll(&EDITOR_CONTROL_HEAD);

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

    TraceLog(LOG_TRACE, "Editor disabled.");
}

void EditorEnabledToggle() {

    if (STATE->isEditorEnabled) EditorDisable();
    else EditorEnable();
}

Rectangle EditorEntityButtonRect(int buttonNumber) {

    float itemX = EDITOR_ENTITIES_AREA.x + ENTITY_BUTTON_WALL_SPACING;
    if (buttonNumber % 2) itemX += ENTITY_BUTTON_SIZE + ENTITY_BUTTON_SPACING;

    float itemY = EDITOR_ENTITIES_AREA.y + ENTITY_BUTTON_WALL_SPACING;
    itemY += (ENTITY_BUTTON_SIZE + ENTITY_BUTTON_SPACING) * (buttonNumber / 2);

    return (Rectangle){ itemX, itemY, ENTITY_BUTTON_SIZE, ENTITY_BUTTON_SIZE };
}

Rectangle EditorControlButtonRect(int buttonNumber) {

    float itemX = EDITOR_CONTROL_AREA.x + CONTROL_BUTTON_WALL_SPACING;
    if (buttonNumber % 2) itemX += CONTROL_BUTTON_WIDTH + CONTROL_BUTTON_SPACING;

    float itemY = EDITOR_CONTROL_AREA.y + CONTROL_BUTTON_WALL_SPACING;
    itemY += (CONTROL_BUTTON_HEIGHT + CONTROL_BUTTON_SPACING) * (buttonNumber / 2);

    return (Rectangle){ itemX, itemY, CONTROL_BUTTON_WIDTH, CONTROL_BUTTON_HEIGHT };
}
