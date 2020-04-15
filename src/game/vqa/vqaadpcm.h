/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief  Decoders for the IMA ADPCM format temp VQA copy.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef VQAVQAADPCM_H
#define VQAVQAADPCM_H

#include "always.h"

// This struct is slightly different in the VQA version of these functions, they are otherwise the same.
struct VQAADPCMStreamType
{
    void *m_Source; // pointer to compressed data
    void *m_Dest; // pointer to uncompressed data
    int32_t m_CompSize; // compressed size
    int32_t m_UnCompSize; // uncompressed size
    int16_t m_BitsPerSample; // bits per sample, 8 or 16 bit are valid
    int16_t m_Channels; // number of channels, 1 or 2 are valid.
    int32_t m_LeftSampleIndex; // index to next sample
    int32_t m_LeftPredicted; // next predicted value
    int32_t m_LeftDifference; // difference from last sample
    int16_t m_LeftCodeBuf; // holds 2 nybbles for decomp
    int16_t m_LeftCode; // current 4bit code
    int16_t m_LeftStep; // step value in table
    int16_t m_LeftIndex; // bit size for decompression
    int32_t m_RightSampleIndex; // These duplicate entries are for a second channel
    int32_t m_RightPredicted;
    int32_t m_RightDifference;
    int16_t m_RightCodeBuf;
    int16_t m_RightCode;
    int16_t m_RightStep;
    int16_t m_RightIndex;
};

void __cdecl VQAADPCM_Stream_Init(VQAADPCMStreamType *stream);
unsigned __cdecl VQAADPCM_Decompress(VQAADPCMStreamType *stream, unsigned bytes);

#endif // VQAVQAADPCM_H
