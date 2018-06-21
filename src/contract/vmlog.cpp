#include "vmlog.h"
#include "univalue.h"
#include "validation.h"
#include <boost/filesystem.hpp>
#include "timedata.h"
#include "util.h"
#include "utilstrencodings.h"

static boost::filesystem::path logFilePath()
{
    return GetDataDir() / "vm_exec_logs.json";
}

static UniValue vmLogToJSON(const EthExecutionResult& execRes, const CTransaction& tx, const CBlock& block)
{
    UniValue result(UniValue::VOBJ);
    if (tx != CTransaction())
        result.push_back(Pair("txid", tx.GetHash().GetHex()));
    result.push_back(Pair("address", execRes.execRes.newAddress.hex()));
    if (block.GetHash() != CBlock().GetHash()) {
        result.push_back(Pair("time", block.GetBlockTime()));
        result.push_back(Pair("blockhash", block.GetHash().GetHex()));
        result.push_back(Pair("blockheight", chainActive.Tip()->nHeight + 1));
    } else {
        result.push_back(Pair("time", GetAdjustedTime()));
        result.push_back(Pair("blockheight", chainActive.Tip()->nHeight));
    }
    UniValue logEntries(UniValue::VARR);
    dev::eth::LogEntries logs = execRes.txRec.log();
    for (dev::eth::LogEntry log : logs) {
        UniValue logEntrie(UniValue::VOBJ);
        logEntrie.push_back(Pair("address", log.address.hex()));
        UniValue topics(UniValue::VARR);
        for (dev::h256 l : log.topics) {
            UniValue topicPair(UniValue::VOBJ);
            topicPair.push_back(Pair("raw", l.hex()));
            topics.push_back(topicPair);
            //TODO add "pretty" field for human readable data
        }
        UniValue dataPair(UniValue::VOBJ);
        dataPair.push_back(Pair("raw", HexStr(log.data)));
        logEntrie.push_back(Pair("data", dataPair));
        logEntrie.push_back(Pair("topics", topics));
        logEntries.push_back(logEntrie);
    }
    result.push_back(Pair("entries", logEntries));
    return result;
}

bool VMLog::fVMlogFileExist = false;
void VMLog::Init()
{
   fVMlogFileExist = boost::filesystem::exists(logFilePath());
}

void VMLog::Write(const std::vector<EthExecutionResult>& res, const CTransaction& tx, const CBlock& block)
{
    const boost::filesystem::path &logFile = logFilePath();
    std::stringstream ss;
    if (fVMlogFileExist) {
        ss << ",";
    } else {
        std::ofstream file(logFile.string(), std::ios::out | std::ios::app);
        file << "{\"logs\":[]}";
        file.close();
    }

    for (size_t i = 0; i < res.size(); i++) {
        ss << vmLogToJSON(res[i], tx, block).write();
        if (i != res.size() - 1) {
            ss << ",";
        } else {
            ss << "]}";
        }
    }

    std::ofstream file(logFile.string(), std::ios::in | std::ios::out);
    file.seekp(-2, std::ios::end);
    file << ss.str();
    file.close();
    fVMlogFileExist = true;
}