/* Copyright (c) 2026 Burak Yazar */

#ifndef PLATFRM_H
#define PLATFRM_H

/* Win32 platform backend — OpenGL 3.3 + GLFW + GLEW + OpenAL.
   On DOS/DJGPP this header is a no-op; the existing VGA/IO/BLAST files
   provide everything. On Win32 (PLATFORM_WIN64 defined) PLATFRM.C
   provides drop-in replacements for those DOS-specific modules. */

#ifdef PLATFORM_WIN64

/*  MSVC compatibility  */
#ifdef _MSC_VER
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES /* enables M_PI etc. in <math.h> */
#endif
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif
#ifdef __inline__
#undef __inline__
#endif
#define __inline__ __forceinline
#endif /* _MSC_VER */

#include "CORE/TYPES.H"

#define WIN32_WINDOW_SCALE 4

/* Audio */
#define PLAT_AL_BUF_COUNT 4
#define PLAT_AL_BUF_FRAMES 512

/*  Port I/O stubs (no hardware access on Win32)  */
#define outp(port, val) ((void)(port), (void)(val))
#define outportw(port, val) ((void)(port), (void)(val))
#define inp(port) ((void)(port), 0)

/*  uclock compatibility (replaces DJGPP <time.h> uclock)  */
typedef long long uclock_t;
#define UCLOCKS_PER_SEC 1000000LL
uclock_t uclock(void);

/*  conio color constants (from DJGPP <conio.h>)  */
#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define LIGHTGRAY 7
#define DARKGRAY 8
#define LIGHTBLUE 9
#define LIGHTGREEN 10
#define LIGHTCYAN 11
#define LIGHTRED 12
#define LIGHTMAGENTA 13
#define YELLOW 14
#define WHITE 15

/**
 * @brief ---- conio compatibility stubs
 * @param c
 */
void textbackground(int c);
void textcolor(int c);
int cprintf(const char *fmt, ...);
int getch(void);

/*  High-resolution platform timer  */
typedef long long plat_ticks_t;
plat_ticks_t plat_ticks(void);
plat_ticks_t plat_ticks_per_sec(void);

/*  DOS file-finding compatibility  */
#define FA_ARCH 0x20
#define FA_DIREC 0x10

struct ffblk
{
   unsigned ff_attrib;
   char ff_name[260]; /* cFileName */
   void *_h;          /* HANDLE (opaque) */
   char _fd[320];     /* WIN32_FIND_DATAA (opaque, <=320 bytes) */
};

int findfirst(const char *path, struct ffblk *ff, unsigned attrib);
int findnext(struct ffblk *ff);

/*  Startup helpers  */

/**
 * @brief Navigate the process CWD to the project root (parent of the EXE directory). Call once at the very top of main() so all relative asset paths work regardless of where the EXE lives (e.g. inside a BIN/ subdirectory).
 */
void platfrm_chdir_project_root(void);

/**
 * @brief Returns pointer to the 768-byte 6-bit DAC palette (256*RGB).
 */
const u8 *platfrm_get_palette(void);

/**
 * @brief Reciprocal table init
 */
void win64_init_recip_table(void);

/* =========================================================================
   Multi-Threading Helpers
   ========================================================================= */

#define PLAT_THREAD_CC __stdcall

typedef void *PlatThread;
typedef void *PlatEvent;

/* Thread function signature */
typedef unsigned long(PLAT_THREAD_CC *PlatThreadFunc)(void *arg);

/* Thread API */
PlatThread plat_thread_create(PlatThreadFunc func, void *arg);
void plat_thread_join(PlatThread thread);
void plat_thread_close(PlatThread thread);

/* Event API */
PlatEvent plat_event_create(int manual_reset, int initial_state);
void plat_event_destroy(PlatEvent event);
void plat_event_signal(PlatEvent event);
void plat_event_reset(PlatEvent event);
void plat_event_wait(PlatEvent event);

/* Atomic API */
long plat_atomic_increment(volatile long *val);
long plat_atomic_decrement(volatile long *val);

/* System API */
int plat_get_core_count(void);

#endif /* PLATFORM_WIN64 */

/* =========================================================================
   Cycle Profiling (Cross-Platform)
   ========================================================================= */

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(__rdtsc)
#define PLAT_RDTSC() __rdtsc()
#else
static __inline__ unsigned long long plat_rdtsc(void)
{
   unsigned int lo, hi;
   __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
   return ((unsigned long long)hi << 32) | lo;
}
#define PLAT_RDTSC() plat_rdtsc()
#endif

typedef struct
{
   unsigned long long total_cycles;
   unsigned int total_calls;
   unsigned int report_threshold;
   const char *name;
} PlatProfiler;

void plat_profiler_init(PlatProfiler *p, const char *name, unsigned int threshold);
void plat_profiler_add(PlatProfiler *p, unsigned long long start_cycles, unsigned long long end_cycles);

#endif /* PLATFRM_H */
