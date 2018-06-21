#ifndef BITCOINX_CONTRACT_TXEXECRECORD_H
#define BITCOINX_CONTRACT_TXEXECRECORD_H

#include "util.h"
#include <libethereum/State.h>
#include <libethereum/Transaction.h>
#include <primitives/transaction.h>
#include <uint256.h>

using logEntriesSerializ = std::vector<std::pair<dev::Address, std::pair<dev::h256s, dev::bytes>>>;

struct TxExecRecordInfo {
    uint256 blockHash;
    uint32_t blockNumber;
    uint256 transactionHash;
    uint32_t transactionIndex;
    dev::Address from;
    dev::Address to;
    uint64_t cumulativeGasUsed;
    uint64_t gasUsed;
    dev::Address contractAddress;
    dev::eth::LogEntries logs;
    dev::eth::TransactionException excepted;
};

class TxExecRecord
{
public:
    static TxExecRecord* Init(const std::string& _path);
    static TxExecRecord* Instance();
    static void Release();

    void Add(dev::h256 hashTx, std::vector<TxExecRecordInfo>& info);

    void Delete(std::vector<CTransactionRef> const& txs);

    std::vector<TxExecRecordInfo> Get(dev::h256 const& hashTx);

    void Commit();

    void ClearCache();

    void Destroy();

private:
    TxExecRecord(std::string const& _path);
    TxExecRecord(const TxExecRecord&) = delete;
    TxExecRecord& operator=(const TxExecRecord&) = delete;
    ~TxExecRecord();

    bool read(dev::h256 const& _key, std::vector<TxExecRecordInfo>& _info);

    logEntriesSerializ logEntriesSerialization(dev::eth::LogEntries const& _logs);

    dev::eth::LogEntries logEntriesDeserialize(logEntriesSerializ const& _logs);

    std::string path;

    leveldb::DB* db;
    leveldb::Options options;

    std::unordered_map<dev::h256, std::vector<TxExecRecordInfo>> m_cache;

    static TxExecRecord* sInstance;
};

#endif // BITCOINX_CONTRACT_TXEXECRECORD_H
