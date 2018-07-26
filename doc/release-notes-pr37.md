RPC changes
------------------

### Mining
- The `getblocktemplate` RPC method now must includes "contract" in template's "rules'" list (BIP 9) to active the contract function. 
```
  {"rules": ["segwit", "contract"]}
```

==Reference Implementation==

* [https://github.com/bitcoin/bips/blob/master/bip-0009.mediawiki#getblocktemplate_changes BIP9]

### Contract
The contract feature is available throught the RPC interface

| New Method            | Notes       |
| :-------------------- | :-----------|
| `callcontract`        | `callcontract` returns a json object with address information and contract execution result. |
| `listcontract`        | `listcontract` returns a list of deployed contract addresses with the balance. |
| `getcontractinfo`     | `getcontractinfo` returns a json object with the contract's low level information. |
| `getcontractstorage`  | `getcontractstorage` returns a json object with the storage information about a contract. |
| `searchexecrecord`    | `searchexecrecord` returns a json object with the transaction execution records in some blocks. |
| `waitforexecrecord`   | `waitforexecrecord` returns a json object with the new transaction execution records information if the contract is confirmed. |
| `getexecrecord`       | `getexecrecord` returns a json object withe a transaction execution record. |

