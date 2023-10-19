#ifndef _CAMERA_H_INCLUDED_
#define _CAMERA_H_INCLUDED_


#include <raylib.h>


typedef struct MyCamera {
    Vector2 pos;
} MyCamera;


extern MyCamera *CAMERA;


void CameraInitialize();

void CameraTick();

// Converts position from the screen coordinates to in game coordinates
Vector2 PosInScreenToScene(Vector2 pos);

// Converts position from in game coordinates to the screen coordinates
Vector2 PosInSceneToScreen(Vector2 pos);

#endif // _CAMERA_H_INCLUDED_
