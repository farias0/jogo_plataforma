#ifndef _DEBUG_H_INCLUDED_
#define _DEBUG_H_INCLUDED_


#include <raylib.h>
#include <vector>

#include "linked_list.hpp"
#include "level/level.hpp"


extern std::vector<LinkedList::Node *> DEBUG_ENTITY_LIST;


// Searches for entity at pos and, if it finds one, shows
// debug info about it, or stops showing if it's enabled already.
void DebugEntityToggle(Vector2 pos);

// Stops showing info about an entity. If it's not showing already, does nothing.
void DebugEntityStop(LinkedList::Node *entity);

// Stops showing info about all entities.
void DebugEntityStopAll();


#endif // _DEBUG_H_INCLUDED_
