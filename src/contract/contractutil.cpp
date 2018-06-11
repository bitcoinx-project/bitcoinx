#include "contractutil.h"
#include "crypto/ripemd160.h"
#include "crypto/sha256.h"

dev::Address ContractUtil::CreateContractAddr(const uint256& txHash, uint32_t outIdx)
{
    std::vector<unsigned char> rawData(txHash.begin(), txHash.end());

    std::vector<unsigned char> outIdxData;
    if (outIdxData.size() < sizeof(outIdx)) {
        outIdxData.resize(sizeof(outIdx));
    }
    std::memcpy(outIdxData.data(), &outIdx, sizeof(outIdx));

    rawData.insert(rawData.end(), outIdxData.begin(), outIdxData.end());

    std::vector<unsigned char> hash1(32);
    CSHA256().Write(rawData.data(), rawData.size()).Finalize(hash1.data());

    std::vector<unsigned char> hash2(20);
    CRIPEMD160().Write(hash1.data(), hash1.size()).Finalize(hash2.data());

    return dev::Address(hash2);
}

dev::Address ContractUtil::CreateContractAddr(const dev::u256& txHash, uint32_t outIdx)
{
    return CreateContractAddr(h256Touint(txHash), outIdx);
}

EthTransaction ContractUtil::CreateEthTransaction(const dev::u256 &value, const EthTransactionParams& params, const dev::Address &sender, const dev::h256 &txHash, uint32_t outIdx, bool bIsSend/* = false */)
{
    EthTransaction txEth;
    if (params.receiveAddress == dev::Address() && !bIsSend) {
        txEth = EthTransaction(value, params.gasPrice, params.gasLimit, params.code, dev::u256(0));
    } else {
        txEth = EthTransaction(value, params.gasPrice, params.gasLimit, params.receiveAddress, params.code, dev::u256(0));
    }
    txEth.forceSender(sender);
    txEth.SetHashWith(txHash);
    txEth.SetOutIdx(outIdx);
    txEth.SetParams(params);

    return txEth;
}