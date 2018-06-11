// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "test_bitcoin.h"

#include "chainparams.h"
#include "consensus/consensus.h"
#include "consensus/validation.h"
#include "crypto/sha256.h"
#include "fs.h"
#include "key.h"
#include "validation.h"
#include "miner.h"
#include "net_processing.h"
#include "pubkey.h"
#include "random.h"
#include "txdb.h"
#include "txmempool.h"
#include "ui_interface.h"
#include "rpc/server.h"
#include "rpc/register.h"
#include "script/sigcache.h"

#include <memory>

#include <boost/filesystem.hpp>
#include <libethashseal/Ethash.h>
#include "contract/config.h"
#include "contract/contractutil.h"
#include "contract/ethstate.h"
#include "contract/staterootview.h"
#include "contract/txexecrecord.h"

void CConnmanTest::AddNode(CNode& node)
{
    LOCK(g_connman->cs_vNodes);
    g_connman->vNodes.push_back(&node);
}

void CConnmanTest::ClearNodes()
{
    LOCK(g_connman->cs_vNodes);
    g_connman->vNodes.clear();
}

uint256 insecure_rand_seed = GetRandHash();
FastRandomContext insecure_rand_ctx(insecure_rand_seed);

extern bool fPrintToConsole;
extern void noui_connect();

BasicTestingSetup::BasicTestingSetup(const std::string& chainName)
{
        SHA256AutoDetect();
        RandomInit();
        ECC_Start();
        SetupEnvironment();
        SetupNetworking();
        InitSignatureCache();
        InitScriptExecutionCache();
        fPrintToDebugLog = false; // don't want to write to debug.log file
        fCheckBlockIndex = true;
        SelectParams(chainName);
        noui_connect();
}

BasicTestingSetup::~BasicTestingSetup()
{
        ECC_Stop();
}

TestingSetup::TestingSetup(const std::string& chainName) : BasicTestingSetup(chainName)
{
    const CChainParams& chainparams = Params();
        // Ideally we'd move all the RPC tests to the functional testing framework
        // instead of unit tests, but for now we need these here.

        RegisterAllCoreRPCCommands(tableRPC);
        ClearDatadirCache();
        pathTemp = fs::temp_directory_path() / strprintf("test_bitcoin_%lu_%i", (unsigned long)GetTime(), (int)(InsecureRandRange(100000)));
        fs::create_directories(pathTemp);
        gArgs.ForceSetArg("-datadir", pathTemp.string());

        // Note that because we don't bother running a scheduler thread here,
        // callbacks via CValidationInterface are unreliable, but that's OK,
        // our unit tests aren't testing multiple parts of the code at once.
        GetMainSignals().RegisterBackgroundSignalScheduler(scheduler);

        mempool.setSanityCheck(1.0);
        pblocktree = new CBlockTreeDB(1 << 20, true);
        pcoinsdbview = new CCoinsViewDB(1 << 23, true);
        pcoinsTip = new CCoinsViewCache(pcoinsdbview);

        // contract
        const auto contractPath = pathTemp / "contractstate";
        dev::eth::Ethash::init();
        fs::create_directories(contractPath);
        StateRootView::Init(contractPath, false);
        StateRootView::Instance()->InitGenesis(chainparams);
        const dev::h256 hashDB(dev::sha3(dev::rlp("")));
        EthState::Init(dev::u256(0), EthState::openDB(contractPath.string(), hashDB, dev::WithExisting::Trust), contractPath.string(), dev::eth::BaseState::Empty);

        dev::h256 stateRoot;
        dev::h256 utxoRoot;
        if (!StateRootView::Instance()->GetRoot(chainparams.GenesisBlock().GetHash(), stateRoot, utxoRoot)) {
            throw std::runtime_error("Load state root view failed.");
        }
        EthState::Instance()->setRoot(stateRoot);
        EthState::Instance()->setUTXORoot(utxoRoot);
        EthState::Instance()->populateFromGenesis();
        EthState::Instance()->db().commit();
        EthState::Instance()->dbUtxo().commit();

        TxExecRecord::Init(contractPath.string());

        if (!LoadGenesisBlock(chainparams)) {
            throw std::runtime_error("LoadGenesisBlock failed.");
        }
        {
            CValidationState state;
            if (!ActivateBestChain(state, chainparams)) {
                throw std::runtime_error("ActivateBestChain failed.");
            }
        }
        nScriptCheckThreads = 3;
        for (int i=0; i < nScriptCheckThreads-1; i++)
            threadGroup.create_thread(&ThreadScriptCheck);
        g_connman = std::unique_ptr<CConnman>(new CConnman(0x1337, 0x1337)); // Deterministic randomness for tests.
        connman = g_connman.get();
        peerLogic.reset(new PeerLogicValidation(connman, scheduler));
}

TestingSetup::~TestingSetup()
{
        threadGroup.interrupt_all();
        threadGroup.join_all();
        GetMainSignals().FlushBackgroundCallbacks();
        GetMainSignals().UnregisterBackgroundSignalScheduler();
        g_connman.reset();
        peerLogic.reset();
        UnloadBlockIndex();
        delete pcoinsTip;
        delete pcoinsdbview;
        delete pblocktree;

        EthState::Release();
        StateRootView::Release();
        TxExecRecord::Release();

        fs::remove_all(pathTemp);
}

TestChain100Setup::TestChain100Setup() : TestingSetup(CBaseChainParams::REGTEST)
{
    // Generate a 100-block chain:
    coinbaseKey.MakeNewKey(true);
    CScript scriptPubKey = CScript() <<  ToByteVector(coinbaseKey.GetPubKey()) << OP_CHECKSIG;
    for (int i = 0; i < COINBASE_MATURITY; i++)
    {
        std::vector<CMutableTransaction> noTxns;
        CBlock b = CreateAndProcessBlock(noTxns, scriptPubKey);
        coinbaseTxns.push_back(*b.vtx[0]);
    }
}

//
// Create a new block with just given transactions, coinbase paying to
// scriptPubKey, and try to add it to the current chain.
//
CBlock
TestChain100Setup::CreateAndProcessBlock(const std::vector<CMutableTransaction>& txns, const CScript& scriptPubKey)
{
    const CChainParams& chainparams = Params();
    std::unique_ptr<CBlockTemplate> pblocktemplate = BlockAssembler(chainparams).CreateNewBlock(scriptPubKey);
    CBlock& block = pblocktemplate->block;

    // Replace mempool-selected txns with just coinbase plus passed-in txns:
    block.vtx.resize(1);
    for (const CMutableTransaction& tx : txns)
        block.vtx.push_back(MakeTransactionRef(tx));
    // IncrementExtraNonce creates a valid coinbase and merkleRoot
    unsigned int extraNonce = 0;
    IncrementExtraNonce(&block, chainActive.Tip(), extraNonce);

    while (!CheckProofOfWork(block.GetHash(), block.nBits, chainparams.GetConsensus())) ++block.nNonce;

    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);
    ProcessNewBlock(chainparams, shared_pblock, true, nullptr);

    CBlock result = block;
    return result;
}

TestChain100Setup::~TestChain100Setup()
{
}


CTxMemPoolEntry TestMemPoolEntryHelper::FromTx(const CMutableTransaction &tx) {
    CTransaction txn(tx);
    return FromTx(txn);
}

CTxMemPoolEntry TestMemPoolEntryHelper::FromTx(const CTransaction &txn) {
    return CTxMemPoolEntry(MakeTransactionRef(txn), nFee, nTime, nHeight,
                           spendsCoinbase, sigOpCost, lp);
}

EthTransaction TestContractHelper::CreateEthTx(
        const dev::u256& value,
        const dev::u256& gasLimit,
        const dev::u256& gasPrice,
        const valtype& data,
        const dev::Address& recipient,
        const dev::h256& txHash,
        uint32_t outIdx/* = 0*/) {

    EthTransactionParams params;
    params.version = EthTxVersion::GetDefault();
    params.gasLimit = gasLimit;
    params.gasPrice = gasPrice;
    params.code = data;
    params.receiveAddress = recipient;
    const dev::Address sender(dev::Address("0101010101010101010101010101010101010101"));
    return ContractUtil::CreateEthTransaction(value, params, sender, txHash, outIdx);
}

EthTransaction TestContractHelper::CreateEthTx(
        const valtype& data,
        const dev::u256& value,
        const dev::u256& gasLimit,
        const dev::u256& gasPrice,
        const dev::h256& hashTransaction,
        const dev::Address& recipient,
        uint32_t nvout) {

    return CreateEthTx(value, gasLimit, gasPrice, data, recipient, hashTransaction, nvout);
}

static CBlock generateBlock()
{
    CBlock block;
    CMutableTransaction tx;
    std::vector<unsigned char> address(ParseHex("abababababababababababababababababababab"));
    tx.vout.push_back(CTxOut(0, CScript() << OP_DUP << OP_HASH160 << address << OP_EQUALVERIFY << OP_CHECKSIG));
    block.vtx.push_back(MakeTransactionRef(CTransaction(tx)));
    return block;
}

std::pair<std::vector<EthExecutionResult>, ExecutionResult> TestContractHelper::Execute(const std::vector<EthTransaction> &txs)
{
    const CBlock block(generateBlock());
    ContractExecutor executor(block, txs, DEFAULT_BLOCK_GAS_LIMIT);
    executor.Execut();

    const std::vector<EthExecutionResult> ethExeResult(executor.GetEthResults());
    ExecutionResult exeResult;
    executor.GetResult(exeResult);

    EthState::Instance()->db().commit();
    EthState::Instance()->dbUtxo().commit();

    return std::make_pair(ethExeResult, exeResult);
}
