#ifndef _FILES_H_INCLUDED_
#define _FILES_H_INCLUDED_


#include "string"


namespace Files {


typedef struct FileData {
    void *data;
    size_t itemSize;
    size_t itemCount;
} FileData;

FileData DataLoad(char *filepath, size_t itemSize);
bool DataSave(char *filepath, FileData data);


std::string TextLoad(std::string filepath);
void TextSave(std::string filepath, std::string data);


} // namespace

#endif // _FILES_H_INCLUDED_
