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
    DrawerCallbackFuncPtr DrawerCallback;
    EventHandlerFuncPtr EventHandler;
    int NotifyFlags;
    int Vmode;
    int VBIBit;
    uint8_t *ImageBuf;
    int ImageWidth;
    int ImageHeight;
    int X1;
    int Y1;
    int FrameRate;
    int DrawRate;
    int TimerMethod;
    int DrawFlags;
    int OptionFlags; // VQAOptionEnum
    int NumFrameBufs;
    int NumCBBufs;
#ifdef BUILD_WITH_DSOUND
    LPDIRECTSOUND SoundObject;
    LPDIRECTSOUNDBUFFER SoundBuffer;
#endif
    void *VocFile;
    uint8_t *AudioBuf;
    unsigned AudioBufSize;
    int AudioRate;
    int Volume;
    int HMIBufSize;
    int DigiHandle;
    int DigiCard;
    int DigiPort;
    int DigiIRQ;
    int DigiDMA;
    int Language; // TODO: correct type
    void *CapFont; // TODO: correct type
    void *EVAFont; // TODO: correct type
};

void VQA_INIConfig(VQAConfig *config);
void VQA_DefaultConfig(VQAConfig *dest);

#endif
