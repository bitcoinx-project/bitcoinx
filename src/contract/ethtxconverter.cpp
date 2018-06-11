#include "ethtxconverter.h"
#include "chainparams.h"
#include "contractutil.h"
#include "config.h"
#include "pubkey.h"
#include "script/interpreter.h"
#include "script/standard.h"
#include "util.h"
#include "validation.h"


bool EthTxConverter::Convert(std::vector<EthTransaction>& ethTxs)
{
    for (size_t i = 0; i < txBit.vout.size(); i++) {
        if (txBit.vout[i].scriptPubKey.HasCreateContractOp() || txBit.vout[i].scriptPubKey.HasSendToContractOp()) {
            if (receiveStack(txBit.vout[i].scriptPubKey)) {
                EthTransactionParams params;
                if (parseEthTxParams(params)) {
                    ethTxs.push_back(createEthTx(params, i));
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
    }
    return true;
}

bool EthTxConverter::receiveStack(const CScript& scriptPubKey)
{
    EvalScript(stack, scriptPubKey, SCRIPT_EXEC_BYTE_CODE, BaseSignatureChecker(), SIGVERSION_BASE, nullptr);
    if (stack.empty())
        return false;

    CScript scriptRest(stack.back().begin(), stack.back().end());
    stack.pop_back();

    opcode = (opcodetype)(*scriptRest.begin());
    if ((opcode == OP_CREATECONTRACT && stack.size() < 4) || (opcode == OP_SENDTOCONTRACT && stack.size() < 5)) {
        stack.clear();
        return false;
    }

    return true;
}

bool EthTxConverter::parseEthTxParams(EthTransactionParams& params)
{
    try {
        dev::Address receiveAddress;
        valtype vecAddr;
        if (opcode == OP_SENDTOCONTRACT) {
            vecAddr = stack.back();
            stack.pop_back();
            receiveAddress = dev::Address(vecAddr);
        }

        if (stack.size() < 4) {
            return false;
        }

        if (stack.back().size() < 1) {
            return false;
        }
        valtype code(stack.back());
        stack.pop_back();
        uint64_t gasPrice = CScriptNum::vch_to_uint64(stack.back());
        stack.pop_back();
        uint64_t gasLimit = CScriptNum::vch_to_uint64(stack.back());
        stack.pop_back();
        if (gasPrice > INT64_MAX || gasLimit > INT64_MAX) {
            return false;
        }
        //we track this as CAmount in some places, which is an int64_t, so constrain to INT64_MAX
        if (gasPrice != 0 && gasLimit > INT64_MAX / gasPrice) {
            //overflows past 64bits, reject this tx
            return false;
        }

        if (stack.back().size() > 4) {
            return false;
        }

        const EthTxVersion version = EthTxVersion::FromRaw((uint32_t)CScriptNum::vch_to_uint64(stack.back()));
        stack.pop_back();
        params.version = version;
        params.gasPrice = dev::u256(gasPrice);
        params.receiveAddress = receiveAddress;
        params.code = code;
        params.gasLimit = dev::u256(gasLimit);
        return true;
    } catch (const scriptnum_error& err) {
        LogPrintf("Incorrect parameters to VM.");
        return false;
    }
}

static valtype GetSenderAddress(const CTransaction& tx, const CCoinsViewCache* coinsView, const std::vector<CTransactionRef>* blockTxs)
{
    CScript script;
    // Can't use script.empty() because an empty script is technically valid
    bool scriptFilled = false;

    // First check the current (or in-progress) block for zero-confirmation change spending that won't yet be in txindex
    if (blockTxs) {
        for (auto btx : *blockTxs) {
            if (btx->GetHash() == tx.vin[0].prevout.hash) {
                script = btx->vout[tx.vin[0].prevout.n].scriptPubKey;
                scriptFilled = true;
                break;
            }
        }
    }
    if (!scriptFilled && coinsView) {
        script = coinsView->AccessCoin(tx.vin[0].prevout).out.scriptPubKey;
        scriptFilled = true;
    }
    if (!scriptFilled) {
        CTransactionRef txPrevout;
        uint256 hashBlock;
        if (GetTransaction(tx.vin[0].prevout.hash, txPrevout, Params().GetConsensus(), hashBlock, true)) {
            script = txPrevout->vout[tx.vin[0].prevout.n].scriptPubKey;
        } else {
            LogPrintf("Error fetching transaction details of tx %s. This will probably cause more errors", tx.vin[0].prevout.hash.ToString());
            return valtype();
        }
    }

    CTxDestination addressBit;
    txnouttype txType = TX_NONSTANDARD;
    if (ExtractDestination(script, addressBit, &txType)) {
        if ((txType == TX_PUBKEY || txType == TX_PUBKEYHASH) &&
            addressBit.type() == typeid(CKeyID)) {
            CKeyID senderAddress(boost::get<CKeyID>(addressBit));
            return valtype(senderAddress.begin(), senderAddress.end());
        }
    }
    // Prevout is not a standard transaction format, so just return 0
    return valtype();
}

EthTransaction EthTxConverter::createEthTx(const EthTransactionParams& params, uint32_t outIdx)
{
    const dev::u256 value(txBit.vout[outIdx].nValue * SATOSHI_2_WEI_RATE);
    const dev::Address sender(GetSenderAddress(txBit, view, blockTransactions));
    const dev::h256 txHash(uintToh256(txBit.GetHash()));
    const bool bIsSend = (opcode == OP_SENDTOCONTRACT);
    return ContractUtil::CreateEthTransaction(value, params, sender, txHash, outIdx, bIsSend);
}
