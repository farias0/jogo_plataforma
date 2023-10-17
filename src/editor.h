#ifndef _EDITOR_H_INCLUDED_
#define _EDITOR_H_INCLUDED_

#include <raylib.h>
#include "assets.h"



typedef enum { 
    Eraser,
    Block,
    Enemy,
    LevelDot,
    PathStraight,
    PathJoin
} EditorItemType;

typedef enum {
    Click,
    Hold
} EditorItemInteraction;

typedef struct EditorItem {
    Sprite sprite;
    EditorItemType type;
    void (*handler)(Vector2);
    EditorItemInteraction interaction;

    struct EditorItem *previous;
    struct EditorItem *next;
} EditorItem;


// The head of the linked list of all the loaded editor itens
extern EditorItem *EDITOR_ITEMS_HEAD;

// Destroy any existing editor itens and then loads the items
// for the current game mode
void SyncEditor();


#endif // _EDITOR_H_INCLUDED_