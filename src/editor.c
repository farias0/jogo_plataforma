#include <raylib.h>

#include "editor.h"
#include "core.h"
#include "level/level.h"
#include "overworld.h"
#include "linked_list.h"


#define EDITOR_ENTITIES_HEIGHT      4*SCREEN_HEIGHT/5

#define ENTITY_BUTTON_SIZE          80
#define ENTITY_BUTTON_SPACING       12
#define ENTITY_BUTTON_WALL_SPACING  (EDITOR_BAR_WIDTH - (ENTITY_BUTTON_SIZE * 2) - ENTITY_BUTTON_SPACING) / 2

#define CONTROL_BUTTON_HEIGHT       40
#define CONTROL_BUTTON_WIDTH        ENTITY_BUTTON_SIZE
#define CONTROL_BUTTON_SPACING      ENTITY_BUTTON_SPACING
#define CONTROL_BUTTON_WALL_SPACING (EDITOR_BAR_WIDTH - (CONTROL_BUTTON_WIDTH * 2) - CONTROL_BUTTON_SPACING) / 2


ListNode *EDITOR_ENTITIES_HEAD = 0;
ListNode *EDITOR_CONTROL_HEAD = 0;

const Rectangle EDITOR_ENTITIES_AREA = (Rectangle){ SCREEN_WIDTH,
                                                    0,
                                                    EDITOR_BAR_WIDTH,
                                                    EDITOR_ENTITIES_HEIGHT };

const Rectangle EDITOR_CONTROL_AREA = (Rectangle){ SCREEN_WIDTH,
                                                    EDITOR_ENTITIES_HEIGHT,
                                                    EDITOR_BAR_WIDTH,
                                                    SCREEN_HEIGHT - EDITOR_ENTITIES_HEIGHT };

const Color EDITOR_BG_COLOR = (Color){ 0, 0, 0, 220 };


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

    loadEditorControlItem(EDITOR_CONTROL_SAVE, "Save", &LevelSave);

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
