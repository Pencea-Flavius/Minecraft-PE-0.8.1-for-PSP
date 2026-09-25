#include "world/level/tile/tiles.h"
#include "world/level/chunk/chunk.h"

void tileNetherReactor(unsigned char data, int f, int* col, int* row) {
    switch (data) {
        case 1:  *col = 14; *row = 9;  break;
        case 2:  *col = 14; *row = 10; break;
        default: *col = 15; *row = 9;  break;
    }
}
