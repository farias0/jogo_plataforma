#include <raylib.h>
#include <algorithm>

#include "debug.hpp"
#include "level/level.hpp"
#include "overworld.hpp"
#include "linked_list.hpp"


std::vector<LinkedList::Node *> DEBUG_ENTITY_LIST = std::vector<LinkedList::Node *>();


void DebugEntityToggle(Vector2 pos) {

    LinkedList::Node *entity;
    switch (GAME_STATE->mode) {
    case MODE_IN_LEVEL:
        entity = Level::EntityGetAt(pos); break;
    case MODE_OVERWORLD:
        entity = OverworldEntityGetAt(pos); break;
    default:
        return;
    }
    
    if (!entity) return;

    auto idx = std::find(DEBUG_ENTITY_LIST.begin(), DEBUG_ENTITY_LIST.end(), entity);
    if (idx != DEBUG_ENTITY_LIST.end()) {
        DEBUG_ENTITY_LIST.erase(idx);
        TraceLog(LOG_TRACE, "Debug entity info disabled entity.");
    } else {
        DEBUG_ENTITY_LIST.push_back(entity);
        TraceLog(LOG_TRACE, "Debug entity info enabled entity.");
    }
}

void DebugEntityStop(LinkedList::Node *entity) {

    auto idx = std::find(DEBUG_ENTITY_LIST.begin(), DEBUG_ENTITY_LIST.end(), entity);

    if (idx != DEBUG_ENTITY_LIST.end()) {
        DEBUG_ENTITY_LIST.erase(idx);
        TraceLog(LOG_TRACE, "Debug entity info disabled entity.");
    }
}

void DebugEntityStopAll() {

    DEBUG_ENTITY_LIST.clear();

    TraceLog(LOG_TRACE, "Debug entity info disabled all entities.");
}
