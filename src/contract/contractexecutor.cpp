#include "contractexecutor.h"
#include "chainparams.h"
#include "config.h"
#include "pubkey.h"
#include "script/standard.h"
#include "timedata.h"
#include "util.h"
#include "validation.h"


bool ContractExecutor::Execut(dev::eth::Permanence type)
{
    for (EthTransaction& tx : mTxs) {
        if (tx.GetParams().version != EthTxVersion::GetDefault()) {
            return false;
        }

        const dev::eth::EnvInfo& envInfo = makeEVMEnvironment();
        if (!tx.isCreation() && !EthState::Instance()->addressInUse(tx.receiveAddress())) {
            dev::eth::ExecutionResult execRes;
            execRes.excepted = dev::eth::TransactionException::Unknown;
            mEthResults.push_back(EthExecutionResult{execRes, dev::eth::TransactionReceipt(dev::h256(), dev::u256(), dev::eth::LogEntries()), CTransaction()});
            continue;
        }
        mEthResults.push_back(EthState::Instance()->execute(envInfo, tx, type, OnOpFunc()));
    }
    EthState::Instance()->db().commit();
    EthState::Instance()->dbUtxo().commit();
    EthState::Instance()->ClearEngineDeletionAddress();
    return true;
}

bool ContractExecutor::GetResult(ExecutionResult& result)
{
    for (size_t i = 0; i < mEthResults.size(); i++) {
        const EthExecutionResult& ethResult = mEthResults[i];
        const uint64_t gasUsed = (uint64_t)ethResult.execRes.gasUsed;
        if (ethResult.execRes.excepted == dev::eth::TransactionException::None) {
            if (mTxs[i].gas() > UINT64_MAX ||
                ethResult.execRes.gasUsed > UINT64_MAX ||
                mTxs[i].gasPrice() > UINT64_MAX) {
                return false;
            }
            const uint64_t gas = (uint64_t)mTxs[i].gas();
            const uint64_t gasPrice = (uint64_t)mTxs[i].gasPrice();

            const int64_t amount = (gas - gasUsed) * gasPrice / SATOSHI_2_WEI_RATE;
            if (amount < 0) {
                return false;
            }

            if (amount > 0) {
                CScript script(CScript() << OP_DUP << OP_HASH160 << mTxs[i].sender().asBytes() << OP_EQUALVERIFY << OP_CHECKSIG);
                result.refundTxOuts.push_back(CTxOut(amount, script));
                result.totalRefund += amount;
            }
        } else {
            if (mTxs[i].value() > 0) {
                CMutableTransaction tx;
                tx.vin.push_back(CTxIn(h256Touint(mTxs[i].GetHashWith()), mTxs[i].GetOutIdx(), CScript() << OP_SPEND));
                CScript script(CScript() << OP_DUP << OP_HASH160 << mTxs[i].sender().asBytes() << OP_EQUALVERIFY << OP_CHECKSIG);
                tx.vout.push_back(CTxOut(CAmount(mTxs[i].value()), script));
                result.transferTxs.push_back(CTransaction(tx));
            }
        }
        result.totalGasUsed += gasUsed;

        if (ethResult.tx != CTransaction()) {
            result.transferTxs.push_back(ethResult.tx);
        }
    }
    return true;
}

std::vector<EthExecutionResult> ContractExecutor::Call(const dev::Address& addrContract, std::vector<unsigned char> opcode, const dev::Address& sender, uint64_t gasLimit)
{
    CBlock block;
    CMutableTransaction tx;

    CBlockIndex* pblockindex = mapBlockIndex[chainActive.Tip()->GetBlockHash()];
    ReadBlockFromDisk(block, pblockindex, Params().GetConsensus());
    block.nTime = GetAdjustedTime();
    block.vtx.erase(block.vtx.begin() + 1, block.vtx.end());

    const uint64_t blockGasLimit = DEFAULT_BLOCK_GAS_LIMIT;
    if (gasLimit == 0) {
        gasLimit = blockGasLimit - 1;
    }

    dev::Address senderAddress = sender;
    if (senderAddress == dev::Address()) {
        senderAddress = dev::Address("ffffffffffffffffffffffffffffffffffffffff");
    }
    tx.vout.push_back(CTxOut(0, CScript() << OP_DUP << OP_HASH160 << senderAddress.asBytes() << OP_EQUALVERIFY << OP_CHECKSIG));
    block.vtx.push_back(MakeTransactionRef(CTransaction(tx)));

    EthTransaction callTransaction(0, 1, dev::u256(gasLimit), addrContract, opcode, dev::u256(0));
    callTransaction.forceSender(senderAddress);
    callTransaction.SetVersion(EthTxVersion::GetDefault());

    ContractExecutor executor(block, std::vector<EthTransaction>(1, callTransaction), blockGasLimit);
    executor.Execut(dev::eth::Permanence::Reverted);
    return executor.GetEthResults();
}

static dev::Address makeEthAddress(const CScript& script)
{
    CTxDestination address;
    txnouttype txType = TX_NONSTANDARD;
    if (ExtractDestination(script, address, &txType)) {
        if ((txType == TX_PUBKEY || txType == TX_PUBKEYHASH) && address.type() == typeid(CKeyID)) {
            CKeyID addrKey(boost::get<CKeyID>(address));
            std::vector<unsigned char> rawData(addrKey.begin(), addrKey.end());
            return dev::Address(rawData);
        }
    }
    // if not standard or not a pubkey or pubkeyhash output, then return 0
    return dev::Address();
}

dev::eth::EnvInfo ContractExecutor::makeEVMEnvironment()
{
    dev::eth::EnvInfo env;
    CBlockIndex* tip = chainActive.Tip();
    env.setNumber(dev::u256(tip->nHeight + 1));
    env.setTimestamp(dev::u256(mBlock.nTime));
    env.setDifficulty(dev::u256(mBlock.nBits));

    dev::eth::LastHashes lh;
    lh.resize(256);
    for (int i = 0; i < 256; i++) {
        if (!tip) {
            break;
        }
        lh[i] = uintToh256(*tip->phashBlock);
        tip = tip->pprev;
    }
    env.setLastHashes(std::move(lh));
    env.setGasLimit(mBlockGasLimit);
    env.setAuthor(makeEthAddress(mBlock.vtx[0]->vout[0].scriptPubKey));
    return env;
}
