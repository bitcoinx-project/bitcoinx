#include "txexecrecord.h"

struct TxExecRecordInfoSerialized {
    std::vector<dev::h256> blockHashes;
    std::vector<uint32_t> blockNumbers;
    std::vector<dev::h256> transactionHashes;
    std::vector<uint32_t> transactionIndexes;
    std::vector<dev::h160> senders;
    std::vector<dev::h160> receivers;
    std::vector<dev::u256> cumulativeGasUsed;
    std::vector<dev::u256> gasUsed;
    std::vector<dev::h160> contractAddresses;
    std::vector<logEntriesSerializ> logs;
    std::vector<uint32_t> excepted;
};

//
// TxExecRecord
//
TxExecRecord *TxExecRecord::sInstance = nullptr;

TxExecRecord* TxExecRecord::Init(const std::string& _path)
{
    if (sInstance == nullptr) {
        sInstance = new TxExecRecord(_path);
    }
    return sInstance;
}

TxExecRecord* TxExecRecord::Instance()
{
    assert(sInstance != nullptr);
    return sInstance;
}

void TxExecRecord::Release()
{
    if (sInstance != nullptr) {
        delete sInstance;
        sInstance = nullptr;
    }
}

TxExecRecord::TxExecRecord(std::string const& _path)
{
    path = _path + "/results";
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, path, &db);
    assert(status.ok());
    LogPrintf("Opened LevelDB in %s successfully\n", path);
}

TxExecRecord::~TxExecRecord()
{
    delete db;
    db = NULL;
}

void TxExecRecord::Add(dev::h256 hashTx, std::vector<TxExecRecordInfo>& info)
{
    m_cache.insert(std::make_pair(hashTx, info));
}

void TxExecRecord::ClearCache()
{
    m_cache.clear();
}

void TxExecRecord::Destroy()
{
    const leveldb::Status &result = leveldb::DestroyDB(path, leveldb::Options());
    LogPrintf("Destroy LevelDB in %s, %s", path, result.ToString());
}

void TxExecRecord::Delete(std::vector<CTransactionRef> const& txs)
{
    for (CTransactionRef tx : txs) {
        dev::h256 hashTx = uintToh256(tx->GetHash());
        m_cache.erase(hashTx);

        std::string keyTemp = hashTx.hex();
        leveldb::Slice key(keyTemp);
        leveldb::Status status = db->Delete(leveldb::WriteOptions(), key);
        assert(status.ok());
    }
}

std::vector<TxExecRecordInfo> TxExecRecord::Get(dev::h256 const& hashTx)
{
    std::vector<TxExecRecordInfo> result;
    auto it = m_cache.find(hashTx);
    if (it == m_cache.end()) {
        if (read(hashTx, result))
            m_cache.insert(std::make_pair(hashTx, result));
    } else {
        result = it->second;
    }
    return result;
}

void TxExecRecord::Commit()
{
    if (m_cache.size()) {
        for (auto const& i : m_cache) {
            std::string valueTemp;
            std::string keyTemp = i.first.hex();
            leveldb::Slice key(keyTemp);
            leveldb::Status status = db->Get(leveldb::ReadOptions(), key, &valueTemp);

            if (status.IsNotFound()) {
                TxExecRecordInfoSerialized tris;

                for (size_t j = 0; j < i.second.size(); j++) {
                    tris.blockHashes.push_back(uintToh256(i.second[j].blockHash));
                    tris.blockNumbers.push_back(i.second[j].blockNumber);
                    tris.transactionHashes.push_back(uintToh256(i.second[j].transactionHash));
                    tris.transactionIndexes.push_back(i.second[j].transactionIndex);
                    tris.senders.push_back(i.second[j].from);
                    tris.receivers.push_back(i.second[j].to);
                    tris.cumulativeGasUsed.push_back(dev::u256(i.second[j].cumulativeGasUsed));
                    tris.gasUsed.push_back(dev::u256(i.second[j].gasUsed));
                    tris.contractAddresses.push_back(i.second[j].contractAddress);
                    tris.logs.push_back(logEntriesSerialization(i.second[j].logs));
                    tris.excepted.push_back(uint32_t(static_cast<int>(i.second[j].excepted)));
                }

                dev::RLPStream streamRLP(11);
                streamRLP << tris.blockHashes << tris.blockNumbers << tris.transactionHashes << tris.transactionIndexes << tris.senders;
                streamRLP << tris.receivers << tris.cumulativeGasUsed << tris.gasUsed << tris.contractAddresses << tris.logs << tris.excepted;

                dev::bytes data = streamRLP.out();
                std::string stringData(data.begin(), data.end());
                leveldb::Slice value(stringData);
                status = db->Put(leveldb::WriteOptions(), key, value);
                assert(status.ok());
            }
        }
        m_cache.clear();
    }
}

bool TxExecRecord::read(dev::h256 const& _key, std::vector<TxExecRecordInfo>& _info)
{
    std::string value;
    std::string keyTemp = _key.hex();
    leveldb::Slice key(keyTemp);
    leveldb::Status s = db->Get(leveldb::ReadOptions(), key, &value);

    if (!s.IsNotFound() && s.ok()) {
        TxExecRecordInfoSerialized tris;

        dev::RLP state(value);
        tris.blockHashes = state[0].toVector<dev::h256>();
        tris.blockNumbers = state[1].toVector<uint32_t>();
        tris.transactionHashes = state[2].toVector<dev::h256>();
        tris.transactionIndexes = state[3].toVector<uint32_t>();
        tris.senders = state[4].toVector<dev::h160>();
        tris.receivers = state[5].toVector<dev::h160>();
        tris.cumulativeGasUsed = state[6].toVector<dev::u256>();
        tris.gasUsed = state[7].toVector<dev::u256>();
        tris.contractAddresses = state[8].toVector<dev::h160>();
        tris.logs = state[9].toVector<logEntriesSerializ>();
        if (state.itemCount() == 11)
            tris.excepted = state[10].toVector<uint32_t>();

        for (size_t j = 0; j < tris.blockHashes.size(); j++) {
            TxExecRecordInfo tri{h256Touint(tris.blockHashes[j]), tris.blockNumbers[j], h256Touint(tris.transactionHashes[j]), tris.transactionIndexes[j], tris.senders[j],
                tris.receivers[j], uint64_t(tris.cumulativeGasUsed[j]), uint64_t(tris.gasUsed[j]), tris.contractAddresses[j], logEntriesDeserialize(tris.logs[j]),
                state.itemCount() == 11 ? static_cast<dev::eth::TransactionException>(tris.excepted[j]) : dev::eth::TransactionException::NoInformation};
            _info.push_back(tri);
        }
        return true;
    }
    return false;
}

logEntriesSerializ TxExecRecord::logEntriesSerialization(dev::eth::LogEntries const& _logs)
{
    logEntriesSerializ result;
    for (dev::eth::LogEntry i : _logs) {
        result.push_back(std::make_pair(i.address, std::make_pair(i.topics, i.data)));
    }
    return result;
}

dev::eth::LogEntries TxExecRecord::logEntriesDeserialize(logEntriesSerializ const& _logs)
{
    dev::eth::LogEntries result;
    for (std::pair<dev::Address, std::pair<dev::h256s, dev::bytes>> i : _logs) {
        result.push_back(dev::eth::LogEntry(i.first, i.second.first, dev::bytes(i.second.second)));
    }
    return result;
}
