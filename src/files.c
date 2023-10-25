#include <stdio.h>
#include <string.h>
#include <raylib.h>

#include "files.h"

#define MODE_READ "ab+"
#define MODE_WRITE "wb+"


static char *LEVELS_DIR = "../levels/";

static char *LEVEL_NAME = "my_level.lvl";

/*
    1048576 bytes = 1 MiB.
    For a struct size of 64 bytes, this means 16384 entities.
*/
static size_t READ_BUFFER_SIZE = 1048576;


static char *getFullPath(char *filename) {

    char *path = MemAlloc(sizeof(char) * 100);
    
    strcat(path, LEVELS_DIR);
    strcat(path, filename);
    
    return path;
}

static FILE *openFile(char *filename, char* mode) {

    filename = getFullPath(filename);

    FILE *file = fopen(filename, mode);

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

FileData readFromFile(FILE *file, size_t itemSize) {

    FileData data;
    void *buffer = MemAlloc(itemSize * READ_BUFFER_SIZE);

    data.itemSize = itemSize;

    data.itemCount = fread(buffer, itemSize, READ_BUFFER_SIZE, file);

    if (!data.itemCount) {
        TraceLog(LOG_ERROR, "Error reading file.");
        return data;
    }

    data.data = MemRealloc(buffer, itemSize * data.itemCount);

    TraceLog(LOG_DEBUG,
        "Read from file. (%d items, %d bytes)", data.itemCount, data.itemSize * data.itemCount);

    return data;
}

static bool writeToFile(FILE *file, FileData data) {

    size_t itemsWritten = fwrite(data.data, data.itemSize, data.itemCount, file);

    if (itemsWritten != data.itemCount) {
        TraceLog(LOG_ERROR,
            "Error writing to file. Written %d items; expected %d.", data.itemCount, itemsWritten);
        return false;
    }

    TraceLog(LOG_DEBUG, "Written to file. (%d bytes)", data.itemSize * data.itemCount);

    return true;
}

bool FileSave(FileData data) {

    FILE *file = openFile(LEVEL_NAME, MODE_WRITE);
    bool result = writeToFile(file, data);
    closeFile(file);
    return result;
}

FileData FileLoad(size_t itemSize) {

    FILE *file = openFile(LEVEL_NAME, MODE_READ);
    rewind(file);
    FileData data = readFromFile(file, itemSize);
    closeFile(file);
    return data;
}
