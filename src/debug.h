#ifndef _DEBUG_H_INCLUDED_
#define _DEBUG_H_INCLUDED_


#include <raylib.h>

#include "linked_list.h"
#include "level/level.h"


extern ListNode *DEBUG_ENTITY_INFO_HEAD;


// Searches for entity at pos and, if it finds one, shows
// debug info about it, or stop showing if it's enabled already.
void DebugEntityToggle(Vector2 pos);

// Stops showing info about an entity. If it's not showing already, does nothing.
void DebugEntityStop(LevelEntity *entity);

// Stops showing info about all entities.
void DebugEntityStopAll();


#endif // _DEBUG_H_INCLUDED_
