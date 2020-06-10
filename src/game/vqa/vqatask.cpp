#include "vqatask.h"
#include "keyboard.h"
#include "vqaaudio.h"
#include "vqaconfig.h"
#include "vqadrawer.h"
#include "vqafile.h"
#include "vqaloader.h"

#ifndef GAME_DLL
BOOL VQAMovieDone = FALSE;
#endif

VQAHandle *VQA_Alloc(void)
{
    VQAHandle *handle = (VQAHandle *)malloc(sizeof(VQAHandle));

    if (handle) {
        memset(handle, 0, sizeof(VQAHandle));
    }

    return handle;
}

void VQA_Free(void *block)
{
    if (block) {
        free(block);
    }
}

void VQA_SetStreamHandler(VQAHandle *handle, StreamHandlerFuncPtr streamhandler)
{
    handle->m_StreamHandler = (StreamHandlerFuncPtr)streamhandler;
}

// Sets Stream Handler function, Poly has 2, Direct Disk to stream and a disk to memory to stream function
void VQA_Init(VQAHandle *handle, StreamHandlerFuncPtr streamhandler)
{
    VQA_SetStreamHandler(handle, streamhandler);
}

// From RA
void VQA_Reset(VQAHandle *handle)
{
    VQAData *data = handle->m_VQABuf;
    data->m_Flags = VQA_DATA_FLAG_ZERO;
    data->m_LoadedFrames = 0;
    data->m_DrawnFrames = 0;
    data->m_StartTime = 0;
    data->m_EndTime = 0;
}

// VQAPlayMode
// 0 = play
// 1 =
// 2 =
// 3 =
VQAErrorType VQA_Play(VQAHandle *handle, VQAPlayMode mode)
{
    VQAErrorType rc = VQAERR_NONE;

#ifdef PLATFORM_WINDOWS
    // RA code
    DWORD priority_class = GetPriorityClass(GetCurrentProcess());
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif
    VQAData *data = handle->m_VQABuf;
    VQAConfig *config = &handle->m_Config;
    VQADrawer *drawer = &data->m_Drawer;

    if (!(data->m_Flags & VQA_DATA_FLAG_32)) {
        VQA_ConfigureDrawer(handle);

        // RA code
        if (config->m_OptionFlags & VQA_OPTION_SOUND_ENABLED) {
            if (data->m_Audio.m_IsLoaded) {
                VQA_StartAudio(handle);
            }
        }

        VQA_SetTimer(handle, 60 * data->m_Drawer.m_CurFrame->m_FrameNum / config->m_DrawRate, config->m_TimerMethod);

        data->m_StartTime = VQA_GetTime(handle);

        if (config->m_OptionFlags & VQA_OPTION_MONO_OUTPUT) {
            // VQA_InitMono(handle);
        }

        data->m_Flags |= VQA_DATA_FLAG_32;
    }

    if (mode <= 1 || mode != 2) {
        if (data->m_Flags & VQA_DATA_FLAG_64) {
            data->m_Flags &= ~VQA_DATA_FLAG_64;

            // RA code
            if (config->m_OptionFlags & VQA_OPTION_SOUND_ENABLED && VQA_StartAudio(handle) != VQAERR_NONE) {
                VQA_StopAudio(handle);
                return VQAERR_EOF;
            }

            VQA_SetTimer(handle, data->m_EndTime, config->m_TimerMethod);
        }

        while (mode != 1) {
            if (data->m_Flags & (VQA_DATA_FLAG_VIDEO_MEMORY_SET | VQA_DATA_FLAG_8)) {
                break;
            }

            if (data->m_Flags & VQA_DATA_FLAG_VIDEO_MEMORY_SET) {
                ++VQAMovieDone;
            } else {
                rc = (VQAErrorType)VQA_LoadFrame(handle);

                if (rc != VQAERR_NONE) {
                    if (rc != VQAERR_NOBUFFER && rc != VQAERR_SLEEPING) {
                        data->m_Flags |= VQA_DATA_FLAG_VIDEO_MEMORY_SET;
                        rc = VQAERR_NONE;
                    }

                } else {
                    ++data->m_LoadedFrames;
                }
            }

            if (config->m_DrawFlags & 2) {
                data->m_Flags |= VQA_DATA_FLAG_8;
                drawer->m_CurFrame->m_Flags = 0;
                drawer->m_CurFrame = drawer->m_CurFrame->m_Next;

            } else {
                rc = (VQAErrorType)data->Draw_Frame(handle);

                if (rc != VQAERR_NONE) {
                    if (rc == VQAERR_EOF) {
                        break;
                    }

                    if (data->m_Flags & VQA_DATA_FLAG_VIDEO_MEMORY_SET && rc == VQAERR_NOBUFFER) {
                        data->m_Flags |= VQA_DATA_FLAG_8;
                    }
                } else {
                    ++data->m_DrawnFrames;

                    VQA_UserUpdate(handle);

                    if ((data->Page_Flip)(handle) != 0) {
                        data->m_Flags |= (VQA_DATA_FLAG_VIDEO_MEMORY_SET | VQA_DATA_FLAG_8);
                    }
                }
            }

            if (config->m_OptionFlags & VQA_OPTION_MONO_OUTPUT) {
                // VQA_UpdateMono(handle);
            }
        }
    } else {
        if (!(data->m_Flags & VQA_DATA_FLAG_64)) {
            data->m_Flags |= VQA_DATA_FLAG_64;
            data->m_EndTime = VQA_GetTime(handle);

            // RA code
            if (data->m_Audio.m_Flags & VQA_AUDIO_FLAG_AUDIO_DMA_TIMER) {
                VQA_StopAudio(handle);
            }
        }

        rc = VQAERR_PAUSED;
    }

    if (data->m_Flags & (VQA_DATA_FLAG_VIDEO_MEMORY_SET | VQA_DATA_FLAG_8) || mode == 3) {
        data->m_EndTime = VQA_GetTime(handle);

        rc = VQAERR_EOF;
    }

    if (mode != 1) {
        if (data->m_Audio.m_Flags & VQA_AUDIO_FLAG_AUDIO_DMA_TIMER) {
            VQA_StopAudio(handle);
        }
    }
#ifdef PLATFORM_WINDOWS
    // RA code
    SetPriorityClass(GetCurrentProcess(), priority_class);
#endif
    return rc;
}

int16_t VQA_SetStop(VQAHandle *handle, int16_t frame)
{
    int16_t stopframe = -1;
    VQAHeader *header = &handle->m_Header;

    if (frame <= 0 || header->m_Frames < frame) {
        return stopframe;
    }

    stopframe = header->m_Frames;
    header->m_Frames = frame;
    return stopframe;
}

int VQA_SetDrawFlags(VQAHandle *handle, int flags)
{
    int oldflags = handle->m_Config.m_DrawFlags;
    handle->m_Config.m_DrawFlags |= flags;
    return oldflags;
}

int VQA_ClearDrawFlags(VQAHandle *handle, int flags)
{
    int oldflags = handle->m_Config.m_DrawFlags;
    handle->m_Config.m_DrawFlags &= ~flags;
    return oldflags;
}

void VQA_GetInfo(VQAHandle *handle, VQAInfo *info)
{
    info->m_NumFrames = handle->m_Header.m_Frames;
    info->m_ImageHeight = handle->m_Header.m_ImageHeight;
    info->m_ImageWidth = handle->m_Header.m_ImageWidth;
    info->m_ImageBuf = handle->m_VQABuf->m_Drawer.m_ImageBuf;
}

void VQA_GetStats(VQAHandle *handle, VQAStatistics *stats)
{
    VQAData *data = handle->m_VQABuf;

    stats->m_MemUsed = data->m_MemUsed;
    stats->m_StartTime = data->m_StartTime;
    stats->m_EndTime = data->m_EndTime;
    stats->m_FramesLoaded = data->m_LoadedFrames;
    stats->m_FramesDrawn = data->m_DrawnFrames;
    stats->m_FramesSkipped = data->m_Drawer.m_NumSkipped;
    stats->m_MaxFrameSize = data->m_Loader.m_MaxFrameSize;
    stats->m_SamplesPlayed = data->m_Audio.m_SamplesPlayed;
}

// Function found in BR, appears its only use there is to force BH,BW and CM to 0;
VQAErrorType VQA_GetBlockWH_ColorMode(VQAHandle *handle, unsigned int *blockwidth, unsigned int *blockheight, int *colormode)
{
    *blockwidth = handle->m_Header.m_BlockWidth;
    *blockheight = handle->m_Header.m_BlockHeight;
    *colormode = handle->m_Header.m_ColorMode;

    return VQAERR_EOF;
}

// Function found in BR;
VQAErrorType VQA_GetXYPos(VQAHandle *handle, uint16_t *xpos, uint16_t *ypos)
{
    *xpos = handle->m_Header.m_Xpos;
    *ypos = handle->m_Header.m_Ypos;

    return VQAERR_EOF;
}

VQAErrorType VQA_UserUpdate(VQAHandle *handle)
{
    VQAErrorType rc = VQAERR_NONE;
    VQAData *data = handle->m_VQABuf;

    if (data->m_Flags & VQA_DATA_FLAG_REPEAT_SAME_TAG) {
        rc = (VQAErrorType)data->Page_Flip(handle);
        data->m_Flipper.m_LastFrameNum = data->m_Flipper.m_CurFrame->m_FrameNum;
        data->m_Flipper.m_CurFrame->m_Flags = 0;
        data->m_Flags &= ~VQA_DATA_FLAG_REPEAT_SAME_TAG;
    }

    return rc;
}
