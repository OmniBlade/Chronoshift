/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief  Functions for handling the VQA palette.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "vqapalette.h"
#include "interpolate.h"
#include "pal.h"
#include <cstring>

using std::memcpy;

#ifndef GAME_DLL
uint8_t *VQPalette;
int VQNumBytes;
BOOL VQSlowpal;
BOOL VQPaletteChange;
#endif

/**
 * Flags a VQA Palette change.
 */
void VQA_Flag_To_Set_Palette(uint8_t *palette, int numbytes, BOOL slowpal)
{
    VQPalette = palette;
    VQNumBytes = numbytes;
    VQSlowpal = slowpal;
    VQPaletteChange = true;
}

/**
 * Changes the VQA Palette.
 */
void __cdecl VQA_SetPalette(uint8_t *palette, int numbytes, BOOL slowpal)
{
    for (int i = 0; i < 768; ++i) {
        palette[i] &= 0x3Fu;
    }

    Increase_Palette_Luminance(palette, 15, 15, 15, 63);

    if (g_PalettesRead) {
        memcpy(g_PaletteInterpolationTable, g_InterpolatedPalettes[g_PaletteCounter++], sizeof(g_PaletteInterpolationTable));
    }

    Set_Palette(palette);
}

/**
 * Changes the VQA Palette after a call to VQA_Flag_To_Set_Palette.
 */
void VQA_Check_VQ_Palette_Set()
{
    if (VQPaletteChange) {
        VQA_SetPalette(VQPalette, VQNumBytes, VQSlowpal);
        VQPaletteChange = false;
    }
}
