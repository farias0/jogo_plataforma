#include <raylib.h>

#include "debug.hpp"
#include "level/level.hpp"
#include "overworld.hpp"
#include "linked_list.hpp"


LinkedList::ListNode *DEBUG_ENTITY_INFO_HEAD = 0;


void DebugEntityToggle(Vector2 pos) {

    LinkedList::NodeItem *entity;
    switch (GAME_STATE->mode) {
    case MODE_IN_LEVEL:
        entity = Level::EntityGetAt(pos); break;
    case MODE_OVERWORLD:
        entity = OverworldEntityGetAt(pos); break;
    default:
        return;
    }
    
    if (!entity) return;

    LinkedList::ListNode *entitysNode = LinkedList::GetNode(DEBUG_ENTITY_INFO_HEAD, entity);

    if (entitysNode) {
        LinkedList::RemoveNode(&DEBUG_ENTITY_INFO_HEAD, entitysNode);
        TraceLog(LOG_TRACE, "Debug entity info disabled entity.");
    } else {
        LinkedList::Add(&DEBUG_ENTITY_INFO_HEAD, entity);
        TraceLog(LOG_TRACE, "Debug entity info enabled entity.");
    }
}

void DebugEntityStop(void *entity) {
    
    LinkedList::ListNode *entitysNode = LinkedList::GetNode(DEBUG_ENTITY_INFO_HEAD, entity);

    if (entitysNode) {
        LinkedList::RemoveNode(&DEBUG_ENTITY_INFO_HEAD, entitysNode);
        TraceLog(LOG_TRACE, "Debug entity info disabled entity.");
    }
}

void DebugEntityStopAll() {

    LinkedList::RemoveAll(&DEBUG_ENTITY_INFO_HEAD);

    TraceLog(LOG_TRACE, "Debug entity info disabled all entities.");
}
