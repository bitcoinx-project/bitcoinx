// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_AMOUNT_H
#define BITCOIN_AMOUNT_H

#include <stdint.h>

/** Amount in satoshis (Can be negative) */
typedef int64_t CAmount;

static const CAmount BTC_2_BCX_RATE = 10000;

static const CAmount COIN = 100000000 / BTC_2_BCX_RATE;


/** No amount larger than this (in satoshi) is valid.
 *
 * Note that this constant is *not* the total money supply, which in Bitcoin
 * currently happens to be less than 21,000,000 BTC for various reasons, but
 * rather a sanity check. As this sanity check is used by consensus-critical
 * validation code, the exact value of the MAX_MONEY constant is consensus
 * critical; in unusual circumstances like a(nother) overflow bug that allowed
 * for the creation of coins out of thin air modification could lead to a fork.
 * */
static const uint64_t MAX_MONEY = (uint64_t)21000000 * COIN * BTC_2_BCX_RATE;
inline bool MoneyRange(const CAmount& nValue) { return (nValue >= 0 && (uint64_t)nValue <= MAX_MONEY); }

#endif //  BITCOIN_AMOUNT_H
