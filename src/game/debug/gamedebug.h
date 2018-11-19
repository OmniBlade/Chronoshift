/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Debug logging and assertion interface.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef GAMEDEBUG_H
#define GAMEDEBUG_H

#include "always.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CHRONOSHIFT_LOGGING

#define DEBUG_INIT(flags) Debug_Init(flags)
#define DEBUG_STOP() Debug_Shutdown()

#define DEBUG_LOG(msg, ...) Debug_Log(msg, ##__VA_ARGS__)
#define DEBUG_LINE_LOG(msg, ...) Debug_Log("%s %d " msg, __FILE__, __LINE__, ##__VA_ARGS__)

enum {
    DEBUG_BUFFER_SIZE = 4096
};

enum DebugOptions
{
    DEBUG_LOG_TO_FILE = 1 << 0,
    DEBUG_LOG_TO_DEBUGGER = 1 << 1,
    DEBUG_LOG_TO_CONSOLE = 1 << 2,
    DEBUG_PREFIX_TICKS = 1 << 3,
};

void Debug_Init(int flags);
void Debug_Shutdown();
int Debug_Get_Flags();
void Debug_Set_Flags(int flags);
void Debug_Log(const char *format, ...);

#else // !CHRONOSHIFT_LOGGING

#define DEBUG_INIT(flags) ((void)0)
#define DEBUG_STOP() ((void)0)

#define DEBUG_LOG(msg, ...) ((void)0)
#define DEBUG_LOG_CONDITIONAL(exp, msg, ...) ((void)0)
#define DEBUG_LINE_LOG(msg, ...) ((void)0)

#endif // CHRONOSHIFT_LOGGING

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GAMEDEBUG_H
