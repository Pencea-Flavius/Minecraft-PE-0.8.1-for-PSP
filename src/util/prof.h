#pragma once

#ifndef PROF
#define PROF 0
#endif

enum {
    PROF_TICK,
    PROF_TPLAYER,
    PROF_TWORLD,
    PROF_TRAND,
    PROF_TPEND,
    PROF_TENT,
    PROF_TTE,
    PROF_TPART,
    PROF_WORLD,
    PROF_STREAM,
    PROF_SGEN,
    PROF_SDECOR,
    PROF_SLIGHT,
    PROF_SDISK,
    PROF_SEVICT,
    PROF_SMISC,
    PROF_LIGHT,
    PROF_REBUILD,
    PROF_CULL,
    PROF_RSCAN,
    PROF_RBUILD,

    PROF_MEMIT,
    PROF_MPACK,
    PROF_MALLOC,
    PROF_MCONV,

    PROF_SKY,
    PROF_ENTITY,
    PROF_WATER,
    PROF_PART,
    PROF_HUD,
    PROF_GESYNC,
    PROF_VBLANK,
    PROF_N
};

enum { PROFC_PARTICLES, PROFC_SECTIONS, PROFC_PENDLIST, PROFC_STREAMIN,
       PROFC_DRAWLIVE, PROFC_PACKVERTS,

       PROFC_MARKED, PROFC_N };

#if PROF
void profListBytes(unsigned bytes);
void profAdd(int slot, int n);
void profBegin(int slot);
void profEnd(int slot);
void profFrameEnd(void);
#else
static inline void profListBytes(unsigned) {}
static inline void profAdd(int, int) {}
static inline void profBegin(int) {}
static inline void profEnd(int) {}
static inline void profFrameEnd(void) {}
#endif
