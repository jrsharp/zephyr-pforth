# pForth for Zephyr

A Zephyr RTOS module providing pForth, a portable Forth interpreter.

## Overview

pForth is a portable Forth implementation written in ANSI C by Phil Burk.
This module integrates pForth with Zephyr RTOS, providing:

- Interactive Forth REPL via Zephyr shell
- Configurable I/O backends (shell, console, custom)
- Optional file I/O via namespace layer
- Kconfig-based configuration

## Quick Start

### 1. Add to your west manifest

```yaml
manifest:
  projects:
    - name: zephyr-pforth
      url: https://github.com/yourorg/zephyr-pforth
      revision: main
      path: modules/lib/pforth
```

### 2. Enable in prj.conf

```
CONFIG_PFORTH=y
CONFIG_PFORTH_SHELL=y
```

### 3. Use from shell

```
uart:~$ forth
pForth - Type BYE or Ctrl-C to exit

: hello ." Hello, Zephyr!" cr ;
hello
Hello, Zephyr!
bye
```

## Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `CONFIG_PFORTH` | n | Enable pForth |
| `CONFIG_PFORTH_SHELL` | y | Add 'forth' shell command |
| `CONFIG_PFORTH_DICTIONARY_SIZE` | 65536 | Dictionary size in bytes |
| `CONFIG_PFORTH_HEADER_SIZE` | 16384 | Header space in bytes |
| `CONFIG_PFORTH_USER_STACK_DEPTH` | 512 | Data stack depth (cells) |
| `CONFIG_PFORTH_RETURN_STACK_DEPTH` | 512 | Return stack depth (cells) |
| `CONFIG_PFORTH_FLOATING_POINT` | n | Enable floating point |
| `CONFIG_PFORTH_FILEIO` | n | Enable file I/O words |
| `CONFIG_PFORTH_IO_SHELL` | y | Use shell for I/O |
| `CONFIG_PFORTH_IO_CONSOLE` | n | Use console device |
| `CONFIG_PFORTH_IO_CUSTOM` | n | Use custom callbacks |
| `CONFIG_PFORTH_STATIC_DICTIONARY` | n | Use pre-compiled dictionary |

## I/O Backends

### Shell Backend (default)

Uses the Zephyr shell for input/output. The `forth` command starts
an interactive REPL within the shell.

### Console Backend

Uses `console_getchar()` and `console_putchar()` directly.
Useful when shell is not available.

### Custom Backend

Application provides I/O callbacks:

```c
#include <pforth/pf_io_zephyr.h>

static int my_out(char c) { ... }
static int my_in(void) { ... }
static int my_query(void) { ... }
static void my_flush(void) { ... }

static const pforth_io_callbacks_t my_io = {
    .out = my_out,
    .in = my_in,
    .query = my_query,
    .flush = my_flush,
};

void main(void) {
    pforth_set_io_callbacks(&my_io);
    pforth_init();
    pforth_run();
}
```

## API

```c
#include <pforth/pforth_zephyr.h>

/* Initialize pForth */
int pforth_init(void);

/* Run interactive interpreter */
int pforth_run(void);

/* Execute Forth code */
int pforth_exec(const char *source);

/* Execute a word by name */
int pforth_exec_word(const char *word);

/* Shutdown */
void pforth_shutdown(void);

/* Check if initialized */
bool pforth_is_initialized(void);
```

## Static Dictionary

For faster startup, you can pre-compile the Forth dictionary:

1. Build pForth on a host system:
   ```bash
   cd pforth-upstream
   mkdir build && cd build
   cmake .. && make
   ```

2. Generate the dictionary header:
   ```bash
   ./pforth ../fth/mkdicdat.fth
   ```

3. Copy `pfdicdat.h` to `csrc/`

4. Enable in prj.conf:
   ```
   CONFIG_PFORTH_STATIC_DICTIONARY=y
   ```

## Memory Requirements

Typical memory usage with default configuration:

- Code: ~40-50 KB (FLASH)
- Dictionary: 64 KB (RAM/heap)
- Stacks: ~8 KB (RAM)

For constrained systems, reduce dictionary and stack sizes.

## License

pForth: Copyright 1994 3DO, Phil Burk, Larry Polansky, David Rosenboom
(Zero-clause BSD-style license - see LICENSE)

Zephyr integration: Copyright (c) 2025 Jon Sharp (MIT)

## Credits

- Phil Burk - pForth author
- Jon Sharp - Zephyr integration
