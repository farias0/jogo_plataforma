#ifndef _FILES_H_INCLUDED_
#define _FILES_H_INCLUDED_


typedef struct FileData {
    void *data;
    size_t itemSize;
    size_t itemCount;
} FileData;


bool FileSave(FileData data);
FileData FileLoad(size_t itemSize);


#endif // _FILES_H_INCLUDED_