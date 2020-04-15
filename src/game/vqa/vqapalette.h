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
#pragma once

#ifndef VQAPALETTE_H
#define VQAPALETTE_H

#include "always.h"

#ifdef GAME_DLL
extern uint8_t *&VQPalette;
extern int &VQNumBytes;
extern BOOL &VQSlowpal;
extern BOOL &VQPaletteChange;
#else
extern uint8_t *VQPalette;
extern int VQNumBytes;
extern BOOL VQSlowpal;
extern BOOL VQPaletteChange;
#endif

void VQA_Flag_To_Set_Palette(uint8_t *palette, int numbytes, BOOL slowpal);
void __cdecl VQA_SetPalette(uint8_t *palette, int numbytes, BOOL slowpal);
void VQA_Check_VQ_Palette_Set();

#endif // VQAPALETTE_H
