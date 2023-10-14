#ifndef _OVERWORLD_H_INCLUDED_
#define _OVERWORLD_H_INCLUDED_


typedef enum OverworldCursorDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT
} OverworldCursorDirection;

void LoadOverworld();
void SelectLevel();
void OverworldMoveCursor(OverworldCursorDirection direction);


#endif // _OVERWORLD_H_INCLUDED_