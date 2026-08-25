/* Copyright (c) 2026 Burak Yazar */

#ifndef CLIPSPM_H
#define CLIPSPM_H
#include "../CORE/TYPES/GFX.H"

/* Inline pool slots before heap fallback.
   MAX_PORTAL_DEPTH(8) + 1 root clip = 9 max in-flight; 12 gives headroom. */
#define CSP_POOL_CAP 12

/**
 * @brief Alloc one ClipSpans from the inline pool; falls back to malloc if full.
 */
ClipSpans *csp_push(void);

/**
 * @brief Release the last pushed ClipSpans (strict LIFO). Frees if heap-allocated.
 */
void csp_pop(void);

#endif /* CLIPSPM_H */
