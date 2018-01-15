#ifndef BITCOIN_HASH_BLAKE2_H
#define BITCOIN_HASH_BLAKE2_H

#include <vector>
#include "serialize.h"
#include "streams.h"
#include "uint256.h"
#include "version.h"

class Blake2
{
public:
    static int hash2b(void *out, size_t outlen, const void *in, size_t inlen);

    /** Compute the 256-bit hash of an object's serialization. */
    template<typename T>
    static uint256 SerializeHash(const T& obj, int nType=SER_GETHASH, int nVersion=PROTOCOL_VERSION)
    {
        static const size_t HASH_SIZE = 32;

        CDataStream ss(nType, nVersion);
        ss << obj;

        uint8_t hash[HASH_SIZE];
        hash2b(hash, HASH_SIZE, ss.data(), ss.size());

        std::vector<uint8_t> vhash(hash, hash + HASH_SIZE);
        return ::uint256(vhash);
    }
};

#endif // BITCOIN_HASH_BLAKE2_H