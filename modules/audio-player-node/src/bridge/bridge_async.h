/*
 * Copyright (c) 2015 Spotify AB
 *
 */

/**
 * \file   bridge_async.h    Add async execute function.
 *
 * \note   All input strings are expected to be in JSON format
 * \note   All output strings are in JSON format
 *
 */

#ifndef BRIDGE_ASYNC_H
#define BRIDGE_ASYNC_H

#include "bridge.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief Execute a command.
 *
 * \param[in]   input     A JSON structure describe the command that should be executed.
 *
 * \return                An SP_BRIDGE_ERROR code
 *
 */
SP_LIBEXPORT(sp_bridge_error) sp_bridge_execute_async(const char *input);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
