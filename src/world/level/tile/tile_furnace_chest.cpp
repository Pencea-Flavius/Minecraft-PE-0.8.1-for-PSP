#include "world/level/tile/tiles.h"
#include "world/level/chunk/chunk.h"

void tileFurnace(unsigned char data, int f, bool lit, int* col, int* row) {
    if (f == F_TOP || f == F_DOWN) { *col = 14; *row = 3; }
    else if (f == faceFromMcpe(data)) { *col = lit ? 13 : 12; *row = lit ? 3 : 2; }
    else                           { *col = 13; *row = 2; }
}

void tileChest(unsigned char data, int f, int* col, int* row) {
    (void)data; (void)f;
    *col = 4; *row = 0;
}
