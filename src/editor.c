#include "editor.h"
#include "global.h"

void EditorSetSelectedItem(EditorItem item, bool set) {

    if (set) {
        STATE->editorSelectedItem = item;
    }
}