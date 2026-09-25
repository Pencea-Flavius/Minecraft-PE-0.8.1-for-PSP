#ifndef MCPSP_NBT_BYTE_ARRAY_TAG_H
#define MCPSP_NBT_BYTE_ARRAY_TAG_H
#include "nbt/tag.h"
#include <cstring>

struct TagMemoryChunk {
    void* data;
    int len;
    TagMemoryChunk() : data(0), len(0) {}
};
class ByteArrayTag : public Tag {
public:
    TagMemoryChunk data;
    ByteArrayTag(const std::string& n) : Tag(n) {}
    ByteArrayTag(const std::string& n, TagMemoryChunk d) : Tag(n), data(d) {}

    ~ByteArrayTag() { delete[] (char*)data.data; data.data = 0; data.len = 0; }
    char getId() const { return TAG_Byte_Array; }
    void write(IDataOutput* o) { o->writeInt(data.len); o->writeBytes(data.data, data.len); }
    void load(IDataInput* i) {

        static const int MAX_LENGTH = 1 << 20;
        int length = i->readInt();
        if (length < 0 || i->failed()) length = 0;
        if (length > MAX_LENGTH) length = MAX_LENGTH;

        delete[] (char*)data.data;
        data.data = length ? new char[length] : 0;
        data.len = length;
        if (length) i->readBytes(data.data, length);
    }
};
#endif
