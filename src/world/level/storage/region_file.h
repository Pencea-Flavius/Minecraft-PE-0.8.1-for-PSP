
#ifndef MCPSP_WORLD_STORAGE_REGION_FILE_H
#define MCPSP_WORLD_STORAGE_REGION_FILE_H

#include <cstdio>

#define RF_MAX_SECTORS (1024 * 255 + 1)

class RegionFile {
public:

    RegionFile(const char* path);
    ~RegionFile();

    bool open();
    bool isOpen() const { return file != 0; }

    bool readChunk(int x, int z, unsigned char** dest, int* destLen);
    bool writeChunk(int x, int z, const unsigned char* data, int len);

private:
    bool write(int sector, const unsigned char* data, int len);
    void close();

    bool sectorKnown(int i) const { return i >= 0 && i < knownSectors; }
    bool sectorIsFree(int i) const {
        return sectorKnown(i) && (freeBits[i >> 3] & (1 << (i & 7))) != 0;
    }
    void sectorSetFree(int i, bool f) {
        if (i < 0 || i >= RF_MAX_SECTORS) return;
        if (i >= knownSectors) knownSectors = i + 1;
        if (f) freeBits[i >> 3] |=  (unsigned char)(1 << (i & 7));
        else   freeBits[i >> 3] &= (unsigned char)~(1 << (i & 7));
    }

    FILE* file;
    char  filename[320];
    int*  offsets;
    int*  emptyChunk;
    unsigned char* freeBits;
    int   knownSectors;
};

#endif
