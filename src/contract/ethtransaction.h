#ifndef BITCOINX_CONTRACT_ETHTRANSACTION_H
#define BITCOINX_CONTRACT_ETHTRANSACTION_H

#include "ethtxversion.h"
#include <libethereum/Transaction.h>


using valtype = std::vector<unsigned char>;

struct EthTransactionParams {
    EthTransactionParams()
        : gasLimit(0), gasPrice(0)
    {
    }

    EthTxVersion version;
    dev::u256 gasLimit;
    dev::u256 gasPrice;
    valtype code;
    dev::Address receiveAddress;

    bool operator!=(const EthTransactionParams& params)
    {
        if (this->version != params.version || this->gasLimit != params.gasLimit ||
            this->gasPrice != params.gasPrice || this->code != params.code ||
            this->receiveAddress != params.receiveAddress) {
            return true;
        }
        return false;
    }
};

class EthTransaction : public dev::eth::Transaction
{
public:
    EthTransaction()
        : dev::eth::Transaction(), nOutIdx(0) {}

    EthTransaction(dev::u256 const& _value, dev::u256 const& _gasPrice, dev::u256 const& _gas, dev::bytes const& _data, dev::u256 const& _nonce = dev::Invalid256)
        : dev::eth::Transaction(_value, _gasPrice, _gas, _data, _nonce), nOutIdx(0) {}

    EthTransaction(dev::u256 const& _value, dev::u256 const& _gasPrice, dev::u256 const& _gas, dev::Address const& _dest, dev::bytes const& _data, dev::u256 const& _nonce = dev::Invalid256)
        : dev::eth::Transaction(_value, _gasPrice, _gas, _dest, _data, _nonce), nOutIdx(0) {}

    void SetHashWith(const dev::h256 hash) { m_hashWith = hash; }
    dev::h256 GetHashWith() const { return m_hashWith; }

    void SetOutIdx(uint32_t idx) { nOutIdx = idx; }
    uint32_t GetOutIdx() const { return nOutIdx; }

    void SetParams(const EthTransactionParams& p) { params = p; }
    const EthTransactionParams& GetParams() const { return params; }

    void SetVersion(const EthTxVersion& v) { params.version = v; }

private:
    uint32_t nOutIdx;
    EthTransactionParams params;
};

#endif // BITCOINX_CONTRACT_ETHTRANSACTION_H
