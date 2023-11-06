#ifndef _RENDER_H_INCLUDED_
#define _RENDER_H_INCLUDED_


#define SYS_MSG_BUFFER_SIZE 1000

void RenderInitialize();

void Render();

void RenderResizeWindow(int width, int height);

// Prints a system message in the screen
void RenderPrintSysMessage(char *msg);

void RenderStartLevelTransitionShader(Vector2 focusPoint, bool isClose);

#endif // _RENDER_H_INCLUDED_
