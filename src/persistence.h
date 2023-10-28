#ifndef _PERSISTENCE_H_INCLUDED_
#define _PERSISTENCE_H_INCLUDED_


#include <stdbool.h>


void PersistenceLevelSave(char *levelName);

bool PersistenceLevelLoad(char *levelName);


#endif // _PERSISTENCE_H_INCLUDED_
