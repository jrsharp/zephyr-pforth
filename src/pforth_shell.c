/*
 * pForth Shell Command Integration
 *
 * Provides a 'forth' command in the Zephyr shell to start
 * the Forth interpreter.
 *
 * Copyright (c) 2025 Jon Sharp
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pforth_shell, CONFIG_PFORTH_LOG_LEVEL);

#include "pforth/pforth_zephyr.h"
#include "pforth/pf_io_zephyr.h"

/* External declaration for setting shell instance */
extern void pforth_set_shell(const struct shell *sh);

/* Flag to track if we're in Forth mode */
static volatile bool forth_running;

/*
 * Shell bypass handler - receives raw input when in Forth mode
 */
static void forth_shell_bypass(const struct shell *sh, uint8_t *data, size_t len)
{
	ARG_UNUSED(sh);

	for (size_t i = 0; i < len; i++) {
		/* Check for Ctrl-C to exit */
		if (data[i] == 0x03) {
			forth_running = false;
			return;
		}
		pforth_input_char(data[i]);
	}
}

/*
 * forth command - start the Forth interpreter
 */
static int cmd_forth(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	int ret;

	/* Initialize pForth if not already done */
	if (!pforth_is_initialized()) {
		shell_print(sh, "Initializing pForth...");
		ret = pforth_init();
		if (ret != 0) {
			shell_error(sh, "Failed to initialize pForth: %d", ret);
			return ret;
		}
	}

	shell_print(sh, "pForth - Type BYE or Ctrl-C to exit");
	shell_print(sh, "");

	/* Clear any custom I/O callbacks and set up shell I/O */
	pforth_set_io_callbacks(NULL);
	pforth_set_shell(sh);

	/* Enable bypass mode to receive raw input */
	forth_running = true;
	shell_set_bypass(sh, forth_shell_bypass);

	/* Run the Forth interpreter */
	ret = pforth_run();

	/* Disable bypass mode */
	shell_set_bypass(sh, NULL);
	forth_running = false;

	shell_print(sh, "");
	shell_print(sh, "Exited pForth");

	return ret;
}

/*
 * forth eval <string> - evaluate a Forth string
 */
static int cmd_forth_eval(const struct shell *sh, size_t argc, char **argv)
{
	if (argc < 2) {
		shell_error(sh, "Usage: forth eval <forth-code>");
		return -EINVAL;
	}

	/* Initialize pForth if not already done */
	if (!pforth_is_initialized()) {
		int ret = pforth_init();
		if (ret != 0) {
			shell_error(sh, "Failed to initialize pForth: %d", ret);
			return ret;
		}
	}

	/* Set up shell I/O for output */
	pforth_set_shell(sh);

	/* Execute each argument as Forth code */
	for (size_t i = 1; i < argc; i++) {
		int ret = pforth_exec(argv[i]);
		if (ret != 0) {
			shell_error(sh, "Forth error: %d", ret);
			return ret;
		}
	}

	return 0;
}

/*
 * forth reset - reset the Forth system
 */
static int cmd_forth_reset(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	if (pforth_is_initialized()) {
		pforth_shutdown();
		shell_print(sh, "pForth reset");
	} else {
		shell_print(sh, "pForth not initialized");
	}

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_forth,
	SHELL_CMD(eval, NULL, "Evaluate Forth code: forth eval <code>", cmd_forth_eval),
	SHELL_CMD(reset, NULL, "Reset the Forth system", cmd_forth_reset),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(forth, &sub_forth,
	"pForth interpreter. Run 'forth' for REPL or 'forth eval <code>'",
	cmd_forth);
