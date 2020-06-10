/**
 * @file
 *
 * @author tomsons26
 *
 * @brief  VQA drawing related functions.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "vqadrawer.h"
#include "dialog.h"
#include "lcw.h"
#include "unvqbuff.h"
#include "vqacaption.h"
#include "vqafile.h"
#include "vqaloader.h"
#include "vqapalette.h"
#include <captainslog.h>
#include <cstring>

using std::memcpy;

int VQA_DrawFrame_Buffer(VQAHandle *handle)
{
    VQAConfig *config = &handle->m_Config;
    VQAData *vqabuf = handle->m_VQABuf;
    VQADrawer *drawer = &vqabuf->m_Drawer;
    VQAFrameNode *curframe = drawer->m_CurFrame;

    if (!(vqabuf->m_Flags & 2)) {
        captainslog_trace("VQA_DrawFrame_Buffer() - About to call VQA_SelectFrame().\n");
        int result = VQA_SelectFrame(handle);

        if (result) {
            captainslog_trace(
                "VQA_DrawFrame_Buffer() - VQA_SelectFrame() failed, returned %s\n", Get_VQA_Error((VQAErrorType)result));
            return result;
        }

        captainslog_trace("VQA_DrawFrame_Buffer() - About to call VQA_PrepareFrame().\n");
        VQA_PrepareFrame(vqabuf);
    }

    if (vqabuf->m_Flags & 1) {
        vqabuf->m_Flags |= 2;
        captainslog_trace("VQA_DrawFrame_Buffer() - About to return VQAERR_SLEEPING.\n");
        return VQAERR_SLEEPING;
    }

    if (vqabuf->m_Flags & 2) {
        ++vqabuf->m_Drawer.m_WaitsOnFlipper;
        vqabuf->m_Flags &= 0xFFFFFFFD;
    }

    if ((curframe->m_Flags & 4) || (vqabuf->m_Drawer.m_Flags & 1)) {
        captainslog_trace("VQA_DrawFrame_Buffer() - About to call VQA_Flag_To_Set_Palette().\n");
        VQA_Flag_To_Set_Palette(
            curframe->m_Palette, curframe->m_PaletteSize, (handle->m_Config.m_OptionFlags & VQA_OPTION_SLOW_PALETTE));
        curframe->m_Flags &= ~4;
        vqabuf->m_Drawer.m_Flags &= ~1;
    }

    captainslog_trace("VQA_DrawFrame_Buffer() - About to call UnVQ.\n");
    vqabuf->UnVQ(curframe->m_Codebook->m_Buffer,
        curframe->m_Pointers,
        vqabuf->m_Drawer.m_ScreenOffset + vqabuf->m_Drawer.m_ImageBuf,
        vqabuf->m_Drawer.m_BlocksPerRow,
        vqabuf->m_Drawer.m_NumRows,
        vqabuf->m_Drawer.m_ImageWidth);
    captainslog_trace("VQA_DrawFrame_Buffer() - UnVQ called successfully.\n");
    vqabuf->m_Drawer.m_LastFrameNum = curframe->m_FrameNum;
    vqabuf->m_Flipper.m_CurFrame = curframe;
    vqabuf->m_Flags |= 1;

    if (config->m_DrawerCallback != nullptr) {
        captainslog_trace("VQA_DrawFrame_Buffer() - About to call DrawerCallback.\n");

        if (config->m_DrawerCallback(vqabuf->m_Drawer.m_ImageBuf, curframe->m_FrameNum)) {
            return VQAERR_EOF;
        }
    }

    captainslog_trace("VQA_DrawFrame_Buffer() - Assigning drawer->CurFrame to next frame.\n");
    drawer->m_CurFrame = curframe->m_Next;

    return VQAERR_NONE;
}

int DrawFrame_Nop(VQAHandle *handle)
{
    return VQAERR_NONE;
}

int PageFlip_Nop(VQAHandle *handle)
{
    return VQAERR_NONE;
}

uint8_t *VQA_GetPalette(VQAHandle *handle)
{
    uint8_t *palette = 0;

    if (handle->m_VQABuf->m_Drawer.m_CurPalSize > 0) {
        palette = handle->m_VQABuf->m_Drawer.m_Palette_24;
    }

    return palette;
}

int VQA_GetPaletteSize(VQAHandle *handle)
{
    return handle->m_VQABuf->m_Drawer.m_CurPalSize;
}

// wip replacement for identical chunks in SetDrawBuffer and ConfigureDrawer
void VQA_SetDrawRect(VQAHandle *handle, unsigned width, unsigned height, int xpos, int ypos)
{
    VQAHeader *header = &handle->m_Header;
    VQAData *vqabuf = handle->m_VQABuf;
    VQAConfig *config = &handle->m_Config;
    unsigned flags_1 = config->m_DrawFlags & 0x30;

    if (xpos != -1 || ypos != -1) {
        if (flags_1 < 32) {
            vqabuf->m_Drawer.m_Y1 = ypos;
            vqabuf->m_Drawer.m_X1 = xpos;
            vqabuf->m_Drawer.m_X2 = vqabuf->m_Drawer.m_X1 + header->m_ImageWidth - 1;
            vqabuf->m_Drawer.m_Y2 = vqabuf->m_Drawer.m_Y1 + header->m_ImageHeight - 1;
        }

        if (flags_1 <= 32) {
            vqabuf->m_Drawer.m_Y1 = height - ypos;
            vqabuf->m_Drawer.m_X1 = width - xpos;
            vqabuf->m_Drawer.m_X2 = vqabuf->m_Drawer.m_X1 - header->m_ImageWidth;
            vqabuf->m_Drawer.m_Y2 = vqabuf->m_Drawer.m_Y1 - header->m_ImageHeight;
        }

        if (flags_1 != 48) {
            vqabuf->m_Drawer.m_Y1 = ypos;
            vqabuf->m_Drawer.m_X1 = xpos;
            vqabuf->m_Drawer.m_X2 = vqabuf->m_Drawer.m_X1 + header->m_ImageWidth - 1;
            vqabuf->m_Drawer.m_Y2 = vqabuf->m_Drawer.m_Y1 + header->m_ImageHeight - 1;
        }

        vqabuf->m_Drawer.m_X1 = xpos;
        vqabuf->m_Drawer.m_Y1 = height - ypos;
        vqabuf->m_Drawer.m_X2 = vqabuf->m_Drawer.m_X1 + header->m_ImageWidth - 1;
        vqabuf->m_Drawer.m_Y2 = vqabuf->m_Drawer.m_Y2 - header->m_ImageHeight - 1;
    } else {
        vqabuf->m_Drawer.m_X1 = (width - header->m_ImageWidth) / 2;
        vqabuf->m_Drawer.m_Y1 = (height - header->m_ImageHeight) / 2;
        vqabuf->m_Drawer.m_X2 = vqabuf->m_Drawer.m_X1 + header->m_ImageWidth - 1;
        vqabuf->m_Drawer.m_Y2 = vqabuf->m_Drawer.m_Y1 + header->m_ImageHeight - 1;
    }
}

void VQA_SetDrawBuffer(VQAHandle *handle, uint8_t *buffer, unsigned width, unsigned height, int xpos, int ypos)
{
    VQADrawer *drawer = &handle->m_VQABuf->m_Drawer;
    unsigned flags = handle->m_Config.m_DrawFlags & 0x30;

    drawer->m_ImageBuf = buffer;
    drawer->m_ImageWidth = width;
    drawer->m_ImageHeight = height;

    if (xpos == -1 || ypos == -1) {
        drawer->m_X1 = (width - handle->m_Header.m_ImageWidth) >> 1;
        drawer->m_Y1 = (height - handle->m_Header.m_ImageHeight) >> 1;
        drawer->m_X2 = drawer->m_X1 + handle->m_Header.m_ImageWidth - 1;
        drawer->m_Y2 = drawer->m_Y1 + handle->m_Header.m_ImageHeight - 1;
    } else {
        if (flags == 0) {
            drawer->m_X1 = xpos;
            drawer->m_Y1 = ypos;
            drawer->m_X2 = drawer->m_X1 + handle->m_Header.m_ImageWidth - 1;
            drawer->m_Y2 = drawer->m_Y1 + handle->m_Header.m_ImageHeight - 1;
        } else if (flags == 0x20) {
            drawer->m_X1 = width - xpos;
            drawer->m_Y1 = height - ypos;
            drawer->m_X2 = drawer->m_X1 - handle->m_Header.m_ImageWidth;
            drawer->m_Y2 = drawer->m_Y1 - handle->m_Header.m_ImageHeight;
        } else if (flags == 0x30) {
            drawer->m_X1 = xpos;
            drawer->m_Y1 = ypos;
            drawer->m_X2 = drawer->m_X1 + handle->m_Header.m_ImageWidth - 1;
            drawer->m_Y2 = drawer->m_Y1 + handle->m_Header.m_ImageHeight - 1;
        } else {
            drawer->m_X1 = xpos;
            drawer->m_Y1 = height - ypos;
            drawer->m_X2 = drawer->m_X1 + handle->m_Header.m_ImageWidth - 1;
            drawer->m_Y2 = drawer->m_Y2 - handle->m_Header.m_ImageHeight - 1;
        }
    }

    drawer->m_ScreenOffset = drawer->m_X1 + drawer->m_Y1 * width;
}

void VQA_ConfigureDrawer(VQAHandle *handle)
{
    VQAData *data = handle->m_VQABuf;
    unsigned flags = handle->m_Config.m_DrawFlags & 0x30;

    if (handle->m_Config.m_X1 == -1 && handle->m_Config.m_Y1 == -1) {
        data->m_Drawer.m_X1 = (data->m_Drawer.m_ImageWidth - handle->m_Header.m_ImageWidth) / 2;
        data->m_Drawer.m_Y1 = (data->m_Drawer.m_ImageHeight - handle->m_Header.m_ImageHeight) / 2;
        data->m_Drawer.m_X2 = data->m_Drawer.m_X1 + handle->m_Header.m_ImageWidth - 1;
        data->m_Drawer.m_Y2 = data->m_Drawer.m_Y1 + handle->m_Header.m_ImageHeight - 1;
    } else {
        if (flags == 0) {
            data->m_Drawer.m_X1 = handle->m_Config.m_X1;
            data->m_Drawer.m_Y1 = handle->m_Config.m_Y1;
            data->m_Drawer.m_X2 = data->m_Drawer.m_X1 + handle->m_Header.m_ImageWidth - 1;
            data->m_Drawer.m_Y2 = data->m_Drawer.m_Y1 + handle->m_Header.m_ImageHeight - 1;
        } else if (flags == 0x20) {
            data->m_Drawer.m_X1 = data->m_Drawer.m_ImageWidth - handle->m_Config.m_X1;
            data->m_Drawer.m_Y1 = data->m_Drawer.m_ImageHeight - handle->m_Config.m_Y1;
            data->m_Drawer.m_X2 = data->m_Drawer.m_X1 - handle->m_Header.m_ImageWidth;
            data->m_Drawer.m_Y2 = data->m_Drawer.m_Y1 - handle->m_Header.m_ImageHeight;
        } else if (flags != 0x30) {
            data->m_Drawer.m_X1 = handle->m_Config.m_X1;
            data->m_Drawer.m_Y1 = handle->m_Config.m_Y1;
            data->m_Drawer.m_X2 = data->m_Drawer.m_X1 + handle->m_Header.m_ImageWidth - 1;
            data->m_Drawer.m_Y2 = data->m_Drawer.m_Y1 + handle->m_Header.m_ImageHeight - 1;
        } else {
            data->m_Drawer.m_X1 = handle->m_Config.m_X1;
            data->m_Drawer.m_Y1 = data->m_Drawer.m_ImageHeight - handle->m_Config.m_Y1;
            data->m_Drawer.m_X2 = data->m_Drawer.m_X1 + handle->m_Header.m_ImageWidth - 1;
            data->m_Drawer.m_Y2 = data->m_Drawer.m_Y2 - handle->m_Header.m_ImageHeight - 1;
        }
    }

    data->m_Drawer.m_BlocksPerRow = handle->m_Header.m_ImageWidth / handle->m_Header.m_BlockWidth;
    data->m_Drawer.m_NumRows = handle->m_Header.m_ImageHeight / handle->m_Header.m_BlockHeight;
    data->m_Drawer.m_NumBlocks = data->m_Drawer.m_NumRows * data->m_Drawer.m_BlocksPerRow;
    int blocksize = handle->m_Header.m_BlockHeight | (handle->m_Header.m_BlockWidth << 8);
    data->UnVQ = UnVQ_Nop;
    data->Draw_Frame = DrawFrame_Nop;
    data->Page_Flip = PageFlip_Nop;

    if (handle->m_Config.m_DrawFlags & 1) {
        switch (blocksize) {
            case 0x402:
                data->UnVQ = UnVQ_4x2;
                break;
            case 0x404:
                data->UnVQ = UnVQ_4x4;
                break;
            default:
                break;
        };
    }

    data->Draw_Frame = VQA_DrawFrame_Buffer;
    data->m_Drawer.m_ScreenOffset = data->m_Drawer.m_X1 + data->m_Drawer.m_Y1 * data->m_Drawer.m_ImageWidth;
}

int VQA_SelectFrame(VQAHandle *handle)
{
    int result;
    VQAConfig *config = &handle->m_Config;
    VQAData *vqabuf = handle->m_VQABuf;
    VQAFrameNode *curframe = vqabuf->m_Drawer.m_CurFrame;

    if (!(curframe->m_Flags & 1)) {
        ++vqabuf->m_Drawer.m_WaitsOnLoader;
        return VQAERR_NOBUFFER;
    }

    if (handle->m_Config.m_OptionFlags & 2) {
        vqabuf->m_Drawer.m_LastFrame = curframe->m_FrameNum;
        return VQAERR_NONE;
    }

    unsigned curtime = VQA_GetTime(handle);
    int desiredframe = handle->m_Config.m_DrawRate * curtime / 60;
    vqabuf->m_Drawer.m_DesiredFrame = desiredframe;

    if (handle->m_Config.m_DrawRate == handle->m_Config.m_FrameRate) {
        if (curframe->m_FrameNum > desiredframe) {
            return VQAERR_NOT_TIME;
        }
    } else if (60u / handle->m_Config.m_DrawRate > curtime - vqabuf->m_Drawer.m_LastTime) {
        return VQAERR_NOT_TIME;
    }

    if (handle->m_Config.m_FrameRate / 5 > curframe->m_FrameNum - vqabuf->m_Drawer.m_LastFrame) {
        if (handle->m_Config.m_DrawFlags & 4) {
            vqabuf->m_Drawer.m_LastFrame = curframe->m_FrameNum;
            result = VQAERR_NONE;

        } else {
            while (true) {
                if (!(curframe->m_Flags & 1)) {
                    return VQAERR_NOBUFFER;
                }

                if (curframe->m_Flags & 2 || curframe->m_FrameNum >= desiredframe) {
                    break;
                }

                if (curframe->m_Flags & 4) {
                    if (curframe->m_Flags & 8) {
                        curframe->m_PaletteSize = LCW_Uncomp((curframe->m_PalOffset + curframe->m_Palette),
                            curframe->m_Palette,
                            vqabuf->m_MaxPalSize); // Beta line, was replaced with
                        curframe->m_Flags &= 0xFFFFFFF7;
                    }

                    memcpy(vqabuf->m_Drawer.m_Palette_24, curframe->m_Palette, curframe->m_PaletteSize);
                    vqabuf->m_Drawer.m_CurPalSize = curframe->m_PaletteSize;
                    vqabuf->m_Drawer.m_Flags |= 1u;
                }

                if (config->m_DrawerCallback != nullptr) {
                    config->m_DrawerCallback(nullptr, curframe->m_FrameNum);
                }

                curframe->m_Flags = 0;
                curframe = curframe->m_Next;
                vqabuf->m_Drawer.m_CurFrame = curframe;
                ++vqabuf->m_Drawer.m_NumSkipped;
            }

            vqabuf->m_Drawer.m_LastFrame = curframe->m_FrameNum;
            vqabuf->m_Drawer.m_LastTime = curtime;
            result = VQAERR_NONE;
        }
    } else {
        vqabuf->m_Drawer.m_LastFrame = curframe->m_FrameNum;
        result = VQAERR_NONE;
    }

    return result;
}

void VQA_PrepareFrame(VQAData *vqabuf)
{
    VQAFrameNode *curframe = vqabuf->m_Drawer.m_CurFrame;
    VQACBNode *codebook = curframe->m_Codebook;

    if (codebook->m_Flags & 0x02) {
        LCW_Uncomp(&codebook->m_Buffer[codebook->m_CBOffset], codebook->m_Buffer, vqabuf->m_MaxCBSize);
        codebook->m_Flags &= ~0x02;
    }

    if (curframe->m_Flags & 0x08) {
        curframe->m_PaletteSize =
            LCW_Uncomp(&curframe->m_Palette[curframe->m_PalOffset], curframe->m_Palette, vqabuf->m_MaxPalSize);
        curframe->m_Flags &= ~0x08;
    }

    if (curframe->m_Flags & 0x10) {
        LCW_Uncomp(&curframe->m_Pointers[curframe->m_PtrOffset], curframe->m_Pointers, vqabuf->m_MaxPtrSize);
        curframe->m_Flags &= ~0x10;
    }
}
