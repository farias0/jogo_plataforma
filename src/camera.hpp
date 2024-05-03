#ifndef _CAMERA_H_INCLUDED_
#define _CAMERA_H_INCLUDED_


#include <raylib.h>

#include "core.hpp"


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

void CameraZoomIn();

void CameraZoomOut();

// Converts position from the screen coordinates to in game coordinates
Vector2 PosInScreenToScene(Vector2 pos);

// Converts position from in game coordinates to the screen coordinates
Vector2 PosInSceneToScreen(Vector2 pos);

// Converts a scale from in game to the screen according to the zoom level
float ScaleInSceneToScreen(float value);

// Converts a dimension from in game to the screen according to the zoom level
Dimensions DimensionsInSceneToScreen(Dimensions dim);


#endif // _CAMERA_H_INCLUDED_
