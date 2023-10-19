#ifndef _CAMERA_H_INCLUDED_
#define _CAMERA_H_INCLUDED_


#include <raylib.h>


typedef struct MyCamera {
    Vector2 pos;
} MyCamera;


extern MyCamera *CAMERA;


void CameraInitialize();
void CameraTick();


#endif // _CAMERA_H_INCLUDED_
