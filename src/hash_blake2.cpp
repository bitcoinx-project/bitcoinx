#include "hash_blake2.h"

extern "C" {
#include "crypto/cblake2/blake2b-ref.c"
}

int Blake2::hash2b(void *out, size_t outlen, const void *in, size_t inlen)
{
    return blake2(out, outlen, in, inlen, nullptr, 0);
}