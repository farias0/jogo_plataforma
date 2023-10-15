#ifndef _EDITOR_H_INCLUDED_
#define _EDITOR_H_INCLUDED_

#include <raylib.h>
#include "assets.h"



typedef enum { 
    Eraser,
    Block,
    Enemy,
} EditorItemType;

typedef struct EditorItem {
    EditorItemType type;
    Sprite sprite;

    struct EditorItem *previous;
    struct EditorItem *next;
} EditorItem;


// The head of the linked list of all the loaded editor itens
extern EditorItem *EDITOR_ITEMS_HEAD;

// Destroy any existing editor itens and then loads the items
// for the current game mode
void SyncEditor();

// Searches for an item of a given type in the loaded editor
// itens list and sets it as selected only if 'set' is true
void EditorSetSelectedItem(EditorItemType type, bool set);


#endif // _EDITOR_H_INCLUDED_