/**
 * @file
 *
 * @author tomsons26
 *
 * @brief  VQA file loader structures and functions for handling various VQA chunks.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "vqaloader.h"
#include "adpcm.h"
#include "basefile.h"
#include "endiantype.h"
#include "globals.h"
#include "lcw.h"
#include "vqaadpcm.h"
#include "vqacaption.h"
#include "vqaconfig.h"
#include <captainslog.h>
#include <cstdlib>
#include <cstring>

using std::memset;

int VQA_Load_FINF(VQAHandle *handle, unsigned iffsize)
{
    VQAData *data = handle->m_VQABuf;

    if (data != nullptr && data->m_Foff != nullptr) {
        if (handle->m_StreamHandler(handle, ACTION_READ, data->m_Foff, (iffsize + 1) & (~1))) {
            captainslog_debug("VQA_Load_FINF() - Failed to read Foff!\n");
            return VQAERR_READ;
        }
    } else if (handle->m_StreamHandler(handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, (iffsize + 1) & (~1))) {
        captainslog_debug("VQA_Load_FINF() - Failed to seek!\n");
        return VQAERR_SEEK;
    }

    captainslog_debug("VQA_Load_FINF() - Foff read successfully.\n");
    return VQAERR_NONE;
}

int VQA_Load_CAP0(VQAHandle *handle, unsigned iffsize)
{
    VQAData *data = handle->m_VQABuf;

    if (data != nullptr && data->m_Foff != nullptr) {
        if (handle->m_StreamHandler(handle, ACTION_READ, data->m_Foff, (iffsize + 1) & (~1))) {
            captainslog_debug("VQA_Load_CAP0() - Failed to read Foff!\n");
            return VQAERR_READ;
        }
    } else if (handle->m_StreamHandler(handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, (iffsize + 1) & (~1))) {
        captainslog_debug("VQA_Load_CAP0() - Failed to seek!\n");
        return VQAERR_SEEK;
    }

    captainslog_debug("VQA_Load_CAP0() - Foff read successfully.\n");
    return VQAERR_NONE;
}

int VQA_Load_CBF0(VQAHandle *handle, unsigned iffsize)
{
    VQALoader *loader = &handle->m_VQABuf->m_Loader;
    VQACBNode *curcb = (VQACBNode *)loader->m_CurCB;

    if (handle->m_StreamHandler(handle, ACTION_READ, loader->m_CurCB->m_Buffer, (iffsize + 1) & (~1))) {
        return VQAERR_READ;
    }

    loader->m_NumPartialCB = 0;
    curcb->m_Flags &= 0xFFFFFFFD;
    curcb->m_CBOffset = 0;
    loader->m_FullCB = curcb;
    loader->m_FullCB->m_Flags &= (~1);
    loader->m_CurCB = curcb->m_Next;

    return VQAERR_NONE;
}

int VQA_Load_CBFZ(VQAHandle *handle, unsigned iffsize)
{
    VQALoader *loader = &handle->m_VQABuf->m_Loader;
    VQACBNode *curcb = loader->m_CurCB;
    unsigned lcwoffset = handle->m_VQABuf->m_MaxCBSize - ((iffsize + 1) & 0xFFFE);

    if (handle->m_StreamHandler(handle, ACTION_READ, &loader->m_CurCB->m_Buffer[lcwoffset], (iffsize + 1) & (~1))) {
        return VQAERR_READ;
    }
    loader->m_NumPartialCB = 0;
    curcb->m_Flags |= 2u;
    curcb->m_CBOffset = lcwoffset;
    loader->m_FullCB = curcb;
    loader->m_FullCB->m_Flags &= (~1);
    loader->m_CurCB = curcb->m_Next;

    return VQAERR_NONE;
}

int VQA_Load_CBP0(VQAHandle *handle, unsigned iffsize)
{
    VQALoader *loader = &handle->m_VQABuf->m_Loader;
    VQACBNode *curcb = (VQACBNode *)loader->m_CurCB;

    if (handle->m_StreamHandler(handle,
            ACTION_READ,
            &loader->m_CurCB->m_Buffer[handle->m_VQABuf->m_Loader.m_PartialCBSize],
            (iffsize + 1) & (~1))) {
        return VQAERR_READ;
    }

    loader->m_PartialCBSize += (uint16_t)iffsize;

    if (handle->m_Header.m_Groupsize == ++loader->m_NumPartialCB) {
        loader->m_NumPartialCB = 0;
        loader->m_PartialCBSize = 0;
        curcb->m_Flags &= 0xFFFFFFFD;
        curcb->m_CBOffset = 0;
        loader->m_FullCB = curcb;
        loader->m_FullCB->m_Flags &= (~1);
        loader->m_CurCB = curcb->m_Next;
    }

    return VQAERR_NONE;
}

int VQA_Load_CBPZ(VQAHandle *handle, unsigned iffsize)
{
    VQAData *data = handle->m_VQABuf;
    VQALoader *loader = &data->m_Loader;
    VQACBNode *curcb = data->m_Loader.m_CurCB;
    unsigned padsize = (iffsize + 1) & (~1);

    if (!data->m_Loader.m_PartialCBSize) {
        curcb->m_CBOffset = data->m_MaxCBSize - (handle->m_Header.m_Groupsize * padsize + 0x64);
    }

    if (handle->m_StreamHandler(
            handle, ACTION_READ, &curcb->m_Buffer[curcb->m_CBOffset] + data->m_Loader.m_PartialCBSize, padsize)) {
        return VQAERR_READ;
    }

    loader->m_PartialCBSize += (uint16_t)iffsize;
    if (handle->m_Header.m_Groupsize == ++loader->m_NumPartialCB) {
        loader->m_NumPartialCB = 0;
        loader->m_PartialCBSize = 0;
        curcb->m_Flags |= 2u;
        loader->m_FullCB = curcb;
        loader->m_FullCB->m_Flags &= (~1);
        loader->m_CurCB = curcb->m_Next;
    }

    return VQAERR_NONE;
}

int VQA_Load_CPL0(VQAHandle *handle, unsigned iffsize)
{
    VQAFrameNode *curframe = handle->m_VQABuf->m_Loader.m_CurFrame;

    if (handle->m_StreamHandler(handle, ACTION_READ, curframe->m_Palette, (iffsize + 1) & (~1))) {
        return VQAERR_READ;
    }

    curframe->m_Flags &= 0xFFFFFFF7;
    curframe->m_PalOffset = 0;
    curframe->m_PaletteSize = (uint16_t)iffsize;

    return VQAERR_NONE;
}

int VQA_Load_CPLZ(VQAHandle *handle, unsigned iffsize)
{
    VQAFrameNode *curframe = handle->m_VQABuf->m_Loader.m_CurFrame;
    unsigned lcwoffset = handle->m_VQABuf->m_MaxPalSize - ((iffsize + 1) & 0xFFFE);

    if (handle->m_StreamHandler(handle, ACTION_READ, &curframe->m_Palette[lcwoffset], (iffsize + 1) & (~1))) {
        return VQAERR_READ;
    }

    curframe->m_Flags |= 8u;
    curframe->m_PalOffset = lcwoffset;
    curframe->m_PaletteSize = (uint16_t)iffsize;

    return VQAERR_NONE;
}

int VQA_Load_VPT0(VQAHandle *handle, unsigned iffsize)
{
    VQAFrameNode *curframe = handle->m_VQABuf->m_Loader.m_CurFrame;

    if (handle->m_StreamHandler(handle, ACTION_READ, curframe->m_Pointers, (iffsize + 1) & (~1))) {
        return VQAERR_READ;
    }

    curframe->m_Flags &= ~0x10;
    curframe->m_PtrOffset = 0;

    return VQAERR_NONE;
}

int VQA_Load_VPTZ(VQAHandle *handle, unsigned iffsize)
{
    VQAFrameNode *curframe = handle->m_VQABuf->m_Loader.m_CurFrame;
    unsigned lcwoffset = handle->m_VQABuf->m_MaxPtrSize - ((iffsize + 1) & (~1));

    if (handle->m_StreamHandler(handle, ACTION_READ, &curframe->m_Pointers[lcwoffset], (iffsize + 1) & (~1))) {
        return VQAERR_READ;
    }

    curframe->m_Flags |= 0x10;
    curframe->m_PtrOffset = lcwoffset;

    return VQAERR_NONE;
}

int VQA_Load_SND0(VQAHandle *handle, unsigned iffsize)
{
    VQAConfig *config = &handle->m_Config;
    VQAData *data = handle->m_VQABuf;
    VQAAudio *audio = &data->m_Audio;

    unsigned size_aligned = ((iffsize + 1) & (~1));

    if (!(config->m_OptionFlags & 1) || !audio->m_Buffer) {
        if (handle->m_StreamHandler(handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, size_aligned)) {
            return VQAERR_SEEK;
        }

        return VQAERR_NONE;
    }

    if (size_aligned <= audio->m_TempBufLen || audio->m_AudBufPos) {
        if (handle->m_StreamHandler(handle, ACTION_READ, audio->m_TempBuf, size_aligned)) {
            return VQAERR_READ;
        }

        audio->m_TempBufSize = iffsize;
        return VQAERR_NONE;
    }

    if (handle->m_StreamHandler(handle, ACTION_READ, audio->m_Buffer, size_aligned)) {
        return VQAERR_READ;
    }

    audio->m_AudBufPos += iffsize;

    for (unsigned i = 0; i < (iffsize / config->m_HMIBufSize); ++i) {
        audio->m_IsLoaded[i] = 1;
    }

    return VQAERR_NONE;
}

int VQA_Load_SND1(VQAHandle *handle, unsigned iffsize)
{
    VQASND1Header snd1hdr;
    VQAConfig *config = &handle->m_Config;
    VQAAudio *audio = &handle->m_VQABuf->m_Audio;

    int size_aligned = ((iffsize + 1) & 0xFFFE);

    if (!(config->m_OptionFlags & 1) || !audio->m_Buffer) {
        if (handle->m_StreamHandler(handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, size_aligned)) {
            return VQAERR_SEEK;
        }

        return VQAERR_NONE;
    }

    if (handle->m_StreamHandler(handle, ACTION_READ, &snd1hdr, sizeof(snd1hdr))) {
        return VQAERR_READ;
    }

    size_aligned -= sizeof(snd1hdr);

    if ((unsigned)snd1hdr.m_OutSize <= audio->m_TempBufLen || audio->m_AudBufPos > 0) {
        if (snd1hdr.m_OutSize == snd1hdr.m_Size) {
            if (handle->m_StreamHandler(handle, ACTION_READ, audio->m_TempBuf, size_aligned)) {
                return VQAERR_READ;
            }
        } else {
            void *decomp_buff = ((audio->m_TempBuf + audio->m_TempBufLen) - size_aligned);

            if (handle->m_StreamHandler(handle, ACTION_READ, decomp_buff, size_aligned)) {
                return VQAERR_READ;
            }

            Audio_Unzap(decomp_buff, audio->m_TempBuf, snd1hdr.m_OutSize);
        }

        audio->m_TempBufSize = snd1hdr.m_OutSize;

        return VQAERR_NONE;
    }

    if (snd1hdr.m_OutSize == snd1hdr.m_Size) {
        if (handle->m_StreamHandler(handle, ACTION_READ, audio->m_Buffer, size_aligned)) {
            return VQAERR_READ;
        }

    } else {
        void *decomp_buff = ((audio->m_Buffer + config->m_AudioBufSize) - size_aligned);

        if (handle->m_StreamHandler(handle, ACTION_READ, decomp_buff, size_aligned)) {
            return VQAERR_READ;
        }

        Audio_Unzap(decomp_buff, audio->m_Buffer, snd1hdr.m_OutSize);
    }

    audio->m_AudBufPos += snd1hdr.m_OutSize;

    for (int i = 0; i < (snd1hdr.m_OutSize / config->m_HMIBufSize); ++i) {
        audio->m_IsLoaded[i] = 1;
    }

    return VQAERR_NONE;
}

int VQA_Load_SND2(VQAHandle *handle, unsigned iffsize)
{
    VQAAudio *audio = &handle->m_VQABuf->m_Audio;
    VQAConfig *config = &handle->m_Config;

    unsigned size_aligned = ((iffsize + 1) & 0xFFFE);

    if (!(config->m_OptionFlags & 1) || !audio->m_Buffer) {
        if (handle->m_StreamHandler(handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, size_aligned)) {
            return VQAERR_SEEK;
        }

        return VQAERR_NONE;
    }

    unsigned decomp_size = size_aligned * (handle->m_VQABuf->m_Audio.m_BitsPerSample / 4);

    if (decomp_size <= handle->m_VQABuf->m_Audio.m_TempBufLen || handle->m_VQABuf->m_Audio.m_AudBufPos) {
        void *buffer = &audio->m_TempBuf[handle->m_VQABuf->m_Audio.m_TempBufLen - size_aligned];

        if (handle->m_StreamHandler(handle, ACTION_READ, buffer, size_aligned)) {
            return VQAERR_READ;
        }

        audio->m_AdcpmInfo.m_Source = buffer;
        audio->m_AdcpmInfo.m_Dest = audio->m_TempBuf;
        VQAADPCM_Decompress(&audio->m_AdcpmInfo, decomp_size);
        audio->m_TempBufSize = decomp_size;

        return VQAERR_NONE;
    }

    void *buffer = &audio->m_Buffer[config->m_AudioBufSize] - size_aligned;
    if (handle->m_StreamHandler(handle, ACTION_READ, buffer, size_aligned)) {
        return VQAERR_READ;
    }

    audio->m_AdcpmInfo.m_Source = buffer;
    audio->m_AdcpmInfo.m_Dest = audio->m_Buffer;
    VQAADPCM_Decompress(&audio->m_AdcpmInfo, decomp_size);
    audio->m_AudBufPos += decomp_size;

    for (unsigned i = 0; i < (decomp_size / config->m_HMIBufSize); ++i) {
        audio->m_IsLoaded[i] = 1;
    }

    return VQAERR_NONE;
}

int VQA_Load_VQF(VQAHandle *handle, unsigned frame_iffsize)
{
    VQAChunkHeader chunk;
    unsigned bytes_loaded = 0;
    VQAData *data = handle->m_VQABuf;
    VQAFrameNode *curframe = data->m_Loader.m_CurFrame;
    unsigned framesize = (frame_iffsize + 1) & (~1);
    VQADrawer *drawer = &handle->m_VQABuf->m_Drawer;

    while (bytes_loaded < framesize) {
        if (handle->m_StreamHandler(handle, ACTION_READ, &chunk, sizeof(chunk))) {
            return VQAERR_EOF;
        }

        unsigned iffsize = (chunk.m_Size << 0x18) & 0xFF000000 | (chunk.m_Size << 8) & 0xFF0000
            | (chunk.m_Size >> 8) & 0xFF00 | (chunk.m_Size >> 0x18);

        bytes_loaded += ((iffsize + 1) & (~1)) + 8;

        switch (chunk.m_ID) {
            // Vector Pointer Table
            case CHUNK_VPT0: {
                captainslog_debug("VQA_Load_VQF() - Found VPT0 chunk.\n");
                if (VQA_Load_VPT0(handle, iffsize)) {
                    return VQAERR_READ;
                }
                continue;
            }

            case CHUNK_VPTZ: {
                captainslog_debug("VQA_Load_VQF() - Found VPTZ chunk.\n");
                if (VQA_Load_VPTZ(handle, iffsize)) {
                    return VQAERR_READ;
                }
                continue;
            }

            // Vector Pointer
            case CHUNK_VPTD: {
                captainslog_debug("VQA_Load_VQF() - Found VPTD chunk.\n");
                if (VQA_Load_VPTZ(handle, iffsize)) {
                    return VQAERR_READ;
                }
                continue;
            }

            case CHUNK_VPTK: {
                captainslog_debug("VQA_Load_VQF() - Found VPTK chunk.\n");
                if (VQA_Load_VPTZ(handle, iffsize)) {
                    return VQAERR_READ;
                }
                curframe->m_Flags |= 2;
                continue;
            }
            case CHUNK_VPTR: {
                captainslog_debug("VQA_Load_VQF() - Found VPTR chunk.\n");
                if (VQA_Load_VPT0(handle, iffsize)) {
                    return VQAERR_READ;
                }
                continue;
            }

            case CHUNK_VPRZ: {
                captainslog_debug("VQA_Load_VQF() - Found VPRZ chunk.\n");
                if (VQA_Load_VPTZ(handle, iffsize)) {
                    return VQAERR_READ;
                }
                continue;
            }

            // BR
            // not proper code, both check and set flags
            case CHUNK_VPDZ: {
                captainslog_debug("VQA_Load_VQF() - Found VPDZ chunk.\n");
                if (VQA_Load_VPTZ(handle, iffsize)) {
                    return VQAERR_READ;
                }
                continue;
            }

            case CHUNK_VPKZ: {
                captainslog_debug("VQA_Load_VQF() - Found VPKZ chunk.\n");
                if (VQA_Load_VPTZ(handle, iffsize)) {
                    return VQAERR_READ;
                }
                continue;
            }
            //

            // CodeBook Full
            case CHUNK_CBF0: {
                captainslog_debug("VQA_Load_VQF() - Found CBF0 chunk.\n");
                if (VQA_Load_CBF0(handle, iffsize)) {
                    return VQAERR_READ;
                }
                continue;
            }

            case CHUNK_CBFZ: {
                captainslog_debug("VQA_Load_VQF() - Found CBFZ chunk.\n");
                if (VQA_Load_CBFZ(handle, iffsize)) {
                    return VQAERR_READ;
                }
                continue;
            }

            // CodeBook Partial
            case CHUNK_CBP0: {
                captainslog_debug("VQA_Load_VQF() - Found CBP0 chunk.\n");
                if (VQA_Load_CBP0(handle, iffsize)) {
                    return VQAERR_READ;
                }
                continue;
            }

            case CHUNK_CBPZ: {
                captainslog_debug("VQA_Load_VQF() - Found CBPZ chunk.\n");
                if (VQA_Load_CBPZ(handle, iffsize)) {
                    return VQAERR_READ;
                }
                continue;
            }

            // Color PaLette or Codebook PaLette
            case CHUNK_CPL0: {
                captainslog_debug("VQA_Load_VQF() - Found CPL0 chunk.\n");
                if (VQA_Load_CPL0(handle, iffsize)) {
                    return VQAERR_READ;
                }
                if (!drawer->m_CurPalSize) {
                    memcpy(drawer->m_Palette_24, curframe->m_Palette, curframe->m_PaletteSize);
                    drawer->m_CurPalSize = curframe->m_PaletteSize;
                }
                curframe->m_Flags |= 4;
                continue;
            }

            case CHUNK_CPLZ: {
                captainslog_debug("VQA_Load_VQF() - Found CPLZ chunk.\n");
                if (VQA_Load_CPLZ(handle, iffsize)) {
                    return VQAERR_READ;
                }
                if (!drawer->m_CurPalSize) {
                    drawer->m_CurPalSize =
                        LCW_Uncomp((curframe->m_PalOffset + curframe->m_Palette), drawer->m_Palette_24, data->m_MaxPalSize);
                }
                curframe->m_Flags |= 4;
                continue;
            }

            default: {
                continue;
            }
        }
    }

    return VQAERR_NONE;
}

int VQA_Open(VQAHandle *handle, const char *filename, VQAConfig *config)
{
    VQAHeader *header = &handle->m_Header;
    VQAChunkHeader chunk;

    if (handle->m_StreamHandler(handle, ACTION_OPEN, (char *)filename, 0)) {
        captainslog_debug("VQA_Open() - Failed to open '%s'.\n", filename);
        return VQAERR_OPEN;
    }

    if (handle->m_StreamHandler(handle, ACTION_READ, &chunk, sizeof(chunk))) {
        VQA_Close(handle);
        captainslog_debug("VQA_Open() - Failed to read FORM chunk header.\n");
        return VQAERR_READ;
    }

    if (chunk.m_ID != CHUNK_FORM || chunk.m_Size <= 0) {
        VQA_Close(handle);
        captainslog_debug("VQA_Open() - Invalid FORM chunk.\n");
        return VQAERR_NOTVQA;
    }

    captainslog_debug("VQA_Open() - FORM chunk parsed correctly.\n");

    if (handle->m_StreamHandler(handle, ACTION_READ, &chunk.m_ID, sizeof(chunk.m_ID))) {
        VQA_Close(handle);
        captainslog_debug("VQA_Open() - Failed to read WVQA chunk id.\n");
        return VQAERR_READ;
    }

    if (chunk.m_ID != CHUNK_WVQA) {
        VQA_Close(handle);
        captainslog_debug("VQA_Open() - Invalid WVQA chunk marker.\n");
        return VQAERR_NOTVQA;
    }

    captainslog_debug("VQA_Open() - WVQA chunk parsed correctly.\n");

    captainslog_debug("VQA_Open() - About to alloc VQAConfig info.\n");

    if (config != nullptr) {
        memcpy(&handle->m_Config, config, sizeof(*config));
    } else {
        VQA_DefaultConfig(&handle->m_Config);
    }

    captainslog_debug("VQA_Open() - About to enter main chunk reading loop.\n");

    BOOL frame_info_found = false;

    while (!frame_info_found) {
        if (handle->m_StreamHandler(handle, ACTION_READ, &chunk, sizeof(chunk))) {
            VQA_Close(handle);
            captainslog_debug("VQA_Open() - Failed to read next chunk.\n");
            return VQAERR_READ;
        }

        captainslog_debug("VQA_Open() - chunk.m_ID is %04s.\n", &chunk.m_ID);
        captainslog_debug("VQA_Open() - chunk.m_Size is %d.\n", &chunk.m_Size);

        int chunk_size = be32toh(chunk.m_Size);

        switch (chunk.m_ID) {
            case CHUNK_VQHD:
                captainslog_debug("VQA_Open() - Found VQHD chunk.\n");

                if (chunk_size != sizeof(VQAHeader)) {
                    VQA_Close(handle);
                    captainslog_debug("VQA_Open() - VQHD (VQAHeader) size mismatch!\n");
                    return VQAERR_NOTVQA;
                }

                captainslog_debug("VQA_Open() - About to read VQHD (VQAHeader) struct from file.\n");

                if (handle->m_StreamHandler(handle, ACTION_READ, header, sizeof(*header))) {
                    VQA_Close(handle);
                    captainslog_debug("VQA_Open() - Failed to read VQHD (VQAHeader) from file!\n");
                    return VQAERR_READ;
                }

                // HHAAAAAAAAAAXXXXX!
                // in LOLG VQAs Groupsize is 0 because it only has one codebook chunk so forcing it to common default here,
                // allows LOLG VQAs to be played
                if (header->m_Groupsize == 0) {
                    header->m_Groupsize = 8;
                }

                if (config->m_ImageWidth == -1) {
                    config->m_ImageWidth = header->m_ImageWidth;
                }

                if (config->m_ImageHeight == -1) {
                    config->m_ImageHeight = header->m_ImageHeight;
                }

                if (handle->m_Config.m_FrameRate == -1) {
                    handle->m_Config.m_FrameRate = header->m_FPS;
                }

                if (handle->m_Config.m_DrawRate == -1) {
                    handle->m_Config.m_DrawRate = header->m_FPS;
                }

                if (handle->m_Config.m_DrawRate == -1 || !handle->m_Config.m_DrawRate) {
                    handle->m_Config.m_DrawRate = header->m_FPS;
                }

                if (header->m_Version >= 2 && !(header->m_Flags & 2)) {
                    config->m_OptionFlags &= 0xBF;
                }

                captainslog_debug("VQA_Open() - About to call VQA_AllocBuffers().\n");

                handle->m_VQABuf = VQA_AllocBuffers(header, &handle->m_Config);
                if (handle->m_VQABuf == nullptr) {
                    VQA_Close(handle);
                    captainslog_debug("VQA_Open() - VQA_AllocBuffers() failed to allocate data!\n");
                    return VQAERR_NOMEM;
                }

                captainslog_debug("VQA_Open() - VQHD (VQAHeader) struct parsed correctly.\n");

                break;

            // TODO: Needs confirming with a 'Poly VQA file.
            case CHUNK_NAME: {
                captainslog_debug("VQA_Open() - Found NAME chunk.\n");
                if (handle->m_StreamHandler(handle, ACTION_READ, &chunk, sizeof(VQAChunkHeader))) {
                    VQA_Close(handle);
                    captainslog_debug("VQA_Open() - Failed to read NAME chunk header!\n");
                    return VQAERR_READ;
                }

                char *buffer = new char[chunk_size + 1];
                memset(buffer, 0, chunk_size + 1);

                if (handle->m_StreamHandler(handle, ACTION_READ, buffer, chunk_size)) {
                    VQA_Close(handle);
                    captainslog_debug("VQA_Open() - Failed to read NAME string!\n");
                    return VQAERR_READ;
                }

                captainslog_debug("VQA_Open() - %s.\n", buffer);

                captainslog_debug("VQA_Open() - NAME chunk parsed correctly.\n");

                break;
            }
            case CHUNK_EVA0:
                captainslog_debug("VQA_Open() - Found EVA0 chunk.\n");

                if (config->m_EVAFont && config->m_OptionFlags & VQA_OPTION_128) {
                    // TODO
                }

                captainslog_debug("VQA_Open() - EVA0 chunk parsed correctly.\n");

                break;

            case CHUNK_CAP0:
                captainslog_debug("VQA_Open() - Found CAP0 chunk.\n");

                if (config->m_CapFont && config->m_OptionFlags & VQA_OPTION_SOUND_ENABLED) {
                    short v78 = 0;
                    if (handle->m_StreamHandler(handle, ACTION_READ, &v78, sizeof(v78))) {
                        VQA_Close(handle);
                        return VQAERR_READ;
                    }

                    if (handle->m_StreamHandler(handle, ACTION_READ, &v78, sizeof(v78))) {
                        VQA_Close(handle);
                        return VQAERR_NOMEM;
                    }

                    // TODO

                    // LCW_Uncomp();

                    // handle->field_C2 = VQA_OpenCaptions(? , config->CapFont);
                    if (handle->field_C2) {
                        VQA_Close(handle);
                        return VQAERR_NOMEM;
                    }
                }

                captainslog_debug("VQA_Open() - CAP0 chunk parsed correctly.\n");

                break;

            case CHUNK_FINF:
                captainslog_debug("VQA_Open() - Found FINF chunk.\n");

                if (VQA_Load_FINF(handle, chunk_size)) {
                    VQA_Close(handle);
                    captainslog_debug("VQA_Open() - Failed to load FINF chunk!\n");
                    return VQAERR_READ;
                }

                captainslog_debug("VQA_Open() - FINF chunk parsed correctly.\n");
                frame_info_found = true;
                break;

            default:
                captainslog_debug("VQA_Open() - Seeking over unsupported chunk '%04s'.\n", &chunk.m_ID);

                if (handle->m_StreamHandler(handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, (chunk_size + 1) & ~1)) {
                    VQA_Close(handle);
                    captainslog_debug("VQA_Open() - Failed to seek forward to next chunk!\n");
                    return VQAERR_SEEK;
                }

                break;
        };
    }

    if (frame_info_found) {
        captainslog_debug("VQA_Open() - Found FINF, preparing audio.\n");

        if (!(header->m_Flags & 1)) {
            handle->m_Config.m_OptionFlags &= VQA_OPTION_SOUND_ENABLED;
        }

        if (handle->m_Config.m_OptionFlags & VQA_OPTION_SOUND_ENABLED) {
#ifdef BUILD_WITH_DSOUND
            if (VQA_OpenAudio(handle, (void *)g_MainWindow)) {
#else
            if (VQA_OpenAudio(handle, nullptr)) {
#endif
                VQA_Close(handle);
                captainslog_debug("VQA_Open() - VQA_OpenAudio() failed!\n");
                return VQAERR_AUDIO;
            }

            VQAAudio *audio = &handle->m_VQABuf->m_Audio;
            captainslog_debug("VQA_Open() - About to init sos audio codec.\n");
            VQAADPCM_Stream_Init(&audio->m_AdcpmInfo);

            if (header->m_Version >= 2) {
                audio->m_AdcpmInfo.m_BitsPerSample = audio->m_BitsPerSample;
                audio->m_AdcpmInfo.m_UnCompSize = audio->m_Channels * (audio->m_BitsPerSample / 8) * audio->m_SampleRate
                    / header->m_FPS * header->m_Frames;
                audio->m_AdcpmInfo.m_Channels = audio->m_Channels;
            } else {
                audio->m_AdcpmInfo.m_BitsPerSample = 8;
                audio->m_AdcpmInfo.m_UnCompSize = 22050 / header->m_FPS * header->m_Frames;
                audio->m_AdcpmInfo.m_Channels = 1;
            }

            audio->m_AdcpmInfo.m_CompSize = audio->m_AdcpmInfo.m_UnCompSize / audio->m_AdcpmInfo.m_BitsPerSample / 4;

            captainslog_debug("VQA_Open() - m_AdcpmInfo.m_BitsPerSample = %d\n", audio->m_AdcpmInfo.m_BitsPerSample);
            captainslog_debug("VQA_Open() - m_AdcpmInfo.m_UnCompSize = %d\n", audio->m_AdcpmInfo.m_UnCompSize);
            captainslog_debug("VQA_Open() - m_AdcpmInfo.m_Channels = %d\n", audio->m_AdcpmInfo.m_Channels);
            captainslog_debug("VQA_Open() - m_AdcpmInfo.m_CompSize = %d\n", audio->m_AdcpmInfo.m_CompSize);
        }

        if ((!(config->m_OptionFlags & 1) || config->m_TimerMethod == 2)) {
            if (VQA_StartTimerInt(handle, config->m_OptionFlags & 0x20)) {
                VQA_Close(handle);
                captainslog_debug("VQA_Open() - VQA_PrimeBuffers() failed!\n");
                return VQAERR_AUDIO;
            }
        }

        if (VQA_PrimeBuffers(handle)) {
            VQA_Close(handle);
            captainslog_debug("VQA_Open() - VQA_PrimeBuffers() failed!\n");
            return VQAERR_READ;
        }
    }

    return VQAERR_NONE;
}

void VQA_Close(VQAHandle *handle)
{
    if (handle->m_Config.m_OptionFlags & 1) {
        VQA_CloseAudio(handle);
    } else {
        VQA_StopTimerInt(handle);
    }

    if (handle->m_VQABuf) {
        VQA_FreeBuffers(handle->m_VQABuf, &handle->m_Config, &handle->m_Header);
    }

    handle->m_StreamHandler(handle, ACTION_CLOSE, nullptr, 0);
    memset(handle, 0, sizeof(VQAHandle));
}

int VQA_LoadFrame(VQAHandle *handle)
{
    unsigned iffsize;
    BOOL frame_loaded = false;
    VQAData *data = handle->m_VQABuf;
    VQALoader *loader = &data->m_Loader;
    VQADrawer *drawer = &handle->m_VQABuf->m_Drawer;
    VQAFrameNode *curframe = data->m_Loader.m_CurFrame;
    VQAChunkHeader chunk = data->m_Chunk;

    if (handle->m_Header.m_Frames <= data->m_Loader.m_CurFrameNum) {
        return VQAERR_EOF;
    }

    if (curframe->m_Flags & 1) {
        ++data->m_Loader.m_WaitsOnDrawer;
        return VQAERR_NOBUFFER;
    }

    if (!(data->m_Flags & 4)) {
        frame_loaded = false;
        data->m_Loader.m_FrameSize = 0;
        curframe->m_Codebook = loader->m_FullCB;
    }

    while (!frame_loaded) {
        if (!(data->m_Flags & 4)) {
            if (handle->m_StreamHandler(handle, ACTION_READ, &chunk, sizeof(VQAChunkHeader))) {
                return VQAERR_EOF;
            }

            iffsize = (chunk.m_Size << 24) & 0xFF000000 | (chunk.m_Size << 8) & 0xFF0000 | (chunk.m_Size >> 8) & 0xFF00
                | (chunk.m_Size >> 24);
            loader->m_FrameSize += iffsize;
        }

        // Sorted by how its commonly in the VQA's
        switch (chunk.m_ID) {
            case CHUNK_FINF:
                captainslog_debug("VQA_LoadFrame() - Found FINF chunk.\n");

                if (VQA_Load_FINF(handle, iffsize)) {
                    return VQAERR_READ;
                }

                continue;

            case CHUNK_VQFK:
                captainslog_debug("VQA_LoadFrame() - Found VQFK chunk.\n");

                if (VQA_Load_VQF(handle, iffsize)) {
                    return VQAERR_READ;
                }

                curframe->m_Flags |= 2;
                frame_loaded = true;
                continue;

            case CHUNK_VQFR:
                captainslog_debug("VQA_LoadFrame() - Found VQFR chunk.\n");

                if (VQA_Load_VQF(handle, iffsize)) {
                    return VQAERR_READ;
                }

                frame_loaded = true;
                continue;

            // Version 3 chunk
            // Vector Quantized Frame Loop???? is in BR, appears to call Load_VQF
            // Not the proper code as there's a few checks and flags it sets
            case CHUNK_VQFL:
                captainslog_debug("VQA_LoadFrame() - Found VQFL chunk.\n");

                if (VQA_Load_VQF(handle, iffsize)) {
                    return VQAERR_READ;
                }

                frame_loaded = true;
                continue;

            // Vector Pointer Table
            case CHUNK_VPT0:
                captainslog_debug("VQA_LoadFrame() - Found VPT0 chunk.\n");

                if (VQA_Load_VPT0(handle, iffsize)) {
                    return VQAERR_READ;
                }

                frame_loaded = true;
                continue;

            case CHUNK_VPTZ:
                captainslog_debug("VQA_LoadFrame() - Found VPTZ chunk.\n");

                if (VQA_Load_VPTZ(handle, iffsize)) {
                    return VQAERR_READ;
                }

                frame_loaded = true;
                continue;

            // Vector Pointer
            case CHUNK_VPTD:
                captainslog_debug("VQA_LoadFrame() - Found VPTD chunk.\n");

                if (VQA_Load_VPTZ(handle, iffsize)) {
                    return VQAERR_READ;
                }

                frame_loaded = true;
                continue;

            case CHUNK_VPTK:
                captainslog_debug("VQA_LoadFrame() - Found VPTK chunk.\n");

                if (VQA_Load_VPTZ(handle, iffsize)) {
                    return VQAERR_READ;
                }

                curframe->m_Flags |= 2;
                frame_loaded = true;
                continue;

            case CHUNK_VPTR:
                captainslog_debug("VQA_LoadFrame() - Found VPTR chunk.\n");

                if (VQA_Load_VPT0(handle, iffsize)) {
                    return VQAERR_READ;
                }

                frame_loaded = true;
                continue;

            case CHUNK_VPRZ:
                captainslog_debug("VQA_LoadFrame() - Found VPRZ chunk.\n");

                if (VQA_Load_VPTZ(handle, iffsize)) {
                    return VQAERR_READ;
                }

                frame_loaded = true;
                continue;

            // CodeBook Full
            case CHUNK_CBF0:
                captainslog_debug("VQA_LoadFrame() - Found CBF0 chunk.\n");

                if (VQA_Load_CBF0(handle, iffsize)) {
                    return VQAERR_READ;
                }

                continue;

            case CHUNK_CBFZ:
                captainslog_debug("VQA_LoadFrame() - Found CBFZ chunk.\n");

                if (VQA_Load_CBFZ(handle, iffsize)) {
                    return VQAERR_READ;
                }

                continue;

            // CodeBook Partial
            case CHUNK_CBP0:
                captainslog_debug("VQA_LoadFrame() - Found CBP0 chunk.\n");

                if (VQA_Load_CBP0(handle, iffsize)) {
                    return VQAERR_READ;
                }

                continue;

            case CHUNK_CBPZ:
                captainslog_debug("VQA_LoadFrame() - Found CBPZ chunk.\n");

                if (VQA_Load_CBPZ(handle, iffsize)) {
                    return VQAERR_READ;
                }

                continue;

            // Color PaLette or Codebook PaLette
            case CHUNK_CPL0:
                captainslog_debug("VQA_LoadFrame() - Found CPL0 chunk.\n");

                if (VQA_Load_CPL0(handle, iffsize)) {
                    return VQAERR_READ;
                }

                if (!drawer->m_CurPalSize) {
                    memcpy(drawer->m_Palette_24, curframe->m_Palette, curframe->m_PaletteSize);
                    drawer->m_CurPalSize = curframe->m_PaletteSize;
                }

                curframe->m_Flags |= 4;

                continue;

            case CHUNK_CPLZ:
                captainslog_debug("VQA_LoadFrame() - Found CPLZ chunk.\n");

                if (VQA_Load_CPLZ(handle, iffsize)) {
                    return VQAERR_READ;
                }

                if (!drawer->m_CurPalSize) {
                    drawer->m_CurPalSize =
                        LCW_Uncomp((curframe->m_PalOffset + curframe->m_Palette), drawer->m_Palette_24, data->m_MaxPalSize);
                }

                curframe->m_Flags |= 4;

                continue;

            case CHUNK_SND0:
                captainslog_debug("VQA_LoadFrame() - Found SND0 chunk.\n");

                if (handle->m_Config.m_OptionFlags & 0x40) {
                    if (handle->m_StreamHandler(handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, (iffsize + 1) & (~1))) {
                        return VQAERR_SEEK;
                    }

                } else {
                    if (VQA_CopyAudio(handle) == VQAERR_SLEEPING) {
                        handle->m_VQABuf->m_Flags |= 4;
                        return VQAERR_SLEEPING;
                    }

                    handle->m_VQABuf->m_Flags &= 0xFB;
                    if (VQA_Load_SND0(handle, iffsize)) {
                        return VQAERR_READ;
                    }
                }

                continue;

            case CHUNK_SNA0:
                captainslog_debug("VQA_LoadFrame() - Found SNA0 chunk.\n");

                if (handle->m_Config.m_OptionFlags & 0x40) {
                    if (VQA_CopyAudio(handle) == VQAERR_SLEEPING) {
                        handle->m_VQABuf->m_Flags |= 4;
                        return VQAERR_SLEEPING;
                    }

                    handle->m_VQABuf->m_Flags &= 0xFB;

                    if (VQA_Load_SND0(handle, iffsize)) {
                        return VQAERR_READ;
                    }

                } else {
                    if (handle->m_StreamHandler(handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, (iffsize + 1) & (~1))) {
                        return VQAERR_SEEK;
                    }
                }

                continue;

            case CHUNK_SND1:
                captainslog_debug("VQA_LoadFrame() - Found SND1 chunk.\n");

                if (handle->m_Config.m_OptionFlags & 0x40) {
                    if (handle->m_StreamHandler(handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, (iffsize + 1) & (~1))) {
                        return VQAERR_SEEK;
                    }
                } else {
                    if (VQA_CopyAudio(handle) == VQAERR_SLEEPING) {
                        handle->m_VQABuf->m_Flags |= 4;
                        return VQAERR_SLEEPING;
                    }

                    handle->m_VQABuf->m_Flags &= 0xFB;

                    if (VQA_Load_SND1(handle, iffsize)) {
                        return VQAERR_READ;
                    }
                }

                continue;

            case CHUNK_SNA1:
                captainslog_debug("VQA_LoadFrame() - Found SNA1 chunk.\n");

                if (handle->m_Config.m_OptionFlags & 0x40) {
                    if (VQA_CopyAudio(handle) == VQAERR_SLEEPING) {
                        handle->m_VQABuf->m_Flags |= 4;
                        return VQAERR_SLEEPING;
                    }

                    handle->m_VQABuf->m_Flags &= 0xFB;

                    if (VQA_Load_SND1(handle, iffsize)) {
                        return VQAERR_READ;
                    }
                } else {
                    if (handle->m_StreamHandler(handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, (iffsize + 1) & (~1))) {
                        return VQAERR_SEEK;
                    }
                }

                continue;

            case CHUNK_SND2:
                captainslog_debug("VQA_LoadFrame() - Found SND2 chunk.\n");

                if (handle->m_Config.m_OptionFlags & 0x40) {
                    if (handle->m_StreamHandler(handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, (iffsize + 1) & (~1))) {
                        return VQAERR_SEEK;
                    }
                } else {
                    if (VQA_CopyAudio(handle) == VQAERR_SLEEPING) {
                        handle->m_VQABuf->m_Flags |= 4;
                        return VQAERR_SLEEPING;
                    }

                    handle->m_VQABuf->m_Flags &= 0xFB;

                    if (VQA_Load_SND2(handle, iffsize)) {
                        return VQAERR_READ;
                    }
                }

                continue;

            case CHUNK_SNA2:
                captainslog_debug("VQA_LoadFrame() - Found SNA2 chunk.\n");

                if (handle->m_Config.m_OptionFlags & 0x40) {
                    if (VQA_CopyAudio(handle) == VQAERR_SLEEPING) {
                        handle->m_VQABuf->m_Flags |= 4;
                        return VQAERR_SLEEPING;
                    }

                    handle->m_VQABuf->m_Flags &= 0xFB;

                    if (VQA_Load_SND2(handle, iffsize)) {
                        return VQAERR_READ;
                    }
                } else if (handle->m_StreamHandler(
                               handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, (iffsize + 1) & (~1))) {
                    return VQAERR_SEEK;
                }

                continue;

            case CHUNK_SN2J:
                captainslog_debug("VQA_LoadFrame() - Found SN2J chunk.\n");
                return VQAERR_SEEK;

            default:
                captainslog_debug("VQA_LoadFrame Read Unsupported Chunk %s\n", &chunk.m_ID);
                break;
        }
    }

    if (handle->m_StreamHandler(handle, ACTION_SEEK, (void *)FS_SEEK_CURRENT, (iffsize + 1) & (~1))) {
        return VQAERR_SEEK;
    }

    if (loader->m_CurFrameNum > 0 && loader->m_FrameSize > loader->m_MaxFrameSize) {
        loader->m_MaxFrameSize = loader->m_FrameSize;
    }

    curframe->m_FrameNum = loader->m_CurFrameNum++;
    loader->m_LastFrameNum = loader->m_CurFrameNum;
    curframe->m_Flags |= 1;
    loader->m_CurFrame = curframe->m_Next;

    return VQAERR_NONE;
}

int VQA_SeekFrame(VQAHandle *handle, int framenum, int fromwhere)
{
    VQAErrorType rc = VQAERR_EOF;
    VQAData *data = handle->m_VQABuf;
    VQAAudio *audio = &data->m_Audio;

    if (audio->m_Flags & 0x40) {
        VQA_StopAudio(handle);
    }

    if (handle->m_Header.m_Frames <= framenum) {
        return rc;
    }

    if (!data->m_Foff) {
        return rc;
    }

    int group = framenum / handle->m_Header.m_Groupsize * handle->m_Header.m_Groupsize;
    if (group >= handle->m_Header.m_Groupsize) {
        group -= handle->m_Header.m_Groupsize;
    }

    if (handle->m_StreamHandler(handle, ACTION_SEEK, nullptr, 2 * ((data->m_Foff[group]) & 0xFFFFFFF))) {
        return VQAERR_SEEK;
    }

    data->m_Loader.m_NumPartialCB = 0;
    data->m_Loader.m_PartialCBSize = 0;
    data->m_Loader.m_FullCB = data->m_CBData;
    data->m_Loader.m_CurCB = data->m_CBData;
    data->m_Loader.m_CurFrameNum = group;

    for (int i = 0; i < framenum - group; ++i) {
        data->m_Loader.m_CurFrame->m_Flags = 0;
        rc = (VQAErrorType)VQA_LoadFrame(handle);

        if (rc) {
            if (rc != VQAERR_NOBUFFER && rc != VQAERR_SLEEPING) {
                rc = VQAERR_EOF;
                break;
            }

            rc = VQAERR_NONE;
        }
    }

    if (rc) {
        return rc;
    }

    data->m_Loader.m_CurFrame->m_Flags = 0;

    for (VQAFrameNode *frame = data->m_Loader.m_CurFrame->m_Next; frame != data->m_Loader.m_CurFrame;
         frame = frame->m_Next) {
        frame->m_Flags = 0;
    }

    data->m_Drawer.m_CurFrame = data->m_Loader.m_CurFrame;

    if (VQA_PrimeBuffers(handle)) {
        rc = VQAERR_EOF;
    } else {
        rc = (VQAErrorType)framenum;
    }

    if (audio->m_Flags & 0x40) {
        VQA_StartAudio(handle);
    }

    return rc;
}

VQAData *VQA_AllocBuffers(VQAHeader *header, VQAConfig *config)
{
    VQACBNode *cbnode;
    VQAFrameNode *framenode;
    VQAFrameNode *last_framenode;
    VQACBNode *last_cbnode;

    if (config->m_NumCBBufs <= 0 || config->m_NumFrameBufs <= 0 /*|| config->AudioBufSize < config->HMIBufSize*/) {
        return 0;
    }

    captainslog_debug("VQA_AllocBuffers() - About to create VQAData struct.\n");
    VQAData *data = (VQAData *)malloc(sizeof(VQAData));

    if (data == nullptr) {
        return nullptr;
    }

    memset(data, 0, sizeof(VQAData));
    data->m_MemUsed = sizeof(VQAData);
    data->m_Drawer.m_LastTime = -60;
    data->m_MaxCBSize = (header->m_BlockHeight * header->m_BlockWidth * header->m_CBentries + 250) & 0xFFFC;
    data->m_MaxPalSize = 1792;
    data->m_MaxPtrSize =
        (2 * (header->m_ImageHeight / header->m_BlockHeight) * (header->m_ImageWidth / header->m_BlockWidth) + 1024)
        & 0xFFFC;
    data->m_Loader.m_LastCBFrame = header->m_Groupsize * ((header->m_Frames - 1) / header->m_Groupsize);

    // BR code
    char color_mode = header->m_ColorMode;

    if (color_mode == 1 || color_mode == 4) {
        data->m_MaxCBSize *= 2;
    }

    // Make a linked list of nodes.
    for (int index = 0; index < config->m_NumCBBufs; ++index) {
        captainslog_debug("VQA_AllocBuffers() - About to create VQACBNode struct %d.\n", index);
        cbnode = (VQACBNode *)malloc(data->m_MaxCBSize + sizeof(VQACBNode));

        if (cbnode == nullptr) {
            VQA_FreeBuffers(data, config, header);
            return nullptr;
        }

        data->m_MemUsed += data->m_MaxCBSize + sizeof(VQACBNode);
        memset(cbnode, 0, sizeof(*cbnode));

        // Set buffer to be the allocated memory in excess of the node struct size.
        cbnode->m_Buffer = reinterpret_cast<uint8_t *>(&cbnode[1]);

        if (index > 0) {
            last_cbnode->m_Next = cbnode;
            last_cbnode = cbnode;
        } else {
            data->m_CBData = cbnode;
            last_cbnode = cbnode;
        }
    }

    // Make a looping list by linking last to first.
    cbnode->m_Next = data->m_CBData;
    data->m_Loader.m_CurCB = data->m_CBData;
    data->m_Loader.m_FullCB = data->m_CBData;

    // Make linked list of framenodes.
    for (int index = 0; index < config->m_NumFrameBufs; ++index) {
        captainslog_debug("VQA_AllocBuffers() - About to create VQAFrameNode struct %d.\n", index);

        framenode = (VQAFrameNode *)malloc(data->m_MaxPalSize + data->m_MaxPtrSize + sizeof(VQAFrameNode));

        if (!framenode) {
            VQA_FreeBuffers(data, config, header);
            return 0;
        }

        data->m_MemUsed += data->m_MaxPalSize + data->m_MaxPtrSize + sizeof(VQAFrameNode);
        memset(framenode, 0, sizeof(VQAFrameNode));
        framenode->m_Pointers = reinterpret_cast<uint8_t *>(&framenode[1]);
        // framenode->m_Palette = &framenode->m_Pointers[1] + data->m_MaxPtrSize; if above &v3[1].m_Pointers;      // is
        // reinterpret_cast<uint8_t*>(&framenode[1]); // then i guess bottom one should too?
        framenode->m_Palette = reinterpret_cast<uint8_t *>(&framenode[1]) + data->m_MaxPtrSize;
        framenode->m_Codebook = data->m_CBData;

        if (index > 0) {
            last_framenode->m_Next = framenode;
            last_framenode = framenode;
        } else {
            data->m_FrameData = framenode;
            last_framenode = framenode;
        }
    }

    framenode->m_Next = data->m_FrameData;
    data->m_Loader.m_CurFrame = data->m_FrameData;
    data->m_Drawer.m_CurFrame = data->m_FrameData;
    data->m_Flipper.m_CurFrame = data->m_FrameData;

    if (config->m_ImageBuf != nullptr) {
        data->m_Drawer.m_ImageBuf = config->m_ImageBuf;
        data->m_Drawer.m_ImageWidth = config->m_ImageWidth;
        data->m_Drawer.m_ImageHeight = config->m_ImageHeight;

    } else if (config->m_DrawFlags & 1) {
        data->m_Drawer.m_ImageBuf = (uint8_t *)malloc(header->m_ImageHeight * header->m_ImageWidth);

        if (data->m_Drawer.m_ImageBuf == nullptr) {
            VQA_FreeBuffers(data, config, header);
            return nullptr;
        }

        data->m_Drawer.m_ImageWidth = header->m_ImageWidth;
        data->m_Drawer.m_ImageHeight = header->m_ImageHeight;
        data->m_MemUsed += header->m_ImageHeight * header->m_ImageWidth; //!!this is!!

    } else {
        data->m_Drawer.m_ImageWidth = config->m_ImageWidth;
        data->m_Drawer.m_ImageHeight = config->m_ImageHeight;
    }

    if (header->m_Flags & 1) {
        if (config->m_OptionFlags & 1) {
            captainslog_debug("VQA_AllocBuffers() - About to setup audio buffers and info.\n");

            VQAAudio *audio = &data->m_Audio;

            if (header->m_Version >= 2) {
                int bitspersample;

                if (config->m_OptionFlags & 0x40 && header->m_Flags & 2) {
                    audio->m_SampleRate = header->m_AltSampleRate;
                    audio->m_Channels = header->m_AltChannels;
                    bitspersample = header->m_AltBitsPerSample;
                } else {
                    audio->m_SampleRate = header->m_SampleRate;
                    audio->m_Channels = header->m_Channels;
                    bitspersample = header->m_BitsPerSample;
                }

                audio->m_BitsPerSample = bitspersample;
                audio->field_38 = (bitspersample / 8) * header->m_SampleRate * header->m_Channels;

            } else {
                audio->m_SampleRate = 22050;
                audio->m_Channels = 1;
                audio->m_BitsPerSample = 8;
                audio->field_38 = 22050;
            }

            if (config->m_AudioBufSize == -1) {
                config->m_AudioBufSize =
                    (audio->field_38 + (audio->field_38 / 2)) / config->m_HMIBufSize * config->m_HMIBufSize;
            }

            if (config->m_AudioBufSize <= 0) {
                VQA_FreeBuffers(data, config, header);
                return nullptr;
            }

            if (config->m_AudioBufSize > 0) {
                if (config->m_AudioBuf != nullptr) {
                    audio->m_Buffer = config->m_AudioBuf;

                } else {
                    audio->m_Buffer = (uint8_t *)malloc(config->m_AudioBufSize);
                    if (audio->m_Buffer == nullptr) {
                        VQA_FreeBuffers(data, config, header);
                        return nullptr;
                    }

                    data->m_MemUsed += config->m_AudioBufSize;
                }

                audio->m_NumAudBlocks = config->m_AudioBufSize / config->m_HMIBufSize;
                audio->m_IsLoaded = (short *)malloc(2 * config->m_AudioBufSize / config->m_HMIBufSize);

                if (audio->m_IsLoaded == nullptr) {
                    VQA_FreeBuffers(data, config, header);
                    return nullptr;
                }

                data->m_MemUsed += audio->m_NumAudBlocks * 2;

                memset(audio->m_IsLoaded, 0, audio->m_NumAudBlocks * 2);

                audio->m_TempBufLen = 2 * (audio->field_38 / header->m_FPS) + 100;
                audio->m_TempBuf = (uint8_t *)malloc(2 * (audio->field_38 / header->m_FPS) + 100);

                if (audio->m_TempBuf == nullptr) {
                    VQA_FreeBuffers(data, config, header);
                    return nullptr;
                }

                data->m_MemUsed += audio->m_TempBufLen;
            }
        }
    }

    captainslog_debug("VQA_AllocBuffers() - About to alloc Foff.\n");

    // This looks like Foff is an array of ints
    data->m_Foff = (int *)malloc(header->m_Frames * 4);

    if (data->m_Foff == nullptr) {
        VQA_FreeBuffers(data, config, header);
        return nullptr;
    }

    data->m_MemUsed += header->m_Frames * 4;

    return data;
}

void VQA_FreeBuffers(VQAData *data, VQAConfig *config, VQAHeader *header)
{
    if (data->m_Foff != nullptr) {
        free(data->m_Foff);
    }

    if (config->m_AudioBuf == nullptr) {
        if (data->m_Audio.m_Buffer != nullptr) {
            free(data->m_Audio.m_Buffer);
        }
    }

    if (data->m_Audio.m_IsLoaded != nullptr) {
        free(data->m_Audio.m_IsLoaded);
    }

    if (data->m_Audio.m_TempBuf != nullptr) {
        free(data->m_Audio.m_TempBuf);
    }

    if (config->m_ImageBuf == nullptr) {
        if (data->m_Drawer.m_ImageBuf != nullptr) {
            free(data->m_Drawer.m_ImageBuf);
        }
    }

    // Free frame nodes.
    VQAFrameNode *frame_this = data->m_FrameData;

    for (int index = 0; index < config->m_NumFrameBufs && frame_this; ++index) {
        VQAFrameNode *frame_next = frame_this->m_Next;
        free(frame_this);
        frame_this = frame_next;
    }

    // Free codebook nodes.
    VQACBNode *cb_this = data->m_CBData;

    for (int index = 0; index < config->m_NumCBBufs && cb_this; ++index) {
        VQACBNode *cb_next = cb_this->m_Next;
        free(cb_this);
        cb_this = cb_next;
    }

    // Free vqa data.
    free(data);
}

int VQA_PrimeBuffers(VQAHandle *handle)
{
    VQAData *data = handle->m_VQABuf;

    for (int index = 0; index < handle->m_Config.m_NumFrameBufs; ++index) {
        VQAErrorType result = (VQAErrorType)VQA_LoadFrame(handle);

        if (result) {
            if (result != VQAERR_NOBUFFER && result != VQAERR_SLEEPING) {
                return result;
            }
        } else {
            ++data->m_LoadedFrames;
        }
    }

    return VQAERR_NONE;
}
