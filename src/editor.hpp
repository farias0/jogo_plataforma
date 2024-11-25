
#ifndef _EDITOR_H_INCLUDED_
#define _EDITOR_H_INCLUDED_


#include <raylib.h>
#include <vector>

#include "assets.hpp"
#include "linked_list.hpp"
#include "core.hpp"


// The background color of the editor
#define EDITOR_BG_COLOR                 { 0x08, 0x08, 0x11, 0xff }

// The size of the side of an square entity button
#define ENTITY_BUTTON_SIZE              80

// The space between two entity buttons
#define ENTITY_BUTTON_SPACING           12

// The height of the control button
#define CONTROL_BUTTON_HEIGHT           40

// The width of a controll button
#define CONTROL_BUTTON_WIDTH            ENTITY_BUTTON_SIZE

// The space between two control buttons
#define CONTROL_BUTTON_SPACING          ENTITY_BUTTON_SPACING

// The color of the entity selection's square
#define EDITOR_SELECTION_RECT_COLOR     { BLUE.r, BLUE.g, BLUE.b, 64 }

// The color of the shade over each selected entity
#define EDITOR_SELECTION_ENTITY_COLOR   { BLUE.r, BLUE.g, BLUE.b, 128 }

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
    EDITOR_ENTITY_GLIDE,
    EDITOR_ENTITY_TEXTBOX,
    EDITOR_ENTITY_CHECKPOINT_PICKUP,
    EDITOR_ENTITY_MOVING_PLATFORM,
    EDITOR_ENTITY_NPC,
    EDITOR_ENTITY_COIN,

    // Overworld
    EDITOR_ENTITY_LEVEL_DOT,
    EDITOR_ENTITY_STRAIGHT,
    EDITOR_ENTITY_PATH_JOIN,
    EDITOR_ENTITY_PATH_IN_L,
    
} EditorEntityType; // TODO are those even used?

typedef enum {

    // If the interation was a click, meaning the button is not being held
    EDITOR_INTERACTION_CLICK    = 1,

    // If the 'alt' key is pressed, so it uses alternative functions
    EDITOR_INTERACTION_ALT      = 2
    
} EditorInteractionTag;

class EditorEntityButton : public LinkedList::Node {

public:
    EditorEntityType type;
    Sprite *sprite;
    void (*handler)(Vector2, int); // Receives the click scene pos and the interaction tags

};


typedef enum { 
    EDITOR_CONTROL_SAVE,
    EDITOR_CONTROL_NEW_LEVEL,
    EDITOR_CONTROL_AUTO_TILE,
} EditorControlType;

class EditorControlButton : public LinkedList::Node {

public:
    EditorControlType type;
    char *label;
    void (*handler)();

};


typedef struct EditorState {

    // If the editor is enabled
    bool isEnabled;

    // The heads for each button list
    LinkedList::Node    *entitiesHead;
    LinkedList::Node    *controlHead;

    // What entity button is currently selected in the editor
    EditorEntityButton *toggledEntityButton;

    EditorEntityButton *defaultEntityButton;


    // Entity selection
    bool                            isSelectingEntities;
    bool                            selectedEntitiesThisFrame;
    Trajectory                      entitySelectionCoords;
    std::vector<LinkedList::Node *> selectedEntities;
    bool                            isSelectionGridlocked;

    // Moving selected entities
    bool        isMovingSelectedEntities;
    bool        movedEntitiesThisFrame;
    Trajectory  selectedEntitiesMoveCoords;

} EditorState;


extern EditorState *EDITOR_STATE;


void EditorInitialize();

// Destroy any existing editor itens and then loads the items
// for the current game mode
void EditorSync();

// Empties all editor itens, keeping it from being used.
void EditorEmpty();

void EditorEnable();

void EditorDisable();

// Enables and disables editor
void EditorEnabledToggle();

// Get the Rectangle that represents the editor bar in the screen
Rectangle EditorBarGetRect();

// Gets the Y of the Editor Bar's divisor in the screen
float EditorBarGetDivisorY();
 
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
