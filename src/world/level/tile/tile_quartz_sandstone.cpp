#include "world/level/tile/tiles.h"
#include "world/level/chunk/chunk.h"

void tileQuartzBlock(unsigned char data, int f, int* col, int* row) {
    if (f == F_TOP)       { *col = 11; *row = 13; }
    else if (f == F_DOWN) { *col = 11; *row = 15; }
    else                  { *col = 11; *row = 14; }
}

void tileChiseledSandstone(unsigned char data, int f, int* col, int* row) {
    if (f == F_TOP || f == F_DOWN) { *col = 0; *row = 11; }
    else                           { *col = 5; *row = 14; }
}

void tileSmoothSandstone(unsigned char data, int f, int* col, int* row) {
    if (f == F_TOP || f == F_DOWN) { *col = 0; *row = 11; }
    else                           { *col = 6; *row = 14; }
}

void tileChiseledQuartz(unsigned char data, int f, int* col, int* row) {
    if (f == F_TOP || f == F_DOWN) { *col = 9; *row = 13; }
    else                           { *col = 9; *row = 14; }
}

void tilePillarQuartz(unsigned char data, int f, int* col, int* row) {
    if (f == F_TOP || f == F_DOWN) { *col = 10; *row = 13; }
    else                           { *col = 10; *row = 14; }
}
