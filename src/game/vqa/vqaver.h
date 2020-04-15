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
#pragma once

#ifndef VQAVER_H
#define VQAVER_H

#include "always.h"

enum VQAVersion
{
    VQA_VER_1 = 0, // 1 - First VQAs, used only in Legend of Kyrandia III.
    VQA_VER_2 = 1, // 2 - Used in C&C, Red Alert, Lands of Lore II, Dune 2000.
    VQA_VER_HICOLOR = 2 // 3 - Lands of Lore III, Blade Runner, Nox and Tiberian Sun.
};

const char *VQA_NameString();
const char *VQA_VersionString();
const char *VQA_DateTimeString();
const char *VQA_IDString();

#endif // VQA_VER_H
