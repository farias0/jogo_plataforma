#ifndef _EDITOR_H_INCLUDED_
#define _EDITOR_H_INCLUDED_

#include <raylib.h>

typedef enum { 
    Block,
    Enemy,
    Eraser
} EditorItem;

void EditorSetSelectedItem(EditorItem item, bool set);


#endif // _EDITOR_H_INCLUDED_