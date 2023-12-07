#ifndef _RENDER_H_INCLUDED_
#define _RENDER_H_INCLUDED_

#include "level/level.h"


#define SYS_MSG_BUFFER_SIZE 1000

#define LEVEL_TRANSITION_ANIMATION_DURATION 0.7 // in seconds


void RenderInitialize();

void Render();

void RenderResizeWindow(int width, int height);

// Prints a system message in the screen
void RenderPrintSysMessage(char *msg);

/*
    Starts the visual effect of transitioning from overworld to level or vice-versa.

    sceneFocusPont: The point in the scene at the center of the effect.
    isClose: If the effect will close on the sceneFocusPont, or, if false, if it will open from it.
*/
void RenderLevelTransitionEffectStart(Vector2 sceneFocusPont, bool isClose);


#endif // _RENDER_H_INCLUDED_
