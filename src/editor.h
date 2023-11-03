#ifndef _EDITOR_H_INCLUDED_
#define _EDITOR_H_INCLUDED_


#include <raylib.h>

#include "assets.h"
#include "linked_list.h"


#define EDITOR_ENTITIES_HEIGHT      4*SCREEN_HEIGHT/5

#define ENTITY_BUTTON_SIZE          80
#define ENTITY_BUTTON_SPACING       12
#define ENTITY_BUTTON_WALL_SPACING  (EDITOR_BAR_WIDTH - (ENTITY_BUTTON_SIZE * 2) - ENTITY_BUTTON_SPACING) / 2

#define CONTROL_BUTTON_HEIGHT       40
#define CONTROL_BUTTON_WIDTH        ENTITY_BUTTON_SIZE
#define CONTROL_BUTTON_SPACING      ENTITY_BUTTON_SPACING
#define CONTROL_BUTTON_WALL_SPACING (EDITOR_BAR_WIDTH - (CONTROL_BUTTON_WIDTH * 2) - CONTROL_BUTTON_SPACING) / 2

#define EDITOR_BAR_WIDTH        200

// The rectangle defining the entity area of the editor in the screen
#define EDITOR_ENTITIES_AREA    (Rectangle){ SCREEN_WIDTH, \
                                                0, \
                                                EDITOR_BAR_WIDTH, \
                                                EDITOR_ENTITIES_HEIGHT }

// The rectangle defining the control area of the editor in the screen
#define EDITOR_CONTROL_AREA     (Rectangle){ SCREEN_WIDTH, \
                                                EDITOR_ENTITIES_HEIGHT, \
                                                EDITOR_BAR_WIDTH, \
                                                SCREEN_HEIGHT - EDITOR_ENTITIES_HEIGHT }

// The background color of the editor
#define EDITOR_BG_COLOR         (Color){ 0, 0, 0, 220 }


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
    EDITOR_CONTROL_NEW_LEVEL
} EditorControlType;

typedef struct EditorControlItem {
    EditorControlType type;

    void (*handler)();

    char *label;
} EditorControlItem;


// The head of the linked list of all the loaded editor entity itens
extern ListNode *EDITOR_ENTITIES_HEAD;

// The head of the linked list of all the loaded editor control itens
extern ListNode *EDITOR_CONTROL_HEAD;


// Destroy any existing editor itens and then loads the items
// for the current game mode
void EditorSync();

// Empties all editor itens, keeping it from being used.
void EditorEmpty();

void EditorEnable();

void EditorDisable();

// Enables and disables editor
void EditorEnabledToggle();

// Calculates and returns an editor entity buttons' coordinates,
// alongside its dimensions
Rectangle EditorEntityButtonRect(int buttonNumber);

// Calculates and returns an editor entity buttons' coordinates,
// alongside its dimensions
Rectangle EditorControlButtonRect(int buttonNumber);

#endif // _EDITOR_H_INCLUDED_
