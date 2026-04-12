/*
 * pForth Static Dictionary Stub for Zephyr
 *
 * This file provides empty stubs when no pre-compiled dictionary
 * is available. pfLoadStaticDictionary() will fail validation
 * and the system will fall back to pfBuildDictionary().
 *
 * To use a real static dictionary:
 * 1. Build pforth on a host system
 * 2. Run: pforth fth/mkdicdat.fth
 * 3. Replace this file with the generated pfdicdat.h
 */

#ifndef PFDICDAT_H
#define PFDICDAT_H

#include <stdint.h>

/*
 * Dictionary configuration values.
 * These minimal values will cause validation to fail,
 * triggering fallback to runtime dictionary building.
 */
#define HEADERPTR       (0)
#define RELCONTEXT      (0)
#define CODEPTR         (0)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IF_LITTLE_ENDIAN (1)
#else
#define IF_LITTLE_ENDIAN (0)
#endif

/*
 * Empty dictionary data.
 * Minimal arrays that won't pass validation.
 */
static const uint8_t MinDicNames[] = { 0 };
static const uint8_t MinDicCode[] = { 0 };

#endif /* PFDICDAT_H */
