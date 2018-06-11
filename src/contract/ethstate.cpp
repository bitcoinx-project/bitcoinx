#include "ethstate.h"
#include "config.h"
#include "contractutil.h"
#include <libethashseal/GenesisInfo.h>
#include <sstream>
#include <util.h>
#include <validation.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

using valtype = std::vector<unsigned char>;

struct BalanceChange {
    BalanceChange()
        : mAdd(0), mSub(0) {}
    BalanceChange(dev::u256 add, dev::u256 sub)
        : mAdd(add), mSub(sub) {}
    dev::u256 mAdd;
    dev::u256 mSub;
};

class TransferTxBuilder
{
public:
    TransferTxBuilder(EthState* _state, const std::vector<TransferInfo>& _transfers, const EthTransaction& _transaction, std::set<dev::Address> _deletionAddresses = std::set<dev::Address>())
        : mState(_state), mTransfers(_transfers), mTransaction(_transaction), mDeletionAddresses(_deletionAddresses)
    {
    }

    CTransaction Build()
    {
        initVins();
        initBalanceChange();
        if (!initNewBalances()) {
            return CTransaction();
        }

        CMutableTransaction tx;
        tx.vin = createVin();
        tx.vout = createVout();

        if (tx.vin.empty() || tx.vout.empty()) {
            return CTransaction();
        }
        return CTransaction(tx);
    }

    std::unordered_map<dev::Address, Vin> GetNewVins(const uint256& txHash)
    {
        std::unordered_map<dev::Address, Vin> vins;
        for (const auto& b : mBalances) {
            if (b.first == mTransaction.sender()) {
                continue;
            }

            if (b.second > 0) {
                vins[b.first] = Vin{uintToh256(txHash), mOutIdxs[b.first], b.second, 1};
            } else {
                vins[b.first] = Vin{uintToh256(txHash), 0, 0, 0};
            }
        }
        return vins;
    }

    bool ReachedVoutLimit() { return mVoutOverflow; }

private:
    void initVins()
    {
        for (const TransferInfo& ti : mTransfers) {
            if (!mVins.count(ti.from)) {
                if (ti.from == mTransaction.sender() && mTransaction.value() > 0) {
                    mVins[ti.from] = Vin{
                        mTransaction.GetHashWith(),
                        mTransaction.GetOutIdx(),
                        mTransaction.value(),
                        1,
                    };
                } else if (const auto a = mState->vin(ti.from)) {
                    mVins[ti.from] = *a;
                }
            }

            if (!mVins.count(ti.to)) {
                if (const auto a = mState->vin(ti.to)) {
                    mVins[ti.to] = *a;
                }
            }
        }
    }

    void initBalanceChange()
    {
        for (const TransferInfo& ti : mTransfers) {
            if (!mBalanceChanges.count(ti.from)) {
                mBalanceChanges[ti.from] = {0, ti.value};
            } else {
                mBalanceChanges[ti.from] = {mBalanceChanges[ti.from].mAdd, mBalanceChanges[ti.from].mSub + ti.value};
            }

            if (!mBalanceChanges.count(ti.to)) {
                mBalanceChanges[ti.to] = {ti.value, 0};
            } else {
                mBalanceChanges[ti.to] = {mBalanceChanges[ti.to].mAdd + ti.value, mBalanceChanges[ti.to].mSub};
            }
        }
    }

    bool initNewBalances()
    {
        for (const auto& infos : mBalanceChanges) {
            dev::u256 balance = 0;
            if ((mVins.count(infos.first) && mVins[infos.first].alive) ||
                (!mVins[infos.first].alive && !isDeletionAddress(infos.first))) {
                balance = mVins[infos.first].value;
            }
            balance += infos.second.mAdd;
            if (balance < infos.second.mSub) {
                return false;
            }
            balance -= infos.second.mSub;
            mBalances[infos.first] = balance;
        }
        return true;
    }

    std::vector<CTxIn> createVin()
    {
        std::vector<CTxIn> vin;
        for (const auto& v : mVins) {
            if ((v.second.value > 0 && v.second.alive) ||
                (v.second.value > 0 && !mVins[v.first].alive && !isDeletionAddress(v.first))) {
                vin.push_back(CTxIn(h256Touint(v.second.hash), v.second.nVout, CScript() << OP_SPEND));
            }
        }
        return vin;
    }

    std::vector<CTxOut> createVout()
    {
        size_t idx = 0;
        std::vector<CTxOut> vout;
        for (const auto& b : mBalances) {
            if (b.second > 0) {
                CScript script;
                const auto* a = mState->account(b.first);
                if (a && a->isAlive()) {
                    // create a no-exec contract output
                    script = CScript() << valtype{0} << valtype{0} << valtype{0} << valtype{0} << b.first.asBytes() << OP_SENDTOCONTRACT;
                } else {
                    script = CScript() << OP_DUP << OP_HASH160 << b.first.asBytes() << OP_EQUALVERIFY << OP_CHECKSIG;
                }
                vout.push_back(CTxOut(CAmount(b.second / SATOSHI_2_WEI_RATE), script));
                mOutIdxs[b.first] = idx;
                idx++;
            }

            if (idx > MAX_CONTRACT_TXOUT_NUM) {
                mVoutOverflow = true;
                return vout;
            }
        }
        return vout;
    }

    bool isDeletionAddress(const dev::Address& addr)
    {
        return mDeletionAddresses.count(addr) != 0;
    }

private:
    EthState* mState;
    const std::vector<TransferInfo>& mTransfers;
    const EthTransaction& mTransaction;
    const std::set<dev::Address> mDeletionAddresses;

    std::map<dev::Address, Vin> mVins;
    std::map<dev::Address, BalanceChange> mBalanceChanges;
    std::map<dev::Address, dev::u256> mBalances;
    std::map<dev::Address, uint32_t> mOutIdxs;
    bool mVoutOverflow = false;
};

//
// EthState
//
EthState* EthState::sInstance = nullptr;

EthState* EthState::Init()
{
    if (sInstance == nullptr) {
        sInstance = new EthState();
    }
    return sInstance;
}

EthState* EthState::Init(u256 const& _accountStartNonce, OverlayDB const& _db, const string& _path, BaseState _bs)
{
    if (sInstance == nullptr) {
        sInstance = new EthState(_accountStartNonce, _db, _path, _bs);
    }
    return sInstance;
}

EthState* EthState::Instance()
{
    assert(sInstance != nullptr);
    return sInstance;
}

void EthState::Release()
{
    if (sInstance != nullptr) {
        delete sInstance;
        sInstance = nullptr;
    }
}

EthState::EthState()
    : State(dev::Invalid256, dev::OverlayDB(), dev::eth::BaseState::PreExisting), mParams((dev::eth::genesisInfo(dev::eth::Network::BCXMainNetwork)))
{
    mUTXODB = OverlayDB();
    mUTXOState = SecureTrieDB<Address, OverlayDB>(&mUTXODB);

    mSealEngine = mParams.createSealEngine();
}

EthState::EthState(u256 const& _accountStartNonce, OverlayDB const& _db, const string& _path, BaseState _bs)
    : State(_accountStartNonce, _db, _bs), mParams((dev::eth::genesisInfo(dev::eth::Network::BCXMainNetwork)))
{
    mUTXODB = EthState::openDB(_path + "/utxo", sha3(rlp("")), WithExisting::Trust);
    mUTXOState = SecureTrieDB<Address, OverlayDB>(&mUTXODB);

    mSealEngine = mParams.createSealEngine();
}

EthState::~EthState()
{
    if (mSealEngine != nullptr) {
        delete mSealEngine;
        mSealEngine = nullptr;
    }
}

template <class DB>
dev::AddressHash commitUTXOCache(std::unordered_map<dev::Address, Vin> const& _cache, dev::eth::SecureTrieDB<dev::Address, DB>& _state)
{
    dev::AddressHash ret;
    for (auto const& i : _cache) {
        if (i.second.alive == 0) {
            _state.remove(i.first);
        } else {
            dev::RLPStream s(4);
            s << i.second.hash << i.second.nVout << i.second.value << i.second.alive;
            _state.insert(i.first, &s.out());
        }
        ret.insert(i.first);
    }
    return ret;
}

static void printException(const EthTransaction& _t, const TransactionException& _e)
{
    std::stringstream ss;
    ss << _e;
    error("EthState::execute, VM exception: %s, txHash=%s, outIdx=%d", ss.str(), h256Touint(_t.GetHashWith()).ToString(), _t.GetOutIdx());
}

EthExecutionResult EthState::execute(EnvInfo const& _envInfo, EthTransaction const& _t, Permanence _p, OnOpFunc const& _onOp)
{
    assert(_t.GetParams().version == EthTxVersion::GetDefault());

    addBalance(_t.sender(), _t.value() + (_t.gas() * _t.gasPrice()));

    if (_t.isCreation()) {
        mNewAddress = ContractUtil::CreateContractAddr(_t.GetHashWith(), _t.GetOutIdx());
    } else {
        mNewAddress = dev::Address();
    }

    mSealEngine->mDeletionAddresses.insert({_t.sender(), _envInfo.author()});

    const h256& oldStateRoot = rootHash();
    bool reachedVoutLimit = false;

    auto onOp = _onOp;
#if ETH_VMTRACE
    if (isChannelVisible<VMTraceChannel>()) {
        onOp = Executive::simpleTrace(); // override tracer
    }
#endif
    // Create and initialize the executive. This will throw fairly cheaply and quickly if the
    // transaction is bad in any way.
    Executive e(*this, _envInfo, *mSealEngine);
    ExecutionResult exeResult;
    e.setResultRecipient(exeResult);

    CTransactionRef transferTx;
    u256 startGasUsed;
    try {
        if (_t.isCreation() && _t.value()) {
            BOOST_THROW_EXCEPTION(CreateWithValue());
        }

        e.initialize(_t);
        // OK - transaction looks valid - execute.
        startGasUsed = _envInfo.gasUsed();
        if (e.execute()) {
            e.revert();
            throw Exception();
        } else {
            e.go(onOp);
        }

        e.finalize();

        if (_p == Permanence::Reverted) {
            m_cache.clear();
            mUTXOCache.clear();
        } else {
            deleteAccounts(mSealEngine->mDeletionAddresses);
            if (exeResult.excepted == TransactionException::None) {
                TransferTxBuilder builder(this, mTransfers, _t, mSealEngine->mDeletionAddresses);
                transferTx = MakeTransactionRef(builder.Build());
                if (builder.ReachedVoutLimit()) {
                    reachedVoutLimit = true;
                    e.revert();
                    throw Exception();
                }
                const std::unordered_map<dev::Address, Vin>& vins = builder.GetNewVins(transferTx->GetHash());
                updateUTXO(vins);
            } else {
                printException(_t, exeResult.excepted);
            }

            commitUTXOCache(mUTXOCache, mUTXOState);
            mUTXOCache.clear();
            const bool removeEmptyAccounts = _envInfo.number() >= mSealEngine->chainParams().u256Param("EIP158ForkBlock");
            commit(removeEmptyAccounts ? State::CommitBehaviour::RemoveEmptyAccounts : State::CommitBehaviour::KeepEmptyAccounts);
        }
    } catch (Exception const& _e) {
        exeResult.excepted = dev::eth::toTransactionException(_e);
        printException(_t, exeResult.excepted);
        exeResult.gasUsed = _t.gas();
        if (_p == Permanence::Reverted) {
            m_cache.clear();
            mUTXOCache.clear();
        } else {
            deleteAccounts(mSealEngine->mDeletionAddresses);
            commit(CommitBehaviour::RemoveEmptyAccounts);
        }
    }

    if (!_t.isCreation()) {
        exeResult.newAddress = _t.receiveAddress();
    }

    mNewAddress = dev::Address();
    mTransfers.clear();

    if (!reachedVoutLimit) {
        return EthExecutionResult{
            exeResult,
            dev::eth::TransactionReceipt(rootHash(), startGasUsed + e.gasUsed(), e.logs()),
            transferTx == nullptr ? CTransaction() : *transferTx,
        };
    }

    // Use old and empty states to create virtual Out Of Gas exception
    const u256& gas = _t.gas();
    exeResult = ExecutionResult();
    exeResult.gasRefunded = 0;
    exeResult.gasUsed = gas;
    exeResult.excepted = TransactionException();
    // Create a refund tx to send back any coins that were suppose to be sent to the contract
    CMutableTransaction refundTx;
    if (_t.value() > 0) {
        refundTx.vin.push_back(CTxIn(h256Touint(_t.GetHashWith()), _t.GetOutIdx(), CScript() << OP_SPEND));
        // Note, if sender was a non-standard tx, this will send the coins to pubkeyhash 0x00,
        // effectively destroying the coins
        CScript script(CScript() << OP_DUP << OP_HASH160 << _t.sender().asBytes() << OP_EQUALVERIFY << OP_CHECKSIG);
        refundTx.vout.push_back(CTxOut(CAmount(_t.value().convert_to<uint64_t>() / SATOSHI_2_WEI_RATE), script));
    }
    // Make sure to use empty transaction if no vouts made
    return EthExecutionResult{
        exeResult,
        dev::eth::TransactionReceipt(oldStateRoot, gas, e.logs()),
        refundTx.vout.empty() ? CTransaction() : CTransaction(refundTx),
    };
}

void EthState::transferBalance(dev::Address const& _from, dev::Address const& _to, dev::u256 const& _value)
{
    subBalance(_from, _value);
    addBalance(_to, _value);
    if (_value > 0) {
        mTransfers.push_back({_from, _to, _value});
    }
}

void EthState::addBalance(dev::Address const& _id, dev::u256 const& _amount)
{
    if (dev::eth::Account* a = account(_id)) {
        // Log empty account being touched. Empty touched accounts are cleared
        // after the transaction, so this event must be also reverted.
        // We only log the first touch (not dirty yet), and only for empty
        // accounts, as other accounts does not matter.
        // TODO: to save space we can combine this event with Balance by having
        //       Balance and Balance+Touch events.
        if (!a->isDirty() && a->isEmpty()) {
            m_changeLog.emplace_back(dev::eth::detail::Change::Touch, _id);
        }

        // Increase the account balance. This also is done for value 0 to mark
        // the account as dirty. Dirty account are not removed from the cache
        // and are cleared if empty at the end of the transaction.
        a->addBalance(_amount);
    } else {
        if (mNewAddress != dev::Address() && !addressInUse(mNewAddress)) {
            const_cast<dev::Address&>(_id) = mNewAddress;
            mNewAddress = dev::Address();
        }
        createAccount(_id, {requireAccountStartNonce(), _amount});
    }

    if (_amount != 0) {
        m_changeLog.emplace_back(dev::eth::detail::Change::Balance, _id, _amount);
    }
}

void EthState::kill(dev::Address _addr)
{
    dev::eth::State::kill(_addr);
    if (Vin* v = const_cast<Vin*>(vin(_addr))) {
        v->alive = 0;
    }
}

const Vin* EthState::vin(const dev::Address& _addr)
{
    const auto it = mUTXOCache.find(_addr);
    if (it == mUTXOCache.end()) {
        const std::string& stateBack = mUTXOState.at(_addr);
        if (stateBack.empty()) {
            return nullptr;
        }

        dev::RLP state(stateBack);
        const auto i = mUTXOCache.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(_addr),
            std::forward_as_tuple(Vin{
                state[0].toHash<dev::h256>(),
                state[1].toInt<uint32_t>(),
                state[2].toInt<dev::u256>(),
                state[3].toInt<uint8_t>(),
            }));
        return &i.first->second;
    }
    return &it->second;
}

std::unordered_map<dev::Address, Vin> EthState::vins() const
{
    std::unordered_map<dev::Address, Vin> ret;
    for (auto& i: mUTXOCache) {
        if (i.second.alive) {
            ret[i.first] = i.second;
        }
    }
    auto addrs = addresses();
    for (auto& i : addrs) {
        if (mUTXOCache.find(i.first) != mUTXOCache.end()) {
            continue;
        }
        auto pvin = const_cast<Vin*>(const_cast<EthState*>(this)->vin(i.first));
        if (pvin){
            ret[i.first] = *pvin;
        }
    }
    return ret;
}

void EthState::deleteAccounts(const std::set<dev::Address>& addrs)
{
    for (const dev::Address& addr : addrs) {
        kill(addr);
    }
}

void EthState::updateUTXO(const std::unordered_map<dev::Address, Vin>& vins)
{
    for (const auto& v : vins) {
        Vin* vi = const_cast<Vin*>(vin(v.first));
        if (vi != nullptr) {
            vi->hash = v.second.hash;
            vi->nVout = v.second.nVout;
            vi->value = v.second.value;
            vi->alive = v.second.alive;
        } else if (v.second.alive > 0) {
            mUTXOCache[v.first] = v.second;
        }
    }
}
