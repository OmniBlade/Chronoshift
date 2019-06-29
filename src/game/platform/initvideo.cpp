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
#include "gbuffer.h"
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
LPDIRECTDRAWPALETTE g_palettePtr;
#endif
#endif

#ifdef BUILD_WITH_DDRAW
static void Process_DD_Result(HRESULT result, int unk)
{
    // TODO? Popped up meesage box for various errors in original.
    return;
}
#endif

static void Check_Overlapped_Blit_Capability()
{
    GraphicBufferClass buff;

    g_OverlappedVideoBlits = true;
    buff.Init(64, 64, 0, 0, GBC_VIDEO_MEM);
    buff.Clear();
    buff.Put_Pixel(0, 0, 0xFF);
    buff.Blit(buff, 0, 0, 0, 1, buff.Get_Width(), buff.Get_Height() - 1);
    uint8_t pixel = buff.Get_Pixel(0, 5);

    if (pixel == 0xFF) {
        g_OverlappedVideoBlits = false;
    }
}

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

        result = g_directDrawObject->SetCooperativeLevel(
            (HWND)handle, (w == 320 ? DDSCL_ALLOWMODEX : 0) | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
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

void Reset_Video_Mode()
{
#ifdef BUILD_WITH_DDRAW
    if (g_directDrawObject) {
        HRESULT result = g_directDrawObject->RestoreDisplayMode();
        Process_DD_Result(result, 0);
        result = g_directDrawObject->Release();
        Process_DD_Result(result, 0);
        g_directDrawObject = nullptr;
    }
#endif
}

BOOL Init_Video()
{
    return 0;
}
