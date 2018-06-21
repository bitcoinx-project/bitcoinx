#include "base58.h"
#include "config.h"
#include "consensus/validation.h"
#include "contractexecutor.h"
#include "contractutil.h"
#include "core_io.h"
#include "primitives/transaction.h"
#include "pubkey.h"
#include "rpc/server.h"
#include "script/standard.h"
#include "staterootview.h"
#include "timedata.h"
#include "univalue.h"
#include "util.h"
#include "utilmoneystr.h"
#include "validation.h"
#include "vmlog.h"
#include "wallet/coincontrol.h"
#include "wallet/rpcwallet.h"
#include "wallet/wallet.h"
#include "txexecrecord.h"
#include "txdb.h"

extern std::unique_ptr<CConnman> g_connman;

UniValue createcontract(const JSONRPCRequest& request)
{
    CWallet* const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    LOCK2(cs_main, pwallet->cs_wallet);
    const uint64_t blockGasLimit = DEFAULT_BLOCK_GAS_LIMIT;
    CAmount nGasPrice = DEFAULT_GAS_PRICE;

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 6)
        throw std::runtime_error(
            "createcontract \"bytecode\" (gaslimit gasprice \"senderaddress\" broadcast)"
            "\nCreate a contract with bytcode.\n" +
            HelpRequiringPassphrase(pwallet) +
            "\nArguments:\n"
            "1. \"bytecode\"  (string, required) contract bytcode.\n"
            "2. gasLimit  (numeric or string, optional) gasLimit, default: " +
            i64tostr(DEFAULT_GAS_LIMIT_OP_CREATE) + ", max: " + i64tostr(blockGasLimit) + "\n"
                                                                                          "3. gasPrice  (numeric or string, optional) gasPrice price per gas unit, default: " +
            FormatMoney(nGasPrice) + ", min:" + FormatMoney(MIN_GAS_PRICE) + "\n"
                                                                             "4. \"senderaddress\" (string, optional) The bitcoinx address that will be used to create the contract.\n"
                                                                             "5. \"broadcast\" (bool, optional, default=true) Whether to broadcast the transaction or not.\n"
                                                                             "6. \"changeToSender\" (bool, optional, default=true) Return the change to the sender.\n"
                                                                             "\nResult:\n"
                                                                             "[\n"
                                                                             "  {\n"
                                                                             "    \"txid\" : (string) The transaction id.\n"
                                                                             "    \"sender\" : (string) " +
            CURRENCY_UNIT + " address of the sender.\n"
                            "    \"hash160\" : (string) ripemd-160 hash of the sender.\n"
                            "    \"address\" : (string) expected contract address.\n"
                            "  }\n"
                            "]\n"
                            "\nExamples:\n" +
            HelpExampleCli("createcontract", "\"60606040525b33600060006101000a81548173ffffffffffffffffffffffffffffffffffffffff02191690836c010000000000000000000000009081020402179055506103786001600050819055505b600c80605b6000396000f360606040526008565b600256\"") + HelpExampleCli("createcontract", "\"60606040525b33600060006101000a81548173ffffffffffffffffffffffffffffffffffffffff02191690836c010000000000000000000000009081020402179055506103786001600050819055505b600c80605b6000396000f360606040526008565b600256\" 6000000 " + FormatMoney(MIN_GAS_PRICE) + " \"QM72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" true"));


    const std::string& bytecode = request.params[0].get_str();
    if (bytecode.size() % 2 != 0 || !CheckHex(bytecode))
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid data (data not hex)");

    uint64_t nGasLimit = DEFAULT_GAS_LIMIT_OP_CREATE;
    if (request.params.size() > 1) {
        nGasLimit = request.params[1].get_int64();
        if (nGasLimit > blockGasLimit)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasLimit (Maximum is: " + i64tostr(blockGasLimit) + ")");
        if (nGasLimit < MINIMUM_GAS_LIMIT)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasLimit (Minimum is: " + i64tostr(MINIMUM_GAS_LIMIT) + ")");
    }

    if (request.params.size() > 2) {
        UniValue uGasPrice = request.params[2];
        if (!ParseMoney(uGasPrice.getValStr(), nGasPrice)) {
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasPrice");
        }
        CAmount maxRpcGasPrice = gArgs.GetArg("-rpcmaxgasprice", MAX_RPC_GAS_PRICE);
        if (nGasPrice > (int64_t)maxRpcGasPrice)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasPrice, Maximum allowed in RPC calls is: " + FormatMoney(maxRpcGasPrice) + " (use -rpcmaxgasprice to change it)");
        if (nGasPrice < (int64_t)MIN_GAS_PRICE)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasPrice (Minimum is: " + FormatMoney(MIN_GAS_PRICE) + ")");
    }

    bool fHasSender = false;
    CBitcoinAddress senderAddress;
    if (request.params.size() > 3) {
        senderAddress.SetString(request.params[3].get_str());
        if (!senderAddress.IsValid())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid bitcoinx address to send from");
        else
            fHasSender = true;
    }

    bool fBroadcast = true;
    if (request.params.size() > 4) {
        fBroadcast = request.params[4].get_bool();
    }

    bool fChangeToSender = true;
    if (request.params.size() > 5) {
        fChangeToSender = request.params[5].get_bool();
    }

    CCoinControl coinControl;

    if (fHasSender) {
        //find a UTXO with sender address

        UniValue results(UniValue::VARR);
        std::vector<COutput> vecOutputs;

        coinControl.fAllowOtherInputs = true;

        assert(pwallet != NULL);
        pwallet->AvailableCoins(vecOutputs, false, NULL, true);

        for (const COutput& out : vecOutputs) {
            CTxDestination address;
            const CScript& scriptPubKey = out.tx->tx->vout[out.i].scriptPubKey;
            bool fValidAddress = ExtractDestination(scriptPubKey, address);

            CBitcoinAddress destAdress(address);

            if (!fValidAddress || senderAddress.Get() != destAdress.Get())
                continue;

            coinControl.Select(COutPoint(out.tx->GetHash(), out.i));

            break;
        }

        if (!coinControl.HasSelected()) {
            throw JSONRPCError(RPC_TYPE_ERROR, "Sender address does not have any unspent outputs");
        }
        if (fChangeToSender) {
            coinControl.destChange = senderAddress.Get();
        }
    }
    EnsureWalletIsUnlocked(pwallet);

    CWalletTx wtx;
    wtx.nTimeSmart = GetAdjustedTime();

    const CAmount nGasFee = nGasPrice * nGasLimit / SATOSHI_2_WEI_RATE;
    const CAmount curBalance = pwallet->GetBalance();

    // Check amount
    if (nGasFee <= 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid amount");

    if (nGasFee > curBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    // Build OP_EXEC script
    CScript scriptPubKey = CScript() << CScriptNum(EthTxVersion::GetDefault().ToRaw()) << CScriptNum(nGasLimit) << CScriptNum(nGasPrice) << ParseHex(bytecode) << OP_CREATECONTRACT;

    // Create and send the transaction
    CReserveKey reservekey(pwallet);
    CAmount nFeeRequired;
    std::string strError;
    std::vector<CRecipient> vecSend;
    int nChangePosRet = -1;
    CRecipient recipient = {scriptPubKey, 0, false};
    vecSend.push_back(recipient);

    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coinControl, true, nGasFee, fHasSender)) {
        if (nFeeRequired > pwallet->GetBalance())
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds!", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    CTxDestination txSenderDest;
    ExtractDestination(pwallet->mapWallet[wtx.tx->vin[0].prevout.hash].tx->vout[wtx.tx->vin[0].prevout.n].scriptPubKey, txSenderDest);

    if (fHasSender && !(senderAddress.Get() == txSenderDest)) {
        throw JSONRPCError(RPC_TYPE_ERROR, "Sender could not be set, transaction was not committed!");
    }

    UniValue result(UniValue::VOBJ);
    if (fBroadcast) {
        CValidationState state;
        if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error: The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of the wallet and coins were spent in the copy but not marked as spent here.");

        const std::string& txId = wtx.GetHash().GetHex();
        result.push_back(Pair("txid", txId));

        CBitcoinAddress txSenderAdress(txSenderDest);
        CKeyID keyid;
        txSenderAdress.GetKeyID(keyid);

        result.push_back(Pair("sender", txSenderAdress.ToString()));
        result.push_back(Pair("hash160", HexStr(valtype(keyid.begin(), keyid.end()))));

        uint32_t outIdx = 0;
        for (const CTxOut& txout : wtx.tx->vout) {
            if (txout.scriptPubKey.HasCreateContractOp()) {
                const dev::Address& contractAddr = ContractUtil::CreateContractAddr(wtx.GetHash(), outIdx);
                result.push_back(Pair("address", HexStr(contractAddr.asBytes())));
                break;
            }
            outIdx++;
        }
    } else {
        const std::string& strHex = EncodeHexTx(*wtx.tx, RPCSerializationFlags());
        result.push_back(Pair("raw transaction", strHex));
    }
    return result;
}

static UniValue executionResultToJSON(const dev::eth::ExecutionResult& exRes)
{
    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("gasUsed", CAmount(exRes.gasUsed)));
    std::stringstream ss;
    ss << exRes.excepted;
    result.push_back(Pair("excepted", ss.str()));
    result.push_back(Pair("newAddress", exRes.newAddress.hex()));
    result.push_back(Pair("output", HexStr(exRes.output)));
    result.push_back(Pair("codeDeposit", static_cast<int32_t>(exRes.codeDeposit)));
    result.push_back(Pair("gasRefunded", CAmount(exRes.gasRefunded)));
    result.push_back(Pair("depositSize", static_cast<int32_t>(exRes.depositSize)));
    result.push_back(Pair("gasForDeposit", CAmount(exRes.gasForDeposit)));
    return result;
}

static UniValue transactionReceiptToJSON(const dev::eth::TransactionReceipt& txRec)
{
    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("stateRoot", txRec.stateRoot().hex()));
    result.push_back(Pair("gasUsed", CAmount(txRec.gasUsed())));
    result.push_back(Pair("bloom", txRec.bloom().hex()));
    UniValue logEntries(UniValue::VARR);
    dev::eth::LogEntries logs = txRec.log();
    for (dev::eth::LogEntry log : logs) {
        UniValue logEntrie(UniValue::VOBJ);
        logEntrie.push_back(Pair("address", log.address.hex()));
        UniValue topics(UniValue::VARR);
        for (dev::h256 l : log.topics) {
            topics.push_back(l.hex());
        }
        logEntrie.push_back(Pair("topics", topics));
        logEntrie.push_back(Pair("data", HexStr(log.data)));
        logEntries.push_back(logEntrie);
    }
    result.push_back(Pair("log", logEntries));
    return result;
}

UniValue callcontract(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 2)
        throw std::runtime_error(
            "callcontract \"address\" \"data\" ( address )\n"
            "\nArgument:\n"
            "1. \"address\"          (string, required) The account address\n"
            "2. \"data\"             (string, required) The data hex string\n"
            "3. address              (string, optional) The sender address hex string\n"
            "4. gasLimit             (string, optional) The gas limit for executing the contract\n");

    LOCK(cs_main);

    const std::string& strAddr = request.params[0].get_str();
    const std::string& data = request.params[1].get_str();

    if (data.size() % 2 != 0 || !CheckHex(data))
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid data (data not hex)");

    if (strAddr.size() != 40 || !CheckHex(strAddr))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Incorrect address");

    dev::Address addrAccount(strAddr);
    if (!EthState::Instance()->addressInUse(addrAccount))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Address does not exist");

    dev::Address senderAddress;
    if (request.params.size() == 3) {
        CBitcoinAddress bcxSender(request.params[2].get_str());
        if (bcxSender.IsValid()) {
            CKeyID keyid;
            bcxSender.GetKeyID(keyid);
            senderAddress = dev::Address(HexStr(valtype(keyid.begin(), keyid.end())));
        } else {
            senderAddress = dev::Address(request.params[2].get_str());
        }
    }
    uint64_t gasLimit = 0;
    if (request.params.size() == 4) {
        gasLimit = request.params[3].get_int();
    }

    const std::vector<EthExecutionResult>& execResults = ContractExecutor::Call(addrAccount, ParseHex(data), senderAddress, gasLimit);
    if (fRecordLogOpcodes) {
        VMLog::Write(execResults);
    }

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("address", strAddr));
    result.push_back(Pair("executionResult", executionResultToJSON(execResults[0].execRes)));
    result.push_back(Pair("transactionReceipt", transactionReceiptToJSON(execResults[0].txRec)));

    return result;
}

UniValue sendtocontract(const JSONRPCRequest& request)
{
    CWallet* const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    LOCK2(cs_main, pwallet->cs_wallet);
    CAmount nGasPrice = DEFAULT_GAS_PRICE;
    const uint64_t blockGasLimit = DEFAULT_BLOCK_GAS_LIMIT;

    if (request.fHelp || request.params.size() < 2 || request.params.size() > 8)
        throw std::runtime_error(
            "sendtocontract \"contractaddress\" \"data\" (amount gaslimit gasprice senderaddress broadcast)"
            "\nSend funds and data to a contract.\n" +
            HelpRequiringPassphrase(pwallet) +
            "\nArguments:\n"
            "1. \"contractaddress\" (string, required) The contract address that will receive the funds and data.\n"
            "2. \"datahex\"  (string, required) data to send.\n"
            "3. \"amount\"      (numeric or string, optional) The amount in " +
            CURRENCY_UNIT + " to send. eg 0.1, default: 0\n"
                            "4. gasLimit  (numeric or string, optional) gasLimit, default: " +
            i64tostr(DEFAULT_GAS_LIMIT_OP_SEND) + ", max: " + i64tostr(blockGasLimit) + "\n"
                                                                                        "5. gasPrice  (numeric or string, optional) gasPrice price per gas unit, default: " +
            FormatMoney(nGasPrice) + ", min:" + FormatMoney(MIN_GAS_PRICE) + "\n"
                                                                             "6. \"senderaddress\" (string, optional) The bitcoinx address that will be used as sender.\n"
                                                                             "7. \"broadcast\" (bool, optional, default=true) Whether to broadcast the transaction or not.\n"
                                                                             "8. \"changeToSender\" (bool, optional, default=true) Return the change to the sender.\n"
                                                                             "\nResult:\n"
                                                                             "[\n"
                                                                             "  {\n"
                                                                             "    \"txid\" : (string) The transaction id.\n"
                                                                             "    \"sender\" : (string) " +
            CURRENCY_UNIT + " address of the sender.\n"
                            "    \"hash160\" : (string) ripemd-160 hash of the sender.\n"
                            "  }\n"
                            "]\n"
                            "\nExamples:\n" +
            HelpExampleCli("sendtocontract", "\"c6ca2697719d00446d4ea51f6fac8fd1e9310214\" \"54f6127f\"") + HelpExampleCli("sendtocontract", "\"c6ca2697719d00446d4ea51f6fac8fd1e9310214\" \"54f6127f\" 12.0015 6000000 " + FormatMoney(MIN_GAS_PRICE) + " \"QM72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\""));


    const std::string& contractaddress = request.params[0].get_str();
    if (contractaddress.size() != 40 || !CheckHex(contractaddress))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Incorrect contract address");

    dev::Address addrAccount(contractaddress);
    if (!EthState::Instance()->addressInUse(addrAccount))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "contract address does not exist");

    const std::string& datahex = request.params[1].get_str();
    if (datahex.size() % 2 != 0 || !CheckHex(datahex))
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid data (data not hex)");

    CAmount nAmount = 0;
    if (request.params.size() > 2) {
        nAmount = AmountFromValue(request.params[2]);
        if (nAmount < 0)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");
    }

    uint64_t nGasLimit = DEFAULT_GAS_LIMIT_OP_SEND;
    if (request.params.size() > 3) {
        nGasLimit = request.params[3].get_int64();
        if (nGasLimit > blockGasLimit)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasLimit (Maximum is: " + i64tostr(blockGasLimit) + ")");
        if (nGasLimit < MINIMUM_GAS_LIMIT)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasLimit (Minimum is: " + i64tostr(MINIMUM_GAS_LIMIT) + ")");
    }

    if (request.params.size() > 4) {
        UniValue uGasPrice = request.params[4];
        if (!ParseMoney(uGasPrice.getValStr(), nGasPrice)) {
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasPrice");
        }
        CAmount maxRpcGasPrice = gArgs.GetArg("-rpcmaxgasprice", MAX_RPC_GAS_PRICE);
        if (nGasPrice > (int64_t)maxRpcGasPrice)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasPrice, Maximum allowed in RPC calls is: " + FormatMoney(maxRpcGasPrice) + " (use -rpcmaxgasprice to change it)");
        if (nGasPrice < (int64_t)MIN_GAS_PRICE)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasPrice (Minimum is: " + FormatMoney(MIN_GAS_PRICE) + ")");
    }

    bool fHasSender = false;
    CBitcoinAddress senderAddress;
    if (request.params.size() > 5) {
        senderAddress.SetString(request.params[5].get_str());
        if (!senderAddress.IsValid())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid bitcoinx address to send from");
        else
            fHasSender = true;
    }

    bool fBroadcast = true;
    if (request.params.size() > 6) {
        fBroadcast = request.params[6].get_bool();
    }

    bool fChangeToSender = true;
    if (request.params.size() > 7) {
        fChangeToSender = request.params[7].get_bool();
    }

    CCoinControl coinControl;

    if (fHasSender) {
        UniValue results(UniValue::VARR);
        std::vector<COutput> vecOutputs;

        coinControl.fAllowOtherInputs = true;

        assert(pwallet != NULL);
        pwallet->AvailableCoins(vecOutputs, false, NULL, true);

        for (const COutput& out : vecOutputs) {
            CTxDestination address;
            const CScript& scriptPubKey = out.tx->tx->vout[out.i].scriptPubKey;
            bool fValidAddress = ExtractDestination(scriptPubKey, address);

            CBitcoinAddress destAdress(address);

            if (!fValidAddress || senderAddress.Get() != destAdress.Get())
                continue;

            coinControl.Select(COutPoint(out.tx->GetHash(), out.i));

            break;
        }

        if (!coinControl.HasSelected()) {
            throw JSONRPCError(RPC_TYPE_ERROR, "Sender address does not have any unspent outputs");
        }
        if (fChangeToSender) {
            coinControl.destChange = senderAddress.Get();
        }
    }

    EnsureWalletIsUnlocked(pwallet);

    CWalletTx wtx;
    wtx.nTimeSmart = GetAdjustedTime();

    const CAmount nGasFee = nGasPrice * nGasLimit / SATOSHI_2_WEI_RATE;
    const CAmount curBalance = pwallet->GetBalance();

    // Check amount
    if (nGasFee <= 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid amount for gas fee");

    if (nAmount + nGasFee > curBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    // Build OP_EXEC_ASSIGN script
    CScript scriptPubKey = CScript() << CScriptNum(EthTxVersion::GetDefault().ToRaw()) << CScriptNum(nGasLimit) << CScriptNum(nGasPrice) << ParseHex(datahex) << ParseHex(contractaddress) << OP_SENDTOCONTRACT;

    // Create and send the transaction
    CReserveKey reservekey(pwallet);
    CAmount nFeeRequired;
    std::string strError;
    std::vector<CRecipient> vecSend;
    int nChangePosRet = -1;
    CRecipient recipient = {scriptPubKey, nAmount, false};
    vecSend.push_back(recipient);

    if (!pwallet->CreateTransaction(vecSend, wtx, reservekey, nFeeRequired, nChangePosRet, strError, coinControl, true, nGasFee, fHasSender)) {
        if (nFeeRequired > pwallet->GetBalance())
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds!", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    CTxDestination txSenderDest;
    ExtractDestination(pwallet->mapWallet[wtx.tx->vin[0].prevout.hash].tx->vout[wtx.tx->vin[0].prevout.n].scriptPubKey, txSenderDest);

    if (fHasSender && !(senderAddress.Get() == txSenderDest)) {
        throw JSONRPCError(RPC_TYPE_ERROR, "Sender could not be set, transaction was not committed!");
    }

    UniValue result(UniValue::VOBJ);

    if (fBroadcast) {
        CValidationState state;
        if (!pwallet->CommitTransaction(wtx, reservekey, g_connman.get(), state))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error: The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of the wallet and coins were spent in the copy but not marked as spent here.");

        const std::string& txId = wtx.GetHash().GetHex();
        result.push_back(Pair("txid", txId));

        CBitcoinAddress txSenderAdress(txSenderDest);
        CKeyID keyid;
        txSenderAdress.GetKeyID(keyid);

        result.push_back(Pair("sender", txSenderAdress.ToString()));
        result.push_back(Pair("hash160", HexStr(valtype(keyid.begin(), keyid.end()))));
    } else {
        const std::string& strHex = EncodeHexTx(*wtx.tx, RPCSerializationFlags());
        result.push_back(Pair("raw transaction", strHex));
    }

    return result;
}

UniValue listcontracts(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() > 2)
        throw std::runtime_error(
                "listcontracts (start maxDisplay)\n"
                "\nArgument:\n"
                "1. start     (numeric or string, optional) The starting account index, default 1\n"
                "2. maxDisplay       (numeric or string, optional) Max accounts to list, default 20\n"
        );

    LOCK(cs_main);

    int start=1;
    if (request.params.size() > 0){
        start = request.params[0].get_int();
        if (start<= 0)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid start, min=1");
    }

    int maxDisplay=20;
    if (request.params.size() > 1){
        maxDisplay = request.params[1].get_int();
        if (maxDisplay <= 0)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid maxDisplay");
    }

    UniValue result(UniValue::VOBJ);

    const auto &map = EthState::Instance()->addresses();
    const int contractsCount = (int)map.size();

    if (contractsCount > 0 && start > contractsCount)
        throw JSONRPCError(RPC_TYPE_ERROR, "start greater than max index "+ itostr(contractsCount));

    const int itStartPos = std::min(start-1,contractsCount);
    int i = 0;
    for (auto it = std::next(map.begin(), itStartPos); it != map.end(); it++)
    {
        result.push_back(Pair(it->first.hex(), ValueFromAmount(CAmount(EthState::Instance()->balance(it->first) / SATOSHI_2_WEI_RATE))));
        i++;
        if (i == maxDisplay) break;
    }

    return result;
}

UniValue gethexaddress(const JSONRPCRequest& request) {
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
                "gethexaddress \"address\"\n"

                        "\nConverts a base58 pubkeyhash address to a hex address for use in smart contracts.\n"

                        "\nArguments:\n"
                        "1. \"address\"      (string, required) The base58 address\n"

                        "\nResult:\n"
                        "\"hexaddress\"      (string) The raw hex pubkeyhash address for use in smart contracts\n"

                        "\nExamples:\n"
                + HelpExampleCli("gethexaddress", "\"address\"")
                + HelpExampleRpc("gethexaddress", "\"address\"")
        );

    CBitcoinAddress address(request.params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid bitcoinx address");

    if(!address.IsPubKeyHash())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Only pubkeyhash addresses are supported");

    return boost::get<CKeyID>(address.Get()).GetReverseHex();
}

UniValue fromhexaddress(const JSONRPCRequest& request) {
    if (request.fHelp || request.params.size() < 1 || request.params.size() > 1)
        throw std::runtime_error(
                "fromhexaddress \"hexaddress\"\n"

                        "\nConverts a raw hex address to a base58 pubkeyhash address\n"

                        "\nArguments:\n"
                        "1. \"hexaddress\"      (string, required) The raw hex address\n"

                        "\nResult:\n"
                        "\"address\"      (string) The base58 pubkeyhash address\n"

                        "\nExamples:\n"
                + HelpExampleCli("fromhexaddress", "\"hexaddress\"")
                + HelpExampleRpc("fromhexaddress", "\"hexaddress\"")
        );
    if (request.params[0].get_str().size() != 40)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid pubkeyhash hex size (should be 40 hex characters)");
    CKeyID raw;
    raw.SetReverseHex(request.params[0].get_str());
    CBitcoinAddress address(raw);

    return address.ToString();
}

UniValue getcontractinfo(const JSONRPCRequest& req)
{
    if (req.fHelp || req.params.size() < 1)
        throw std::runtime_error(
                "getcontractinfo \"address\"\n"
                "\nArgument:\n"
                "1. \"address\"          (string, required) The account address\n"
                );

    LOCK(cs_main);

    std::string addr = req.params[0].get_str();
    if(addr.size() != 40 || !CheckHex(addr))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Incorrect address");

    dev::Address addrAccount(addr);
    if(!EthState::Instance()->addressInUse(addrAccount))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Address does not exist");

    UniValue ret(UniValue::VOBJ);

    ret.push_back(Pair("address", addr));
    ret.push_back(Pair("balance", CAmount(EthState::Instance()->balance(addrAccount) / SATOSHI_2_WEI_RATE)));
    std::vector<uint8_t> code(EthState::Instance()->code(addrAccount));
    auto storage(EthState::Instance()->storage(addrAccount));

    UniValue storageUV(UniValue::VOBJ);
    for (auto j: storage)
    {
        UniValue e(UniValue::VOBJ);
        e.push_back(Pair(dev::toHex(j.second.first), dev::toHex(j.second.second)));
        storageUV.push_back(Pair(j.first.hex(), e));
    }

    ret.push_back(Pair("storage", storageUV));

    ret.push_back(Pair("code", HexStr(code.begin(), code.end())));

    std::unordered_map<dev::Address, Vin> vins = EthState::Instance()->vins();
    if(vins.count(addrAccount)){
        UniValue vin(UniValue::VOBJ);
        valtype vchHash(vins[addrAccount].hash.asBytes());
        vin.push_back(Pair("hash", HexStr(vchHash.rbegin(), vchHash.rend())));
        vin.push_back(Pair("nVout", uint64_t(vins[addrAccount].nVout)));
        vin.push_back(Pair("value", uint64_t(vins[addrAccount].value / SATOSHI_2_WEI_RATE)));
        ret.push_back(Pair("vin", vin));
    }
    return ret;
}

struct TemporaryState {
    EthState* globalStateRef;
    dev::h256 oldHashStateRoot;
    dev::h256 oldHashUTXORoot;

    TemporaryState(EthState* _globalStateRef)
        : globalStateRef(_globalStateRef),
          oldHashStateRoot(globalStateRef->rootHash()),
          oldHashUTXORoot(globalStateRef->rootHashUTXO())
    {
    }

    void SetRoot(dev::h256 newHashStateRoot, dev::h256 newHashUTXORoot)
    {
        globalStateRef->setRoot(newHashStateRoot);
        globalStateRef->setUTXORoot(newHashUTXORoot);
    }

    ~TemporaryState()
    {
        globalStateRef->setRoot(oldHashStateRoot);
        globalStateRef->setUTXORoot(oldHashUTXORoot);
    }
    TemporaryState() = delete;
    TemporaryState(const TemporaryState&) = delete;
    TemporaryState& operator=(const TemporaryState&) = delete;
    TemporaryState(TemporaryState&&) = delete;
    TemporaryState& operator=(TemporaryState&&) = delete;
};

UniValue getcontractstorage(const JSONRPCRequest& req)
{
    if (req.fHelp || req.params.size() < 1)
        throw std::runtime_error(
                "getstorage \"address\"\n"
                "\nArgument:\n"
                "1. \"address\"          (string, required) The address to get the storage from\n"
                "2. \"blockNum\"         (string, optional) Number of block to get state from, \"latest\" keyword supported. Latest if not passed.\n"
                "3. \"index\"            (number, optional) Zero-based index position of the storage\n"
                );

    LOCK(cs_main);

    std::string addr = req.params[0].get_str();
    if(addr.size() != 40 || !CheckHex(addr)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Incorrect address");
    }

    TemporaryState ts(EthState::Instance());
    if (req.params.size() > 1) {
        if (req.params[1].isNum()) {
            auto blockNum = req.params[1].get_int();
            if((blockNum < 0 && blockNum != -1) || blockNum > chainActive.Height()) {
                throw JSONRPCError(RPC_INVALID_PARAMS, "Incorrect block number");
            }

            if(blockNum != -1) {
                dev::h256 stateRootHash;
                dev::h256 utxoRootHash;
                if (!StateRootView::Instance()->GetRoot(chainActive[blockNum]->GetBlockHash(), stateRootHash, utxoRootHash)) {
                    throw JSONRPCError(RPC_INVALID_PARAMS, "Incorrect block number, contract non-active");
                }
                ts.SetRoot(stateRootHash, utxoRootHash);
            }
        } else {
            throw JSONRPCError(RPC_INVALID_PARAMS, "Incorrect block number");
        }
    }

    dev::Address addrAccount(addr);
    if(!EthState::Instance()->addressInUse(addrAccount)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Address does not exist");
    }

    UniValue ret(UniValue::VOBJ);

    bool onlyIndex = req.params.size() > 2;
    uint32_t index = 0;
    if (onlyIndex)
        index = req.params[2].get_int();

    auto storage(EthState::Instance()->storage(addrAccount));

    if (onlyIndex) {
        if (index >= storage.size()) {
            std::ostringstream stringStream;
            stringStream << "Storage size: " << storage.size() << " got index: " << index;
            throw JSONRPCError(RPC_INVALID_PARAMS, stringStream.str());
        }
        auto elem = std::next(storage.begin(), index);
        UniValue e(UniValue::VOBJ);

        storage = {{elem->first, {elem->second.first, elem->second.second}}};
    }
    for (const auto& j: storage)
    {
        UniValue e(UniValue::VOBJ);
        e.push_back(Pair(dev::toHex(j.second.first), dev::toHex(j.second.second)));
        ret.push_back(Pair(j.first.hex(), e));
    }
    return ret;
}

static size_t parseUInt(const UniValue& val, size_t defaultVal)
{
    if (val.isNull()) {
        return defaultVal;
    } else {
        int n = val.get_int();
        if (n < 0) {
            throw JSONRPCError(RPC_INVALID_PARAMS, "Expects unsigned integer");
        }

        return n;
    }
}

static int parseBlockHeight(const UniValue& val)
{
    if (val.isStr()) {
        auto blockKey = val.get_str();

        if (blockKey == "latest") {
            return latestblock.height;
        } else {
            throw JSONRPCError(RPC_INVALID_PARAMS, "invalid block number");
        }
    }

    if (val.isNum()) {
        int blockHeight = val.get_int();

        if (blockHeight < 0) {
            return latestblock.height;
        }

        return blockHeight;
    }

    throw JSONRPCError(RPC_INVALID_PARAMS, "invalid block number");
}

static int parseBlockHeight(const UniValue& val, int defaultVal)
{
    if (val.isNull()) {
        return defaultVal;
    } else {
        return parseBlockHeight(val);
    }
}

static dev::h160 parseParamH160(const UniValue& val)
{
    if (!val.isStr()) {
        throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid hex 160");
    }

    auto addrStr = val.get_str();

    if (addrStr.length() != 40 || !CheckHex(addrStr)) {
        throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid hex 160 string");
    }
    return dev::h160(addrStr);
}

static void parseParam(const UniValue& val, std::vector<dev::h160>& h160s)
{
    if (val.isNull()) {
        return;
    }

    // Treat a string as an array of length 1
    if (val.isStr()) {
        h160s.push_back(parseParamH160(val.get_str()));
        return;
    }

    if (!val.isArray()) {
        throw JSONRPCError(RPC_INVALID_PARAMS, "Expect an array of hex 160 strings");
    }

    auto vals = val.getValues();
    h160s.resize(vals.size());

    std::transform(vals.begin(), vals.end(), h160s.begin(), [](UniValue val) -> dev::h160 {
        return parseParamH160(val);
    });
}

static void parseParam(const UniValue& val, std::set<dev::h160>& h160s)
{
    std::vector<dev::h160> v;
    parseParam(val, v);
    h160s.insert(v.begin(), v.end());
}

static void parseParam(const UniValue& val, std::vector<boost::optional<dev::h256>>& h256s)
{
    if (val.isNull()) {
        return;
    }

    if (!val.isArray()) {
        throw JSONRPCError(RPC_INVALID_PARAMS, "Expect an array of hex 256 strings");
    }

    auto vals = val.getValues();
    h256s.resize(vals.size());

    std::transform(vals.begin(), vals.end(), h256s.begin(), [](UniValue val) -> boost::optional<dev::h256> {
        if (val.isNull()) {
            return boost::optional<dev::h256>();
        }

        if (!val.isStr()) {
            throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid hex 256 string");
        }

        auto addrStr = val.get_str();

        if (addrStr.length() != 64 || !CheckHex(addrStr)) {
            throw JSONRPCError(RPC_INVALID_PARAMS, "Invalid hex 256 string");
        }

        return boost::optional<dev::h256>(dev::h256(addrStr));
    });
}

static void addJSON(UniValue& entry, const TxExecRecordInfo& resExec)
{
    entry.push_back(Pair("blockHash", resExec.blockHash.GetHex()));
    entry.push_back(Pair("blockNumber", uint64_t(resExec.blockNumber)));
    entry.push_back(Pair("transactionHash", resExec.transactionHash.GetHex()));
    entry.push_back(
        Pair("transactionIndex", uint64_t(resExec.transactionIndex)));
    entry.push_back(Pair("from", resExec.from.hex()));
    entry.push_back(Pair("to", resExec.to.hex()));
    entry.push_back(
        Pair("cumulativeGasUsed", CAmount(resExec.cumulativeGasUsed)));
    entry.push_back(Pair("gasUsed", CAmount(resExec.gasUsed)));
    entry.push_back(Pair("contractAddress", resExec.contractAddress.hex()));
    std::stringstream ss;
    ss << resExec.excepted;
    entry.push_back(Pair("excepted", ss.str()));
}

static void addJSON(UniValue& logEntry, const dev::eth::LogEntry& log, bool includeAddress)
{
    if (includeAddress) {
        logEntry.push_back(Pair("address", log.address.hex()));
    }

    UniValue topics(UniValue::VARR);
    for (dev::h256 hash : log.topics) {
        topics.push_back(hash.hex());
    }
    logEntry.push_back(Pair("topics", topics));
    logEntry.push_back(Pair("data", HexStr(log.data)));
}

static void TxExecRecordInfoToJSON(const TxExecRecordInfo& resExec, UniValue& entry)
{
    addJSON(entry, resExec);

    const auto& logs = resExec.logs;
    UniValue logEntries(UniValue::VARR);
    for (const auto& log : logs) {
        UniValue logEntry(UniValue::VOBJ);
        addJSON(logEntry, log, true);
        logEntries.push_back(logEntry);
    }
    entry.push_back(Pair("log", logEntries));
}

class WaitForLogsParams
{
public:
    int fromBlock;
    int toBlock;

    int minconf;

    std::set<dev::h160> addresses;
    std::vector<boost::optional<dev::h256>> topics;

    // bool wait;
    WaitForLogsParams(const UniValue& params)
    {
        std::unique_lock<std::mutex> lock(cs_blockchange);
        fromBlock = parseBlockHeight(params[0], latestblock.height + 1);
        toBlock = parseBlockHeight(params[1], -1);
        parseFilter(params[2]);
        minconf = parseUInt(params[3], 6);
    }

private:
    void parseFilter(const UniValue& val)
    {
        if (val.isNull()) {
            return;
        }

        parseParam(val["addresses"], addresses);
        parseParam(val["topics"], topics);
    }
};

UniValue waitforexecrecord(const JSONRPCRequest& request_)
{
    // this is a long poll function. force cast to non const pointer
    JSONRPCRequest& request = (JSONRPCRequest&)request_;
    if (request.fHelp) {
        throw std::runtime_error(
            "waitforexecrecord (fromBlock) (toBlock) (filter) (minconf)\n"
            "requires -logevents to be enabled\n"
            "\nWaits for a new logs and return matching log entries. When the call returns, it also specifies the next block number to start waiting for new logs.\n"
            "By calling waitforexecrecord repeatedly using the returned `nextBlock` number, a client can receive a stream of up-to-date log entires.\n"
            "\nThis call is different from the similarly named `waitforexecrecord`. This call returns individual matching log entries, `searchexecrecord` returns a transaction receipt if one of the log entries of that transaction matches the filter conditions.\n"
            "\nArguments:\n"
            "1. fromBlock (int | \"latest\", optional, default=null) The block number to start looking for logs. ()\n"
            "2. toBlock   (int | \"latest\", optional, default=null) The block number to stop looking for logs. If null, will wait indefinitely into the future.\n"
            "3. filter    ({ addresses?: Hex160String[], topics?: Hex256String[] }, optional default={}) Filter conditions for logs. Addresses and topics are specified as array of hexadecimal strings\n"
            "4. minconf   (uint, optional, default=6) Minimal number of confirmations before a log is returned\n"
            "\nResult:\n"
            "An object with the following properties:\n"
            "1. logs (LogEntry[]) Array of matchiing log entries. This may be empty if `filter` removed all entries."
            "2. count (int) How many log entries are returned."
            "3. nextBlock (int) To wait for new log entries haven't seen before, use this number as `fromBlock`"
            "\nUsage:\n"
            "`waitforexecrecord` waits for new logs, starting from the tip of the chain.\n"
            "`waitforexecrecord 600` waits for new logs, but starting from block 600. If there are logs available, this call will return immediately.\n"
            "`waitforexecrecord 600 700` waits for new logs, but only up to 700th block\n"
            "`waitforexecrecord null null` this is equivalent to `waitforexecrecord`, using default parameter values\n"
            "`waitforexecrecord null null` { \"addresses\": [ \"ff0011...\" ], \"topics\": [ \"c0fefe\"] }` waits for logs in the future matching the specified conditions\n"
            "\nSample Output:\n"
            "{\n  \"entries\": [\n    {\n      \"blockHash\": \"56d5f1f5ec239ef9c822d9ed600fe9aa63727071770ac7c0eabfc903bf7316d4\",\n      \"blockNumber\": 3286,\n      \"transactionHash\": \"00aa0f041ce333bc3a855b2cba03c41427cda04f0334d7f6cb0acad62f338ddc\",\n      \"transactionIndex\": 2,\n      \"from\": \"3f6866e2b59121ada1ddfc8edc84a92d9655675f\",\n      \"to\": \"8e1ee0b38b719abe8fa984c986eabb5bb5071b6b\",\n      \"cumulativeGasUsed\": 23709,\n      \"gasUsed\": 23709,\n      \"contractAddress\": \"8e1ee0b38b719abe8fa984c986eabb5bb5071b6b\",\n      \"topics\": [\n        \"f0e1159fa6dc12bb31e0098b7a1270c2bd50e760522991c6f0119160028d9916\",\n        \"0000000000000000000000000000000000000000000000000000000000000002\"\n      ],\n      \"data\": \"00000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000003\"\n    }\n  ],\n\n  \"count\": 7,\n  \"nextblock\": 801\n}\n");
    }

    if (!fLogEvents)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Events indexing disabled");

    WaitForLogsParams params(request.params);

    request.PollStart();
    std::vector<std::vector<uint256>> hashesToBlock;
    int curheight = 0;
    auto& addresses = params.addresses;
    auto& filterTopics = params.topics;
    while (curheight == 0) {
        {
            LOCK(cs_main);
            curheight = pblocktree->ReadHeightIndex(params.fromBlock, params.toBlock, params.minconf,
                hashesToBlock, addresses);
        }

        // if curheight >= fromBlock. Blockchain extended with new log entries. Return next block height to client.
        //    nextBlock = curheight + 1
        // if curheight == 0. No log entry found in index. Wait for new block then try again.
        //    nextBlock = fromBlock
        // if curheight == -1. Incorrect parameters has entered.
        //
        // if curheight advanced, but all filtered out, API should return empty array, but advancing the cursor anyway.

        if (curheight > 0) {
            break;
        }

        if (curheight == -1) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Incorrect params");
        }

        // check the range
        if (params.toBlock > -1 && latestblock.height >= params.toBlock) {
            break;
        }

        // wait for a new block to arrive
        {
            while (true) {
                std::unique_lock<std::mutex> lock(cs_blockchange);
                auto blockHeight = latestblock.height;
                request.PollPing();

                cond_blockchange.wait_for(lock, std::chrono::milliseconds(1000));
                if (latestblock.height > blockHeight) {
                    break;
                }

                // TODO: maybe just merge `IsRPCRunning` this into PollAlive
                if (!request.PollAlive() || !IsRPCRunning()) {
                    LogPrintf("waitforexecrecord client disconnected\n");
                    return NullUniValue;
                }
            }
        }
    }

    LOCK(cs_main);

    UniValue jsonLogs(UniValue::VARR);

    for (const auto& txHashes : hashesToBlock) {
        for (const auto& txHash : txHashes) {
            std::vector<TxExecRecordInfo> receipts = TxExecRecord::Instance()->Get(
                uintToh256(txHash));

            for (const auto& receipt : receipts) {
                for (const auto& log : receipt.logs) {
                    bool includeLog = true;

                    if (!filterTopics.empty()) {
                        for (size_t i = 0; i < filterTopics.size(); i++) {
                            auto filterTopic = filterTopics[i];

                            if (!filterTopic) {
                                continue;
                            }

                            auto filterTopicContent = filterTopic.get();
                            auto topicContent = log.topics[i];

                            if (topicContent != filterTopicContent) {
                                includeLog = false;
                                break;
                            }
                        }
                    }


                    if (!includeLog) {
                        continue;
                    }

                    UniValue jsonLog(UniValue::VOBJ);
                    addJSON(jsonLog, log, false);
                    jsonLogs.push_back(jsonLog);
                }

                UniValue jsonLog(UniValue::VOBJ);
                addJSON(jsonLog, receipt);
                jsonLogs.push_back(jsonLog);
            }
        }
    }

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("entries", jsonLogs));
    result.push_back(Pair("count", (int)jsonLogs.size()));
    result.push_back(Pair("nextblock", curheight + 1));

    return result;
}

class SearchLogsParams
{
public:
    size_t fromBlock;
    size_t toBlock;
    size_t minconf;

    std::set<dev::h160> addresses;
    std::vector<boost::optional<dev::h256>> topics;

    SearchLogsParams(const UniValue& params)
    {
        std::unique_lock<std::mutex> lock(cs_blockchange);

        setFromBlock(params[0]);
        setToBlock(params[1]);

        parseParam(params[2]["addresses"], addresses);
        parseParam(params[3]["topics"], topics);

        minconf = parseUInt(params[4], 0);
    }

private:
    void setFromBlock(const UniValue& val)
    {
        if (!val.isNull()) {
            fromBlock = parseBlockHeight(val);
        } else {
            fromBlock = latestblock.height;
        }
    }

    void setToBlock(const UniValue& val)
    {
        if (!val.isNull()) {
            toBlock = parseBlockHeight(val);
        } else {
            toBlock = latestblock.height;
        }
    }
};

UniValue searchexecrecord(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 2)
        throw std::runtime_error(
            "searchexecrecord <fromBlock> <toBlock> (address) (topics)\n"
            "requires -logevents to be enabled"
            "\nArgument:\n"
            "1. \"fromBlock\"        (numeric, required) The number of the earliest block (latest may be given to mean the most recent block).\n"
            "2. \"toBlock\"          (string, required) The number of the latest block (-1 may be given to mean the most recent block).\n"
            "3. \"address\"          (string, optional) An address or a list of addresses to only get logs from particular account(s).\n"
            "4. \"topics\"           (string, optional) An array of values from which at least one must appear in the log entries. The order is important, if you want to leave topics out use null, e.g. [\"null\", \"0x00...\"]. \n"
            "\nExamples:\n" +
            HelpExampleCli("searchexecrecord", "0 100 '{\"addresses\": [\"12ae42729af478ca92c8c66773a3e32115717be4\"]}' '{\"topics\": [\"null\",\"b436c2bf863ccd7b8f63171201efd4792066b4ce8e543dde9c3e9e9ab98e216c\"]}'") + HelpExampleRpc("searchexecrecord", "0 100 {\"addresses\": [\"12ae42729af478ca92c8c66773a3e32115717be4\"]} {\"topics\": [\"null\",\"b436c2bf863ccd7b8f63171201efd4792066b4ce8e543dde9c3e9e9ab98e216c\"]}"));

    if (!fLogEvents)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Events indexing disabled");
    int curheight = 0;

    LOCK(cs_main);

    SearchLogsParams params(request.params);

    std::vector<std::vector<uint256>> hashesToBlock;

    curheight = pblocktree->ReadHeightIndex(params.fromBlock, params.toBlock, params.minconf, hashesToBlock, params.addresses);
    if (curheight == -1) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Incorrect params");
    }

    UniValue result(UniValue::VARR);

    auto topics = params.topics;

    for (const auto& hashesTx : hashesToBlock) {
        for (const auto& e : hashesTx) {
            std::vector<TxExecRecordInfo> txexecrecordinfo = TxExecRecord::Instance()->Get(uintToh256(e));
            for (const auto& receipt : txexecrecordinfo) {
                //  if(receipt.logs.empty()) {
                //      continue;
                //  }

                if (!topics.empty()) {
                    for (size_t i = 0; i < topics.size(); i++) {
                        const auto& tc = topics[i];

                        if (!tc) {
                            continue;
                        }

                        for (const auto& log : receipt.logs) {
                            auto filterTopicContent = tc.get();

                            if (i >= log.topics.size()) {
                                continue;
                            }

                            if (filterTopicContent == log.topics[i]) {
                                goto push;
                            }
                        }
                    }

                    // Skip the log if none of the topics are matched
                    continue;
                }

            push:

                UniValue tri(UniValue::VOBJ);
                TxExecRecordInfoToJSON(receipt, tri);
                result.push_back(tri);
            }
        }
    }

    return result;
}

UniValue getexecrecord(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 1)
        throw std::runtime_error(
            "getexecrecord \"hash\"\n"
            "requires -logevents to be enabled"
            "\nArgument:\n"
            "1. \"hash\"          (string, required) The transaction hash\n");
    if (!fLogEvents)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Events indexing disabled");
    LOCK(cs_main);
    std::string hashTemp = request.params[0].get_str();
    if (hashTemp.size() != 64) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Incorrect hash");
    }
    uint256 hash(uint256S(hashTemp));
    std::vector<TxExecRecordInfo> txexecrecordinfo = TxExecRecord::Instance()->Get(uintToh256(hash));
    UniValue result(UniValue::VARR);
    for (TxExecRecordInfo& t : txexecrecordinfo) {
        UniValue tri(UniValue::VOBJ);
        TxExecRecordInfoToJSON(t, tri);
        result.push_back(tri);
    }
    return result;
}
