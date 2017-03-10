// keccak.h
// 19-Nov-11  Markku-Juhani O. Saarinen <mjos@iki.fi>

#ifndef KECCAK_H
#define KECCAK_H

#include <stdint.h>
#include <string.h>

#ifndef KECCAK_ROUNDS
#define KECCAK_ROUNDS 24
#endif

#ifndef ROTL64
#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))
#endif

#if defined(__cplusplus)
extern "C" {
#endif

// compute a keccak hash (md) of given byte length from "in"
int keccak(const uint8_t *in, size_t inlen, uint8_t *md, size_t mdlen);

// update the state
void keccakf(uint64_t st[25], int norounds);

#if defined(__cplusplus)
}
#endif

#endif

