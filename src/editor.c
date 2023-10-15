#include <raylib.h>

#include "editor.h"
#include "global.h"
#include "entities/entity.h"
#include "entities/level.h"
#include "entities/enemy.h"

EditorItem *EDITOR_ITEMS_HEAD = 0;

void clearEditorItems() {
    
    EditorItem *current = EDITOR_ITEMS_HEAD;
    EditorItem *next;

    while (current) {
        next = current->next;
        MemFree(current);
        current = next;
    }

    EDITOR_ITEMS_HEAD = 0;
}

EditorItem *loadEditorItem(Sprite sprite, void (*handler), EditorItemInteraction interaction) {

    EditorItem *newItem = MemAlloc(sizeof(EditorItem));
    newItem->sprite = sprite;
    newItem->handler = handler;
    newItem->interaction = interaction;

    if (EDITOR_ITEMS_HEAD) {

        EditorItem *lastItem = EDITOR_ITEMS_HEAD;
        while (lastItem->next != 0) { lastItem = lastItem->next; }

        lastItem->next = newItem;
        newItem->previous = lastItem;

    } else {
        EDITOR_ITEMS_HEAD = newItem;
    }

    return newItem;
}

void loadInLevelEditor() {

    loadEditorItem(EraserSprite, &DestroyEntityOn, Hold);
    STATE->editorSelectedItem =
        loadEditorItem(BlockSprite, &AddBlockToLevel, Hold);
    loadEditorItem(EnemySprite, &AddEnemyToLevel, Click);

    TraceLog(LOG_DEBUG, "Editor loaded in level itens.");
}

void loadOverworldEditor() {

    STATE->editorSelectedItem =
        loadEditorItem(LevelDotSprite, 0, Click);

    TraceLog(LOG_DEBUG, "Editor loaded overworld itens.");
}

void SyncEditor() {

    clearEditorItems();

    switch (STATE->mode) {
    
    case InLevel:
        loadInLevelEditor();
        break;

    case Overworld:
        loadOverworldEditor();
        break;

    default:
        TraceLog(LOG_ERROR, "Could not find editor items list for game mode %d.", STATE->mode);
    }
}
