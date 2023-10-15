#include <raylib.h>

#include "editor.h"
#include "global.h"


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

void loadEditorItem(EditorItemType type, Sprite sprite) {

    EditorItem *newItem = MemAlloc(sizeof(EditorItem));
    newItem->type = type;
    newItem->sprite = sprite;

    if (EDITOR_ITEMS_HEAD) {

        EditorItem *lastItem = EDITOR_ITEMS_HEAD;
        while (lastItem->next != 0) { lastItem = lastItem->next; }

        lastItem->next = newItem;
        newItem->previous = lastItem;

    } else {
        EDITOR_ITEMS_HEAD = newItem;
    }

    TraceLog(LOG_DEBUG, "Editor item of type %d loaded.", type);
}

void loadInLevelEditor() {

    clearEditorItems();

    loadEditorItem(Eraser, EraserSprite);
    loadEditorItem(Block, BlockSprite);
    loadEditorItem(Enemy, EnemySprite);

    TraceLog(LOG_DEBUG, "Editor loaded in level itens.");
}

void loadOverworldEditor() {

    clearEditorItems();

    TraceLog(LOG_DEBUG, "Editor loaded overworld itens.");
}

void SyncEditor() {

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

void EditorSetSelectedItem(EditorItemType type, bool set) {

    if (!set) return;

    EditorItem *currentItem = EDITOR_ITEMS_HEAD;
    bool found = false;

    while (currentItem != 0) {
        if (currentItem->type == type) {
            found = true;
            break;
        }
        
        currentItem = currentItem->next;
    }

    if (found) {

        STATE->editorSelectedItem = type;
        // TraceLog(LOG_DEBUG, "Item of type %d selected.", type);

    } else {
        TraceLog(LOG_ERROR, "Could not find items of type %d on loaded editor items.", type);
    }
}