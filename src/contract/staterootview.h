#ifndef BITCOINX_CONTRACT_STATEROOTVIEW_H
#define BITCOINX_CONTRACT_STATEROOTVIEW_H

#include "dbwrapper.h"
#include "fs.h"
#include "uint256.h"
#include <libdevcore/FixedHash.h>
#include "chainparams.h"

class StateRootView
{
public:
    static StateRootView* Init(const fs::path& path, bool fWipe = false);
    static StateRootView* Instance();
    static void Release();

    bool InitGenesis(const CChainParams& chainparams);
    bool SetRoot(const uint256& blockHash, const dev::h256& stateRoot, const dev::h256& utxoRoot);
    bool GetRoot(const uint256& blockHash, dev::h256& stateRoot, dev::h256& utxoRoot);

private:
    StateRootView(const fs::path& path, bool fWipe = false);
    StateRootView(const StateRootView&) = delete;
    StateRootView& operator=(const StateRootView&) = delete;
    ~StateRootView();

private:
    CDBWrapper mDB;

    static StateRootView* sInstance;
};

#endif // BITCOINX_CONTRACT_STATEROOTVIEW_H