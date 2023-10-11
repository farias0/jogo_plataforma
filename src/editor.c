#include "editor.h"
#include "global.h"

void EditorSetSelectedItem(EditorItem item, bool set) {

    if (set && STATE->editorSelectedItem != item) {

        STATE->editorSelectedItem = item;
        
        TraceLog(LOG_DEBUG, "Item %d selected.", item);
    }
}