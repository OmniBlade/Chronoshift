/**
 * @file
 *
 * @author peterkohaut
 * @author tomsons26
 *
 * @brief Functions for decoding vector quantized blocks.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "unvqbuff.h"

// Big thanks to peterkohaut from ScummVM team who did a reimplementaiont of the original assembly code.

void __cdecl UnVQ_4x2(
    uint8_t *codebook, uint8_t *pointers, uint8_t *buffer, unsigned blocks_per_row, unsigned num_rows, unsigned buff_width)
{
    uint32_t *cb_offset = (uint32_t *)codebook;
    uint32_t blocks_per_row_left = 0;
    uint8_t block_id[4];
    uint32_t data_end;
    uint32_t vptr_pos;
    uint32_t video_pos;
    uint32_t row_offset;

    video_pos = 0;
    vptr_pos = 0;
    row_offset = 0;
    blocks_per_row_left = blocks_per_row;
    data_end = blocks_per_row * num_rows;

    while (vptr_pos < data_end) {
        block_id[0] = pointers[vptr_pos];
        block_id[1] = pointers[vptr_pos + blocks_per_row * num_rows];
        block_id[2] = 0;
        block_id[3] = 0;
        vptr_pos++;

        if (block_id[1] == 15) { // 255 for LOLG videos.
            // make all bytes same color as first byte
            block_id[1] = block_id[0];
            block_id[2] = block_id[0];
            block_id[3] = block_id[0];
            *(uint32_t *)&buffer[video_pos] = *(uint32_t *)block_id;
            *(uint32_t *)&buffer[video_pos + buff_width] = *(uint32_t *)block_id;
        } else {
            *(uint32_t *)&buffer[video_pos] = cb_offset[2 * *(uint32_t *)block_id];
            *(uint32_t *)&buffer[video_pos + buff_width] = cb_offset[2 * *(uint32_t *)block_id + 1];
        }

        video_pos += 4;
        blocks_per_row_left--;

        if (blocks_per_row_left == 0) {
            row_offset += 2 * buff_width; // move pointer in video memory by two rows
            video_pos = row_offset;
            blocks_per_row_left = blocks_per_row;
        }
    }
}

void __cdecl UnVQ_4x4(
    uint8_t *codebook, uint8_t *pointers, uint8_t *buffer, unsigned blocks_per_row, unsigned num_rows, unsigned buff_width)
{
    uint32_t *cb_offset = (uint32_t *)codebook;
    uint32_t blocks_per_row_left = 0;
    uint8_t block_id[4];
    uint32_t data_end;
    uint32_t vptr_pos;
    uint32_t video_pos;
    uint32_t row_offset;

    video_pos = 0;
    vptr_pos = 0;
    row_offset = 0;
    blocks_per_row_left = blocks_per_row;
    data_end = blocks_per_row * num_rows;

    while (vptr_pos < data_end) {
        block_id[0] = pointers[vptr_pos];
        block_id[1] = pointers[vptr_pos + blocks_per_row * num_rows];
        block_id[2] = 0;
        block_id[3] = 0;
        vptr_pos++;

        if (block_id[1] == 255) {
            // make all bytes same color as first byte
            block_id[1] = block_id[0];
            block_id[2] = block_id[0];
            block_id[3] = block_id[0];
            *(uint32_t *)&buffer[video_pos] = *(uint32_t *)block_id;
            *(uint32_t *)&buffer[video_pos + 1 * buff_width] = *(uint32_t *)block_id;
            *(uint32_t *)&buffer[video_pos + 2 * buff_width] = *(uint32_t *)block_id;
            *(uint32_t *)&buffer[video_pos + 3 * buff_width] = *(uint32_t *)block_id;
        } else {
            *(uint32_t *)&buffer[video_pos] = cb_offset[4 * *(uint32_t *)block_id];
            *(uint32_t *)&buffer[video_pos + 1 * buff_width] = cb_offset[4 * *(uint32_t *)block_id + 1];
            *(uint32_t *)&buffer[video_pos + 2 * buff_width] = cb_offset[4 * *(uint32_t *)block_id + 2];
            *(uint32_t *)&buffer[video_pos + 3 * buff_width] = cb_offset[4 * *(uint32_t *)block_id + 3];
        }

        video_pos += 4;
        blocks_per_row_left--;

        if (blocks_per_row_left == 0) {
            row_offset += 4 * buff_width; // move pointer in video memory by four rows
            video_pos = row_offset;
            blocks_per_row_left = blocks_per_row;
        }
    }
}

void __cdecl UnVQ_Nop(
    uint8_t *codebook, uint8_t *pointers, uint8_t *buffer, unsigned blocks_per_row, unsigned num_rows, unsigned buff_width)
{
    // Noop.
}
