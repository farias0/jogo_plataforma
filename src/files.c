#include <stdio.h>
#include <string.h>
#include <raylib.h>

#include "files.h"

#define MODE_READ "ab+"
#define MODE_WRITE "wb+"


static size_t MAX_ITEM_COUNT = 16384;


static FILE *openFile(char *filepath, char* mode) {

    FILE *file = fopen(filepath, mode);

    if (!file) {
        TraceLog(LOG_ERROR, "Could not open file %s.", filepath);
        return false;
    }

    TraceLog(LOG_TRACE, "Opened file '%s'.", filepath);

    return file;
}

static void closeFile(FILE *file) {

    fclose(file);
    TraceLog(LOG_TRACE, "Closed file.");
}

FileData readFromFile(FILE *file, size_t itemSize) {

    FileData data;
    void *buffer = MemAlloc(itemSize * MAX_ITEM_COUNT);

    data.itemSize = itemSize;

    data.itemCount = fread(buffer, itemSize, MAX_ITEM_COUNT, file);

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

    if (data.itemCount > MAX_ITEM_COUNT) {
        TraceLog(LOG_ERROR,
            "Writing to file, exceeded max item count. Count: %d, max: %d.", data.itemCount, MAX_ITEM_COUNT);
        return false;
    }

    size_t itemsWritten = fwrite(data.data, data.itemSize, data.itemCount, file);

    if (itemsWritten != data.itemCount) {
        TraceLog(LOG_ERROR,
            "Error writing to file. Written %d items; expected %d.", data.itemCount, itemsWritten);
        return false;
    }

    TraceLog(LOG_DEBUG, "Written to file. (%d bytes)", data.itemSize * data.itemCount);

    return true;
}

bool FileSave(char *filepath, FileData data) {

    FILE *file = openFile(filepath, MODE_WRITE);
    bool result = writeToFile(file, data);
    closeFile(file);
    return result;
}

FileData FileLoad(char *filepath, size_t itemSize) {

    FILE *file = openFile(filepath, MODE_READ);
    rewind(file);
    FileData data = readFromFile(file, itemSize);
    closeFile(file);
    return data;
}
