/*
 * pForth Zephyr Implementation
 *
 * Wraps the pForth API for use in Zephyr applications.
 *
 * Copyright (c) 2025 Jon Sharp
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pforth, CONFIG_PFORTH_LOG_LEVEL);

#include "pforth/pforth_zephyr.h"

/* Include pForth headers */
#include "pf_all.h"

/* Static state */
static bool initialized;
static PForthDictionary dictionary;
static PForthTask task;

int pforth_init(void)
{
	if (initialized) {
		LOG_WRN("pForth already initialized");
		return 0;
	}

	LOG_INF("Initializing pForth...");

	/* Initialize terminal I/O */
	sdTerminalInit();

	/* Initialize pForth globals (sets BASE=10, etc.) */
	pfInit();

	/* Create task with configured stack sizes */
	task = pfCreateTask(CONFIG_PFORTH_USER_STACK_DEPTH,
			    CONFIG_PFORTH_RETURN_STACK_DEPTH);
	if (!task) {
		LOG_ERR("Failed to create pForth task");
		return -ENOMEM;
	}

	pfSetCurrentTask(task);

#ifdef CONFIG_PFORTH_STATIC_DICTIONARY
	/* Load pre-compiled dictionary */
	LOG_DBG("Loading static dictionary");
	dictionary = pfLoadStaticDictionary();
#else
	/* Build dictionary from scratch */
	LOG_DBG("Building dictionary (header=%d, code=%d)",
		CONFIG_PFORTH_HEADER_SIZE, CONFIG_PFORTH_DICTIONARY_SIZE);
	dictionary = pfBuildDictionary(CONFIG_PFORTH_HEADER_SIZE,
				       CONFIG_PFORTH_DICTIONARY_SIZE);
#endif

	if (!dictionary) {
		LOG_ERR("Failed to create pForth dictionary");
		pfDeleteTask(task);
		task = NULL;
		return -ENOMEM;
	}

#ifdef CONFIG_PFORTH_QUIET
	pfSetQuiet(1);
#else
	pfSetQuiet(0);
#endif

	initialized = true;
	LOG_INF("pForth initialized");

	return 0;
}

int pforth_run(void)
{
	if (!initialized) {
		LOG_ERR("pForth not initialized");
		return -EINVAL;
	}

	LOG_DBG("Starting pForth interpreter");

	/* Run the Forth outer interpreter */
	ThrowCode result = pfQuit();

	LOG_DBG("pForth interpreter exited: %ld", (long)result);

	return (int)result;
}

int pforth_exec(const char *source)
{
	if (!initialized) {
		return -EINVAL;
	}

	if (!source || !*source) {
		return 0;
	}

	/* Execute the Forth source string */
	/* Note: pForth doesn't have a direct "execute string" API,
	 * so we use pfExecIfDefined for word execution.
	 * For full source execution, we'd need to feed it through
	 * the interpreter loop character by character.
	 */
	return (int)pfExecIfDefined(source);
}

int pforth_exec_word(const char *word)
{
	if (!initialized) {
		return -EINVAL;
	}

	if (!word || !*word) {
		return 0;
	}

	return (int)pfExecIfDefined(word);
}

void pforth_shutdown(void)
{
	if (!initialized) {
		return;
	}

	LOG_INF("Shutting down pForth");

	if (dictionary) {
		pfDeleteDictionary(dictionary);
		dictionary = NULL;
	}

	if (task) {
		pfDeleteTask(task);
		task = NULL;
	}

	sdTerminalTerm();

	initialized = false;
}

bool pforth_is_initialized(void)
{
	return initialized;
}

/* Stack access functions would need access to pForth internals.
 * For now, these are stubs. A full implementation would need
 * to expose the stack pointer from pForth.
 */

int pforth_push(intptr_t value)
{
	(void)value;
	LOG_WRN("pforth_push not yet implemented");
	return -ENOTSUP;
}

int pforth_pop(intptr_t *value)
{
	(void)value;
	LOG_WRN("pforth_pop not yet implemented");
	return -ENOTSUP;
}

int pforth_depth(void)
{
	LOG_WRN("pforth_depth not yet implemented");
	return 0;
}
