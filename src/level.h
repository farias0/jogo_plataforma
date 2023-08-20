#ifndef _LEVEL_H_INCLUDED_
#define _LEVEL_H_INCLUDED_

#include <raylib.h>


typedef struct Level {

    int tilesCount;
    Rectangle *tiles;      // Hitboxes representing solid geometry
} Level;


Level *LoadLevel();
void InitializeLevel(Level *level);
void DrawLevel(Level *level);

#endif // _LEVEL_H_INCLUDED_