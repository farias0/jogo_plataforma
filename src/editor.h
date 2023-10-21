#ifndef _EDITOR_H_INCLUDED_
#define _EDITOR_H_INCLUDED_


#include <raylib.h>

#include "assets.h"
#include "linked_list.h"


#define EDITOR_BAR_WIDTH        200


typedef enum { 
    EDITOR_ENTITY_ERASER,
    EDITOR_ENTITY_BLOCK,
    EDITOR_ENTITY_ENEMY,
    EDITOR_ENTITY_LEVEL_DOT,
    EDITOR_ENTITY_STRAIGHT,
    EDITOR_ENTITY_PATH_JOIN,
    EDITOR_ENTITY_PATH_IN_L,
} EditorEntityType;

typedef enum {
    EDITOR_INTERACTION_CLICK,
    EDITOR_INTERACTION_HOLD
} EditorItemInteraction;

typedef struct EditorEntityItem {
    EditorEntityType type;

    void (*handler)(Vector2);

    Sprite sprite;
    EditorItemInteraction interaction;
} EditorEntityItem;


typedef enum { 
    EDITOR_CONTROL_SAVE,
} EditorControlType;

typedef struct EditorControlItem {
    EditorControlType type;

    void (*handler)(Vector2);

    char *label;
} EditorControlItem;


extern const Rectangle EDITOR_RECT;

extern const Color EDITOR_BG_COLOR;

extern const char* EDITOR_LABEL;


// The head of the linked list of all the loaded editor entity itens
extern ListNode *EDITOR_ENTITIES_HEAD;

// The head of the linked list of all the loaded editor control itens
extern ListNode *EDITOR_CONTROL_HEAD;


// Destroy any existing editor itens and then loads the items
// for the current game mode
void EditorSync();

// Calculates and returns an editor buttons' coordinates,
// alongside its dimensions
Rectangle EditorButtonGetRect(int buttonNumber);


#endif // _EDITOR_H_INCLUDED_
