/*
 * pForth Zephyr Public API
 *
 * This header provides the public API for using pForth in Zephyr applications.
 *
 * Copyright (c) 2025 Jon Sharp
 * SPDX-License-Identifier: MIT
 */

#ifndef PFORTH_ZEPHYR_H
#define PFORTH_ZEPHYR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize pForth
 *
 * Initializes the pForth interpreter, creates the dictionary and task.
 * Must be called before any other pForth functions.
 *
 * @return 0 on success, negative error code on failure
 */
int pforth_init(void);

/**
 * @brief Run the pForth interpreter loop
 *
 * Starts the interactive Forth interpreter. This function runs
 * until the user executes BYE or an error occurs.
 *
 * @return 0 on normal exit, negative error code on failure
 */
int pforth_run(void);

/**
 * @brief Execute a Forth string
 *
 * Compiles and executes the given Forth source string.
 *
 * @param source Null-terminated Forth source string
 * @return 0 on success, non-zero Forth throw code on error
 */
int pforth_exec(const char *source);

/**
 * @brief Execute a Forth word by name
 *
 * Looks up and executes a Forth word.
 *
 * @param word Null-terminated word name
 * @return 0 on success, non-zero if word not found or throws
 */
int pforth_exec_word(const char *word);

/**
 * @brief Shutdown pForth
 *
 * Releases all pForth resources. After calling this, pforth_init()
 * must be called again before using pForth.
 */
void pforth_shutdown(void);

/**
 * @brief Check if pForth is initialized
 *
 * @return true if pForth is ready, false otherwise
 */
bool pforth_is_initialized(void);

/**
 * @brief Push a value onto the Forth data stack
 *
 * @param value Value to push (cell-sized integer)
 * @return 0 on success, -1 if stack overflow
 */
int pforth_push(intptr_t value);

/**
 * @brief Pop a value from the Forth data stack
 *
 * @param value Pointer to receive the popped value
 * @return 0 on success, -1 if stack underflow
 */
int pforth_pop(intptr_t *value);

/**
 * @brief Get the current stack depth
 *
 * @return Number of items on the data stack
 */
int pforth_depth(void);

#ifdef __cplusplus
}
#endif

#endif /* PFORTH_ZEPHYR_H */
