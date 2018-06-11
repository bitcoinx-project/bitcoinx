#ifndef BITCOINX_CONTRACT_CONTRACTUTIL_H
#define BITCOINX_CONTRACT_CONTRACTUTIL_H

#include "ethtransaction.h"
#include "uint256.h"
#include <libdevcrypto/Common.h>

class ContractUtil
{
public:
    static dev::Address CreateContractAddr(const uint256& txHash, uint32_t outIdx);
    static dev::Address CreateContractAddr(const dev::u256& txHash, uint32_t outIdx);

    static EthTransaction CreateEthTransaction(const dev::u256& value, const EthTransactionParams& params, const dev::Address& sender, const dev::h256& txHash, uint32_t outIdx, bool bIsSend = false);
};

#endif // BITCOINX_CONTRACT_CONTRACTUTIL_H