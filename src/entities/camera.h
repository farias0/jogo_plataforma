#ifndef _CAMERA_H_INCLUDED_
#define _CAMERA_H_INCLUDED_


#include <raylib.h>

#include "entity.h"

Entity *InitializeCamera(Entity *listItem);
void CameraTick();


#endif // _CAMERA_H_INCLUDED_