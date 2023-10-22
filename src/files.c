#include <stdio.h>
#include <string.h>
#include <raylib.h>

#include "files.h"


static char *LEVELS_DIR = "../levels/";

static char *LEVEL_NAME = "my_level.lvl";


static char *getFullPath(char *filename) {

    char *path = MemAlloc(sizeof(char) * 100);
    
    strcat(path, LEVELS_DIR);
    strcat(path, filename);
    
    return path;
}

static FILE *openFile(char *filename) {

    filename = getFullPath(filename);

    FILE *file = fopen(filename, "wb+");

    if (!file) {
        TraceLog(LOG_ERROR, "Could not open file %s.", filename);
        return false;
    }

    TraceLog(LOG_DEBUG, "Opened file '%s'.", filename);

    return file;
}

static void closeFile(FILE *file) {

    fclose(file);
    TraceLog(LOG_DEBUG, "Closed file.");
}

static bool writeToFile(FILE *file, void *data, size_t structSize, size_t itemsCount) {

    size_t itemsWritten = fwrite(data, structSize, itemsCount, file);

    if (itemsWritten != itemsCount) {
        TraceLog(LOG_ERROR,
            "Error writing to file. Written %d items; expected %d.", itemsCount, itemsWritten);
        return false;
    }

    TraceLog(LOG_DEBUG, "Written to file. (%d bytes)", structSize * itemsCount);

    return true;
}

void *loadFromFile(char *filename) {
    return filename; // TODO
}

bool FilesSave(void *data, size_t structSize, size_t itemsCount) {
    FILE *file = openFile(LEVEL_NAME);
    bool result = writeToFile(file, data, structSize, itemsCount);
    closeFile(file);
    return result;
}

void *FilesLoad() {
    return loadFromFile(LEVEL_NAME);
}
