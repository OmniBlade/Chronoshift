/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief  VQA event functions.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef VQATASK_H
#define VQATASK_H

#include "vqafile.h"

struct VQAInfo
{
    int m_NumFrames;
    int m_ImageWidth;
    int m_ImageHeight;
    uint8_t *m_ImageBuf; // New in newer code (RA onwards)
};

struct VQAStatistics
{
    int m_StartTime;
    int m_EndTime;
    int m_FramesLoaded;
    int m_FramesDrawn;
    int m_FramesSkipped;
    int m_MaxFrameSize;
    unsigned m_SamplesPlayed;
    unsigned m_MemUsed;
};

enum VQAPlayMode
{
    UNKNOWN,
};

#ifdef GAME_DLL
extern BOOL &VQAMovieDone;
#else
extern BOOL VQAMovieDone;
#endif

VQAHandle *VQA_Alloc();
void VQA_Free(void *block);
void VQA_Init(VQAHandle *vqa, StreamHandlerFuncPtr streamhandler);
void VQA_Reset(VQAHandle *vqa);
VQAErrorType VQA_Play(VQAHandle *vqa, VQAPlayMode mode);
void VQA_SetStreamHandler(VQAHandle *vqa, StreamHandlerFuncPtr streamhandler);
int16_t VQA_SetStop(VQAHandle *vqa, int16_t frame);
int VQA_SetDrawFlags(VQAHandle *vqa, int flags);
int VQA_ClearDrawFlags(VQAHandle *vqa, int flags);
void VQA_GetInfo(VQAHandle *vqa, VQAInfo *info);
void VQA_GetStats(VQAHandle *vqa, VQAStatistics *stats);
VQAErrorType VQA_GetBlockWH_ColorMode(VQAHandle *vqa, unsigned *blockwidth, unsigned *blockheight, int *colormode);
VQAErrorType VQA_GetXYPos(VQAHandle *vqa, uint16_t *xpos, uint16_t *ypos);
VQAErrorType VQA_UserUpdate(VQAHandle *vqa);

#endif
