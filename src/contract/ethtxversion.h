#ifndef BITCOINX_CONTRACT_ETHTXVERSION_H
#define BITCOINX_CONTRACT_ETHTXVERSION_H

#include <stdint.h>

struct EthTxVersion {
    EthTxVersion()
        : mRaw(0)
    {
    }

    uint32_t ToRaw() const
    {
        return mRaw;
    }

    bool operator==(const EthTxVersion &v) const
    {
        return mRaw == v.mRaw;
    }

    bool operator!=(const EthTxVersion &v) const
    {
        return mRaw != v.mRaw;
    }

    static EthTxVersion FromRaw(uint32_t data)
    {
        EthTxVersion v;
        v.mRaw = data;
        return v;
    }

    static EthTxVersion GetDefault()
    {
        EthTxVersion v;
        v.mRaw = 1;
        return v;
    }

    static EthTxVersion GetNoExec()
    {
        EthTxVersion v;
        v.mRaw = 0;
        return v;
    }

private:
    uint32_t mRaw;
};

#endif // BITCOINX_CONTRACT_ETHTXVERSION_H