/*
 * pForth Static Dictionary Stub
 *
 * This is a placeholder. For a pre-compiled dictionary:
 * 1. Build pforth on a host system
 * 2. Run: pforth fth/mkdicdat.fth
 * 3. Replace this file with the generated pfdicdat.h content
 *
 * Copyright (c) 2025 Jon Sharp
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(pforth, CONFIG_PFORTH_LOG_LEVEL);

/*
 * When PF_STATIC_DIC is not defined, pfLoadStaticDictionary()
 * returns NULL and pforth falls back to pfBuildDictionary().
 *
 * To enable static dictionary:
 * 1. Generate pfdicdat.h on a host
 * 2. Add: zephyr_compile_definitions(PF_STATIC_DIC)
 * 3. Replace this file with the generated content
 */
