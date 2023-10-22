#include <stdio.h>
#include <string.h>
#include <raylib.h>

#include "files.h"


static char *LEVELS_DIR = "../levels/";


static bool writeToFile(char *name, void *data, size_t structSize, size_t itemsCount) {

    FILE *file;
    char path[100];
    
    strcat(path, LEVELS_DIR);
    strcat(path, name);

    file = fopen(path, "wb+");

    if (!file) {
        TraceLog(LOG_ERROR, "Could not open file %s.", path);
        return false;
    }

    TraceLog(LOG_DEBUG, "Opened file %s.", path);


    size_t itemsWritten = fwrite(data, structSize, itemsCount, file);

    if (itemsWritten != itemsCount) {
        TraceLog(LOG_ERROR,
            "Error writing to file %s. Written %d items; expected %d.", path, itemsCount, itemsWritten);
        return false;
    }


    fclose(file);
    TraceLog(LOG_DEBUG, "Closed file %s.", path);

    return true;
}

bool FilesSave(void *data, size_t structSize, size_t itemsCount) {
    return writeToFile("my_level.lvl", data, structSize, itemsCount);
}
