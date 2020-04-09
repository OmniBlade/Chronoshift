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
#pragma once

#ifndef UNVQBUFF_H
#define UNVQBUFF_H

#include "always.h"

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl UnVQ_4x2(
    uint8_t *codebook, uint8_t *pointers, uint8_t *buffer, unsigned blocks_per_row, unsigned num_rows, unsigned buff_width);
void __cdecl UnVQ_4x4(
    uint8_t *codebook, uint8_t *pointers, uint8_t *buffer, unsigned blocks_per_row, unsigned num_rows, unsigned buff_width);
void __cdecl UnVQ_Nop(
    uint8_t *codebook, uint8_t *pointers, uint8_t *buffer, unsigned blocks_per_row, unsigned num_rows, unsigned buff_width);

#ifdef __cplusplus
} // __cplusplus
#endif

#endif
