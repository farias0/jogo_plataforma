#ifndef _PERSISTENCE_H_INCLUDED_
#define _PERSISTENCE_H_INCLUDED_


#include <stdbool.h>
#include <string>

#define LEVEL_NAME_BUFFER_SIZE 400


// To be implemented by persistable entities
class IPersistable { 

/*
	Persisted levels are text files following roughly this format:
	
	levelname:My Level
	entity_type_1:item1=x;item2=y;item3=z
	entity_type_2:item1=a;item2=b;item3=c
*/

public:

	// Returns a string with the entity's data to be saved to persistence.
	// The string should be assembled by adding the persisting fields using persistenceAddValue().
	virtual std::string PersistanceSerialize() = 0;

	// Loads persistence data to an existing entity.
	// To extract each field using persistenceReadValue() and initialize the entity accordingly.
	// NOTE: Ideally this would be a static function creating a new instance of the entity,
	// but C++ doesn't support virtual static member functions.
	virtual void PersistenceParse(const std::string &data) = 0;

	// The ID unique to an entity type that can be persisted.
	// This ID should be associated to the entity's initalization function at PersistenceLevelLoad().
	virtual const std::string &PersitenceEntityID() = 0;

	/*
		Adds a value to a data string. To be used inside PersistenceSerialize().
		@param *line The line to receive the new value
		@param field The field to be added to the line
		@param value The value to be added to the field
	*/
	virtual void persistanceAddValue(std::string *line, const std::string &field, const std::string &value) final {
		*line += field + "=" + value + ";";
	}

	/*
		Searches for a value in a data string. To be used inside PersistenceParse().
		@param line The line to be searched
		@param field The field to be searched in the line
		@return The value corresponding to the field
	*/
	virtual std::string persistenceReadValue(const std::string &line, const std::string &field) final {
		std::string label = field + "=";
		size_t labelStart = line.find(label);
		size_t valueStart = labelStart + label.length();
		size_t valueEnd = line.find(";", valueStart); 
		return line.substr(valueStart, valueEnd - valueStart);
	}
};


// Level

void PersistenceLevelSave(char *levelName);

bool PersistenceLevelLoad(char *levelName);

// Copies the name of the dropped level into the buffer. Returns 'true' if successful.
bool PersistenceGetDroppedLevelName(char *nameBuffer);


// Overworld

void PersistenceOverworldSave();

bool PersistenceOverworldLoad();


#endif // _PERSISTENCE_H_INCLUDED_
