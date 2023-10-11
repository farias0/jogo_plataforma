#ifndef _EDITOR_H_INCLUDED_
#define _EDITOR_H_INCLUDED_

#include <raylib.h>

typedef enum { 
    Eraser,
    Block,
    Enemy,
} EditorItem;

// Set item as selected only if 'set' is true
void EditorSetSelectedItem(EditorItem item, bool set);


#endif // _EDITOR_H_INCLUDED_