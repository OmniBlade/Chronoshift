/**
 * @file
 *
 * @author tomsons26
 *
 * @brief  VQA drawing related functions.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef VQADRAWER_H
#define VQADRAWER_H

#include "always.h"

struct DisplayInfo;
struct VQAData;
struct VQAFrameNode;
struct VQAHandle;

struct DisplayInfo
{
    unsigned m_Mode;
    unsigned m_XRes;
    unsigned m_YRes;
    unsigned m_VBIbit;
    unsigned m_Extended;
};

struct VQADrawer
{
    VQAFrameNode *m_CurFrame;
    unsigned m_Flags;
    DisplayInfo *m_Display;
    uint8_t *m_ImageBuf;
    int m_ImageWidth;
    int m_ImageHeight;
    int m_X1;
    int m_Y1;
    int m_X2;
    int m_Y2;
    int m_ScreenOffset;
    int m_CurPalSize;
    uint8_t m_Palette_24[768];
    uint8_t m_Palette_15[512];
    int m_BlocksPerRow;
    int m_NumRows;
    int m_NumBlocks;
    int m_MaskStart;
    int m_MaskWidth;
    int m_MaskHeight;
    int m_LastTime;
    int m_LastFrame;
    int m_LastFrameNum;
    int m_DesiredFrame;
    int m_NumSkipped;
    int m_WaitsOnFlipper;
    int m_WaitsOnLoader;
};

uint8_t *VQA_GetPalette(VQAHandle *handle);
int VQA_GetPaletteSize(VQAHandle *handle);
void VQA_SetDrawBuffer(VQAHandle *handle, uint8_t *buffer, unsigned width, unsigned height, int xpos, int ypos);
void VQA_ConfigureDrawer(VQAHandle *handle);
int VQA_SelectFrame(VQAHandle *handle);
void VQA_PrepareFrame(VQAData *data);

#endif