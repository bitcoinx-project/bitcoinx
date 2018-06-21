# Start the bitcoinxd 
Start bitcoind daemon in regtest mode to create a private blockchain
```
$ ./bitcoinxd -regtest -daemon
```
check the network status
```
$ netstat -an |grep 19005
```
The private regtest network is listening on port 19005 as show below
```
tcp4       0      0  127.0.0.1.19005        *.*                    LISTEN
tcp6       0      0  ::1.19005              *.*                    LISTEN
```

# Check the blockchain info
```
$ ./bitcoinx-cli -regtest getblockchaininfo
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

# Check the wallet status
```
$ ./bitcoinx-cli -regtest getwalletinfo
{
  "walletname": "wallet.dat",
  "walletversion": 139900,
  "balance": 0.0000,
  "unconfirmed_balance": 0.0000,
  "immature_balance": 0.0000,
  "txcount": 0,
  "keypoololdest": 1528777296,
  "keypoolsize": 1000,
  "keypoolsize_hd_internal": 1000,
  "paytxfee": 0.0000,
  "hdmasterkeyid": "0b57e2e185f7277b2d6e56f97ceae733a1fc43a5"
}
$ ./bitcoinx-cli -regtest listaccounts
{
  "": 0.0000
}
```

# Check the default address
```
$ ./bitcoinx-cli -regtest getaccountaddress ""
muZWgKaRMVv8cnJHC6FBoT1X1F6TZyPm5d
```

# Get the new address
The bitcoinx reference client maintains a pool of addresses, the size of which is displayed by keypoolsize when you use the command getinfo.
To get one of these addresses, you can use the getnewaddress command
```
./bitcoinx-cli -regtest getnewaddress "test"
mxLLMBjrFSD4vv3gbtepT8KLnuQdJuVpCT
```

# Check the account again
```
./bitcoinx-cli -regtest listaccounts
{
  "": 0.0000
  "test": 0.0000
}
```

# Mining
Generate a block
```
./bitcoinx-cli -regtest generate 1
[
  "6a8abc2893b830ff054d56c251cf1a828b569cd022c0db7c8d6d6b05a0eb61cb"
]
```

# Show the blockchain info
```
bitcoinx-cli -regtest getblockchaininfo
{
  "chain": "regtest",
  "blocks": 1,
  "headers": 1,
  "bestblockhash": "6a8abc2893b830ff054d56c251cf1a828b569cd022c0db7c8d6d6b05a0eb61cb",
  "difficulty": 4.656542373906925e-10,
  "mediantime": 1527749451,
  "verificationprogress": 1,
  "chainwork": "0000000000000000000000000000000000000000000000000000000000000004",
  "pruned": true,
  // more
```

# Check the transaction and balance
```
$ ./bitcoinx-cli -regtest listtransactions
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

$ ./bitcoinx-cli -regtest getbalance
0.0000
```
Why isn't your balance 500,000? Because the block you mined isn't "confirmed" yet. On the real Bitcoin blockchain, a block is confirmed when there have been 6 blocks after it mined, but on our testnet, confirmation takes 100 additional blocks. 

# Mining 100 More Blocks
Execute this command to mine 100 more blocks:
```
./bitcoinx-cli -regtest generate 100
```
You see 100 block hashes, as shown below
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

# Show the information about the blockchain
```
./bitcoinx-cli -regtest getblockchaininfo
```
Now the blockchain contains 101 blocks, as shown below
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

# Get the balance
```
$ ./bitcoinx-cli -regtest getbalance
500000.0000
```

# Viewing Your Unspend Bitcoins
```
$ ./bitcoinx-cli -regtest listunspent
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
