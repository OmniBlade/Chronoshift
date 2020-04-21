/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief  VQA audio functions.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "vqaaudio.h"
#include "vqafile.h"
#include "vqaloader.h"
#include "vqatask.h"
#include <algorithm>
#include <captainslog.h>
#include <sys/timeb.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef GAME_DLL
extern int &g_AudioFlags;
extern int &g_TimerIntCount;
extern MMRESULT &g_VQATimer;
extern int &g_TimerMethod;
extern int &g_VQATickCount;
extern int &g_TickOffset;
extern int &g_SuspendAudioCallback;
extern int &g_VQAAudioPaused;
extern VQAHandle *&g_AudioVQAHandle;
#else
int g_AudioFlags;
int g_TimerIntCount;
#ifdef BUILD_WITH_DSOUND
MMRESULT g_VQATimer;
#endif
int g_TimerMethod;
int g_VQATickCount;
int g_TickOffset;
int g_SuspendAudioCallback;
int g_VQAAudioPaused;
VQAHandle *g_AudioVQAHandle;
#endif

// 8192 has some chopping issues, like its not overlapping correctlying between each chunk?
// 8192 * 4 seems to fix the above for the short sample but there is still slight chopping when INTRO is played.
#define BUFFER_CHUNK_SIZE 8192 * 4 // was 8192 in RA 8192 is 186ms?
#define TIMER_DELAY 16
#define TIMER_RESOLUTION 1
#define TARGET_RESOLUTION 10 // 10-millisecond target resolution

BOOL Move_HMI_Audio_Block_To_Direct_Sound_Buffer()
{
#ifdef BUILD_WITH_DSOUND
    VQAConfig *config = &g_AudioVQAHandle->m_Config;
    VQAData *data = g_AudioVQAHandle->m_VQABuf;
    VQAAudio *audio = &data->m_Audio;
    void *ptr1;
    DWORD bytes1;
    void *ptr2;
    DWORD bytes2;
    DWORD status;

    if (audio->m_PrimaryBufferPtr->Lock(audio->field_B0, config->m_HMIBufSize, &ptr1, &bytes1, &ptr2, &bytes2, 0) != DS_OK
        || config->m_SoundBuffer->GetStatus(&status) == DSERR_BUFFERLOST) {
        return false;
    }

    memcpy(ptr1, &audio->m_Buffer[audio->m_PlayPosition], config->m_HMIBufSize);
    audio->m_PrimaryBufferPtr->Unlock(ptr1, bytes1, ptr2, bytes2);
    audio->field_B0 += config->m_HMIBufSize;

    if (audio->field_B0 >= audio->field_AC) {
        audio->field_B0 = 0;
    }

    audio->field_14 = audio->field_10 + 1;

    if (audio->field_14 >= audio->m_NumAudBlocks) {
        audio->field_14 = 0;
    }

    if (audio->m_IsLoaded[audio->field_14] != 1) {
        if (VQAMovieDone) {
            ++audio->field_B4;
        }

        ++audio->m_NumSkipped;
        config->m_DrawFlags &= 0xFB;

        return false;
    }

    audio->m_IsLoaded[audio->field_10] = 0;
    audio->m_PlayPosition += config->m_HMIBufSize;
    audio->field_10 = audio->field_10 + 1;

    if (audio->m_PlayPosition >= config->m_AudioBufSize) {
        audio->m_PlayPosition = 0;
        audio->field_10 = 0;
    }

    ++audio->field_B4;
#endif
    return true;
}

#ifdef BUILD_WITH_DSOUND
void __stdcall VQA_SoundTimerCallback(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    ++g_VQATickCount;
}

void __stdcall VQA_AudioCallback(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    if (!g_SuspendAudioCallback && !g_VQAAudioPaused) {
        VQAConfig *config = &g_AudioVQAHandle->m_Config;
        VQAData *data = g_AudioVQAHandle->m_VQABuf;
        VQAAudio *audio = &data->m_Audio;

        if (audio->m_PrimaryBufferPtr != nullptr) {
            BOOL buffer_restored = false;
            BOOL restart = false;
            DWORD playCursor;
            DWORD writeOffset;
            DWORD status;

            if (audio->m_PrimaryBufferPtr->GetCurrentPosition(&playCursor, &writeOffset) == DSERR_BUFFERLOST
                || config->m_SoundBuffer->GetStatus(&status) == DSERR_BUFFERLOST) {
                config->m_SoundBuffer->Restore();
                audio->m_PrimaryBufferPtr->Restore();
                audio->m_PrimaryBufferPtr->Stop();
                audio->field_B0 = 0;
                audio->field_B8 = 0;

                buffer_restored = true;
                restart = true;
            }

            if (playCursor >= audio->field_B0) {
                if (((audio->field_AC * 3) / 4) < playCursor && audio->field_B0 <= 0) {
                    restart = true;
                }
            } else if ((audio->field_B0 - playCursor) <= (audio->field_AC / 4)) {
                restart = true;
            }

            if (restart && Move_HMI_Audio_Block_To_Direct_Sound_Buffer() && buffer_restored) {
                audio->m_PrimaryBufferPtr->Play(0, 0, DSBPLAY_LOOPING);
            }
        }
    }
}
#endif

int VQA_StartTimerInt(VQAHandle *handle, int a2)
{
#ifdef BUILD_WITH_DSOUND
    VQAData *data = handle->m_VQABuf;
    VQAAudio *audio = &data->m_Audio;

    if (!(g_AudioFlags & 0x30)) {
        g_VQATimer = timeSetEvent(TIMER_DELAY,
            TIMER_RESOLUTION,
            VQA_SoundTimerCallback,
            (DWORD_PTR)nullptr,
            TIME_CALLBACK_FUNCTION | TIME_PERIODIC);

        if (!g_VQATimer) {
            return -1;
        }

        g_AudioFlags |= VQA_AUDIO_FLAG_UNKNOWN016;
    }

    audio->m_Flags |= VQA_AUDIO_FLAG_UNKNOWN016;
    ++g_TimerIntCount;
#endif
    return 0;
}

void VQA_StopTimerInt(VQAHandle *handle)
{
#ifdef BUILD_WITH_DSOUND
    if (g_TimerIntCount > 0) {
        --g_TimerIntCount;
    }

    if ((g_AudioFlags & 0x30) == 0x10 && g_TimerIntCount == 0) {
        timeKillEvent(g_VQATimer);
    }

    g_AudioFlags &= 0xCF;
#endif
}

int VQA_OpenAudio(VQAHandle *handle, void *hwnd)
{
#ifdef BUILD_WITH_DSOUND
    VQAConfig *config = &handle->m_Config;
    VQAData *data = handle->m_VQABuf;
    VQAHeader *header = &handle->m_Header;
    VQAAudio *audio = &data->m_Audio;
    audio->field_10 = 0;
    HRESULT dsretval;

    if (config->m_SoundObject == nullptr) {
        if ((dsretval = DirectSoundCreate(nullptr, &config->m_SoundObject, nullptr)) != DS_OK) {
            captainslog_debug("VQA_OpenAudio() - Unable to create Direct Sound Object. Error code %d.\n", dsretval);
            return -1;
        }

        audio->field_BC = 1;

        if ((dsretval = config->m_SoundObject->SetCooperativeLevel((HWND)hwnd, DSSCL_PRIORITY)) != DS_OK) {
            captainslog_debug("VQA_OpenAudio() - Unable to set Direct Sound cooperative level. Error code %d.\n", dsretval);
            config->m_SoundObject->Release();
            config->m_SoundObject = nullptr;
            return -1;
        }

        if (config->m_SoundBuffer == nullptr) {
            captainslog_debug("VQA_OpenAudio() - Setting up buffer structs.\n");

            // Set up DSBUFFERDESC structure.
            memset(&audio->m_BufferDesc, 0, sizeof(audio->m_BufferDesc));
            audio->m_BufferDesc.dwSize = sizeof(audio->m_BufferDesc);
            audio->m_BufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;

            // Set up buffer format structure.
            memset(&audio->m_BuffFormat, 0, sizeof(audio->m_BuffFormat));
            audio->m_BuffFormat.wFormatTag = WAVE_FORMAT_PCM; // Format code
            audio->m_BuffFormat.nChannels = 2; // Number of interleaved channels

            // Sampling rate (blocks per second)
            if (config->m_AudioRate == -1) {
                int rate = 0;

                if (header->m_FPS == config->m_FrameRate) {
                    audio->m_BuffFormat.nSamplesPerSec = audio->m_SampleRate;
                    rate = audio->m_SampleRate;
                } else {
                    rate = config->m_FrameRate * audio->m_SampleRate / header->m_FPS;
                    audio->m_BuffFormat.nSamplesPerSec = rate;
                }

                config->m_AudioRate = rate;
                captainslog_debug("VQA_OpenAudio() - AudioRate = %d.\n", config->m_AudioRate);
            } else {
                audio->m_BuffFormat.nSamplesPerSec = config->m_AudioRate;
            }

            audio->m_BuffFormat.wBitsPerSample = 16; // Bits per sample
            audio->m_BuffFormat.nBlockAlign =
                audio->m_BuffFormat.nChannels * audio->m_BuffFormat.wBitsPerSample / 8; // Data block size (bytes)
            audio->m_BuffFormat.cbSize = 0; // Size of the extension (0 or 22)
            audio->m_BuffFormat.nAvgBytesPerSec = audio->m_BuffFormat.nBlockAlign * audio->m_BuffFormat.nSamplesPerSec;

            captainslog_debug("VQA_OpenAudio() - nSamplesPerSec = %d.\n", audio->m_BuffFormat.nSamplesPerSec);
            captainslog_debug("VQA_OpenAudio() - nBlockAlign = %d.\n", audio->m_BuffFormat.nBlockAlign);
            captainslog_debug("VQA_OpenAudio() - nAvgBytesPerSec = %d.\n", audio->m_BuffFormat.nAvgBytesPerSec);
            captainslog_debug("VQA_OpenAudio() - About to call SoundObject->CreateSoundBuffer().\n");

            if ((dsretval = config->m_SoundObject->CreateSoundBuffer(&audio->m_BufferDesc, &config->m_SoundBuffer, nullptr))
                != DS_OK) {
                captainslog_debug(
                    "VQA_OpenAudio() - Unable to create Direct Sound primary buffer. Error code %d.\n", dsretval);
                config->m_SoundObject->Release();
                config->m_SoundObject = nullptr;

                return -1;
            }

            audio->field_C0 = 1;
        }

        if (audio->field_BC) {
            captainslog_debug("VQA_OpenAudio() - About to call SoundObject->Release().\n");
            config->m_SoundObject->Release();
            config->m_SoundObject = nullptr;
            audio->field_BC = 0;
        }
    }

    audio->m_Flags |= VQA_AUDIO_FLAG_UNKNOWN001;
    g_AudioFlags |= VQA_AUDIO_FLAG_UNKNOWN001;
    captainslog_debug("VQA_OpenAudio(exit)\n");
#endif
    return 0;
}

void VQA_CloseAudio(VQAHandle *handle)
{
#ifdef BUILD_WITH_DSOUND
    VQAConfig *config = &handle->m_Config;
    VQAData *data = handle->m_VQABuf;
    VQAAudio *audio = &data->m_Audio;

    VQA_StopAudio(handle);
    g_AudioFlags &= 0xF3;
    audio->m_Flags &= 0xF3;

    if (audio->field_C0) {
        config->m_SoundBuffer->Stop();
        config->m_SoundBuffer->Release();
        config->m_SoundBuffer = 0;
        audio->field_C0 = 0;
    }

    if (audio->field_BC) {
        config->m_SoundObject->Release();
        config->m_SoundObject = nullptr;
        audio->field_BC = 0;
    }

    audio->m_Flags &= 0xFC;
    g_AudioFlags &= 0xBC;
#endif
}

int VQA_StartAudio(VQAHandle *handle)
{
#ifdef BUILD_WITH_DSOUND
    VQAConfig *config = &handle->m_Config;
    VQAData *data = handle->m_VQABuf;
    VQAAudio *audio = &data->m_Audio;

    g_AudioVQAHandle = handle;

    if (g_AudioFlags & 0x40) {
        return -1;
    }

    if (audio->m_PrimaryBufferPtr != nullptr) {
        audio->m_PrimaryBufferPtr->Release();
        audio->m_PrimaryBufferPtr = nullptr;
    }

    audio->field_AC = config->m_HMIBufSize * 4;

    // Set up DSBUFFERDESC structure.
    memset(&audio->m_BufferDesc, 0, sizeof(audio->m_BufferDesc));
    audio->m_BufferDesc.dwSize = sizeof(audio->m_BufferDesc);
    audio->m_BufferDesc.dwFlags = DSBCAPS_CTRLVOLUME; // Buffer flags
    audio->m_BufferDesc.dwBufferBytes = audio->field_AC;
    audio->m_BufferDesc.lpwfxFormat = &audio->m_BuffFormat;

    // Set up buffer format structure.
    memset(&audio->m_BuffFormat, 0, sizeof(audio->m_BuffFormat));
    audio->m_BuffFormat.nSamplesPerSec = audio->m_SampleRate;
    audio->m_BuffFormat.nChannels = audio->m_Channels;
    audio->m_BuffFormat.wBitsPerSample = audio->m_BitsPerSample;
    audio->m_BuffFormat.wFormatTag = WAVE_FORMAT_PCM; // Format code
    audio->m_BuffFormat.nBlockAlign =
        audio->m_BuffFormat.nChannels * audio->m_BuffFormat.wBitsPerSample / 8; // Data block size (bytes)
    audio->m_BuffFormat.nAvgBytesPerSec = audio->m_BuffFormat.nBlockAlign * audio->m_BuffFormat.nSamplesPerSec;

    if (config->m_SoundBuffer->SetFormat(&audio->m_BuffFormat) != DS_OK) {
        if (audio->m_BitsPerSample == 16) {
            audio->m_BuffFormat.wBitsPerSample = 8;

            int tmp_bits = audio->m_BuffFormat.wBitsPerSample;
            int tmp_bytes = audio->m_BuffFormat.nAvgBytesPerSec;
            int tmp_align = audio->m_BuffFormat.nBlockAlign;

            audio->m_BuffFormat.nBlockAlign =
                audio->m_BuffFormat.nChannels * audio->m_BuffFormat.wBitsPerSample / 8; // Data block size (bytes)
            audio->m_BuffFormat.nAvgBytesPerSec = audio->m_BuffFormat.nBlockAlign * audio->m_BuffFormat.nSamplesPerSec;
            config->m_SoundBuffer->SetFormat(&audio->m_BuffFormat);
            audio->m_BuffFormat.nBlockAlign = tmp_align;
            audio->m_BuffFormat.wBitsPerSample = tmp_bits;
            audio->m_BuffFormat.nAvgBytesPerSec = tmp_bytes;
        }
    }

    if (config->m_SoundBuffer->Play(0, 0, DSBPLAY_LOOPING) != DS_OK) {
        return -1;
    }

    audio->field_B0 = 0;
    audio->field_B4 = 0;
    Move_HMI_Audio_Block_To_Direct_Sound_Buffer();
    Move_HMI_Audio_Block_To_Direct_Sound_Buffer();
    audio->m_PrimaryBufferPtr->SetCurrentPosition(0);

    if (audio->m_PrimaryBufferPtr->Play(0, 0, DSBPLAY_LOOPING) != DS_OK) {
        return -1;
    }

    audio->m_PrimaryBufferPtr->SetVolume(-(1000 * (0x8000 - (config->m_Volume * 128)) / 0x8000));
    timeBeginPeriod(TIMER_DELAY);
    audio->m_SoundTimerHandle = timeSetEvent(
        TIMER_DELAY, TIMER_RESOLUTION, VQA_SoundTimerCallback, (DWORD_PTR)nullptr, TIME_CALLBACK_FUNCTION | TIME_PERIODIC);
    audio->m_Flags |= 0x40;
    g_AudioFlags |= 0x40;
#endif
    return 0;
}

void VQA_StopAudio(VQAHandle *handle)
{
#ifdef BUILD_WITH_DSOUND
    VQAConfig *config = &handle->m_Config;
    VQAData *data = handle->m_VQABuf;
    VQAAudio *audio = &data->m_Audio;

    if (g_AudioFlags & VQA_AUDIO_FLAG_AUDIO_DMA_TIMER) {
        config->m_SoundBuffer->Stop();
        timeKillEvent(audio->m_SoundTimerHandle);
        timeEndPeriod(TIMER_DELAY);
        audio->m_SoundTimerHandle = 0;

        if (audio->m_PrimaryBufferPtr) {
            audio->m_PrimaryBufferPtr->Stop();
            audio->m_PrimaryBufferPtr->Release();
            audio->m_PrimaryBufferPtr = nullptr;
        }

        audio->m_Flags &= 0xBF;
        g_AudioFlags &= 0xBF;
    }

    g_AudioVQAHandle = nullptr;
#endif
}

int VQA_CopyAudio(VQAHandle *handle)
{
#ifdef BUILD_WITH_DSOUND
    VQAConfig *config = &handle->m_Config;
    VQAData *data = handle->m_VQABuf;
    VQAAudio *audio = &data->m_Audio;

    if (config->m_OptionFlags & 1) {
        if (audio->m_Buffer != nullptr) {
            if (audio->m_TempBufSize > 0) {
                int current_block = audio->m_AudBufPos / config->m_HMIBufSize;
                int next_block = std::min(audio->m_TempBufSize + current_block, audio->m_NumAudBlocks);

                if (audio->m_IsLoaded[next_block] == 1) {
                    return -10;
                }

                // Need to loop back and treat like circular buffer?
                if (next_block < current_block) {
                    int end_space = config->m_AudioBufSize - audio->m_AudBufPos;
                    int remaining = audio->m_TempBufSize - end_space;
                    memcpy(&audio->m_Buffer[audio->m_AudBufPos], audio->m_TempBuf, end_space);
                    memcpy(audio->m_Buffer, &audio->m_TempBuf[end_space], remaining);
                    audio->m_AudBufPos = remaining;
                    audio->m_TempBufSize = 0;

                    for (unsigned i = current_block; i < audio->m_NumAudBlocks; ++i) {
                        audio->m_IsLoaded[i] = 1;
                    }

                    for (int i = 0; i < next_block; ++i) {
                        audio->m_IsLoaded[i] = 1;
                    }
                } else {
                    memcpy(&audio->m_Buffer[audio->m_AudBufPos], audio->m_TempBuf, audio->m_TempBufSize);
                    audio->m_AudBufPos += audio->m_TempBufSize;
                    audio->m_TempBufSize = 0;

                    for (int i = current_block; i < next_block; ++i) {
                        audio->m_IsLoaded[i] = 1;
                    }
                }
            }
        }
    }
#endif
    return 0;
}

void VQA_SetTimer(VQAHandle *handle, int time, unsigned method)
{
#ifdef BUILD_WITH_DSOUND
    if (method == -1) {
        if (g_AudioFlags & VQA_AUDIO_FLAG_AUDIO_DMA_TIMER) {
            method = 3;
        } else if (g_AudioFlags & (VQA_AUDIO_FLAG_UNKNOWN016 | VQA_AUDIO_FLAG_UNKNOWN032)) {
            method = 2;
        } else {
            method = 1;
        }

        if (!(g_AudioFlags & VQA_AUDIO_FLAG_AUDIO_DMA_TIMER) && method == 3) {
            method = 2;
        }

        if (!(g_AudioFlags & (VQA_AUDIO_FLAG_UNKNOWN016 | VQA_AUDIO_FLAG_UNKNOWN032)) && method == 2) {
            method = 1;
        }
    }

    g_TimerMethod = 0;
    g_TickOffset = time - VQA_GetTime(handle);
#endif
}

unsigned VQA_GetTime(VQAHandle *handle)
{
#ifdef BUILD_WITH_DSOUND
    static unsigned last_chunksmovedtoaudiobuffer;
    static unsigned last_totalbytes;
    static unsigned last_ticks;
    static unsigned last_playcursor;
    static unsigned last_hmibufsize;
    static unsigned last_lastchunkposition;
    static unsigned this_chunksmovedtoaudiobuffer;
    static unsigned this_totalbytes;
    static unsigned this_ticks;
    static unsigned this_playcursor;
    static unsigned this_hmibufsize;
    static unsigned this_lastchunkposition;
    unsigned ticks = 0;
    unsigned bytes = 0;
    unsigned result_time = 0;
    DWORD play_cursor;
    DWORD write_offset;
    VQAData *data = handle->m_VQABuf;
    VQAConfig *config = &handle->m_Config;
    VQAAudio *audio = &handle->m_VQABuf->m_Audio;

    switch (VQA_TimerMethod()) {
        default:
        case VQA_AUDIO_TIMER_METHOD_DOS: {
            struct timeb mytime;
            ftime(&mytime);
            result_time = (g_TickOffset + 60 * (mytime.millitm + 1000 * mytime.time) / 1000);
            break;
        }

        case VQA_AUDIO_TIMER_METHOD_INTERRUPT: {
            result_time = g_TickOffset + g_VQATickCount;
            break;
        }

        case VQA_AUDIO_TIMER_METHOD_DMA: {
            break;
        }
    };

    this_chunksmovedtoaudiobuffer = audio->field_B4;
    this_hmibufsize = config->m_HMIBufSize;

    if (audio->m_PrimaryBufferPtr != nullptr) {
        if (audio->m_PrimaryBufferPtr->GetCurrentPosition(&play_cursor, &write_offset) != DS_OK) {
            bytes = config->m_HMIBufSize * (audio->field_B4 - 1);
        } else {
            int v5 = config->m_HMIBufSize * audio->field_B4;
            this_playcursor = play_cursor;
            this_lastchunkposition = audio->field_B8;

            if (this_lastchunkposition > 0) {
                bytes = play_cursor - audio->field_B8 + v5;
            } else {
                if ((3 * audio->field_AC) >> 2 >= play_cursor) {
                    bytes = play_cursor - audio->field_B8 + v5;
                } else {
                    bytes = v5 - (audio->field_AC - play_cursor);
                }
            }
        }
    } else {
        bytes = config->m_HMIBufSize * (audio->field_B4 - 1);
    }

    this_totalbytes = bytes;
    int v0 =
        g_TickOffset + 60 * (bytes / audio->m_BuffFormat.nChannels / (audio->m_BuffFormat.wBitsPerSample / 8)) / audio->m_SampleRate;

    if (v0 <= 100 || (ticks = last_ticks, v0 - last_ticks <= 20)) {
        last_chunksmovedtoaudiobuffer = this_chunksmovedtoaudiobuffer;
        last_totalbytes = this_totalbytes;
        last_ticks = this_ticks;
        last_playcursor = this_playcursor;
        last_hmibufsize = this_hmibufsize;
        last_lastchunkposition = this_lastchunkposition;
    } else if (VQA_TimerMethod() == 3) {
        ++last_ticks;
        result_time = ticks + 1;
    }

    return result_time;
#else
    return 0;
#endif
}

int VQA_TimerMethod()
{
    return g_TimerMethod;
}