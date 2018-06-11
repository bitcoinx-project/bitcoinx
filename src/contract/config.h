#ifndef BITCOINX_CONTRACT_CONFIG_H
#define BITCOINX_CONTRACT_CONFIG_H

#include <cstddef>
#include <stdint.h>

/** Minimum gas limit that is allowed in a transaction within a block - prevent
 * various types of tx and mempool spam **/
static const uint64_t MINIMUM_GAS_LIMIT = 10000;

static const uint64_t MEMPOOL_MIN_GAS_LIMIT = 20000;

static const uint64_t DEFAULT_GAS_LIMIT_OP_CREATE = 500000;
static const uint64_t DEFAULT_GAS_LIMIT_OP_SEND = 50000;

static const uint64_t MAX_BLOCK_GAS_LIMIT = 1000000000;
static const uint64_t DEFAULT_BLOCK_GAS_LIMIT = 40000000;

// Gas price
static const uint64_t MIN_GAS_PRICE = 40;
static const uint64_t DEFAULT_GAS_PRICE = 40;
static const uint64_t MAX_RPC_GAS_PRICE = 100;

// Contract executions with less gas than this are not standard
// Make sure is always equal or greater than MINIMUM_GAS_LIMIT (which we can't reference here due to insane header dependency chains)
static const uint64_t STANDARD_MINIMUM_GAS_LIMIT = 10000;

// contract executions with a price cheaper than this (in satoshis) are not standard
static const uint64_t STANDARD_MINIMUM_GAS_PRICE = 1;

static const size_t MAX_CONTRACT_TXOUT_NUM = 1000;

// Satoshis to Weis rate
static const uint64_t SATOSHI_2_WEI_RATE = 10000;


#endif // BITCOINX_CONTRACT_CONFIG_H
