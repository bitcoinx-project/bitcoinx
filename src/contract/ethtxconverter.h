#ifndef BITCOINX_CONTRACT_ETHTxCONVERTER_H
#define BITCOINX_CONTRACT_ETHTxCONVERTER_H

#include "coins.h"
#include "ethtransaction.h"
#include "ethtxversion.h"
#include "primitives/transaction.h"

class EthTxConverter
{
public:
    EthTxConverter(CTransaction tx, CCoinsViewCache* v = NULL, const std::vector<CTransactionRef>* blockTxs = NULL)
        : txBit(tx), view(v), blockTransactions(blockTxs)
    {
    }

    bool Convert(std::vector<EthTransaction>& ethTxs);

private:
    bool receiveStack(const CScript& scriptPubKey);
    bool parseEthTxParams(EthTransactionParams& params);

    EthTransaction createEthTx(const EthTransactionParams& params, uint32_t outIdx);

    const CTransaction txBit;
    const CCoinsViewCache* view;
    std::vector<valtype> stack;
    opcodetype opcode;
    const std::vector<CTransactionRef>* blockTransactions;
};

#endif // BITCOINX_CONTRACT_ETHTxCONVERTER_H