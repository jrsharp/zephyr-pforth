/*
 * pForth Zephyr Configuration Header
 *
 * This file is included by pForth via PF_USER_INC1 and configures
 * the Forth system for Zephyr RTOS.
 *
 * Copyright (c) 2025 Jon Sharp
 * SPDX-License-Identifier: MIT
 */

#ifndef PF_ZEPHYR_CONFIG_H
#define PF_ZEPHYR_CONFIG_H

#include <zephyr/kernel.h>
#include <string.h>
#include <stdlib.h>

/* Use PSRAM for pForth memory allocation on ESP32 */
#ifdef CONFIG_ESP_SPIRAM
/* Static arrays of plain data that need not use internal RAM */
#define PF_EXT_RAM __attribute__((section(".ext_ram.bss")))
#include <zephyr/multi_heap/shared_multi_heap.h>
/* pForth uses pfAllocMem/pfFreeMem - route to PSRAM via shared_multi_heap */
#define pfInitMemoryAllocator()
#define pfAllocMem(size)   shared_multi_heap_alloc(SMH_REG_ATTR_EXTERNAL, size)
#define pfFreeMem(ptr)     shared_multi_heap_free(ptr)
/* Tell pf_mem.h we're providing custom allocator */
#define PF_USER_MALLOC     "pforth/pf_zephyr_config.h"
#else
/* Fallback to Zephyr's standard heap */
#define pfInitMemoryAllocator()
#define pfAllocMem(size)   k_malloc(size)
#define pfFreeMem(ptr)     k_free(ptr)
#define PF_USER_MALLOC     "pforth/pf_zephyr_config.h"
#endif

/* Use custom character I/O - implemented in pf_io_zephyr.c */
/* Note: PF_USER_CHARIO path is relative to the include directories */
#define PF_USER_CHARIO     "pforth/pf_io_zephyr.h"

/* Key echo behavior - Zephyr I/O layer handles echo */
#define PF_KEY_ECHOS       0

/* Enable/disable file I/O based on Kconfig */
/* Note: PF_NO_FILEIO is defined via CMakeLists.txt when CONFIG_PFORTH_FILEIO=n */
#ifdef CONFIG_PFORTH_FILEIO
    #ifdef CONFIG_PFORTH_FILEIO_NAMESPACE
        #define PF_USER_FILEIO "pforth/pf_fileio_zephyr.h"
        /* Path to system.fth on namespace filesystem */
        #define SYSTEM_LOAD_FILE "/n/local/forth/system.fth"
    #endif
#endif

/* Floating point support based on Kconfig */
#ifdef CONFIG_PFORTH_FLOATING_POINT
    #include <math.h>
    #define PF_SUPPORT_FP 1
    typedef double PF_FLOAT;
    /* Use standard math functions */
    #define fp_acos   acos
    #define fp_asin   asin
    #define fp_atan   atan
    #define fp_atan2  atan2
    #define fp_cos    cos
    #define fp_cosh   cosh
    #define fp_fabs   fabs
    #define fp_floor  floor
    #define fp_log    log
    #define fp_log10  log10
    #define fp_log1p  log1p
    #define fp_pow    pow
    #define fp_sin    sin
    #define fp_sinh   sinh
    #define fp_sqrt   sqrt
    #define fp_tan    tan
    #define fp_tanh   tanh
    #define fp_round  round
#else
    #define PF_SUPPORT_FP 0
    /* Stub type needed for header compatibility even when FP disabled */
    typedef double PF_FLOAT;
    /* Stub FP functions - these should never be called when FP is disabled */
    static inline double fp_acos(double x) { (void)x; return 0.0; }
    static inline double fp_asin(double x) { (void)x; return 0.0; }
    static inline double fp_atan(double x) { (void)x; return 0.0; }
    static inline double fp_atan2(double y, double x) { (void)y; (void)x; return 0.0; }
    static inline double fp_cos(double x) { (void)x; return 0.0; }
    static inline double fp_cosh(double x) { (void)x; return 0.0; }
    static inline double fp_fabs(double x) { (void)x; return 0.0; }
    static inline double fp_floor(double x) { (void)x; return 0.0; }
    static inline double fp_log(double x) { (void)x; return 0.0; }
    static inline double fp_log10(double x) { (void)x; return 0.0; }
    static inline double fp_log1p(double x) { (void)x; return 0.0; }
    static inline double fp_pow(double x, double y) { (void)x; (void)y; return 0.0; }
    static inline double fp_sin(double x) { (void)x; return 0.0; }
    static inline double fp_sinh(double x) { (void)x; return 0.0; }
    static inline double fp_sqrt(double x) { (void)x; return 0.0; }
    static inline double fp_tan(double x) { (void)x; return 0.0; }
    static inline double fp_tanh(double x) { (void)x; return 0.0; }
    static inline double fp_round(double x) { (void)x; return 0.0; }
#endif

/* EOF constant — not always available in Zephyr's minimal libc */
#ifndef EOF
#define EOF (-1)
#endif

/* Use standard C library functions */
/* Zephyr provides these in its minimal libc or newlib */

/* Debug printing - use Zephyr logging */
#ifdef CONFIG_LOG
    #include <zephyr/logging/log.h>
    #define PRT(x) /* Disabled - use LOG_* macros instead */
#else
    #define PRT(x)
#endif

/* Unused parameter macro */
#ifndef TOUCH
    #define TOUCH(x) ((void)(x))
#endif

#endif /* PF_ZEPHYR_CONFIG_H */
