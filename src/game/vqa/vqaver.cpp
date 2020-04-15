/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief  VQA player version info.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "vqaver.h"
#include "gitverinfo.h"
#include <cstdio>

using std::snprintf;

#define VQA_LIBNAME "VQALib (" PLATFORM_NAME ")"

// Version numbers
#define VQA_VERSION_MAJOR "2"
#define VQA_VERSION_MINOR "4"
#define VQA_VERSION_REVISION "2"

// Full version strings
#define VQA_VERSION VQA_VERSION_MAJOR "." VQA_VERSION_MINOR "." VQA_VERSION_REVISION

const char *VQA_NameString()
{
    return VQA_LIBNAME;
}

const char *VQA_VersionString()
{
    return VQA_VERSION;
}

const char *VQA_DateTimeString()
{
    static char _date[50];

    if (*_date == '\0') {
        snprintf(_date, sizeof(_date), "%s %s", g_gitCommitDate, g_gitCommitTime);
    }

    return _date;
}

const char *VQA_IDString()
{
    static char _id[100];

    if (*_id == '\0') {
        snprintf(_id, sizeof(_id), "%s %s (%s %s)", VQA_LIBNAME, VQA_VERSION, g_gitCommitDate, g_gitCommitTime);
    }

    return _id;
}
