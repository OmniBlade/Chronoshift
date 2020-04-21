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
#include "vqaconfig.h"
#include "ini.h"
#include "rawfile.h"
#include <cstdlib>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

using std::sscanf;

// clang-format off
VQAConfig _defaultconfig = {
    nullptr,          // DrawerCallback
    nullptr,          // EventHandler
    0,                // NotifyFlags
    19,               // Vmode
    -1,               // VBIBit
    nullptr,          // ImageBuf
    320,              // ImageWidth
    200,              // ImageHeight
    -1,               // X1
    -1,               // Y1
    -1,               // FrameRate
    -1,               // DrawRate
    -1,               // TimerMethod
    0,                // DrawFlags
    VQA_OPTION_SOUND_ENABLED,   // OptionFlags
    6,                // NumFrameBufs
    3,                // NumCBBufs
#ifdef BUILD_WITH_DSOUND
    nullptr,          // SoundObject
    nullptr,          // PrimarySoundBuffer
#endif
    nullptr,          // VocFile
    nullptr,          // AudioBuf
    (unsigned)-1,               // AudioBufSize
    -1,               // AudioRate
    255,              // Volume
    8192,             // HMIBufSize
    -1,               // DigiHandle
    -1,               // DigiCard
    -1,               // DigiPort
    -1,               // DigiIRQ
    -1,               // DigiDMA
    VQA_LANG_ENGLISH, // Language
    nullptr,          // CapFont
    nullptr           // EVAFont
};
// clang-format on

void VQA_INIConfig(VQAConfig *config)
{
    char strbuf[80];
    memset(config, 0, sizeof(VQAConfig));
    config->m_AudioBufSize = 0x8000;
    config->m_HMIBufSize = 2048;
    config->m_DigiHandle = -1;
    config->m_Volume = 255;
    config->m_DigiCard = -1;
    config->m_DigiPort = -1;
    config->m_DigiIRQ = -1;
    config->m_DigiDMA = -1;
    config->m_NumFrameBufs = 6;
    config->m_NumCBBufs = 3;

    const char *ininame = getenv("VQACFG");

    if (!ininame) {
        ininame = "player.ini";
    }

    INIClass configini;
    RawFileClass configfile(ininame);

    configini.Load(configfile);
    config->m_FrameRate = configini.Get_Int("Player", "FrameRate", -1);
    configini.Get_String("Player", "DrawRate", "Variable", strbuf, sizeof(strbuf));

    if (!strcasecmp(strbuf, "Variable")) {
        config->m_DrawRate = -1;
    } else {
        config->m_DrawRate = 0;
    }

    config->m_AudioRate = configini.Get_Int("Player", "AudioRate", -1);

    if (configini.Get_Bool("Player", "SoundEnabled", false)) {
        config->m_OptionFlags |= VQA_OPTION_SOUND_ENABLED;
    } else {
        config->m_OptionFlags &= VQA_OPTION_SOUND_ENABLED;
    }

    configini.Get_String("Player", "Port", "-1", strbuf, sizeof(strbuf));
    if (!strcasecmp(strbuf, "-1")) {
        config->m_DigiPort = -1;
    } else {
        sscanf(strbuf, "%x", &config->m_DigiPort);
    }

    config->m_DigiIRQ = configini.Get_Int("Player", "IRQ", -1);

    config->m_DigiDMA = configini.Get_Int("Player", "DMA", -1);

    if (configini.Get_Bool("Player", "SingleStep", false)) {
        config->m_OptionFlags |= VQA_OPTION_SINGLESTEP;
        config->m_DrawFlags |= 4;
    } else {
        config->m_OptionFlags &= VQA_OPTION_SINGLESTEP;
    }

    if (configini.Get_Bool("Player", "SlowPalette", false)) {
        config->m_OptionFlags |= VQA_OPTION_SLOW_PALETTE;
    } else {
        config->m_OptionFlags &= VQA_OPTION_SLOW_PALETTE;
    }

    if (configini.Get_Bool("Player", "MonoOutput", false)) {
        config->m_OptionFlags |= VQA_OPTION_MONO_OUTPUT;
    } else {
        config->m_OptionFlags &= VQA_OPTION_MONO_OUTPUT;
    }
}

void VQA_DefaultConfig(VQAConfig *dest)
{
    memcpy(dest, &_defaultconfig, sizeof(*dest));
}
