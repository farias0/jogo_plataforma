#ifndef _OVERWORLD_H_INCLUDED_
#define _OVERWORLD_H_INCLUDED_


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
} OverworldTileType;


void LoadOverworld();
void SelectLevel();
void OverworldMoveCursor(OverworldCursorDirection direction);
void AddTileToOverworld(Vector2 pos);

#endif // _OVERWORLD_H_INCLUDED_