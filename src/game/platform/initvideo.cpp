/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Initialisation functions for graphics API.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "initvideo.h"
#include "globals.h"

#ifndef CHRONOSHIFT_STANDALONE
#ifdef BUILD_WITH_DDRAW
LPDIRECTDRAW &g_directDrawObject = Make_Global<LPDIRECTDRAW>(0x006B1490);
LPDIRECTDRAWSURFACE &g_paletteSurface = Make_Global<LPDIRECTDRAWSURFACE>(0x006B18A4);
tagPALETTEENTRY *const g_paletteEntries = Make_Pointer<tagPALETTEENTRY>(0x006B149C);
LPDIRECTDRAWPALETTE &g_palettePtr = Make_Global<LPDIRECTDRAWPALETTE>(0x006B189C);
#endif
#else
#ifdef BUILD_WITH_DDRAW
LPDIRECTDRAW g_directDrawObject;
LPDIRECTDRAWSURFACE g_paletteSurface = nullptr;
tagPALETTEENTRY g_paletteEntries[256];
LPDIRECTDRAWPALETTE &g_palettePtr;
#endif
#endif

#ifdef BUILD_WITH_DDRAW
static void Process_DD_Result(HRESULT result, int unk)
{
    return;
}

static void Check_Overlapped_Blit_Capability()
{
    // TODO actual check.
    g_OverlappedVideoBlits = true;
}
#endif
BOOL Set_Video_Mode(uintptr_t handle, int w, int h, int bpp)
{
#ifdef BUILD_WITH_DDRAW
    HRESULT result;

    if (g_directDrawObject == nullptr) {
        result = DirectDrawCreate(nullptr, &g_directDrawObject, nullptr);
        Process_DD_Result(result, 0);

        if (result != DD_OK) {
            return false;
        }

        result = g_directDrawObject->SetCooperativeLevel((HWND)handle, (w == 320 ? DDSCL_ALLOWMODEX : 0) | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
        Process_DD_Result(result, 0);
    }

    if (g_directDrawObject->SetDisplayMode(w, h, bpp) != DD_OK) {
        g_directDrawObject->Release();
        g_directDrawObject = nullptr;

        return false;
    }

    // Fixes compiler error, looks like watcom directx sdk header has wrong prototype.
#ifdef COMPILER_WATCOM
    result = g_directDrawObject->CreatePalette(
        DDPCAPS_8BIT | DDPCAPS_ALLOW256, g_paletteEntries, (LPDIRECTDRAWPALETTE)&g_palettePtr, nullptr);
#else
    result = g_directDrawObject->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, g_paletteEntries, &g_palettePtr, nullptr);
#endif
    Process_DD_Result(result, 0);

    if (result != DD_OK) {
        return false;
    }

    Check_Overlapped_Blit_Capability();

    return true;
#endif
}
