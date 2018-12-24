// Copyright (c) 2018-2019, CUT coin
// Copyright (c) 2014-2018, The Monero Project
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "common/int-util.h"
#include "hash-ops.h"
#include "oaes_lib.h"
#include "variant2_int_sqrt.h"

#define MEMORY         (1 << 21) // 2MB scratchpad
#define ITER           (1 << 20)
#define AES_BLOCK_SIZE  16
#define AES_KEY_SIZE    32
#define INIT_SIZE_BLK   8
#define INIT_SIZE_BYTE (INIT_SIZE_BLK * AES_BLOCK_SIZE)

extern int aesb_single_round(const uint8_t *in, uint8_t*out, const uint8_t *expandedKey);
extern int aesb_pseudo_round(const uint8_t *in, uint8_t *out, const uint8_t *expandedKey);

void slow_hash_allocate_state(void)
{
  // Do nothing, this is just to maintain compatibility with the upgraded slow-hash.c
  return;
}

void slow_hash_free_state(void)
{
  // As above
  return;
}

static void (*const extra_hashes[4])(const void *, size_t, char *) = {
  hash_extra_blake, hash_extra_groestl, hash_extra_jh, hash_extra_skein
};

#define U64(x) ((uint64_t *) (x))

static void copy_block(void *dst, const void *src) {
  memcpy(dst, src, AES_BLOCK_SIZE);
}

static void xor_blocks(uint8_t *a, const uint8_t *b) {
  size_t i;
  for (i = 0; i < AES_BLOCK_SIZE; i++) {
    a[i] ^= b[i];
  }
}

static inline uint32_t rotate32(uint32_t a, uint32_t s) {
  // s can be 0, is s >> 32 a zero or undefined? It is probaly a, which is fine.
  return (a << s) | (a >> (32 - s));
}

static void rotate4x32(uint32_t *a, uint32_t s) {
  for (int i = 0; i < 4; ++i)
    a[i] = rotate32(a[i], s);
}

static void rotate4x32_3(uint32_t *d, const uint32_t *a, uint32_t s) {
  for (int i = 0; i < 4; ++i)
    d[i] = rotate32(a[i], s);
}

static void add4x32(uint32_t *a, const void *b) {
  for (int i = 0; i < 4; ++i)
    a[i] += ((uint32_t *)b)[i];
}

static void sub4x32(uint32_t *a, const void *b) {
  for (int i = 0; i < 4; ++i)
    a[i] -= ((uint32_t *)b)[i];
}

static void mul4x32(uint32_t *a, const uint32_t *b) {
  for (int i = 0; i < 4; ++i)
    a[i] *= b[i];
}

static void xor4x32(void *a, const void *b) {
  for (int i = 0; i < 4; ++i)
    ((uint32_t *)a)[i] ^= ((uint32_t *)b)[i];
}

#pragma pack(push, 1)
union cn_slow_hash_state {
  union hash_state hs;
  struct {
    uint8_t k[64];
    uint8_t init[INIT_SIZE_BYTE];
  };
};
#pragma pack(pop)

void cn_slow_hash(const void *data, size_t length, char *hash, int variant, int prehashed) {
#ifndef FORCE_USE_HEAP
  uint8_t long_state[MEMORY];
#else
  uint8_t *long_state = (uint8_t *)malloc(MEMORY);
#endif

  union cn_slow_hash_state state;
  uint8_t text[INIT_SIZE_BYTE];
  uint8_t a8[AES_BLOCK_SIZE];
  uint8_t b8[AES_BLOCK_SIZE];
  uint32_t a[4], b[4];
  size_t i, j;
  uint8_t aes_key[AES_KEY_SIZE];
  oaes_ctx *aes_ctx;

  if (prehashed) {
    memcpy(&state.hs, data, length);
  } else {
    hash_process(&state.hs, data, length);
  }
  memcpy(text, state.init, INIT_SIZE_BYTE);
  memcpy(aes_key, state.hs.b, AES_KEY_SIZE);
   aes_ctx = (oaes_ctx *) oaes_alloc();

  oaes_key_import_data(aes_ctx, aes_key, AES_KEY_SIZE);
  for (i = 0; i < MEMORY / INIT_SIZE_BYTE; i++) {
    for (j = 0; j < INIT_SIZE_BLK; j++) {
      aesb_pseudo_round(&text[AES_BLOCK_SIZE * j], &text[AES_BLOCK_SIZE * j], aes_ctx->key->exp_data);
    }
    memcpy(&long_state[i * INIT_SIZE_BYTE], text, INIT_SIZE_BYTE);
  }

  for (i = 0; i < 16; i++) {
    a8[i] = state.k[     i] ^ state.k[32 + i];
    b8[i] = state.k[16 + i] ^ state.k[48 + i];
  }

  copy_block(a, a8);
  copy_block(b, b8);

  for (i = 0; i < ITER / 2; i++) {
    uint32_t c[4], tmp[4];
    copy_block(c, long_state + (a[0] & 0x1FFFC0));
    rotate4x32(c, b[0] & 31);
    add4x32(c, long_state + (a[0] & 0x1FFFC0) + AES_BLOCK_SIZE);
    rotate4x32(c, b[1] & 31);
    add4x32(c, long_state + (a[0] & 0x1FFFC0) + AES_BLOCK_SIZE * 2);
    rotate4x32(c, b[2] & 31);
    add4x32(c, long_state + (a[0] & 0x1FFFC0) + AES_BLOCK_SIZE * 3);
    rotate4x32(c, b[3] & 31);

    aesb_single_round((uint8_t *)c, (uint8_t *)c, (uint8_t *)a);

    for(j = 0; j < 4; j++) {
      rotate4x32_3(tmp, b, a[j] & 31);
      add4x32(tmp, c);
      xor4x32(long_state + (a[0] & 0x1FFFC0) + j * AES_BLOCK_SIZE, tmp);
    }

    copy_block(tmp, long_state + (c[0] & 0x1FFFC0));
    rotate4x32(tmp, a[0] & 31);
    sub4x32(tmp, long_state + (c[0] & 0x1FFFC0) + AES_BLOCK_SIZE);
    rotate4x32(tmp, a[1] & 31);
    sub4x32(tmp, long_state + (c[0] & 0x1FFFC0) + AES_BLOCK_SIZE * 2);
    rotate4x32(tmp, a[2] & 31);
    sub4x32(tmp, long_state + (c[0] & 0x1FFFC0) + AES_BLOCK_SIZE * 3);
    rotate4x32(tmp, a[3] & 31);

    copy_block(b, c);

    mul4x32(c, tmp);
    add4x32(a, c);

    for(j = 0; j < 4; j ++) {
      rotate4x32_3(c, a, tmp[j] & 31);
      xor4x32(long_state + (b[0] & 0x1FFFC0) + j * AES_BLOCK_SIZE, c);
    }

    xor4x32(a, tmp);
  }

  memcpy(text, state.init, INIT_SIZE_BYTE);
  oaes_key_import_data(aes_ctx, &state.hs.b[32], AES_KEY_SIZE);
  for (i = 0; i < MEMORY / INIT_SIZE_BYTE; i++) {
    for (j = 0; j < INIT_SIZE_BLK; j++) {
      xor_blocks(&text[j * AES_BLOCK_SIZE], &long_state[i * INIT_SIZE_BYTE + j * AES_BLOCK_SIZE]);
      aesb_pseudo_round(&text[AES_BLOCK_SIZE * j], &text[AES_BLOCK_SIZE * j], aes_ctx->key->exp_data);
    }
  }
  memcpy(state.init, text, INIT_SIZE_BYTE);
  hash_permutation(&state.hs);
  /*memcpy(hash, &state, 32);*/
  extra_hashes[state.hs.b[0] & 3](&state, 200, hash);
  oaes_free((OAES_CTX **) &aes_ctx);
#ifdef FORCE_USE_HEAP
  free(long_state);
#endif
}

