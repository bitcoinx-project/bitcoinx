#ifndef BITCOINX_CONTRACT_VMLOG_H
#define BITCOINX_CONTRACT_VMLOG_H

#include "ethstate.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include <vector>

class VMLog
{
public:
    static void Init();
    static void Write(const std::vector<EthExecutionResult>& res, const CTransaction& tx = CTransaction(), const CBlock& block = CBlock());

private:
    static bool fVMlogFileExist;
};


#endif // BITCOINX_CONTRACT_VMLOG_H