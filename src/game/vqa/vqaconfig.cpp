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
    nullptr,          //DrawerCallback
    nullptr,          //EventHandler
    0,                //NotifyFlags
    19,               //Vmode
    -1,               //VBIBit
    nullptr,          //ImageBuf
    320,              //ImageWidth
    200,              //ImageHeight
    -1,               //X1
    -1,               //Y1
    -1,               //FrameRate
    -1,               //DrawRate
    -1,               //TimerMethod
    0,                //DrawFlags
    VQA_OPTION_SOUND_ENABLED,   //OptionFlags
    6,                //NumFrameBufs
    3,                //NumCBBufs
    nullptr,          //SoundObject
    nullptr,          //PrimarySoundBuffer
    nullptr,          //VocFile
    nullptr,          //AudioBuf
    -1,               //AudioBufSize
    -1,               //AudioRate
    255,              //Volume
    8192,             //HMIBufSize
    -1,               //DigiHandle
    -1,               //DigiCard
    -1,               //DigiPort
    -1,               //DigiIRQ
    -1,               //DigiDMA
    VQA_LANG_ENGLISH, //Language
    nullptr,          //CapFont
    nullptr           //EVAFont
};
// clang-format on

void VQA_INIConfig(VQAConfig *config)
{
    char strbuf[80];
    memset(config, 0, sizeof(VQAConfig));
    config->AudioBufSize = 0x8000;
    config->HMIBufSize = 2048;
    config->DigiHandle = -1;
    config->Volume = 255;
    config->DigiCard = -1;
    config->DigiPort = -1;
    config->DigiIRQ = -1;
    config->DigiDMA = -1;
    config->NumFrameBufs = 6;
    config->NumCBBufs = 3;

    const char *ininame = getenv("VQACFG");

    if (!ininame) {
        ininame = "player.ini";
    }

    INIClass configini;
    RawFileClass configfile(ininame);

    configini.Load(configfile);
    config->FrameRate = configini.Get_Int("Player", "FrameRate", -1);
    configini.Get_String("Player", "DrawRate", "Variable", strbuf, sizeof(strbuf));

    if (!strcasecmp(strbuf, "Variable")) {
        config->DrawRate = -1;
    } else {
        config->DrawRate = 0;
    }

    config->AudioRate = configini.Get_Int("Player", "AudioRate", -1);

    if (configini.Get_Bool("Player", "SoundEnabled", false)) {
        config->OptionFlags |= VQA_OPTION_SOUND_ENABLED;
    } else {
        config->OptionFlags &= VQA_OPTION_SOUND_ENABLED;
    }

    configini.Get_String("Player", "Port", "-1", strbuf, sizeof(strbuf));
    if (!strcasecmp(strbuf, "-1")) {
        config->DigiPort = -1;
    } else {
        sscanf(strbuf, "%x", &config->DigiPort);
    }

    config->DigiIRQ = configini.Get_Int("Player", "IRQ", -1);

    config->DigiDMA = configini.Get_Int("Player", "DMA", -1);

    if (configini.Get_Bool("Player", "SingleStep", false)) {
        config->OptionFlags |= VQA_OPTION_SINGLESTEP;
        config->DrawFlags |= 4;
    } else {
        config->OptionFlags &= VQA_OPTION_SINGLESTEP;
    }

    if (configini.Get_Bool("Player", "SlowPalette", false)) {
        config->OptionFlags |= VQA_OPTION_SLOW_PALETTE;
    } else {
        config->OptionFlags &= VQA_OPTION_SLOW_PALETTE;
    }

    if (configini.Get_Bool("Player", "MonoOutput", false)) {
        config->OptionFlags |= VQA_OPTION_MONO_OUTPUT;
    } else {
        config->OptionFlags &= VQA_OPTION_MONO_OUTPUT;
    }
}

void VQA_DefaultConfig(VQAConfig *dest)
{
    memcpy(dest, &_defaultconfig, sizeof(*dest));
}
