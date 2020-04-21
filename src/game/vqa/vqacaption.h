/**
 * @file
 *
 * @author tomsons26
 *
 * @brief  VQA subtitle functions.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef VQACAPTION_H
#define VQACAPTION_H

#include "always.h"

struct CaptionText
{
    int16_t field_0;
    int16_t field_2;
    int16_t field_4; // bitfield options - 0x80 = black background? could be TextPrintType?
    int16_t field_6;
    int field_8;
    int16_t field_C;
    int field_E; // sometype of char or text buffer?
    int16_t field_12; // text width?
    int16_t field_14; // text height?
};

struct CaptionInfo
{
    CaptionText *field_0;
    int field_4;
    int field_8;
    int field_C;
    void *m_Font;
    int16_t m_XPos;
    int16_t m_YPos;
    int16_t m_Width;
    int16_t m_Height;
    int field_1C;
    CaptionText field_20[3];
};

struct CaptionNode
{
    CaptionNode *m_Prev;
    CaptionNode *m_Next;
    int16_t field_8;
};

struct CaptionList
{
    int field_0;
    int field_4;
    CaptionNode *m_Next;
};

CaptionInfo *VQA_OpenCaptions(void *a1, void *font);
void VQA_CloseCaptions(CaptionInfo *info);
void VQA_DoCaptions(CaptionInfo *info, unsigned a2);
CaptionNode *VQA_AddCaptionNode(CaptionList *list, CaptionText *text);
void VQA_RemCaptionNode(CaptionList *list, CaptionNode *node);

#endif // VQACAPTION_H
