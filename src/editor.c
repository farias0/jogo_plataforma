#include <raylib.h>

#include "editor.h"
#include "core.h"
#include "level/level.h"
#include "overworld.h"
#include "linked_list.h"

#define EDITOR_BUTTON_SIZE 80
#define EDITOR_BUTTON_SPACING 12
#define EDITOR_BUTTON_WALL_SPACING (EDITOR_BAR_WIDTH - (EDITOR_BUTTON_SIZE * 2) - EDITOR_BUTTON_SPACING) / 2


ListNode *EDITOR_ENTITIES_HEAD = 0;
ListNode *EDITOR_CONTROL_HEAD = 0;

const Rectangle EDITOR_RECT = (Rectangle){ SCREEN_WIDTH, 5, EDITOR_BAR_WIDTH, SCREEN_HEIGHT };

const Color EDITOR_BG_COLOR = (Color){ 150, 150, 150, 40 };

const char* EDITOR_LABEL = "Editor";


EditorEntityItem *loadEditorEntityItem(
    EditorEntityType type, Sprite sprite, void (*handler), EditorItemInteraction interaction) {

        EditorEntityItem *newItem = MemAlloc(sizeof(EditorEntityItem));
        newItem->type = type;
        newItem->handler = handler;
        newItem->sprite = sprite;
        newItem->interaction = interaction;

        ListNode *node = MemAlloc(sizeof(ListNode));
        node->item = newItem;
        LinkedListAdd(&EDITOR_ENTITIES_HEAD, node);

        return newItem;
}

EditorControlItem *loadEditorControlItem(EditorControlType type, char *label, void (*handler)) {

    EditorControlItem *newItem = MemAlloc(sizeof(EditorControlItem));
    newItem->type = type;
    newItem->handler = handler;
    newItem->label = label;

    ListNode *node = MemAlloc(sizeof(ListNode));
    node->item = newItem;
    LinkedListAdd(&EDITOR_CONTROL_HEAD, node);

    return newItem;
}

void loadInLevelEditor() {

    loadEditorEntityItem(EDITOR_ENTITY_ERASER, EraserSprite, &LevelEntityRemoveAt, EDITOR_INTERACTION_HOLD);
    STATE->editorSelectedItem =
        loadEditorEntityItem(EDITOR_ENTITY_BLOCK, BlockSprite, &LevelBlockCheckAndAdd, EDITOR_INTERACTION_HOLD);
    loadEditorEntityItem(EDITOR_ENTITY_ENEMY, EnemySprite, &LevelEnemyCheckAndAdd, EDITOR_INTERACTION_CLICK);

    loadEditorControlItem(EDITOR_CONTROL_SAVE, "Save", 0);

    TraceLog(LOG_DEBUG, "Editor loaded in level itens.");
}

void loadOverworldEditor() {

    loadEditorEntityItem(EDITOR_ENTITY_ERASER, EraserSprite, &OverworldTileRemoveAt, EDITOR_INTERACTION_CLICK);
    STATE->editorSelectedItem =
        loadEditorEntityItem(EDITOR_ENTITY_LEVEL_DOT, LevelDotSprite, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);
    loadEditorEntityItem(EDITOR_ENTITY_PATH_JOIN, PathTileJoinSprite, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);
    loadEditorEntityItem(EDITOR_ENTITY_STRAIGHT, PathTileStraightSprite, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);
    loadEditorEntityItem(EDITOR_ENTITY_PATH_IN_L, PathTileInLSprite, &OverworldTileAddOrInteract, EDITOR_INTERACTION_CLICK);

    TraceLog(LOG_DEBUG, "Editor loaded overworld itens.");
}

void EditorSync() {

    LinkedListRemoveAll(&EDITOR_ENTITIES_HEAD);
    LinkedListRemoveAll(&EDITOR_CONTROL_HEAD);

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

Rectangle EditorButtonGetRect(int buttonNumber) {

    float itemX = EDITOR_RECT.x + EDITOR_BUTTON_WALL_SPACING;
    if (buttonNumber % 2) itemX += EDITOR_BUTTON_SIZE + EDITOR_BUTTON_SPACING;

    float itemY = EDITOR_RECT.y + EDITOR_BUTTON_WALL_SPACING;
    itemY += (EDITOR_BUTTON_SIZE + EDITOR_BUTTON_SPACING) * (buttonNumber / 2);

    return (Rectangle){ itemX, itemY, EDITOR_BUTTON_SIZE, EDITOR_BUTTON_SIZE };
}
