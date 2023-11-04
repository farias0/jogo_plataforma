#ifndef _RENDER_H_INCLUDED_
#define _RENDER_H_INCLUDED_


#define SYS_MSG_BUFFER_SIZE 1000

void RenderInitialize();

void Render();

// Prints a system message in the screen
void RenderPrintSysMessage(char *msg);


#endif // _RENDER_H_INCLUDED_
