#ifndef _OVERWORLD_H_INCLUDED_
#define _OVERWORLD_H_INCLUDED_


#include "assets.h"


typedef enum OverworldCursorDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT
} OverworldCursorDirection;

typedef enum OverworldTileType {
    LEVEL_DOT,
    STRAIGHT_PATH,
    JOIN_PATH,
    PATH_IN_L
} OverworldTileType;


extern SpriteDimensions OverworldGridDimensions;

void LoadOverworld();
void SelectLevel();
void OverworldMoveCursor(OverworldCursorDirection direction);

// Adds tile to overworld in according to the item selected in the editor.
void AddTileToOverworld(Vector2 pos);

// Removes tile from overworld and destroys it, if present and if allowed.
void RemoveTileFromOverWorld(Vector2 pos);


#endif // _OVERWORLD_H_INCLUDED_
