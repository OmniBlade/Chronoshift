/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief  VQA audio functions.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef VQAAUDIO_H
#define VQAAUDIO_H

#include "always.h"
#include "vqaadpcm.h"

#ifdef GAME_DLL
#include "hooker.h"
#define DIRECTSOUND_VERSION 0x0600
#endif

#ifdef BUILD_WITH_DSOUND
#include <mmsystem.h>

#include <dsound.h>
#endif

struct VQAHandle;

enum VQATimerMethod
{
    VQA_AUDIO_TIMER_METHOD_M1 = -1,
    VQA_AUDIO_TIMER_METHOD_NONE = 0, // TODO: Needs confirming
    VQA_AUDIO_TIMER_METHOD_DOS = 1,
    VQA_AUDIO_TIMER_METHOD_INTERRUPT = 2,
    VQA_AUDIO_TIMER_METHOD_DMA = 3, // DMA - Direct memory access
};

DEFINE_ENUMERATION_BITWISE_OPERATORS(VQATimerMethod);

enum VQAAudioFlags
{
    // VQA_AUDIO_FLAG_DOS_TIMER = unknown value,//Method 1
    VQA_AUDIO_FLAG_UNKNOWN001 = 0x1,
    VQA_AUDIO_FLAG_UNKNOWN002 = 0x2,
    VQA_AUDIO_FLAG_UNKNOWN004 = 0x4,
    VQA_AUDIO_FLAG_UNKNOWN008 = 0x8,
    VQA_AUDIO_FLAG_UNKNOWN016 = 0x10,
    VQA_AUDIO_FLAG_UNKNOWN032 = 0x20,
    VQA_AUDIO_FLAG_INTERRUPT_TIMER = VQA_AUDIO_FLAG_UNKNOWN016 | VQA_AUDIO_FLAG_UNKNOWN032, // bit 48//Method 2
    VQA_AUDIO_FLAG_AUDIO_DMA_TIMER = 0x40, // Method 3
    VQA_AUDIO_FLAG_UNKNOWN128 = 0x80,
};

DEFINE_ENUMERATION_BITWISE_OPERATORS(VQAAudioFlags);
extern int &g_AudioFlags;

struct VQAAudio
{
    uint8_t *m_Buffer;
    unsigned m_AudBufPos;
    int16_t *m_IsLoaded;
    unsigned m_NumAudBlocks;
    unsigned field_10; // block position?
    unsigned field_14; // block position for IsLoaded?
    uint8_t *m_TempBuf;
    unsigned m_TempBufSize;
    unsigned m_TempBufLen;
    unsigned m_Flags;
    unsigned m_PlayPosition;
    unsigned m_SamplesPlayed;
    unsigned m_NumSkipped;
    uint16_t m_SampleRate; // 22050, 44100, etc.
    int8_t m_Channels; // Mono = 1, Stereo = 2, etc.
    int8_t m_BitsPerSample; // 8 bits = 8, 16 bits = 16, etc.
    int field_38; // Bytes or samples per second?
    VQAADPCMStreamType m_AdcpmInfo;
#ifdef BUILD_WITH_DSOUND
    MMRESULT m_SoundTimerHandle;
#endif
    int field_7C;
#ifdef BUILD_WITH_DSOUND
    LPDIRECTSOUNDBUFFER m_PrimaryBufferPtr;
    WAVEFORMATEX m_BuffFormat;
    DSBUFFERDESC m_BufferDesc;
#endif
    int16_t field_AA;
    unsigned field_AC; // HMIBufSize * 4 set field_AC, and field_AC sets dwBufferBytes.
    unsigned field_B0; // current chunk position or offset?
    int field_B4; // use to set ChunksMovedToAudioBuffer in VQA_GetTime
    int field_B8; // use to set LastChunkPosition in VQA_GetTime
    int field_BC; // set when the sound object is created.
    int field_C0; // set when the sound buffer is created.
};

int VQA_StartTimerInt(VQAHandle *handle, int a2);
void VQA_StopTimerInt(VQAHandle *handle);

int VQA_OpenAudio(VQAHandle *handle, HWND hWnd);
void VQA_CloseAudio(VQAHandle *handle);

int VQA_StartAudio(VQAHandle *handle);
void VQA_StopAudio(VQAHandle *handle);

int VQA_CopyAudio(VQAHandle *handle);

void VQA_SetTimer(VQAHandle *handle, int time, unsigned method);
unsigned VQA_GetTime(VQAHandle *handle);
int VQA_TimerMethod();

#endif // VQAAUDIO_H
