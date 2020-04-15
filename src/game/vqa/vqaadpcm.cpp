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
#include "vqaadpcm.h"
#include "endiantype.h"
#include <algorithm>
#include <cstring>

using std::memset;

// clang-format off
static const int16_t g_ADPCMStepTab[89] = {
    7,     8,     9,     10,    11,    12,     13,    14,    16,
    17,    19,    21,    23,    25,    28,     31,    34,    37,
    41,    45,    50,    55,    60,    66,     73,    80,    88,
    97,    107,   118,   130,   143,   157,    173,   190,   209,
    230,   253,   279,   307,   337,   371,    408,   449,   494,
    544,   598,   658,   724,   796,   876,    963,   1060,  1166,
    1282,  1411,  1552,  1707,  1878,  2066,   2272,  2499,  2749,
    3024,  3327,  3660,  4026,  4428,  4871,   5358,  5894,  6484,
    7132,  7845,  8630,  9493,  10442, 11487,  12635, 13899, 15289,
    16818, 18500, 20350, 22385, 24623, 27086,  29794, 32767
};

static const int16_t g_ADPCMIndexTab[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8
};
// clang-format on

void __cdecl VQAADPCM_Stream_Init(VQAADPCMStreamType *stream)
{
    stream->m_LeftCode = 0;
    stream->m_LeftCodeBuf = 0;
    stream->m_LeftIndex = 0;
    stream->m_LeftStep = g_ADPCMStepTab[stream->m_LeftIndex];
    stream->m_LeftPredicted = 0;
    stream->m_LeftSampleIndex = 0;
    stream->m_RightCode = 0;
    stream->m_RightCodeBuf = 0;
    stream->m_RightIndex = 0;
    stream->m_RightStep = g_ADPCMStepTab[stream->m_RightIndex];
    stream->m_RightPredicted = 0;
    stream->m_RightSampleIndex = 0;
}

unsigned __cdecl VQAADPCM_Decompress(VQAADPCMStreamType *stream, unsigned bytes)
{
    int16_t current_nybble;
    unsigned step;
    int sample;
    unsigned full_length;

    full_length = bytes;
    stream->m_LeftSampleIndex = 0;
    stream->m_RightSampleIndex = 0;

    if (stream->m_BitsPerSample == 16) {
        bytes /= 2;
    }

    uint8_t *src = static_cast<uint8_t *>(stream->m_Source);
    int16_t *dst = static_cast<int16_t *>(stream->m_Dest);

    // Handle stereo
    if (stream->m_Channels == 2) {
        current_nybble = 0;
        for (int i = bytes; i > 0; i -= 2) {
            if ((stream->m_LeftSampleIndex & 1) != 0) {
                current_nybble = stream->m_LeftCodeBuf >> 4;
                stream->m_LeftCode = current_nybble;
            } else {
                stream->m_LeftCodeBuf = *src;
                // Stereo is interleaved so skip a byte for this channel
                src += 2;
                current_nybble = stream->m_LeftCodeBuf & 0xF;
                stream->m_LeftCode = current_nybble;
            }

            step = stream->m_LeftStep;
            stream->m_LeftDifference = step >> 3;

            if ((current_nybble & 4) != 0) {
                stream->m_LeftDifference += step;
            }

            if ((current_nybble & 2) != 0) {
                stream->m_LeftDifference += step >> 1;
            }

            if ((current_nybble & 1) != 0) {
                stream->m_LeftDifference += step >> 2;
            }

            if ((current_nybble & 8) != 0) {
                stream->m_LeftDifference = -stream->m_LeftDifference;
            }

            sample = std::clamp(stream->m_LeftDifference + stream->m_LeftPredicted, -32768, 32767);
            stream->m_LeftPredicted = sample;

            if (stream->m_BitsPerSample == 16) {
                *dst = sample;
                // Stereo is interleaved so skip a sample for this channel
                dst += 2;
            } else {
                *dst++ = ((sample & 0xFF00) >> 8) ^ 0x80;
            }

            stream->m_LeftIndex += g_ADPCMIndexTab[stream->m_LeftCode & 0x7];
            stream->m_LeftIndex = std::clamp<int16_t>(stream->m_LeftIndex, 0, 88);
            ++stream->m_LeftSampleIndex;
            stream->m_LeftStep = g_ADPCMStepTab[stream->m_LeftIndex];
        }

        src = static_cast<uint8_t *>(stream->m_Source) + 1;
        dst = reinterpret_cast<int16_t *>(static_cast<char *>(stream->m_Dest) + 1);

        if (stream->m_BitsPerSample == 16) {
            dst = static_cast<int16_t *>(stream->m_Dest) + 1;
        }

        for (int i = bytes; i > 0; i -= 2) {
            if ((stream->m_RightSampleIndex & 1) != 0) {
                current_nybble = stream->m_RightCodeBuf >> 4;
                stream->m_RightCode = current_nybble;
            } else {
                stream->m_RightCodeBuf = *src;
                // Stereo is interleaved so skip a byte for this channel
                src += 2;
                current_nybble = stream->m_RightCodeBuf & 0xF;
                stream->m_RightCode = current_nybble;
            }

            step = stream->m_RightStep;
            stream->m_RightDifference = step >> 3;

            if ((current_nybble & 4) != 0) {
                stream->m_RightDifference += step;
            }

            if ((current_nybble & 2) != 0) {
                stream->m_RightDifference += step >> 1;
            }

            if ((current_nybble & 1) != 0) {
                stream->m_RightDifference += step >> 2;
            }

            if ((current_nybble & 8) != 0) {
                stream->m_RightDifference = -stream->m_RightDifference;
            }

            sample = std::clamp(stream->m_RightDifference + stream->m_RightPredicted, -32768, 32767);
            stream->m_RightPredicted = sample;

            if (stream->m_BitsPerSample == 16) {
                *dst = sample;
                // Stereo is interleaved so skip a sample for this channel
                dst += 2;
            } else {
                *dst++ = ((sample & 0xFF00) >> 8) ^ 0x80;
            }

            stream->m_RightIndex += g_ADPCMIndexTab[stream->m_RightCode & 0x7];
            stream->m_RightIndex = std::clamp<int16_t>(stream->m_RightIndex, 0, 88);
            ++stream->m_RightSampleIndex;
            stream->m_RightStep = g_ADPCMStepTab[stream->m_RightIndex];
        }
    } else {
        for (int i = bytes; i > 0; --i) {
            if ((stream->m_LeftSampleIndex & 1) != 0) {
                current_nybble = stream->m_LeftCodeBuf >> 4;
                stream->m_LeftCode = current_nybble;
            } else {
                stream->m_LeftCodeBuf = *src++;
                current_nybble = stream->m_LeftCodeBuf & 0xF;
                stream->m_LeftCode = current_nybble;
            }

            step = stream->m_LeftStep;
            stream->m_LeftDifference = step >> 3;

            if ((current_nybble & 4) != 0) {
                stream->m_LeftDifference += step;
            }

            if ((current_nybble & 2) != 0) {
                stream->m_LeftDifference += step >> 1;
            }

            if ((current_nybble & 1) != 0) {
                stream->m_LeftDifference += step >> 2;
            }

            if ((current_nybble & 8) != 0) {
                stream->m_LeftDifference = -stream->m_LeftDifference;
            }

            sample = std::clamp(stream->m_LeftDifference + stream->m_LeftPredicted, -32768, 32767);
            stream->m_LeftPredicted = sample;

            if (stream->m_BitsPerSample == 16) {
                *dst++ = sample;
            } else {
                *dst = ((sample & 0xFF00) >> 8) ^ 0x80;
                dst = reinterpret_cast<int16_t *>(reinterpret_cast<char *>(dst) + 1);
            }

            stream->m_LeftIndex += g_ADPCMIndexTab[stream->m_LeftCode & 0x7];
            stream->m_LeftIndex = std::clamp<int16_t>(stream->m_LeftIndex, 0, 88);
            ++stream->m_LeftSampleIndex;
            stream->m_LeftStep = g_ADPCMStepTab[stream->m_LeftIndex];
        };
    }

    return full_length;
}
