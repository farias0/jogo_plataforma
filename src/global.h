#ifndef _GLOBAL_H_INCLUDED_
#define _GLOBAL_H_INCLUDED_


#include "entity.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define FLOOR_HEIGHT 600
#define FLOOR_DEATH_HEIGHT 800 // Below this y the player dies


// An unspecified node in the linked list of all the entities
extern Entity *ENTITIES;

// The entity "player" in the linked list of all the entities
extern Entity *PLAYER;

// The entity "camera" in the linked list of all the entities
extern Entity *CAMERA;


#endif // _GLOBAL_H_INCLUDED_