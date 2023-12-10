#include <raylib.h>

#include "debug.hpp"
#include "level/level.hpp"
#include "overworld.hpp"
#include "linked_list.hpp"


ListNode *DEBUG_ENTITY_INFO_HEAD = 0;


void DebugEntityToggle(Vector2 pos) {

    void *entity;
    switch (GAME_STATE->mode) {
    case MODE_IN_LEVEL:
        entity = LevelEntityGetAt(pos); break;
    case MODE_OVERWORLD:
        entity = OverworldEntityGetAt(pos); break;
    default:
        return;
    }
    
    if (!entity) return;

    ListNode *entitysNode = LinkedListGetNode(DEBUG_ENTITY_INFO_HEAD, entity);

    if (entitysNode) {
        LinkedListRemoveNode(&DEBUG_ENTITY_INFO_HEAD, entitysNode);
        TraceLog(LOG_TRACE, "Debug entity info disabled entity.");
    } else {
        LinkedListAdd(&DEBUG_ENTITY_INFO_HEAD, entity);
        TraceLog(LOG_TRACE, "Debug entity info enabled entity.");
    }
}

void DebugEntityStop(void *entity) {
    
    ListNode *entitysNode = LinkedListGetNode(DEBUG_ENTITY_INFO_HEAD, entity);

    if (entitysNode) {
        LinkedListRemoveNode(&DEBUG_ENTITY_INFO_HEAD, entitysNode);
        TraceLog(LOG_TRACE, "Debug entity info disabled entity.");
    }
}

void DebugEntityStopAll() {

    LinkedListRemoveAll(&DEBUG_ENTITY_INFO_HEAD);

    TraceLog(LOG_TRACE, "Debug entity info disabled all entities.");
}
