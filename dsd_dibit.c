/*
 * Copyright (C) 2010 DSD Author
 * GPG Key ID: 0x3F1D7FD0 (74EF 430D F7F2 0A48 FCE6  F630 FAA2 635D 3F1D 7FD0)
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "dsd.h"

static unsigned int 
invert_dibit(unsigned int dibit)
{
    return (dibit ^ 2);
}

static unsigned int digitize (dsd_state* state, int symbol)
{
  // determine dibit state
  if (((state->synctype >> 1) == 4) || ((state->synctype >> 1) == 5) || ((state->synctype >> 1) == 9)) {
      //  8 +D-STAR
      //  9 -D-STAR
      // 10 +D-STAR_HD
      // 11 -D-STAR_HD
      // 18 +ProVoice
      // 19 -ProVoice
      unsigned int dibit;
      if (symbol > state->center) {
        dibit = 0;
      } else {
        dibit = 1;
      }
      if ((state->synctype & 1) == 1) {
        dibit ^= 1;
      }
      *state->dibit_buf_p++ = dibit;
      return dibit;
  } else {
      //  0 +P25p1
      //  1 -P25p1
      //  2 +X2-TDMA (non inverted signal data frame)
      //  3 -X2-TDMA (inverted signal data frame)
      //  4 +DMR (non inverted signal data frame)
      //  5 -DMR (inverted signal data frame)
      //  6 +NXDN (non inverted data frame)
      //  7 -NXDN (inverted data frame)
      // 12 +DMR (non inverted signal voice frame)
      // 13 -DMR (inverted signal voice frame)
      // 14 +X2-TDMA (non inverted signal voice frame)
      // 15 -X2-TDMA (inverted signal voice frame)
      // 16 +NXDN (non inverted voice frame)
      // 17 -NXDN (inverted voice frame)
      unsigned int dibit;

      // Choose the symbol according to the regions delimited by center, umid and lmid
      if (symbol > state->center) {
          if (symbol > state->umid) {
              dibit = 1;               // +3
          } else {
              dibit = 0;               // +1
          }
      } else {
          if (symbol < state->lmid) {
              dibit = 3;               // -3
          } else {
              dibit = 2;               // -1
          }
      }

      if ((state->synctype & 1) == 1) { 
        dibit = invert_dibit(dibit);
      }
      *state->dibit_buf_p++ = dibit;
      return dibit;
  }
}

/**
 * \brief This important method reads the last analog signal value (getSymbol call) and digitizes it.
 * Depending of the ongoing transmission it in converted into a bit (0/1) or a di-bit (00/01/10/11).
 */
unsigned int
getDibit (dsd_opts* opts, dsd_state* state)
{
  unsigned int dibit;
  int symbol = getSymbol (opts, state, 1);
  state->sbuf[state->sidx] = symbol;

  // Increase sidx
  if (state->sidx == (state->ssize - 1)) {
      state->sidx = 0;
  } else {
      state->sidx++;
  }

  if (state->dibit_buf_p > state->dibit_buf + 1024) {
    state->dibit_buf_p = state->dibit_buf + 256;
  }

  dibit = digitize (state, symbol);
  return dibit;
}

void
skipDibit (dsd_opts * opts, dsd_state * state, int count)
{
  int i;

  for (i = 0; i < count; i++) {
      getDibit (opts, state);
  }
}

