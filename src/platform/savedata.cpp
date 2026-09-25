#include "platform/savedata.h"
#include <cstring>

namespace {

struct Entry {
    const char*  key;
    unsigned short fmt;
    unsigned int   len;
    unsigned int   max;
    const void*    data;
};

void put32(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)(v);        p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16);  p[3] = (unsigned char)(v >> 24);
}
void put16(unsigned char* p, unsigned short v) {
    p[0] = (unsigned char)(v);        p[1] = (unsigned char)(v >> 8);
}

}

int sfoBuildSavedata(unsigned char* out, int cap, const char* title,
                     const char* saveTitle, const char* detail, const char* dirName) {

    static const char kCategory[] = "MS";
    const unsigned int zeroInt = 0;

    static unsigned char fileList[3168];
    static unsigned char params[128];
    memset(fileList, 0, sizeof(fileList));
    memset(params, 0, sizeof(params));

    const Entry entries[] = {
        { "CATEGORY",           0x0204, 3,    4,    kCategory },
        { "PARENTAL_LEVEL",     0x0404, 4,    4,    &zeroInt  },
        { "SAVEDATA_DETAIL",    0x0204, 0,    1024, detail    },
        { "SAVEDATA_DIRECTORY", 0x0204, 0,    64,   dirName   },
        { "SAVEDATA_FILE_LIST", 0x0004, 3168, 3168, fileList  },
        { "SAVEDATA_PARAMS",    0x0004, 128,  128,  params    },
        { "SAVEDATA_TITLE",     0x0204, 0,    128,  saveTitle },
        { "TITLE",              0x0204, 0,    128,  title     },
    };
    const int n = (int)(sizeof(entries) / sizeof(entries[0]));

    int keyBytes = 0;
    for (int i = 0; i < n; i++) keyBytes += (int)strlen(entries[i].key) + 1;

    while (keyBytes & 3) keyBytes++;

    const int keyStart  = 0x14 + n * 16;
    const int dataStart = keyStart + keyBytes;
    int dataBytes = 0;
    for (int i = 0; i < n; i++) dataBytes += (int)entries[i].max;
    const int total = dataStart + dataBytes;
    if (cap < total) return 0;
    memset(out, 0, (size_t)total);

    put32(out + 0x00, 0x46535000u);
    put32(out + 0x04, 0x00000101u);
    put32(out + 0x08, (unsigned int)keyStart);
    put32(out + 0x0C, (unsigned int)dataStart);
    put32(out + 0x10, (unsigned int)n);

    int keyOff = 0, dataOff = 0;
    for (int i = 0; i < n; i++) {
        const Entry& e = entries[i];
        unsigned int len = e.len;
        if (e.fmt == 0x0204 && len == 0)
            len = (unsigned int)strlen((const char*)e.data) + 1;
        if (len > e.max) len = e.max;

        unsigned char* ix = out + 0x14 + i * 16;
        put16(ix + 0, (unsigned short)keyOff);
        put16(ix + 2, e.fmt);
        put32(ix + 4, len);
        put32(ix + 8, e.max);
        put32(ix + 12, (unsigned int)dataOff);

        const int klen = (int)strlen(e.key) + 1;
        memcpy(out + keyStart + keyOff, e.key, (size_t)klen);
        keyOff += klen;

        if (e.fmt == 0x0404) put32(out + dataStart + dataOff, *(const unsigned int*)e.data);
        else                 memcpy(out + dataStart + dataOff, e.data, (size_t)len);
        dataOff += (int)e.max;
    }
    return total;
}
