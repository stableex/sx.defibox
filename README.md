# **`Defibox`**

> Peripheral EOSIO smart contracts for interacting with Defibox

## Quickstart

```c++
#include "defibox.hpp"

// get reserves
const uint64_t pair_id = 12; // EOS/USDT pair
const pair<asset, asset> reserves = defibox::swap::getReserves( pair_id, symbol_code{"EOS"}, symbol_code{"USDT"});

// calculate price
const asset quantity = asset{10000, symbol{"EOS", 4}};
const asset out = defibox::swap::getAmountOut( quantity, reserves.first, reserves.second );
// => "2.6500 USDT"
```

## Table of Content

- [STATIC `getAmountOut`](#action-getAmountOut)
- [STATIC `getAmountIn`](#action-getAmountIn)
- [STATIC `quote`](#action-quote)
