#ifndef _CAMERA_H_INCLUDED_
#define _CAMERA_H_INCLUDED_


#include <raylib.h>

#include "core.hpp"


typedef struct MyCamera {

    // The position (origin) of the camera in the scene
    Vector2 pos;

    // The zoom the player applied to the scene
    float zoom; // The larger, the more zoomed out

    /*
        It should be reconsidered if data and procedures regarding how to transform the
        scene data in screen data should be responsibiity of the camera.

        Maybe it should be the renderer's responsability to compensate to fullscreen
        stretches and offsets.
    */

    // In how much to stretch position data when converting from scene to screen and vice versa
    // to compensate for the difference between the game's native resolution and the fullscreen resolutoin   
    float fullscreenStretch;

    // How much to vertically offset the rendering frame to centralize it, in case of wider resolutions
    int sceneXOffset;
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

// Converts position from in game coordinates to the screen coordinates,
// applying parallax speed
Vector2 PosInSceneToScreenParallax(Vector2 pos, float parallaxSpeed);

// Converts a scale from in game to the screen according to the zoom level
float ScaleInSceneToScreen(float value);

// Converts a dimension from in game to the screen according to the zoom level
Dimensions DimensionsInSceneToScreen(Dimensions dim);


#endif // _CAMERA_H_INCLUDED_
