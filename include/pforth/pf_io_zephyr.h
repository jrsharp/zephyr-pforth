/*
 * pForth Zephyr I/O Header
 *
 * Declares the I/O functions for pForth on Zephyr.
 * This file is included by pForth when PF_USER_CHARIO is defined.
 *
 * Copyright (c) 2025 Jon Sharp
 * SPDX-License-Identifier: MIT
 */

#ifndef PF_IO_ZEPHYR_H
#define PF_IO_ZEPHYR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Core terminal I/O functions required by pForth.
 * These are implemented in pf_io_zephyr.c
 */
int  sdTerminalOut(char c);
int  sdTerminalEcho(char c);
int  sdTerminalFlush(void);
int  sdTerminalIn(void);
int  sdQueryTerminal(void);
void sdTerminalInit(void);
void sdTerminalTerm(void);

/* Sleep function */
intptr_t sdSleepMillis(intptr_t msec);

/* Input enable/disable (for XON/XOFF flow control - not used on Zephyr) */
#define sdEnableInput()
#define sdDisableInput()

/*
 * Custom I/O callback interface.
 * Applications can provide their own I/O functions.
 */
typedef struct {
    int  (*out)(char c);           /* Output a character */
    int  (*in)(void);              /* Input a character (blocking) */
    int  (*query)(void);           /* Check if input available */
    void (*flush)(void);           /* Flush output */
} pforth_io_callbacks_t;

/*
 * Set custom I/O callbacks.
 * If callbacks is NULL, reverts to default I/O.
 */
void pforth_set_io_callbacks(const pforth_io_callbacks_t *callbacks);

/*
 * Get the current I/O callbacks.
 */
const pforth_io_callbacks_t *pforth_get_io_callbacks(void);

/*
 * Shell backend functions (only available when CONFIG_PFORTH_IO_SHELL=y)
 */
#if defined(CONFIG_SHELL) || defined(__DOXYGEN__)
struct shell;  /* Forward declaration */

/**
 * Set the shell instance for I/O.
 * Called before starting the interpreter from shell context.
 */
void pforth_set_shell(const struct shell *sh);

/**
 * Feed an input character from shell bypass handler.
 * Called by the shell when in bypass mode.
 */
void pforth_input_char(char c);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PF_IO_ZEPHYR_H */
