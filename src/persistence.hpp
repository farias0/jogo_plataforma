#ifndef _PERSISTENCE_H_INCLUDED_
#define _PERSISTENCE_H_INCLUDED_


#include <stdbool.h>

#define LEVEL_NAME_BUFFER_SIZE 400


// Level

void PersistenceLevelSave(char *levelName);

bool PersistenceLevelLoad(char *levelName);

// Copies the name of the dropped level into the buffer. Returns 'true' if successful.
bool PersistenceGetDroppedLevelName(char *nameBuffer);


// Overworld

void PersistenceOverworldSave();

bool PersistenceOverworldLoad();


#endif // _PERSISTENCE_H_INCLUDED_
