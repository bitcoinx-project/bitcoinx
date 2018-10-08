#ifndef EVENTLOG_H
#define EVENTLOG_H
#include <string>
#include <vector>
#include <QMap>
#include <QVariant>

class ExecRPCCommand;

class EventLog
{
public:
    /**
     * @brief EventLog Constructor
     */
    EventLog();

    /**
     * @brief ~EventLog Destructor
     */
    ~EventLog();

    /**
     * @brief search Search for log events
     * @param fromBlock Begin from block
     * @param toBlock End to block
     * @param addresses Contract address
     * @param topics Event topics
     * @param result Result of the performed call
     * @return success of the operation
     */
    bool search(int64_t fromBlock, int64_t toBlock, const std::vector<std::string> addresses, const std::vector<std::string> topics, QVariant& result);

    /**
     * @brief searchTokenTx Search the event log for token transactions
     * @param fromBlock Begin from block
     * @param toBlock End to block
     * @param strContractAddress Token contract address
     * @param strSenderAddress Token sender address
     * @param result Result of the performed call
     * @return success of the operation
     */
    bool searchTokenTx(int64_t fromBlock, int64_t toBlock, std::string strContractAddress, std::string strSenderAddress, QVariant& result);
private:
    // Set command data
    void setStartBlock(int64_t fromBlock);
    void setEndBlock(int64_t toBlock);
    void setAddresses(const std::vector<std::string> addresses);
    void setTopics(const std::vector<std::string> topics);

    ExecRPCCommand* m_RPCCommand;
    QMap<QString, QString> m_lstParams;
};

#endif // EVENTLOG_H
