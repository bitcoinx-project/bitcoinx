#ifndef BITCOINX_CONTRACT_ETHSTATE_H
#define BITCOINX_CONTRACT_ETHSTATE_H


#include "ethtransaction.h"
#include <crypto/ripemd160.h>
#include <crypto/sha256.h>
#include <libethereum/State.h>
#include <libevm/ExtVMFace.h>
#include <primitives/transaction.h>
#include <uint256.h>

#include <libethereum/ChainParams.h>
#include <libethcore/SealEngine.h>
#include <libethereum/Executive.h>

using OnOpFunc = std::function<void(uint64_t, uint64_t, dev::eth::Instruction, dev::bigint, dev::bigint, dev::bigint, dev::eth::VM*, dev::eth::ExtVMFace const*)>;

struct TransferInfo {
    dev::Address from;
    dev::Address to;
    dev::u256 value;
};

struct Vin {
    dev::h256 hash;
    uint32_t nVout;
    dev::u256 value;
    uint8_t alive;
};

struct EthExecutionResult {
    dev::eth::ExecutionResult execRes;
    dev::eth::TransactionReceipt txRec;
    CTransaction tx;
};

class EthState : public dev::eth::State
{
public:
    static EthState* Init();
    static EthState* Init(dev::u256 const& _accountStartNonce, dev::OverlayDB const& _db, const std::string& _path, dev::eth::BaseState _bs = dev::eth::BaseState::PreExisting);

    static EthState* Instance();
    static void Release();

    EthExecutionResult execute(dev::eth::EnvInfo const& _envInfo, EthTransaction const& _t, dev::eth::Permanence _p = dev::eth::Permanence::Committed, dev::eth::OnOpFunc const& _onOp = OnOpFunc());

    void populateFromGenesis()
    {
        populateFrom(mParams.genesisState);
    }

    void setUTXORoot(dev::h256 const& _r)
    {
        mUTXOCache.clear();
        mUTXOState.setRoot(_r);
    }

    dev::h256 rootHashUTXO() const { return mUTXOState.root(); }

    std::unordered_map<dev::Address, Vin> vins() const;

    dev::OverlayDB const& dbUtxo() const { return mUTXODB; }
    dev::OverlayDB& dbUtxo() { return mUTXODB; }

    void SetEngineSchedule()
    {
        if (mSealEngine != nullptr) {
            mSealEngine->setBCXSchedule(dev::eth::EIP158Schedule);
        }
    }

    void ClearEngineDeletionAddress()
    {
        if (mSealEngine != nullptr) {
            mSealEngine->mDeletionAddresses.clear();
        }
    }

    friend class TransferTxBuilder;

private:
    EthState();
    EthState(dev::u256 const& _accountStartNonce, dev::OverlayDB const& _db, const std::string& _path, dev::eth::BaseState _bs = dev::eth::BaseState::PreExisting);
    EthState(const EthState&) = delete;
    EthState& operator=(const EthState&) = delete;
    virtual ~EthState();

    virtual void transferBalance(dev::Address const& _from, dev::Address const& _to, dev::u256 const& _value) override;
    virtual void addBalance(dev::Address const& _id, dev::u256 const& _amount) override;
    virtual void kill(dev::Address _addr) override;

    const Vin* vin(const dev::Address& _addr);

    void deleteAccounts(const std::set<dev::Address>& addrs);

    void updateUTXO(const std::unordered_map<dev::Address, Vin>& vins);

private:
    static EthState* sInstance;

    dev::Address mNewAddress;

    std::vector<TransferInfo> mTransfers;

    dev::OverlayDB mUTXODB;
    dev::eth::SecureTrieDB<dev::Address, dev::OverlayDB> mUTXOState;
    std::unordered_map<dev::Address, Vin> mUTXOCache;

    dev::eth::ChainParams mParams;
    dev::eth::SealEngineFace* mSealEngine;
};



#endif // BITCOINX_CONTRACT_ETHSTATE_H
