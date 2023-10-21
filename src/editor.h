#ifndef _EDITOR_H_INCLUDED_
#define _EDITOR_H_INCLUDED_


#include <raylib.h>

#include "assets.h"
#include "linked_list.h"


#define EDITOR_BAR_WIDTH        200


typedef enum { 
    Eraser,
    Block,
    Enemy,
    LevelDot,
    PathStraight,
    PathJoin,
    PathInL,
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


extern const Rectangle EDITOR_RECT;

extern const Color EDITOR_BG_COLOR;

extern const char* EDITOR_LABEL;


// The head of the linked list of all the loaded editor itens
extern ListNode *EDITOR_ITEMS_HEAD;

// Destroy any existing editor itens and then loads the items
// for the current game mode
void EditorSync();

// Calculates and returns an editor buttons' coordinates,
// alongside its dimensions
Rectangle EditorButtonGetRect(int buttonNumber);


#endif // _EDITOR_H_INCLUDED_
