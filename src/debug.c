#include <raylib.h>

#include "debug.h"
#include "level/level.h"
#include "linked_list.h"


ListNode *DEBUG_ENTITY_INFO_HEAD = 0;


void DebugEntityToggle(LevelEntity *entity) {
    
    if (!entity) return;

    ListNode *entitysNode = LinkedListGetNode(DEBUG_ENTITY_INFO_HEAD, entity);

    if (entitysNode) {
        LinkedListRemoveNode(&DEBUG_ENTITY_INFO_HEAD, entitysNode);
        TraceLog(LOG_TRACE, "Debug entity info removed entity;");
    } else {
        LinkedListAdd(&DEBUG_ENTITY_INFO_HEAD, entity);
        TraceLog(LOG_TRACE, "Debug entity info added entity;");
    }
}

void DebugEntityStop(LevelEntity *entity) {
    ListNode *entitysNode = LinkedListGetNode(DEBUG_ENTITY_INFO_HEAD, entity);
    if (entitysNode) {
        LinkedListRemoveNode(&DEBUG_ENTITY_INFO_HEAD, entitysNode);
        TraceLog(LOG_TRACE, "Debug entity info stopped showing entity.");
    }
}

void DebugEntityStopAll() {
    LinkedListRemoveAll(&DEBUG_ENTITY_INFO_HEAD);
    TraceLog(LOG_TRACE, "Debug entity info stopped showing all entities.");
}
