#include <raylib.h>
#include <algorithm>

#include "debug.hpp"
#include "level/level.hpp"
#include "overworld.hpp"
#include "linked_list.hpp"


std::vector<LinkedList::Node *> DEBUG_ENTITY_INFO_HEAD = std::vector<LinkedList::Node *>();


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

    auto idx = std::find(DEBUG_ENTITY_INFO_HEAD.begin(), DEBUG_ENTITY_INFO_HEAD.end(), entity);
    if (idx != DEBUG_ENTITY_INFO_HEAD.end()) {
        DEBUG_ENTITY_INFO_HEAD.erase(idx);
        TraceLog(LOG_TRACE, "Debug entity info disabled entity.");
    } else {
        DEBUG_ENTITY_INFO_HEAD.push_back(entity);
        TraceLog(LOG_TRACE, "Debug entity info enabled entity.");
    }
}

void DebugEntityStop(LinkedList::Node *entity) {

    auto idx = std::find(DEBUG_ENTITY_INFO_HEAD.begin(), DEBUG_ENTITY_INFO_HEAD.end(), entity);

    if (idx != DEBUG_ENTITY_INFO_HEAD.end()) {
        DEBUG_ENTITY_INFO_HEAD.erase(idx);
        TraceLog(LOG_TRACE, "Debug entity info disabled entity.");
    }
}

void DebugEntityStopAll() {

    DEBUG_ENTITY_INFO_HEAD.clear();

    TraceLog(LOG_TRACE, "Debug entity info disabled all entities.");
}
