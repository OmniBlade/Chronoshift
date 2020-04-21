/**
 * @file
 *
 * @author CCHyper
 * @author tomsons26
 *
 * @brief  VQA player configuration.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef VQACONFIG_H
#define VQACONFIG_H

#include "always.h"

#ifdef BUILD_WITH_DSOUND
#ifdef GAME_DLL
#define DIRECTSOUND_VERSION 0x0600 // Direct Sound Version 6.0
#endif
#include <mmsystem.h>

#include <dsound.h>
#endif

enum VQAOptionEnum
{
    VQA_OPTION_SOUND_ENABLED = 1 << 0,
    VQA_OPTION_SINGLESTEP = 1 << 1,
    VQA_OPTION_MONO_OUTPUT = 1 << 2,
    VQA_OPTION_8 = 1 << 3,
    VQA_OPTION_SLOW_PALETTE = 1 << 4,
    VQA_OPTION_32 = 1 << 5,
    VQA_SETTINGS_ALTERNATIVE_AUDIO = 1 << 6,
    VQA_OPTION_128 = 1 << 7,
    VQA_OPTION_256 = 1 << 8,
    VQA_OPTION_512 = 1 << 9,
};

enum VQALanguageType
{
    VQA_LANG_ENGLISH = 0
};

DEFINE_ENUMERATION_BITWISE_OPERATORS(VQAOptionEnum);

// Need to confirm it does return,
typedef int (*DrawerCallbackFuncPtr)(void *buffer, int frame_number);
typedef int (*EventHandlerFuncPtr)(int mode, uint8_t *pal, int pal_size);

struct VQAConfig
{
    DrawerCallbackFuncPtr m_DrawerCallback;
    EventHandlerFuncPtr m_EventHandler;
    int m_NotifyFlags;
    int m_VMode;
    int m_VBIBit;
    uint8_t *m_ImageBuf;
    int m_ImageWidth;
    int m_ImageHeight;
    int m_X1;
    int m_Y1;
    int m_FrameRate;
    int m_DrawRate;
    int m_TimerMethod;
    int m_DrawFlags;
    int m_OptionFlags; // VQAOptionEnum
    int m_NumFrameBufs;
    int m_NumCBBufs;
#ifdef BUILD_WITH_DSOUND
    LPDIRECTSOUND m_SoundObject;
    LPDIRECTSOUNDBUFFER m_SoundBuffer;
#endif
    void *m_VocFile;
    uint8_t *m_AudioBuf;
    unsigned m_AudioBufSize;
    int m_AudioRate;
    int m_Volume;
    int m_HMIBufSize;
    int m_DigiHandle;
    int m_DigiCard;
    int m_DigiPort;
    int m_DigiIRQ;
    int m_DigiDMA;
    int m_Language; // TODO: correct type
    void *m_CapFont; // TODO: correct type
    void *m_EVAFont; // TODO: correct type
};

void VQA_INIConfig(VQAConfig *config);
void VQA_DefaultConfig(VQAConfig *dest);

#endif
