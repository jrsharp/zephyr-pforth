/*
 * pForth Zephyr File I/O Header
 *
 * Provides file I/O functions for pForth using the namespace layer.
 * This file is included by pForth when PF_USER_FILEIO is defined.
 *
 * The namespace layer provides unified access to:
 *   - Local 9P servers (/dev, /env, filesystem)
 *   - Remote 9P mounts (/kbd, network filesystems)
 *
 * Copyright (c) 2025 Jon Sharp
 * SPDX-License-Identifier: MIT
 */

#ifndef PF_FILEIO_ZEPHYR_H
#define PF_FILEIO_ZEPHYR_H

#include <stdint.h>
#include <stddef.h>

/* Get file_offset_t from pForth types */
#include "pf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * FileStream wrapper for namespace file descriptors
 *
 * pForth expects FileStream* pointers, but the namespace layer uses
 * integer file descriptors. This structure wraps the fd and provides
 * the necessary interface.
 */
typedef struct {
    int fd;           /* Namespace file descriptor */
    int eof;          /* EOF flag */
    int error;        /* Error flag */
} PfFileStream;

typedef PfFileStream FileStream;

/* Standard streams (not used in embedded context, but required) */
extern FileStream *PF_STDIN;
extern FileStream *PF_STDOUT;

/* Seek modes */
#define PF_SEEK_SET  0
#define PF_SEEK_CUR  1
#define PF_SEEK_END  2

/* Note: file_offset_t is already defined in pf_types.h as 'long' */

/*
 * File I/O functions for pForth
 *
 * These map to the namespace layer (ns_open, ns_read, etc.)
 */

/**
 * Open a file
 *
 * @param FileName Path to file (namespace-aware: /dev/..., /kbd/..., etc.)
 * @param Mode     fopen-style mode string ("r", "w", "a", "r+", "w+", "a+")
 * @return FileStream pointer on success, NULL on failure
 */
FileStream *sdOpenFile(const char *FileName, const char *Mode);

/**
 * Close a file
 *
 * @param Stream File stream to close
 * @return 0 on success, non-zero on failure
 */
int32_t sdCloseFile(FileStream *Stream);

/**
 * Read from a file
 *
 * @param ptr     Buffer to read into
 * @param Size    Size of each element
 * @param nItems  Number of elements to read
 * @param Stream  File stream
 * @return Number of elements read
 */
int32_t sdReadFile(void *ptr, int32_t Size, int32_t nItems, FileStream *Stream);

/**
 * Write to a file
 *
 * @param ptr     Buffer to write from
 * @param Size    Size of each element
 * @param nItems  Number of elements to write
 * @param Stream  File stream
 * @return Number of elements written
 */
int32_t sdWriteFile(void *ptr, int32_t Size, int32_t nItems, FileStream *Stream);

/**
 * Read a single character
 *
 * @param Stream File stream
 * @return Character read, or -1 on EOF/error
 */
int32_t sdInputChar(FileStream *Stream);

/**
 * Seek within a file
 *
 * @param Stream   File stream
 * @param Position Offset to seek to
 * @param Mode     PF_SEEK_SET, PF_SEEK_CUR, or PF_SEEK_END
 * @return 0 on success, non-zero on failure
 */
int32_t sdSeekFile(FileStream *Stream, file_offset_t Position, int32_t Mode);

/**
 * Get current position in file
 *
 * @param Stream File stream
 * @return Current offset, or -1 on error
 */
file_offset_t sdTellFile(FileStream *Stream);

/**
 * Flush file buffers
 *
 * @param Stream File stream
 * @return 0 on success, non-zero on failure
 */
int32_t sdFlushFile(FileStream *Stream);

/**
 * Delete a file
 *
 * @param FileName Path to file
 * @return 0 on success, non-zero on failure
 */
int32_t sdDeleteFile(const char *FileName);

/**
 * Rename a file
 *
 * @param OldName Current path
 * @param NewName New path
 * @return 0 on success, non-zero on failure
 */
int32_t sdRenameFile(const char *OldName, const char *NewName);

/**
 * Resize a file
 *
 * @param Stream  File stream
 * @param NewSize New size in bytes
 * @return 0 on success, throw code on failure
 */
int32_t sdResizeFile(FileStream *Stream, uint64_t NewSize);

#ifdef __cplusplus
}
#endif

#endif /* PF_FILEIO_ZEPHYR_H */
