#ifndef BITCOINX_CONTRACT_CONTRACTEXECUTOR_H
#define BITCOINX_CONTRACT_CONTRACTEXECUTOR_H

#include "ethstate.h"
#include "ethtransaction.h"
#include "ethtxversion.h"
#include "primitives/block.h"

struct ExecutionResult {
    uint64_t totalGasUsed = 0;
    CAmount totalRefund = 0;
    std::vector<CTxOut> refundTxOuts;
    std::vector<CTransaction> transferTxs;
};

class ContractExecutor
{
public:
    ContractExecutor(const CBlock& _block, const std::vector<EthTransaction>& _txs, const uint64_t _blockGasLimit)
        : mTxs(_txs), mBlock(_block), mBlockGasLimit(_blockGasLimit)
    {
    }

    bool Execut(dev::eth::Permanence type = dev::eth::Permanence::Committed);

    bool GetResult(ExecutionResult& result);
    const std::vector<EthExecutionResult>& GetEthResults() const { return mEthResults; }

    static std::vector<EthExecutionResult> Call(const dev::Address& addrContract, std::vector<unsigned char> opcode, const dev::Address& sender = dev::Address(), uint64_t gasLimit = 0);

private:
    dev::eth::EnvInfo makeEVMEnvironment();

    std::vector<EthTransaction> mTxs;
    const CBlock& mBlock;
    const uint64_t mBlockGasLimit;

    std::vector<EthExecutionResult> mEthResults;
};

#endif // BITCOINX_CONTRACT_CONTRACTEXECUTOR_H