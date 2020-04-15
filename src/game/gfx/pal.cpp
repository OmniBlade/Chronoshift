/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Low level palette handling.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "pal.h"
#include "initvideo.h"
#include <algorithm>
#include <cstring>

using std::memcpy;

enum
{
    PALETTE_SIZE = 768,
};

#ifndef GAME_DLL
uint8_t g_CurrentPalette[PALETTE_SIZE];
#endif

void Set_Palette(void *pal)
{
    memcpy(g_CurrentPalette, pal, PALETTE_SIZE);
    Set_Video_Palette(pal);
}

void Increase_Palette_Luminance(uint8_t *pal, int r, int g, int b, int max)
{
    for (int i = 0; i < PALETTE_SIZE; i += 3) {
        pal[i] = std::min(max, pal[i] + r * pal[i] / 100);
        pal[i + 1] = std::min(max, pal[i + 1] + g * pal[i + 1] / 100);
        pal[i + 2] = std::min(max, pal[i + 2] + b * pal[i + 2] / 100);
    }
}
