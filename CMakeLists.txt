# pForth - Portable Forth for Zephyr
#
# Copyright (c) 2025 Jon Sharp
# SPDX-License-Identifier: MIT

if(CONFIG_PFORTH)

# pForth C sources (from upstream pforth)
set(PFORTH_CSRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/csrc)

zephyr_library()

zephyr_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PFORTH_CSRC_DIR}
)

# Define pForth configuration based on Kconfig
zephyr_compile_definitions(
    PF_USER_INC1="pforth/pf_zephyr_config.h"
)

if(CONFIG_PFORTH_FLOATING_POINT)
    zephyr_compile_definitions(PF_SUPPORT_FP=1)
else()
    zephyr_compile_definitions(PF_SUPPORT_FP=0)
endif()

if(NOT CONFIG_PFORTH_FILEIO)
    zephyr_compile_definitions(PF_NO_FILEIO)
endif()

if(CONFIG_PFORTH_APP_CUSTOM)
    # Empties csrc/pfcustom.c; the application supplies
    # CustomFunctionTable[] and CompileCustomFunctions().
    zephyr_compile_definitions(PF_USER_CUSTOM=1)
endif()

# Core pForth sources
zephyr_library_sources(
    ${PFORTH_CSRC_DIR}/pf_core.c
    ${PFORTH_CSRC_DIR}/pf_inner.c
    ${PFORTH_CSRC_DIR}/pf_words.c
    ${PFORTH_CSRC_DIR}/pfcompil.c
    ${PFORTH_CSRC_DIR}/pf_text.c
    ${PFORTH_CSRC_DIR}/pf_mem.c
    ${PFORTH_CSRC_DIR}/pf_cglue.c
    ${PFORTH_CSRC_DIR}/pf_io.c
    ${PFORTH_CSRC_DIR}/pf_save.c
    ${PFORTH_CSRC_DIR}/pf_clib.c
    ${PFORTH_CSRC_DIR}/pfcustom.c
)

# Zephyr integration sources
zephyr_library_sources(
    src/pf_io_zephyr.c
    src/pforth_zephyr.c
)

if(CONFIG_PFORTH_FILEIO AND CONFIG_PFORTH_FILEIO_NAMESPACE)
    # The namespace layer (ns.h) is provided by the application
    # Add application's source directory to include path
    zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/src)
    zephyr_library_sources(
        src/pf_fileio_zephyr.c
    )
endif()

if(CONFIG_PFORTH_SHELL)
    zephyr_library_sources(
        src/pforth_shell.c
    )
endif()

if(CONFIG_PFORTH_STATIC_DICTIONARY)
    # Include the pre-compiled dictionary
    # Note: You must generate pfdicdat.h first by running:
    #   pforth fth/mkdicdat.fth
    # on a host system, then copy it to csrc/
    zephyr_compile_definitions(PF_STATIC_DIC)
endif()

endif() # CONFIG_PFORTH
