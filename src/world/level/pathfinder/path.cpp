#include "world/level/pathfinder/path.h"
#include "world/entity/entity.h"

int Path::p = 0;

Path::Path() : length(0), index(0) { ++p; }
Path::~Path() { destroy(); }

Path& Path::operator=(const Path& o) {
    if (this == &o) return *this;
    length = o.length;
    index  = o.index;
    for (int i = 0; i < length; ++i) nodes[i] = o.nodes[i];
    return *this;
}

bool Path::isEmpty() const { return length == 0; }

void Path::copyNodes(Node** src, int len) {
    if (len > MAX_PATH) len = MAX_PATH;
    length = len;
    index = 0;
    for (int i = 0; i < len; ++i) nodes[i] = *src[i];
}

void Path::destroy() { index = length = 0; }

Node* Path::currentPos() { return length ? (Node*)&nodes[index] : 0; }
Vec3  Path::currentPos(Entity* e) const { return getPos(e, index); }
void  Path::next() { index++; }
int   Path::getSize() const { return length; }
bool  Path::isDone() const { return index >= length; }
Node* Path::last() const { return length > 0 ? (Node*)&nodes[length - 1] : 0; }
Node* Path::get(int i) const { return (Node*)&nodes[i]; }
int   Path::getIndex() const { return index; }
void  Path::setIndex(int i) { index = i; }

void Path::setSize(int size) {
    if (size < 0) size = 0;
    if (size > length) return;
    length = size;
}

bool Path::sameAs(const Path& other) const {
    if (other.length != length) return false;
    for (int i = 0; i < length; ++i)
        if (nodes[i].x != other.nodes[i].x ||
            nodes[i].y != other.nodes[i].y ||
            nodes[i].z != other.nodes[i].z) return false;
    return true;
}

Vec3 Path::getPos(Entity* e, int i) const {
    float x = nodes[i].x + (int)(e->bbWidth + 1) * 0.5f;
    float z = nodes[i].z + (int)(e->bbWidth + 1) * 0.5f;
    float y = nodes[i].y;
    return Vec3(x, y, z);
}
