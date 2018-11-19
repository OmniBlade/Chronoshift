/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Debug assertion interface.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef GAMEASSERT_H
#define GAMEASSERT_H

#include "always.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CHRONOSHIFT_ASSERTS
#include "debugbreak.h"
#include "gamedebug.h"

extern bool ExitOnAssert; // Exit application on assertion when break button is pressed?
extern bool IgnoreAllAsserts; // Ignore all assertionss.
extern int GlobalIgnoreCount; // The number of assertions to ignore on a global basis.
extern int TotalAssertions; // The total number of assertions.
extern bool BreakOnException; // Break to debugger when a throw assertion is triggered.

enum
{
    ASSERT_BUFFER_SIZE = 4096
};

#define DEBUG_ASSERT(exp) \
    if (!(exp)) { \
        static volatile bool _ignore_assert = false; \
        static volatile bool _break = false; \
        if (!_ignore_assert) { \
            DEBUG_LOG( \
                "ASSERTION FAILED!\n" \
                "  File:%s\n  Line:%d\n  Function:%s\n  Expression:%s\n\n", \
                __FILE__, \
                __LINE__, \
                __CURRENT_FUNCTION__, \
                #exp); \
            Debug_Assert(#exp, __FILE__, __LINE__, __CURRENT_FUNCTION__, nullptr, _ignore_assert, _break); \
        } \
        if (_break) { \
            __debugbreak(); \
        } \
    }

#define DEBUG_ASSERT_PRINT(exp, msg, ...) \
    if (!(exp)) { \
        DEBUG_LOG( \
            "ASSERTION FAILED!\n" \
            "  File:%s\n  Line:%d\n  Function:%s\n  Expression:%s\n  Message:" msg "\n\n", \
            __FILE__, \
            __LINE__, \
            __CURRENT_FUNCTION__, \
            #exp, \
            ##__VA_ARGS__); \
    } 

#define DEBUG_ASSERT_THROW(exp, msg, ...) \
    if (!(exp)) { \
        DEBUG_LOG( \
            "ASSERTION FAILED!\n" \
            "  File:%s\n  Line:%d\n  Function:%s\n  Expression:%s\n  Message:" msg "\n\n", \
            __FILE__, \
            __LINE__, \
            __CURRENT_FUNCTION__, \
            #exp, \
            ##__VA_ARGS__); \
        if (BreakOnException) { \
            __debugbreak(); \
        } \
        throw except; \
    }

void Debug_Assert(char const *expr, char const *file, int const line, char const *func, char const *msg, volatile bool &_ignore, volatile bool &_break);

#else // !CHRONOSHIFT_ASSERTS

#define DEBUG_ASSERT(exp) if (!(exp)) {}
#define DEBUG_ASSERT_PRINT(exp, msg, ...) if (!(exp)) {}
#define DEBUG_ASSERT_THROW(exp, msg, ...) if (!(exp)) {}

#endif // CHRONOSHIFT_ASSERTS

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GAMEASSERT_H
