// Copyright (c) 2014-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "validation.h"
#include "net.h"

#include "test/test_bitcoin.h"

#include <boost/signals2/signal.hpp>
#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(main_tests, TestingSetup)

static int Halvings2Height(int nHalvings, const Consensus::Params& consensusParams)
{
    if (consensusParams.BCXHeight > 0) {
        if (nHalvings < 3) {
            return nHalvings * consensusParams.nSubsidyHalvingInterval;
        } else if (nHalvings == 3) {
            return 1154444;
        } else {
            return 1154443 + (nHalvings - 3) * consensusParams.nBCXSubsidyHalvingInterval;
        }
    } else {
       return nHalvings * consensusParams.nSubsidyHalvingInterval; 
    }
}

static void TestBlockSubsidyHalvings(const Consensus::Params& consensusParams)
{
    const int maxHalvings = 64;
    const CAmount nInitialSubsidy = 50 * COIN * BTC_2_BCX_RATE;
    const int bcx2btcHalvingRate = consensusParams.nBCXSubsidyHalvingInterval / consensusParams.nSubsidyHalvingInterval;

    CAmount nPreviousSubsidy = nInitialSubsidy * 2; // for height == 0
    BOOST_CHECK_EQUAL(nPreviousSubsidy, nInitialSubsidy * 2);
    for (int nHalvings = 0; nHalvings < maxHalvings; nHalvings++) {
        const int nHeight = Halvings2Height(nHalvings, consensusParams);
        CAmount nSubsidy = GetBlockSubsidy(nHeight, consensusParams);
        if (0 < consensusParams.BCXHeight && consensusParams.BCXHeight <= nHeight) {
            nSubsidy = nSubsidy * bcx2btcHalvingRate;
            BOOST_CHECK((nSubsidy == nPreviousSubsidy / 2) || ((nSubsidy + 2) == nPreviousSubsidy / 2));
        } else {
            BOOST_CHECK_EQUAL(nSubsidy, nPreviousSubsidy / 2);
        }
        BOOST_CHECK(nSubsidy <= nInitialSubsidy);
        nPreviousSubsidy = nSubsidy;
    }
    BOOST_CHECK_EQUAL(GetBlockSubsidy(Halvings2Height(maxHalvings, consensusParams), consensusParams), 0);
}

static void TestBlockSubsidyHalvings(int nSubsidyHalvingInterval)
{
    Consensus::Params consensusParams;
    consensusParams.nSubsidyHalvingInterval = nSubsidyHalvingInterval;
    consensusParams.BCXHeight = 0;
    TestBlockSubsidyHalvings(consensusParams);
}

BOOST_AUTO_TEST_CASE(block_subsidy_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    TestBlockSubsidyHalvings(chainParams->GetConsensus()); // As in main
    TestBlockSubsidyHalvings(150); // As in regtest
    TestBlockSubsidyHalvings(1000); // Just another interval
}

BOOST_AUTO_TEST_CASE(subsidy_limit_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    const CAmount nInitialSubsidy = 50 * COIN * BTC_2_BCX_RATE;
    CAmount nSum = 0;
    for (int nHeight = 0; nHeight < 30000000; nHeight++) {
        CAmount nSubsidy = GetBlockSubsidy(nHeight, chainParams->GetConsensus());
        if (nSubsidy == 0) {
            break;
        }

        if (nHeight != chainParams->GetConsensus().BCXHeight) {
            BOOST_CHECK(nSubsidy <= nInitialSubsidy);
        }
        nSum += nSubsidy;
        BOOST_CHECK(MoneyRange(nSum));
    }
    BOOST_CHECK_EQUAL(nSum, 2099999986350000ULL);
}

bool ReturnFalse() { return false; }
bool ReturnTrue() { return true; }

BOOST_AUTO_TEST_CASE(test_combiner_all)
{
    boost::signals2::signal<bool (), CombinerAll> Test;
    BOOST_CHECK(Test());
    Test.connect(&ReturnFalse);
    BOOST_CHECK(!Test());
    Test.connect(&ReturnTrue);
    BOOST_CHECK(!Test());
    Test.disconnect(&ReturnFalse);
    BOOST_CHECK(Test());
    Test.disconnect(&ReturnTrue);
    BOOST_CHECK(Test());
}
BOOST_AUTO_TEST_SUITE_END()
