#ifndef MESH_SINK_H
#define MESH_SINK_H

struct ChunkVertex;
struct MeshSink;

struct MeshSink {
    ChunkVertex* buf[4];
    int          cap[4];
    int          n[4];
    int          total[4];

    bool       (*flush)(MeshSink*, int layer);
    void*        ctx;
};

#ifndef MESH_RESERVE_CHECK
#define MESH_RESERVE_CHECK 0
#endif

#if MESH_RESERVE_CHECK
extern int g_sinkReserved[4];
extern int g_sinkStart[4];
extern unsigned int g_sinkOverruns;
#endif

static inline void sinkCheckPrev(MeshSink* sk, int layer) {
#if MESH_RESERVE_CHECK
    if (g_sinkReserved[layer] >= 0 && sk->n[layer] - g_sinkStart[layer] > g_sinkReserved[layer])
        g_sinkOverruns++;
    g_sinkReserved[layer] = -1;
#else
    (void)sk; (void)layer;
#endif
}

static inline bool sinkReserve(MeshSink* sk, int layer, int need) {
    sinkCheckPrev(sk, layer);
    if (sk->n[layer] + need <= sk->cap[layer]) {
#if MESH_RESERVE_CHECK

        g_sinkReserved[layer] = need; g_sinkStart[layer] = sk->n[layer];
#endif
        return true;
    }
    if (!sk->flush) return false;
    if (!sk->flush(sk, layer)) return false;
#if MESH_RESERVE_CHECK
    g_sinkReserved[layer] = need; g_sinkStart[layer] = sk->n[layer];
#endif

    return need <= sk->cap[layer];
}

static inline int sinkCount(const MeshSink* sk, int layer) {
    return sk->total[layer] + sk->n[layer];
}

int meshSectionSink(const World* w, int ox, int oz, int y0, int y1,
                    MeshSink* sk, int* nLava, bool leavesOpaque, bool leavesCull);

struct DrawVertex;
struct World;

void chunkPackInto(DrawVertex* d, const ChunkVertex* s, int n,
                   int ox, int oy, int oz, int* qlo, int* qhi);

DrawVertex* chunkPackFinish(const DrawVertex* staging, int n);

float chunkPackDecodeY(int q, int oy);

#endif
