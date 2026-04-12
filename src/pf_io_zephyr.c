/*
 * pForth Zephyr I/O Backend
 *
 * Provides character I/O for pForth on Zephyr RTOS.
 * Supports multiple backends: shell, console device, or custom callbacks.
 *
 * Copyright (c) 2025 Jon Sharp
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pforth_io, CONFIG_PFORTH_LOG_LEVEL);

#include "pforth/pf_io_zephyr.h"

#if defined(CONFIG_PFORTH_IO_SHELL)
#include <zephyr/shell/shell.h>
static const struct shell *current_shell;
#endif

#if defined(CONFIG_PFORTH_IO_CONSOLE)
#include <zephyr/console/console.h>
#endif

/* Custom I/O callbacks */
static const pforth_io_callbacks_t *custom_io;

/* Input buffer for shell backend */
#if defined(CONFIG_PFORTH_IO_SHELL)
#include <zephyr/sys/ring_buffer.h>
#define PFORTH_INPUT_BUF_SIZE 64
static uint8_t input_buf_data[PFORTH_INPUT_BUF_SIZE];
static struct ring_buf input_buf;
static K_SEM_DEFINE(input_sem, 0, 1);
#endif

void pforth_set_io_callbacks(const pforth_io_callbacks_t *callbacks)
{
	custom_io = callbacks;
}

const pforth_io_callbacks_t *pforth_get_io_callbacks(void)
{
	return custom_io;
}

#if defined(CONFIG_PFORTH_IO_SHELL)
/*
 * Set the shell instance for I/O.
 * Called by the forth shell command before starting the interpreter.
 */
void pforth_set_shell(const struct shell *sh)
{
	current_shell = sh;
}

/*
 * Feed input character from shell.
 * Called by the shell backend when input is available.
 */
void pforth_input_char(char c)
{
	if (ring_buf_put(&input_buf, (uint8_t *)&c, 1) == 1) {
		k_sem_give(&input_sem);
	}
}
#endif

int sdTerminalOut(char c)
{
	if (custom_io && custom_io->out) {
		return custom_io->out(c);
	}

#if defined(CONFIG_PFORTH_IO_SHELL)
	if (current_shell) {
		shell_fprintf(current_shell, SHELL_NORMAL, "%c", c);
		return 0;
	}
#endif

#if defined(CONFIG_PFORTH_IO_CONSOLE)
	console_putchar(c);
	return 0;
#endif

	return -1;
}

int sdTerminalEcho(char c)
{
	/* Echo is same as output for most backends */
	return sdTerminalOut(c);
}

int sdTerminalFlush(void)
{
	if (custom_io && custom_io->flush) {
		custom_io->flush();
		return 0;
	}

#if defined(CONFIG_PFORTH_IO_SHELL)
	/* Shell handles flushing automatically */
	return 0;
#endif

#if defined(CONFIG_PFORTH_IO_CONSOLE)
	/* Console handles flushing automatically */
	return 0;
#endif

	return 0;
}

int sdTerminalIn(void)
{
	if (custom_io && custom_io->in) {
		return custom_io->in();
	}

#if defined(CONFIG_PFORTH_IO_SHELL)
	uint8_t c;

	/* Wait for input */
	while (ring_buf_get(&input_buf, &c, 1) == 0) {
		k_sem_take(&input_sem, K_FOREVER);
	}
	return (int)c;
#endif

#if defined(CONFIG_PFORTH_IO_CONSOLE)
	return console_getchar();
#endif

	return -1;
}

int sdQueryTerminal(void)
{
	if (custom_io && custom_io->query) {
		return custom_io->query();
	}

#if defined(CONFIG_PFORTH_IO_SHELL)
	return !ring_buf_is_empty(&input_buf);
#endif

#if defined(CONFIG_PFORTH_IO_CONSOLE)
	/* Console doesn't have a non-blocking query */
	return 0;
#endif

	return 0;
}

void sdTerminalInit(void)
{
#if defined(CONFIG_PFORTH_IO_SHELL)
	ring_buf_init(&input_buf, sizeof(input_buf_data), input_buf_data);
#endif

#if defined(CONFIG_PFORTH_IO_CONSOLE)
	console_init();
#endif

	LOG_DBG("pForth terminal initialized");
}

void sdTerminalTerm(void)
{
#if defined(CONFIG_PFORTH_IO_SHELL)
	current_shell = NULL;
	ring_buf_reset(&input_buf);
#endif

	LOG_DBG("pForth terminal terminated");
}

intptr_t sdSleepMillis(intptr_t msec)
{
	k_sleep(K_MSEC(msec));
	return 0;
}
