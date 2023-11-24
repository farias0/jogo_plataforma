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
#define EDITOR_BG_COLOR                 (Color){ 0, 0, 0, 220 }

// The color of the entity selection
#define EDITOR_SELECTION_RECT_COLOR     (Color){ BLUE.r, BLUE.g, BLUE.b, 64 }

#define EDITOR_SELECTION_ENTITY_COLOR   (Color){ BLUE.r, BLUE.g, BLUE.b, 128 }

// How transparent, compared to the original, the ghost of the
// entities being moved will be
#define EDITOR_SELECTION_MOVE_TRANSPARENCY      128


typedef enum { 

    EDITOR_ENTITY_ERASER,

    // In level
    EDITOR_ENTITY_BLOCK,
    EDITOR_ENTITY_ACID,
    EDITOR_ENTITY_ENEMY,
    EDITOR_ENTITY_EXIT,

    // Overworld
    EDITOR_ENTITY_LEVEL_DOT,
    EDITOR_ENTITY_STRAIGHT,
    EDITOR_ENTITY_PATH_JOIN,
    EDITOR_ENTITY_PATH_IN_L,
    
} EditorEntityType;

typedef enum {
    EDITOR_INTERACTION_CLICK,
    EDITOR_INTERACTION_HOLD
} EditorInteractionType;

typedef struct EditorEntityButton {

    EditorEntityType type;
    Sprite sprite;
    void (*handler)(Vector2); // Receives the click scene pos
    EditorInteractionType interactionType;

} EditorEntityButton;


typedef enum { 
    EDITOR_CONTROL_SAVE,
    EDITOR_CONTROL_NEW_LEVEL
} EditorControlType;

typedef struct EditorControlButton {

    EditorControlType type;
    char *label;
    void (*handler)();

} EditorControlButton;


typedef struct EditorState {

    // Entity selection
    bool        isSelectingEntities;
    bool        selectedEntitiesThisFrame;
    Trajectory  entitySelectionCoords;
    ListNode *  selectedEntities;

    // Moving selected entities
    bool        isMovingSelectedEntities;
    bool        movedEntitiesThisFrame;
    Trajectory  selectedEntitiesMoveCoords;

} EditorState;


// TODO move loaded editor buttons to EditorState

// The head of the linked list of all the loaded editor entity itens
extern ListNode *EDITOR_ENTITIES_HEAD;

// The head of the linked list of all the loaded editor control itens
extern ListNode *EDITOR_CONTROL_HEAD;

extern EditorState *EDITOR_STATE;


void EditorInitialize();

void EditorStateReset();

// Destroy any existing editor itens and then loads the items
// for the current game mode
void EditorSync();

// Empties all editor itens, keeping it from being used.
void EditorEmpty();

void EditorEnable();

void EditorDisable();

// Enables and disables editor
void EditorEnabledToggle();

void EditorTick();

void EditorEntityButtonSelect(EditorEntityButton *item);

// Handles the selection of entities by a cursor dragging.
void EditorSelectEntities(Vector2 cursorPos);

// Cancels the selection of entities.
void EditorSelectionCancel();

// Moves a cluster of selected entities, if the cursor is above one.
// Otherwise, returns false;
bool EditorSelectedEntitiesMove(Vector2 cursorPos);

// Calculates and returns an editor entity buttons' coordinates,
// alongside its dimensions
Rectangle EditorEntityButtonRect(int buttonNumber);

// Calculates and returns an editor entity buttons' coordinates,
// alongside its dimensions
Rectangle EditorControlButtonRect(int buttonNumber);

// Converts current editor's entity selection coordinates from
// a trajectory to a rectangle (with positive dimensions, so missing direction).
Rectangle EditorSelectionGetRect();

// Calculates an entity's position according to the current move coordinates for
// the selected entities, as well as parameters like the current game mode's grid.
Vector2 EditorEntitySelectionCalcMove(Vector2 hitbox);


#endif // _EDITOR_H_INCLUDED_
