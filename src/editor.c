#include <raylib.h>

#include "editor.h"
#include "core.h"
#include "level/level.h"
#include "overworld.h"
#include "linked_list.h"

#define EDITOR_BUTTON_SIZE 80
#define EDITOR_BUTTON_SPACING 12
#define EDITOR_BUTTON_WALL_SPACING (EDITOR_BAR_WIDTH - (EDITOR_BUTTON_SIZE * 2) - EDITOR_BUTTON_SPACING) / 2


ListNode *EDITOR_ITEMS_HEAD = 0;

const Rectangle EDITOR_RECT = (Rectangle){ SCREEN_WIDTH, 5, EDITOR_BAR_WIDTH, SCREEN_HEIGHT };

const Color EDITOR_BG_COLOR = (Color){ 150, 150, 150, 40 };

const char* EDITOR_LABEL = "Editor";


EditorItem *loadEditorItem(EditorItemType type, Sprite sprite, void (*handler), EditorItemInteraction interaction) {

    EditorItem *newItem = MemAlloc(sizeof(EditorItem));
    newItem->sprite = sprite;
    newItem->type = type;
    newItem->handler = handler;
    newItem->interaction = interaction;

    ListNode *node = MemAlloc(sizeof(ListNode));
    node->item = newItem;
    LinkedListAdd(&EDITOR_ITEMS_HEAD, node);

    return newItem;
}

void loadInLevelEditor() {

    loadEditorItem(Eraser, EraserSprite, &LevelEntityRemoveAt, Hold);
    STATE->editorSelectedItem =
        loadEditorItem(Block, BlockSprite, &LevelBlockCheckAndAdd, Hold);
    loadEditorItem(Enemy, EnemySprite, &LevelEnemyCheckAndAdd, Click);

    TraceLog(LOG_DEBUG, "Editor loaded in level itens.");
}

void loadOverworldEditor() {

    loadEditorItem(Eraser, EraserSprite, &OverworldTileRemoveAt, Click);
    STATE->editorSelectedItem =
        loadEditorItem(LevelDot, LevelDotSprite, &OverworldTileAddOrInteract, Click);
    loadEditorItem(PathJoin, PathTileJoinSprite, &OverworldTileAddOrInteract, Click);
    loadEditorItem(PathStraight, PathTileStraightSprite, &OverworldTileAddOrInteract, Click);
    loadEditorItem(PathInL, PathTileInLSprite, &OverworldTileAddOrInteract, Click);

    TraceLog(LOG_DEBUG, "Editor loaded overworld itens.");
}

void EditorSync() {

    LinkedListRemoveAll(&EDITOR_ITEMS_HEAD);

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
