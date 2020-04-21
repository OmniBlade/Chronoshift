/**
 * @file
 *
 * @author tomsons26
 *
 * @brief  VQA file releated definitions.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef VQAFILE_H
#define VQAFILE_H

#include "vqaaudio.h"
#include "vqaconfig.h"
#include "vqadrawer.h"

class FileClass;
struct CaptionInfo;

enum VQAColorModeType
#ifndef COMPILER_WATCOM
    : int8_t // Force size to 8 bit.
#endif
{
    COLORMODE_256 = 0,
    COLORMODE_15BIT = 1,
    COLORMODE_UNKNOWN1 = 2,
    COLORMODE_UNKNOWN2 = 3,
    COLORMODE_16BIT = 4,
};


enum VQACBNodeFlags
{
    VQACB_CODEBOOK_0x1 = 0x1, // Unset for FullCB after loading in Load_CBF0 Load_CBFZ, Load_CBP0 and Load_CBPZ
    VQACB_COMPRESSED_CODEBOOK = 0x2, // Sset by Load_CBPZ, processed by Prepare_Frame then unset
    VQACB_CODEBOOK_0x4 = 0x4,
    VQACB_COMPRESSED_PALETTE = 0x8, // Load_CPLZ, processed by Prepare_Frame then unset
    VQACB_COMPRESSED_POINTERS = 0x10, // Load_VPTZ, processed by Prepare_Frame then unset
};

enum VQAFrameNodeFlags
{
    FRAMENODE_FRAME_LOADED = 0x1, // Set at end of VQA_LoadFrame
    FRAMENODE_COMPRESSED_POINTERS_K = 0x2, // set after VPTK chunk read, calls Load_VPTZ
    FRAMENODE_PALETTE = 0x4, // confirmed, set after Load_CPL0, Load_CPLZ call
    FRAMENODE_COMPRESSED_PALETTE = 0x8, // confirmed, set by Load_CPLZ, processed by Select_Frame then unset
    FRAMENODE_COMPRESSED_POINTERS = 0x10, // confirmed, set by Load_VPTZ
    FRAMENODE_VECTOR_POINTER_TABLE_R = 0x20,
    FRAMENODE_VPTR_BUFFERED = 0x40,
    FRAMENODE_0x80 = 0x80,
    FRAMENODE_LOOP_REPEATED = 0x100,
    FRAMENODE_LOOP_SWITCHED = 0x200,
    FRAMENODE_CUSTOM_DATA = 0x400,
    FRAMENODE_UNKNOWN1 = 0x800,
};

enum VQAErrorType
{
    VQAERR_NONE = 0, // a error code 0 exists and seems to be used when all is fine, but None is used in the same case too?
    VQAERR_EOF = -1, // Same code in LOL2 shows its EOF but seems to be also used for fatal errors
    VQAERR_OPEN = -2,
    VQAERR_READ = -3,
    VQAERR_WRITE = -4,
    VQAERR_SEEK = -5,
    VQAERR_NOTVQA = -6,
    VQAERR_NOMEM = -7,
    VQAERR_NOBUFFER = -8,
    VQAERR_NOT_TIME = -9,
    VQAERR_SLEEPING = -10,
    VQAERR_VIDEO = -11,
    VQAERR_AUDIO = -12,
    VQAERR_PAUSED = -13,
};

enum VQAReturnType
{
    VQARETURNCODE_FAILED = 0,
    VQARETURNCODE_OK = 1,
};

enum VQAStreamActionType
{
    ACTION_ALWAYS_FALSE1 = 1,
    ACTION_ALWAYS_FALSE2 = 2,
    ACTION_OPEN = 3,
    ACTION_CLOSE = 4,
    ACTION_READ = 5,
    ACTION_WRITE = 6,
    ACTION_SEEK = 7,
    ACTION_EOF = 8,
    ACTION_SIZE = 9,
};

typedef int (*StreamHandlerFuncPtr)(void *, VQAStreamActionType, void *, int);

struct VQACBNode
{
    uint8_t *m_Buffer;
    VQACBNode *m_Next;
    unsigned m_Flags;
    unsigned m_CBOffset;
};

struct VQAFrameNode
{
    uint8_t *m_Pointers;
    VQACBNode *m_Codebook;
    uint8_t *m_Palette;
    VQAFrameNode *m_Next;
    unsigned m_Flags;
    int m_FrameNum;
    int m_PtrOffset;
    int m_PalOffset;
    int m_PaletteSize;
};

// Used in later VQA versions to clip to a specfic size
struct VQAClipNode
{
    unsigned m_Width;
    unsigned m_Height;
};

struct VQAFlipper
{
    VQAFrameNode *m_CurFrame;
    int m_LastFrameNum;
};

#pragma pack(push, 1)
struct VQAHeader
{
    uint16_t m_Version;
    //      Known Header flag info
    //    1 = HasSound
    //    2 = HasAlternativeAudio
    //    4 = Set on LOLG VQA's that have a transparent/to be keyed background, it makes it use a special UnVQ function set
    //      Always set in BR/TS VQA's
    //      Set on LOL3 intro VQA
    //    8 = Always set in TS VQA's
    //      Set on LOL3 intro VQA
    //      Set on some LOLG VQA's, can only be decoded with UnVQ_4x2 in LOLG which is nearly identical to RA's but RA has
    //      value 15 while this has -1, haven't found code that checks it tho
    //   16 = Always set in TS VQA's
    //      Set on LOL3 intro VQA
    //      Set on most BR VQA's
    uint16_t m_Flags;
    uint16_t m_Frames;
    uint16_t m_ImageWidth;
    uint16_t m_ImageHeight;
    uint8_t m_BlockWidth;
    uint8_t m_BlockHeight;
    uint8_t m_FPS;
    uint8_t m_Groupsize;
    uint16_t m_Num1Colors;
    uint16_t m_CBentries;
    uint16_t m_Xpos;
    uint16_t m_Ypos;
    uint16_t m_MaxFramesize;
    uint16_t m_SampleRate;
    uint8_t m_Channels;
    uint8_t m_BitsPerSample;
    // Ones below need to be double confirmed as this is from a old DB of mine
    uint16_t m_AltSampleRate; // Streams in old header
    uint8_t m_AltChannels; // char FutureUse[12] was here
    uint8_t m_AltBitsPerSample;
    VQAColorModeType m_ColorMode; // Confirmed
    uint8_t field_21;
    unsigned int m_MaxCompressedCBSize;
    unsigned int field_26;
};
#pragma pack(pop)

struct VQAHandle
{
    void *m_VQAStream;
    StreamHandlerFuncPtr m_StreamHandler;
    VQAData *m_VQABuf;
    VQAConfig m_Config;
    VQAHeader m_Header;
    int m_VocFH;
    CaptionInfo *field_BE; // EVA info?
    CaptionInfo *field_C2; // Captions info?
};

struct VQAChunkHeader
{
    uint32_t m_ID; // ID of the chunk header - FourCC
    uint32_t m_Size; // Length of the chunk data, in bytes.
};

#endif