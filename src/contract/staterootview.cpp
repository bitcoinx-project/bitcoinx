#include "staterootview.h"
#include <libdevcore/RLP.h>
#include <libdevcore/SHA3.h>

//! memory allocated to state root DB specific cache (8 MiB)
static const int64_t DB_CACHE_SIZE = 8 << 20;

static const dev::h256 DEFAULT_ROOT = dev::sha3(dev::rlp(""));


StateRootView* StateRootView::sInstance = nullptr;

StateRootView* StateRootView::Init(const fs::path& path, bool fWipe)
{
    if (sInstance == nullptr) {
        sInstance = new StateRootView(path, fWipe);
    }
    return sInstance;
}

StateRootView* StateRootView::Instance()
{
    assert(sInstance != nullptr);
    return sInstance;
}

void StateRootView::Release()
{
    if (sInstance != nullptr) {
        delete sInstance;
        sInstance = nullptr;
    }
}

StateRootView::StateRootView(const fs::path& path, bool fWipe)
    : mDB(path / "stateroot", DB_CACHE_SIZE, false, fWipe, true)
{
}

StateRootView::~StateRootView()
{
}

bool StateRootView::InitGenesis(const CChainParams& chainparams)
{
    return SetRoot(chainparams.GenesisBlock().GetHash(), DEFAULT_ROOT, DEFAULT_ROOT);
}

bool StateRootView::SetRoot(const uint256& blockHash, const dev::h256& stateRoot, const dev::h256& utxoRoot)
{
    const std::pair<uint256, uint256> root(h256Touint(stateRoot), h256Touint(utxoRoot));
    return mDB.Write(blockHash, root);
}

bool StateRootView::GetRoot(const uint256& blockHash, dev::h256& stateRoot, dev::h256& utxoRoot)
{
    std::pair<uint256, uint256> root;
    if (mDB.Read(blockHash, root)) {
        stateRoot = uintToh256(root.first);
        utxoRoot = uintToh256(root.second);
        return true;
    }

    return false;
}