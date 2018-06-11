# 启动 bitcoinx 守护进程
执行这个命令启动 bitcoinx 守护进程：
```
./bitcoinxd -regtest -daemon
```
执行此命令以查看侦听进程：
```
netstat -pant
```
我们的私人 regtest 网络的守护进程正在侦听端口 19005，如下所示。真正的 bitcoinx 网络监听端口 9005，公共测试网络侦听端口 19005。
```
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name
tcp        0      0 127.0.0.1:6379          0.0.0.0:*               LISTEN      -
tcp        0      0 0.0.0.0:9005            0.0.0.0:*               LISTEN      105197/./bitcoinxd
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      -
tcp        0      0 127.0.0.1:25            0.0.0.0:*               LISTEN      -
tcp        0      0 0.0.0.0:19005           0.0.0.0:*               LISTEN      105197/./bitcoinxd
```

# 检查区块链
执行此命令可查看有关区块链的信息：
```
./bitcoinx-cli -regtest getblockchaininfo
```
向上滚动查看输出的开始。此时，我们的区块链包含零区块，如下所示。
```
{
  "chain": "regtest",
  "blocks": 0,
  "headers": 0,
  "bestblockhash": "0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206",
  "difficulty": 4.656542373906925e-10,
  "mediantime": 1296688602,
  "verificationprogress": 1,
  "chainwork": "0000000000000000000000000000000000000000000000000000000000000002",
  "pruned": true,
  // more
```

# 检查电子钱包状态
执行此命令：
```
./bitcoinx-cli -regtest getinfo
```
您的钱包的 balance 为零，如下所示。
```
{
  "deprecation-warning": "WARNING: getinfo is deprecated and will be fully removed in 0.16. Projects should transition to using getblockchaininfo, getnetworkinfo, and getwalletinfo before upgrading to 0.16",
  "version": 160100,
  "protocolversion": 70015,
  "walletversion": 139900,
  "balance": 0.0000,
  "blocks": 0,
  "timeoffset": 0,
  "connections": 0,
  "proxy": "",
  "difficulty": 4.656542373906925e-10,
  "testnet": false,
  "keypoololdest": 1527748085,
  "keypoolsize": 2000,
  "paytxfee": 0.0000,
  "relayfee": 0.1000,
  "errors": ""
}
```

或者执行
```
./bitcoinx-cli -regtest listaccounts
```
您的钱包的余额为零，如下所示。
```
{
  "": 0.0000
}
```

查看默认地址
```
./bitcoinx-cli -regtest getaccountaddress ""
mn1yDNH9zNNB39SMtZNwzoH9DGuz44Kswa
```

获得新地址

您可以拥有任意数量的地址。如果你想让其他人难以发现你的钱在哪里，你可以为每笔交易创建一个新的 bitcoinx 地址。当人们说比特币是“匿名的”时，这就是人们的意思。

执行此命令以从可用的地址池中获取新地址（默认情况下为 100）。
```
./bitcoinx-cli -regtest getnewaddressi "test"
```
如下所示。这是一个很长的随机字节序列。地址以 m 或 n 开头，因为它位于测试网络上。真正区块链上的 bitcoinx 地址以 1 或 3 开头。
```
n4kVsa58dxXiNoLbie8hQznrBGNrkFqdES
```

再次查看账户
```
./bitcoinx-cli -regtest listaccounts
```
您的钱包的余额为零，如下所示。
```
{
  "": 0.0000
  "test": 0.0000
}
```

# 挖矿
执行这些命令来挖矿，并再次查看有关区块链的信息：
```
./bitcoinx-cli -regtest generate 1
[
  "10db89f79b5c287a575b7c7b8e4b0d300d584555f957f92fc5d97c80f2524b2d"
]
```

现在区块链包含一个区块，如下所示。
```
bitcoinx-cli -regtest getblockchaininfo
{
  "chain": "regtest",
  "blocks": 1,
  "headers": 1,
  "bestblockhash": "10db89f79b5c287a575b7c7b8e4b0d300d584555f957f92fc5d97c80f2524b2d",
  "difficulty": 4.656542373906925e-10,
  "mediantime": 1527749451,
  "verificationprogress": 1,
  "chainwork": "0000000000000000000000000000000000000000000000000000000000000004",
  "pruned": true,
  // more
```

在真正的 bitcoinx 区块链中，难度设置得高得多，因此挖矿需要昂贵的专用计算机集群。但是我们的测试网络的难度非常低，所以挖矿很容易。

查看交易和余额

执行此命令查看您的所有交易：
```
./bitcoinx-cli -regtest listtransactions
```
有一笔交易，数量为 500000
```
[
  {
    "account": "",
    "address": "mwPFqCA3Rmv5XgDEMo2X55Ye96g8C9VhSY",
    "category": "immature",
    "amount": 500000.0000,
    "vout": 0,
    "confirmations": 1,
    "generated": true,
    "blockhash": "10db89f79b5c287a575b7c7b8e4b0d300d584555f957f92fc5d97c80f2524b2d",
    "blockindex": 0,
    "blocktime": 1527749451,
    "txid": "9af8aabd69564be1c6ec484a5a480e01484e0233341f8d44a73c02c2500999f0",
    "walletconflicts": [
    ],
    "time": 1527749451,
    "timereceived": 1527749451,
    "bip125-replaceable": "no"
  }
]
```

执行此命令查看您的余额：
```
./bitcoinx-cli -regtest getbalance
```
您的余额为 0，如下所示。
```
0.0000
```
为什么你的余额不是 500000 ？因为您开采的区块尚未 确认。在真正的 bitcoinx 区块链中，一个区块在开采后有 6 个区块被确认，但在我们的测试网上，确认需要 100 个额外的区块。

挖 100 个更多块

执行这个命令来挖 100 个更多的块：
```
./bitcoinx-cli -regtest generate 100
```
您会看到 100 个块哈希，如下所示
```
[
  "0f4b113ae606d4fa9bdbd352b4ea1b4dcbd5e8506f286245c450ed152b40b04d",
  "2dcfbe5f3579846dbf196dc36751bd9e1fe7c1ad1d16e416fe18ad11029e89c2",
  "7d10ab1b1f3669adad7564214435da508933c0a477e72d73477c9709274aa336",
  "66a831351c1304fa96279ff0c04449942be36a7dd0bbcdfe39308ab9250b7a1a",
  "0f580818252070fc66c49cc6197c5cf88c1f83110fc899ff43bd135e08f0877b",
  "3e3b6a559f9c65ef14dfec810be731e07716361391ce0937bf785caf7ace23a2",
  "0f75636285b3625c0ddcf56256496e3557280293bda54cce133a8774eca59f40",
  "404da8c21e52d80be0b9c241a82da5b14c867029dd7b15c1d179b9f98927ec74",
  "12d9f6c762397a1aac1d9d56a627c687fe1523cbfb0d50368da230b87f39e266",
  // more
```

执行此命令可查看有关区块链的信息：
```
./bitcoinx-cli -regtest getblockchaininfo
```
现在区块链包含 101 个区块，如下所示。
```
{
  "chain": "regtest",
  "blocks": 101,
  "headers": 101,
  "bestblockhash": "03e0a1864f1d40b89ec1ec92cc53c3f901ac8ed8ea8cd711e3a4b75812430101",
  "difficulty": 4.656542373906925e-10,
  "mediantime": 1527750167,
  "verificationprogress": 1,
  "chainwork": "00000000000000000000000000000000000000000000000000000000000000cc",
  "pruned": true,
  // more
```


执行此命令查看您的余额：
```
./bitcoinx-cli -regtest getbalance
```
您的余额为 500000，如下所示
```
500000.0000
```

查看您未使用的 BCX

执行此命令查看您未使用的 BCX
```
./bitcoinx-cli -regtest listunspent
```
你看到一个交易有 500000，这是可以消费的，如下所示。
```
[
  {
    "txid": "9af8aabd69564be1c6ec484a5a480e01484e0233341f8d44a73c02c2500999f0",
    "vout": 0,
    "address": "mwPFqCA3Rmv5XgDEMo2X55Ye96g8C9VhSY",
    "scriptPubKey": "210297f3ed6a34bda8d28ee535e72190aab19f0c9c6fecb8a71e2e0ac2e710da8dedac",
    "amount": 500000.0000,
    "confirmations": 101,
    "spendable": true,
    "solvable": true,
    "safe": true
  }
]
```
