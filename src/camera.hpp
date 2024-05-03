#ifndef _CAMERA_H_INCLUDED_
#define _CAMERA_H_INCLUDED_


#include <raylib.h>


typedef struct MyCamera {
    Vector2 pos;
    float zoom; // The larger, the more zoomed out
} MyCamera;


extern MyCamera *CAMERA;


void CameraInitialize();

void CameraTick();

// Ask camera to follow the controlled entity
void CameraFollow();

// Sets the Camera to the default position in level
void CameraLevelCentralizeOnPlayer();

// Pans the camera around
void CameraPanningMove(Vector2 mousePos);

// Stops the process of panning the camera
void CameraPanningStop();

// Reset the camera to its original position after panning
void CameraPanningReset();

// If the camera is offset from being panned
bool CameraIsPanned();

// Converts position from the screen coordinates to in game coordinates
Vector2 PosInScreenToScene(Vector2 pos);

// Converts position from in game coordinates to the screen coordinates
Vector2 PosInSceneToScreen(Vector2 pos);

float ScaleInSceneToScreen(float value);


#endif // _CAMERA_H_INCLUDED_
