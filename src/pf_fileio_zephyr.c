/*
 * pForth Zephyr File I/O Implementation
 *
 * Implements file I/O for pForth using the namespace layer.
 * This allows pForth to access any namespace-mounted resource:
 *   - Local filesystems (/fs/...)
 *   - 9P servers (/kbd/..., /dev/...)
 *   - Union mounts and binds
 *
 * Copyright (c) 2025 Jon Sharp
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <string.h>
#include <errno.h>

LOG_MODULE_REGISTER(pforth_fileio, CONFIG_PFORTH_LOG_LEVEL);

#include "pforth/pf_fileio_zephyr.h"

/* Include namespace layer - provided by the application */
#include "ns.h"
#include <zephyr/9p/protocol.h>

/* Maximum number of open file streams */
#define MAX_FILE_STREAMS 8

/* Default path prefix for relative filenames (no leading /) */
#define PFORTH_DEFAULT_PATH "/n/local/forth/"

/* Maximum path length for resolved filenames */
#define MAX_PATH_LEN 128

/* File stream pool */
static PfFileStream file_streams[MAX_FILE_STREAMS];
static bool file_streams_initialized;

/* Standard streams (placeholder - not really usable in embedded) */
static PfFileStream stdin_stream = { .fd = -1, .eof = 0, .error = 0 };
static PfFileStream stdout_stream = { .fd = -1, .eof = 0, .error = 0 };
FileStream *PF_STDIN = &stdin_stream;
FileStream *PF_STDOUT = &stdout_stream;

/*
 * Initialize file stream pool
 */
static void init_file_streams(void)
{
    if (file_streams_initialized) {
        return;
    }

    for (int i = 0; i < MAX_FILE_STREAMS; i++) {
        file_streams[i].fd = -1;
        file_streams[i].eof = 0;
        file_streams[i].error = 0;
    }

    file_streams_initialized = true;
}

/*
 * Allocate a file stream
 */
static PfFileStream *alloc_file_stream(void)
{
    init_file_streams();

    for (int i = 0; i < MAX_FILE_STREAMS; i++) {
        if (file_streams[i].fd < 0) {
            file_streams[i].eof = 0;
            file_streams[i].error = 0;
            return &file_streams[i];
        }
    }

    return NULL;  /* No free streams */
}

/*
 * Free a file stream
 */
static void free_file_stream(PfFileStream *stream)
{
    if (stream) {
        stream->fd = -1;
        stream->eof = 0;
        stream->error = 0;
    }
}

/*
 * Resolve filename to full path.
 * Relative paths (not starting with /) get the default prefix prepended.
 */
static const char *resolve_path(const char *filename, char *buf, size_t buflen)
{
    if (!filename || !*filename) {
        return filename;
    }

    /* Already absolute path? */
    if (filename[0] == '/') {
        return filename;
    }

    /* Prepend default path prefix */
    int ret = snprintf(buf, buflen, "%s%s", PFORTH_DEFAULT_PATH, filename);
    if (ret >= (int)buflen) {
        LOG_ERR("Path too long: %s%s", PFORTH_DEFAULT_PATH, filename);
        return filename;  /* Fall back to original */
    }

    LOG_DBG("Resolved '%s' -> '%s'", filename, buf);
    return buf;
}

/*
 * Parse fopen-style mode string to namespace mode
 *
 * Supported modes:
 *   "r"  - read only
 *   "w"  - write only, create/truncate
 *   "a"  - write only, append (create if needed)
 *   "r+" - read/write
 *   "w+" - read/write, create/truncate
 *   "a+" - read/write, append
 *
 * Note: Append mode is simulated by seeking to end after open.
 */
static int parse_mode(const char *mode, uint8_t *ns_mode, bool *create, bool *truncate, bool *append)
{
    if (!mode || !*mode) {
        return -EINVAL;
    }

    *create = false;
    *truncate = false;
    *append = false;

    switch (mode[0]) {
    case 'r':
        if (mode[1] == '+') {
            *ns_mode = NINEP_ORDWR;
        } else {
            *ns_mode = NINEP_OREAD;
        }
        break;

    case 'w':
        *create = true;
        *truncate = true;
        if (mode[1] == '+') {
            *ns_mode = NINEP_ORDWR;
        } else {
            *ns_mode = NINEP_OWRITE;
        }
        break;

    case 'a':
        *create = true;
        *append = true;
        if (mode[1] == '+') {
            *ns_mode = NINEP_ORDWR;
        } else {
            *ns_mode = NINEP_OWRITE;
        }
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

FileStream *sdOpenFile(const char *FileName, const char *Mode)
{
    uint8_t ns_mode;
    bool create, truncate, append;
    int fd;
    char resolved_path[MAX_PATH_LEN];
    const char *path;

    LOG_DBG("sdOpenFile: %s mode=%s", FileName, Mode);

    if (parse_mode(Mode, &ns_mode, &create, &truncate, &append) < 0) {
        LOG_ERR("Invalid mode: %s", Mode);
        return NULL;
    }

    PfFileStream *stream = alloc_file_stream();
    if (!stream) {
        LOG_ERR("No free file streams");
        return NULL;
    }

    /* Resolve relative paths to full namespace paths */
    path = resolve_path(FileName, resolved_path, sizeof(resolved_path));

    /* Try to open existing file first */
    fd = ns_open(path, ns_mode);

    if (fd < 0 && create) {
        /* File doesn't exist, try to create it */
        fd = ns_create(path, 0644, ns_mode);
    }

    if (fd < 0) {
        LOG_DBG("Failed to open %s: %d", path, fd);
        free_file_stream(stream);
        return NULL;
    }

    stream->fd = fd;

    /* Handle truncate - for now, just seek to start (truncate not fully supported) */
    if (truncate) {
        ns_seek(fd, 0, FS_SEEK_SET);
        /* Note: True truncation would require ns_truncate or similar */
    }

    /* Handle append by seeking to end */
    if (append) {
        /* SEEK_END not fully supported in namespace layer yet,
         * but we can read to find EOF or just start at 0 */
        /* For now, just start at current position */
    }

    LOG_DBG("Opened %s as fd %d", path, fd);
    return stream;
}

int32_t sdCloseFile(FileStream *Stream)
{
    if (!Stream || Stream->fd < 0) {
        return -1;
    }

    LOG_DBG("sdCloseFile: fd %d", Stream->fd);

    int ret = ns_close(Stream->fd);
    free_file_stream(Stream);

    return ret < 0 ? -1 : 0;
}

int32_t sdReadFile(void *ptr, int32_t Size, int32_t nItems, FileStream *Stream)
{
    if (!Stream || Stream->fd < 0 || !ptr || Size <= 0 || nItems <= 0) {
        return 0;
    }

    size_t total_bytes = (size_t)Size * (size_t)nItems;
    size_t bytes_read = 0;
    uint8_t *buf = (uint8_t *)ptr;

    while (bytes_read < total_bytes) {
        int ret = ns_read(Stream->fd, buf + bytes_read, total_bytes - bytes_read);

        if (ret < 0) {
            Stream->error = 1;
            break;
        }

        if (ret == 0) {
            Stream->eof = 1;
            break;
        }

        bytes_read += ret;
    }

    /* Return number of complete items read */
    return (int32_t)(bytes_read / Size);
}

int32_t sdWriteFile(void *ptr, int32_t Size, int32_t nItems, FileStream *Stream)
{
    if (!Stream || Stream->fd < 0 || !ptr || Size <= 0 || nItems <= 0) {
        return 0;
    }

    size_t total_bytes = (size_t)Size * (size_t)nItems;
    size_t bytes_written = 0;
    const uint8_t *buf = (const uint8_t *)ptr;

    while (bytes_written < total_bytes) {
        int ret = ns_write(Stream->fd, buf + bytes_written, total_bytes - bytes_written);

        if (ret < 0) {
            Stream->error = 1;
            break;
        }

        if (ret == 0) {
            break;  /* Can't write more */
        }

        bytes_written += ret;
    }

    /* Return number of complete items written */
    return (int32_t)(bytes_written / Size);
}

int32_t sdInputChar(FileStream *Stream)
{
    if (!Stream || Stream->fd < 0) {
        return -1;
    }

    uint8_t c;
    int ret = ns_read(Stream->fd, &c, 1);

    if (ret < 0) {
        Stream->error = 1;
        return -1;
    }

    if (ret == 0) {
        Stream->eof = 1;
        return -1;
    }

    return (int32_t)c;
}

int32_t sdSeekFile(FileStream *Stream, file_offset_t Position, int32_t Mode)
{
    if (!Stream || Stream->fd < 0) {
        return -1;
    }

    int whence;
    switch (Mode) {
    case PF_SEEK_SET:
        whence = FS_SEEK_SET;
        break;
    case PF_SEEK_CUR:
        whence = FS_SEEK_CUR;
        break;
    case PF_SEEK_END:
        whence = FS_SEEK_END;
        break;
    default:
        return -1;
    }

    int64_t ret = ns_seek(Stream->fd, Position, whence);
    if (ret < 0) {
        Stream->error = 1;
        return -1;
    }

    /* Clear EOF on successful seek */
    Stream->eof = 0;

    return 0;
}

file_offset_t sdTellFile(FileStream *Stream)
{
    if (!Stream || Stream->fd < 0) {
        return -1;
    }

    return (file_offset_t)ns_tell(Stream->fd);
}

int32_t sdFlushFile(FileStream *Stream)
{
    /* Namespace layer doesn't buffer, so flush is a no-op */
    (void)Stream;
    return 0;
}

int32_t sdDeleteFile(const char *FileName)
{
    if (!FileName) {
        return -1;
    }

    char resolved_path[MAX_PATH_LEN];
    const char *path = resolve_path(FileName, resolved_path, sizeof(resolved_path));

    int ret = ns_remove(path);
    return ret < 0 ? -1 : 0;
}

int32_t sdRenameFile(const char *OldName, const char *NewName)
{
    /* Rename not directly supported by namespace layer.
     * Would need to implement copy + delete. */
    (void)OldName;
    (void)NewName;
    LOG_WRN("sdRenameFile not implemented");
    return -1;
}

int32_t sdResizeFile(FileStream *Stream, uint64_t NewSize)
{
    /* Resize/truncate not supported by namespace layer */
    (void)Stream;
    (void)NewSize;
    LOG_WRN("sdResizeFile not implemented");
    return -1;  /* Would be THROW_RESIZE_FILE */
}

/* FS_SEEK_* constants are provided by <zephyr/fs/fs.h> */
