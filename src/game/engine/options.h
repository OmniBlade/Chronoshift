/**
 * @file
 *
 * @Author CCHyper, OmniBlade
 *
 * @brief Global variables that are used amongst many classes.
 *
 * @copyright Redalert++ is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef OPTIONS_H
#define OPTIONS_H

#include "always.h"
#include "fixed.h"
#include "keyboard.h"
#include "palette.h"

enum GameSpeedEnum
{
    GAMESPEED_0 = 0,
    GAMESPEED_1 = 1,
    GAMESPEED_2 = 2,
    GAMESPEED_3 = 3,
    GAMESPEED_4 = 4,
    GAMESPEED_5 = 5,
    GAMESPEED_6 = 6,
    GAMESPEED_7 = 7,
};

enum ScrollSpeedEnum
{
    SCROLLSPEED_0 = 0,
    SCROLLSPEED_1 = 1,
    SCROLLSPEED_2 = 2,
    SCROLLSPEED_3 = 3,
    SCROLLSPEED_4 = 4,
    SCROLLSPEED_5 = 5,
};

class OptionsClass
{
public:
    OptionsClass();

    void One_Time();
    void Process() {}
    void Save_Settings();
    void Load_Settings();
    void Adjust_Palette(
        PaletteClass &src, PaletteClass &dst, fixed brightness, fixed saturation, fixed tint, fixed contrast) const;
    void Fixup_Palette() const;
    int Normalize_Volume(int volume) const { return volume * Volume; }
    int Normalize_Delay(int delay) const;
    void Set_Shuffle(BOOL shuffle) { ScoreShuffles = shuffle; }
    void Set_Repeat(BOOL repeat) { ScoreRepeats = repeat; }
    void Set_Score_Volume(fixed volume, BOOL beep = false);
    void Set_Sound_Volume(fixed volume, BOOL beep = false);
    void Set_Brightness(fixed brightness);
    void Set_Saturation(fixed saturation);
    void Set_Contrast(fixed contrast);
    void Set_Tint(fixed tint);
    void Set();
    fixed Get_Score_Volume() { return ScoreVolume; }
    BOOL Get_Repeat() { return ScoreRepeats; }
    BOOL Get_Shuffle() { return ScoreShuffles; }
    fixed Get_Brightness() { return (Brightness - fixed::_1_4) / fixed::_1_2; }
    fixed Get_Saturation() { return Saturation; }
    fixed Get_Contrast() { return (Contrast - fixed::_1_4) / fixed::_1_2; }
    fixed Get_Tint() { return Tint; }


#ifndef RAPP_STANDALONE
    static void Hook_Me();
#endif

private:
    int GameSpeed;
    int ScrollRate;
    fixed Volume;
    fixed ScoreVolume;
    fixed MultiplayerScoreVolume;
    fixed Brightness;
    fixed Tint;
    fixed Saturation;
    fixed Contrast;
#ifndef RAPP_NO_BITFIELDS
    // Union/Struct required to get correct packing when compiler packing set to 1.
    union
    {
        struct
        {
            bool AutoScroll : 1; // & 1
            bool ScoreRepeats : 1; // & 2
            bool ScoreShuffles : 1; // & 4
            bool PaletteScroll : 1; // & 8
        };
        int Bitfield;
    };
#else
    bool AutoScroll;
    bool ScoreRepeats;
    bool ScoreShuffles;
    bool PaletteScroll;
#endif
    KeyType KeyForceMove1;
    KeyType KeyForceMove2;
    KeyType KeyForceAttack1;
    KeyType KeyForceAttack2;
    KeyType KeySelect1;
    KeyType KeySelect2;
    KeyType KeyScatter;
    KeyType KeyStop;
    KeyType KeyGuard;
    KeyType KeyNext;
    KeyType KeyPrevious;
    KeyType KeyFormation;
    KeyType KeyHome1;
    KeyType KeyHome2;
    KeyType KeyBase;
    KeyType KeyResign;
    KeyType KeyAlliance;
    KeyType KeyBookmark1;
    KeyType KeyBookmark2;
    KeyType KeyBookmark3;
    KeyType KeyBookmark4;
    KeyType KeySelectView;
    KeyType KeyRepairToggle;
    KeyType KeyRepairOn;
    KeyType KeyRepairOff;
    KeyType KeySellToggle;
    KeyType KeySellOn;
    KeyType KeySellOff;
    KeyType KeyMapToggle;
    KeyType KeySidebarUp;
    KeyType KeySidebarDown;
    KeyType KeyOption1;
    KeyType KeyOption2;
    KeyType KeyScrollLeft;
    KeyType KeyScrollRight;
    KeyType KeyScrollUp;
    KeyType KeyScrollDown;
    KeyType KeyQueueMove1;
    KeyType KeyQueueMove2;
    KeyType KeyTeam1;
    KeyType KeyTeam2;
    KeyType KeyTeam3;
    KeyType KeyTeam4;
    KeyType KeyTeam5;
    KeyType KeyTeam6;
    KeyType KeyTeam7;
    KeyType KeyTeam8;
    KeyType KeyTeam9;
    KeyType KeyTeam10;
};

#ifndef RAPP_STANDALONE
#include "hooker.h"

inline void OptionsClass::Hook_Me()
{
#ifdef COMPILER_WATCOM
    //Hook_Function(0x00525884, *OptionsClass::Adjust_Palette);
#endif
}
#endif

#endif // OPTIONS_H