#include <raylib.h>

#include "editor.h"
#include "core.h"
#include "inlevel/level.h"
#include "inlevel/enemy.h"
#include "overworld.h"
#include "linked_list.h"


ListNode *EDITOR_ITEMS_HEAD = 0;


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
