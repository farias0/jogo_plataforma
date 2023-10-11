#ifndef _CAMERA_H_INCLUDED_
#define _CAMERA_H_INCLUDED_


#include <raylib.h>

#include "entity.h"

// Initializes a new camera and returns the list's head.
Entity *InitializeCamera(Entity *head, Entity **newCamera);
void CameraTick();


#endif // _CAMERA_H_INCLUDED_