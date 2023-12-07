#ifndef _DEBUG_H_INCLUDED_
#define _DEBUG_H_INCLUDED_


#include "linked_list.h"
#include "level/level.h"


extern ListNode *DEBUG_ENTITY_INFO_HEAD;


// Shows info about entity, or stop showing if it's there already.
void DebugEntityToggle(LevelEntity *entity);

// Stops showing info about an entity. If it's not showing already, does nothing.
void DebugEntityStop(LevelEntity *entity);

// Stops showing info about all entities.
void DebugEntityStopAll();


#endif // _DEBUG_H_INCLUDED_
