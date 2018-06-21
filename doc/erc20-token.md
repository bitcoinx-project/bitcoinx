# Starting the Bitcoin Daemon
```
$ ./bitcoinxd -regtest -daemon
```

# Mining 600 blocks to active the contract:
```
$ ./bitcoinx-cli -regtest generate 600
```

# Check the balance:
```
$ ./bitcoinx-cli -regtest listaccounts
{
  "": 130225000.0000
}
```
# Generate an address to test contract:
```
$ ./bitcoinx-cli -regtest getnewaddress "testtoken"
muBTfgkcFKqJf2qEhRNjGEbPgqMJvCgtxH
```

# Viewing your balance:
```
$ ./bitcoinx-cli -regtest listaccounts
{
  "": 130225000.0000,
  "testtoken": 0.0000
}
```
# Sending bitcoinx to the new address
```
$ ./bitcoinx-cli -regtest sendtoaddress muBTfgkcFKqJf2qEhRNjGEbPgqMJvCgtxH 8000000
$ ./bitcoinx-cli -regtest sendtoaddress muBTfgkcFKqJf2qEhRNjGEbPgqMJvCgtxH 80
$ ./bitcoinx-cli -regtest sendtoaddress muBTfgkcFKqJf2qEhRNjGEbPgqMJvCgtxH 80
$ ./bitcoinx-cli -regtest sendtoaddress muBTfgkcFKqJf2qEhRNjGEbPgqMJvCgtxH 80
$ ./bitcoinx-cli -regtest sendtoaddress muBTfgkcFKqJf2qEhRNjGEbPgqMJvCgtxH 80000000
```

# Mining a block:
```
$ ./bitcoinx-cli -regtest generate 1
```

# Viewing your balance:
```
$ ./bitcoinx-cli -regtest listaccounts
{
  "": 42274756.6344,
  "testtoken": 88000240.0000
}
```

# Get the default address:
```
$ ./bitcoinx-cli -regtest getaddressesbyaccount ""
[
  "mqjWKYkPtWP2fZepSfCawfUtpDJNUhAxgS"
]
```

# Convert the address to HASH160:
```
$ ./bitcoinx-cli -regtest gethexaddress mqjWKYkPtWP2fZepSfCawfUtpDJNUhAxgS
7010fbaefc158ced94f255b3f4aaf18bf59cf1dc
```

# Get the address of testtoken:
```
$ ./bitcoinx-cli -regtest getaddressesbyaccount "testtoken"
[
  "muBTfgkcFKqJf2qEhRNjGEbPgqMJvCgtxH"
]
```

# Convert the address to HASH160:
```
./bitcoinx-cli -regtest gethexaddress muBTfgkcFKqJf2qEhRNjGEbPgqMJvCgtxH 
95e1f607cb08aacfe33bbf848cfc2fe5f8772682
```

# ERC20 Token
Open [Remix Solidity IDE](https://ethereum.github.io/browser-solidity/#version=soljson-v0.4.19+commit.c4cbbb05.js&optimize=false&gist=) 

Load the StandardToken source code from url below :
```
https://gist.github.com/luoqeng/81fc6a558409df02a6d1ba11ff30464f
```

# Compile the StandardToken contract
+ Select the StandardToken and Click `Start to compile` 
+ Select the StandardToken and Click the `Details`, Copy the object value in `BYTECODE` section

# Deploy StandardToken 

StandardToken constuctor have four parameters：
+ `name`: MyTestToken
+ `symbol`: MTT
+ `decimals`: 8
+ `totalSupply`: 1000

Deploying a contract usually requires the following steps:
1. Compile the contract into bytecode by solidity compiler 
2. Encode the parameters of ABI into bytes [ethabi tool](https://github.com/paritytech/ethabi) ABI encode:
```
$ ethabi encode params -v string MyTestToken -v string MTT -v uint8 8 -v uint256 1000 --lenient
000000000000000000000000000000000000000000000000000000000000008000000000000000000000000000000000000000000000000000000000000000c0000000000000000000000000000000000000000000000000000000000000000800000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000000b4d7954657374546f6b656e00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000034d54540000000000000000000000000000000000000000000000000000000000

```

3. connect 1 and 2 and call the rpc methord createcontract by bitcoinxd
4. wait the confirm
5. Record the address of the contract and owner of the contract for subsequent use

```
$ ./bitcoinx-cli -regtest createcontract 606060405234156200001057600080fd5b6040516200182a3803806200182a83398101604052808051820191906020018051820191906020018051906020019091908051906020019091905050826001908051906020019062000064929190620000ee565b5083600090805190602001906200007d929190620000ee565b5081600260006101000a81548160ff021916908360ff1602179055508060038190555080600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002081905550505050506200019d565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f106200013157805160ff191683800117855562000162565b8280016001018555821562000162579182015b828111156200016157825182559160200191906001019062000144565b5b50905062000171919062000175565b5090565b6200019a91905b80821115620001965760008160009055506001016200017c565b5090565b90565b61167d80620001ad6000396000f3006060604052600436106100ba576000357c0100000000000000000000000000000000000000000000000000000000900463ffffffff16806306fdde03146100bf578063095ea7b31461014d57806318160ddd146101a757806323b872dd146101d0578063313ce56714610249578063661884631461027857806370a08231146102d257806395d89b411461031f578063a9059cbb146103ad578063be45fd6214610407578063d73dd6231461048c578063dd62ed3e146104e6575b600080fd5b34156100ca57600080fd5b6100d2610552565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101125780820151818401526020810190506100f7565b50505050905090810190601f16801561013f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b341561015857600080fd5b61018d600480803573ffffffffffffffffffffffffffffffffffffffff169060200190919080359060200190919050506105fa565b604051808215151515815260200191505060405180910390f35b34156101b257600080fd5b6101ba6106ec565b6040518082815260200191505060405180910390f35b34156101db57600080fd5b61022f600480803573ffffffffffffffffffffffffffffffffffffffff1690602001909190803573ffffffffffffffffffffffffffffffffffffffff169060200190919080359060200190919050506106f6565b604051808215151515815260200191505060405180910390f35b341561025457600080fd5b61025c610a9a565b604051808260ff1660ff16815260200191505060405180910390f35b341561028357600080fd5b6102b8600480803573ffffffffffffffffffffffffffffffffffffffff16906020019091908035906020019091905050610ab1565b604051808215151515815260200191505060405180910390f35b34156102dd57600080fd5b610309600480803573ffffffffffffffffffffffffffffffffffffffff16906020019091905050610d39565b6040518082815260200191505060405180910390f35b341561032a57600080fd5b610332610d82565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610372578082015181840152602081019050610357565b50505050905090810190601f16801561039f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34156103b857600080fd5b6103ed600480803573ffffffffffffffffffffffffffffffffffffffff16906020019091908035906020019091905050610e2a565b604051808215151515815260200191505060405180910390f35b341561041257600080fd5b61048a600480803573ffffffffffffffffffffffffffffffffffffffff1690602001909190803590602001909190803590602001908201803590602001908080601f0160208091040260200160405190810160405280939291908181526020018383808284378201915050505050509190505061103c565b005b341561049757600080fd5b6104cc600480803573ffffffffffffffffffffffffffffffffffffffff16906020019091908035906020019091905050611379565b604051808215151515815260200191505060405180910390f35b34156104f157600080fd5b61053c600480803573ffffffffffffffffffffffffffffffffffffffff1690602001909190803573ffffffffffffffffffffffffffffffffffffffff1690602001909190505061156c565b6040518082815260200191505060405180910390f35b61055a61163d565b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156105f05780601f106105c5576101008083540402835291602001916105f0565b820191906000526020600020905b8154815290600101906020018083116105d357829003601f168201915b5050505050905090565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b6000600354905090565b60008073ffffffffffffffffffffffffffffffffffffffff168373ffffffffffffffffffffffffffffffffffffffff161415151561073357600080fd5b600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115151561078157600080fd5b600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115151561080c57600080fd5b610855600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054836115f3565b600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055506108e1600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020548361160c565b600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055506109aa600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054836115f3565b600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a3600190509392505050565b6000600260009054906101000a900460ff16905090565b600080600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054905080831115610bc2576000600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002081905550610c4d565b610bcc81846115f3565b600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055505b8373ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008873ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546040518082815260200191505060405180910390a3600191505092915050565b6000600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020549050919050565b610d8a61163d565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610e205780601f10610df557610100808354040283529160200191610e20565b820191906000526020600020905b815481529060010190602001808311610e0357829003601f168201915b5050505050905090565b60008073ffffffffffffffffffffffffffffffffffffffff168373ffffffffffffffffffffffffffffffffffffffff1614151515610e6757600080fd5b600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020548211151515610eb557600080fd5b610efe600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054836115f3565b600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002081905550610f8a600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020548361160c565b600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a36001905092915050565b6000808311151561104c57600080fd5b6110558461162a565b15611181578390508073ffffffffffffffffffffffffffffffffffffffff1663c0ee0b8a3385856040518463ffffffff167c0100000000000000000000000000000000000000000000000000000000028152600401808473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200183815260200180602001828103825283818151815260200191508051906020019080838360005b8381101561111f578082015181840152602081019050611104565b50505050905090810190601f16801561114c5780820380516001836020036101000a031916815260200191505b50945050505050600060405180830381600087803b151561116c57600080fd5b6102c65a03f1151561117d57600080fd5b5050505b6111d383600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546115f390919063ffffffff16565b600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208190555061126883600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205461160c90919063ffffffff16565b600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002081905550816040518082805190602001908083835b6020831015156112e157805182526020820191506020810190506020830392506112bc565b6001836020036101000a03801982511681845116808217855250505050505090500191505060405180910390208473ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167fe19260aff97b920c7df27010903aeb9c8d2be5d310a2c67824cf3f15396e4c16866040518082815260200191505060405180910390a450505050565b6000611401600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020548361160c565b600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546040518082815260200191505060405180910390a36001905092915050565b6000600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054905092915050565b600082821115151561160157fe5b818303905092915050565b600080828401905083811015151561162057fe5b8091505092915050565b600080823b905060008111915050919050565b6020604051908101604052806000815250905600a165627a7a72305820a26f56c19d817d210435708440e76391f6ef3236518a14088f6c9cf336649acd0029000000000000000000000000000000000000000000000000000000000000008000000000000000000000000000000000000000000000000000000000000000c0000000000000000000000000000000000000000000000000000000000000000800000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000000b4d7954657374546f6b656e00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000034d54540000000000000000000000000000000000000000000000000000000000 40000000 0.004 muBTfgkcFKqJf2qEhRNjGEbPgqMJvCgtxH
{
  "txid": "5eafcc871ccfaabefc4d311cc2144a24c63b4c54d453c14623b7ade5f8bcf4da",
  "sender": "muBTfgkcFKqJf2qEhRNjGEbPgqMJvCgtxH",
  "hash160": "95e1f607cb08aacfe33bbf848cfc2fe5f8772682",
  "address": "a51f0b09623c8b57b61f4faf03f93cb5cb1ba618"
}
```

Generate a block:
```
$ ./bitcoinx-cli -regtest generate 1
```

Viewing the contract info by address:
```
$ ./bitcoinx-cli -regtest getcontractinfo a51f0b09623c8b57b61f4faf03f93cb5cb1ba618
{
  "address": "a51f0b09623c8b57b61f4faf03f93cb5cb1ba618",
  "balance": 0,
  "storage": {
    "2477df5dcca1fcbd1e4d98dcdc349f3c76f72e8e532c767bac8f58132bd3a27e": {
      "15b2c48f2ffd1f13064ea33590ae83a2b4e4ea3161c715ed9b317592509e14b7": "00000000000000000000000000000000000000000000000000000000000003ff"
    },
    "290decd9548b62a8d60345a988386fc84ba6bc95484008f6362f93160ef3e563": {
      "0000000000000000000000000000000000000000000000000000000000000000": "4dffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
    },
    "405787fa12a823e0f2b7631cc41b3ba8828b3321ca811111fa75cd3aa3bb5ace": {
      "0000000000000000000000000000000000000000000000000000000000000002": "0000000000000000000000000000000000000000000000000000000000000008"
    },
    "b10e2d527612073b26eecdfd717e6a320cf44b4afac2b0732d9fcbe2b7fa0cf6": {
      "0000000000000000000000000000000000000000000000000000000000000001": "4dffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
    },
    "c2575a0e9e593c00f959f8c92f12db2869c3395a3b0502d05e2516446f71f85b": {
      "0000000000000000000000000000000000000000000000000000000000000003": "00000000000000000000000000000000000000000000000000000000000003ff"
    }
  },
  // more
```

# Test contract function
ABI is the interface for interacting with contracts. To use the contracts deployed in the blockchain, you need to use the ethabi tool to convert contract functions and parameters to the ABI data

Create an empty file, StandToken.json, click on `Details` in the Remix Solidity IDE, copy the contents of `web3.eth.contract` in the code, and save it to StandardToken.json. For example, the above contract ABI is as follows:
```
[{"constant":true,"inputs":[],"name":"name","outputs":[{"name":"","type":"string"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"_spender","type":"address"},{"name":"_value","type":"uint256"}],"name":"approve","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"totalSupply","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"_from","type":"address"},{"name":"_to","type":"address"},{"name":"_value","type":"uint256"}],"name":"transferFrom","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"decimals","outputs":[{"name":"","type":"uint8"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"_spender","type":"address"},{"name":"_subtractedValue","type":"uint256"}],"name":"decreaseApproval","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[{"name":"_owner","type":"address"}],"name":"balanceOf","outputs":[{"name":"balance","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"symbol","outputs":[{"name":"","type":"string"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"_to","type":"address"},{"name":"_value","type":"uint256"}],"name":"transfer","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"name":"_to","type":"address"},{"name":"_value","type":"uint256"},{"name":"_data","type":"bytes"}],"name":"transfer","outputs":[],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":false,"inputs":[{"name":"_spender","type":"address"},{"name":"_addedValue","type":"uint256"}],"name":"increaseApproval","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[{"name":"_owner","type":"address"},{"name":"_spender","type":"address"}],"name":"allowance","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"inputs":[{"name":"name","type":"string"},{"name":"symbol","type":"string"},{"name":"decimals","type":"uint8"},{"name":"totalSupply","type":"uint256"}],"payable":false,"stateMutability":"nonpayable","type":"constructor"},{"anonymous":false,"inputs":[{"indexed":true,"name":"from","type":"address"},{"indexed":true,"name":"to","type":"address"},{"indexed":false,"name":"value","type":"uint256"},{"indexed":true,"name":"data","type":"bytes"}],"name":"Transfer","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"name":"from","type":"address"},{"indexed":true,"name":"to","type":"address"},{"indexed":false,"name":"value","type":"uint256"}],"name":"Transfer","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"name":"owner","type":"address"},{"indexed":true,"name":"spender","type":"address"},{"indexed":false,"name":"value","type":"uint256"}],"name":"Approval","type":"event"}]

```

function hash:
```
FUNCTIONHASHES
{
    "66188463": "decreaseApproval(address,uint256)",
    "dd62ed3e": "allowance(address,address)",
    "095ea7b3": "approve(address,uint256)",
    "70a08231": "balanceOf(address)",
    "313ce567": "decimals()",
    "d73dd623": "increaseApproval(address,uint256)",
    "06fdde03": "name()",
    "95d89b41": "symbol()",
    "18160ddd": "totalSupply()",
    "a9059cbb": "transfer(address,uint256)",
    "be45fd62": "transfer(address,uint256,bytes)",
    "23b872dd": "transferFrom(address,address,uint256)"
}
```

The function `balanceOf` ABI encode: 
95e1f607cb08aacfe33bbf848cfc2fe5f8772682 is hash160 of the contract's address 
```
$ ethabi encode function ./StandardToken.json balanceOf -p 95e1f607cb08aacfe33bbf848cfc2fe5f8772682 --lenient
70a0823100000000000000000000000095e1f607cb08aacfe33bbf848cfc2fe5f8772682
```

Query the number of tokens owned by the contract account
a51f0b09623c8b57b61f4faf03f93cb5cb1ba618 is the contract's address, output is the hex number of tokens: 
```
$ ./bitcoinx-cli -regtest callcontract a51f0b09623c8b57b61f4faf03f93cb5cb1ba618 70a0823100000000000000000000000095e1f607cb08aacfe33bbf848cfc2fe5f8772682
{
  "address": "a51f0b09623c8b57b61f4faf03f93cb5cb1ba618",
  "executionResult": {
    "gasUsed": 23341,
    "excepted": "None",
    "newAddress": "a51f0b09623c8b57b61f4faf03f93cb5cb1ba618",
    "output": "00000000000000000000000000000000000000000000000000000000000003ff",
    "codeDeposit": 0,
    "gasRefunded": 0,
    "depositSize": 0,
    "gasForDeposit": 0
  },
  "transactionReceipt": {
    "stateRoot": "b793837a8419c6195bf36ddb5b2f61769a6b6f6685ec684eaa1b22151526230d",
    "gasUsed": 23341,
    "bloom": "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
    "log": [
    ]
  }
}
```

Query the number of tokens owned by the default account we will send to
7010fbaefc158ced94f255b3f4aaf18bf59cf1dc is the default address hash160
output is the hexadecimal number of tokens 
balanceOf ABI encode: 
```
$ ethabi encode function ./StandardToken.json balanceOf -p 7010fbaefc158ced94f255b3f4aaf18bf59cf1dc --lenient
70a082310000000000000000000000007010fbaefc158ced94f255b3f4aaf18bf59cf1dc

$ bitcoinx-cli -regtest callcontract a51f0b09623c8b57b61f4faf03f93cb5cb1ba618 70a082310000000000000000000000007010fbaefc158ced94f255b3f4aaf18bf59cf1dc
{
  "address": "a51f0b09623c8b57b61f4faf03f93cb5cb1ba618",
  "executionResult": {
    "gasUsed": 23341,
    "excepted": "None",
    "newAddress": "a51f0b09623c8b57b61f4faf03f93cb5cb1ba618",
    "output": "0000000000000000000000000000000000000000000000000000000000000000",
    "codeDeposit": 0,
    "gasRefunded": 0,
    "depositSize": 0,
    "gasForDeposit": 0
  },
  "transactionReceipt": {
    "stateRoot": "b793837a8419c6195bf36ddb5b2f61769a6b6f6685ec684eaa1b22151526230d",
    "gasUsed": 23341,
    "bloom": "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
    "log": [
    ]
  }
}
```

Because our contract code overloaded the transfer function, and currently 'ethabi' does not support overloading, so we need to separately copy `tansfer` fucntion, create an empty file TransferToken.json, save the following json content:
```
[{
    "constant":false,
    "inputs":[
        {
            "name":"_to",
            "type":"address"
        },
        {
            "name":"_value",
            "type":"uint256"
        }
    ],
    "name":"transfer",
    "outputs":[
        {
            "name":"",
            "type":"bool"
        }
    ],
    "payable":false,
    "stateMutability":"nonpayable",
    "type":"function"
}]
```

transfer function ABI encode: 
```
$ ethabi encode function ./TransferToken.json transfer -p 7010fbaefc158ced94f255b3f4aaf18bf59cf1dc 1 --lenient
a9059cbb0000000000000000000000007010fbaefc158ced94f255b3f4aaf18bf59cf1dc0000000000000000000000000000000000000000000000000000000000000001
```

Call the contract, send a token to the default account, due to the need to store the consumption of gas, call sendtocontract to generate the transaction:
```
$ ./bitcoinx-cli -regtest sendtocontract a51f0b09623c8b57b61f4faf03f93cb5cb1ba618 a9059cbb0000000000000000000000007010fbaefc158ced94f255b3f4aaf18bf59cf1dc0000000000000000000000000000000000000000000000000000000000000001 0 40000000 0.004 muBTfgkcFKqJf2qEhRNjGEbPgqMJvCgtxH
{
  "txid": "e588179f130424aa90dd2fd283c07ca6ad3712414f296097646d30912c19d5b5",
  "sender": "muBTfgkcFKqJf2qEhRNjGEbPgqMJvCgtxH",
  "hash160": "95e1f607cb08aacfe33bbf848cfc2fe5f8772682"
}
```

Generate a block to confirm the transaction:
```
./bitcoinx-cli -regtest generate 1
```

Call the contract, check the number of tokens in the default account, the output is 1: 
```
./bitcoinx-cli -regtest callcontract a51f0b09623c8b57b61f4faf03f93cb5cb1ba618 70a082310000000000000000000000007010fbaefc158ced94f255b3f4aaf18bf59cf1dc
{
  "address": "a51f0b09623c8b57b61f4faf03f93cb5cb1ba618",
  "executionResult": {
    "gasUsed": 23341,
    "excepted": "None",
    "newAddress": "a51f0b09623c8b57b61f4faf03f93cb5cb1ba618",
    "output": "0000000000000000000000000000000000000000000000000000000000000001",
    "codeDeposit": 0,
    "gasRefunded": 0,
    "depositSize": 0,
    "gasForDeposit": 0
  },
  "transactionReceipt": {
    "stateRoot": "7032463229b3ebfdfd7a0d0ff3ffaee8a8b59860579e315318f110a71d0488d9",
    "gasUsed": 23341,
    "bloom": "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
    "log": [
    ]
  }
}
```

# Tools 
+ [Remix Solidity IDE](https://ethereum.github.io/browser-solidity/#version=soljson-v0.4.19+commit.c4cbbb05.js&optimize=false&gist=) Compile and Debug
+ [ethabi tool](https://github.com/paritytech/ethabi) ABI Encode: 
