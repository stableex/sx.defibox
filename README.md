# **`Defibox`**

> Peripheral EOSIO smart contracts for interacting with Defibox

## Quickstart

```c++
#include "defibox.hpp"

// user input
const asset quantity = asset{10000, symbol{"EOS", 4}};
const uint64_t pair_id = 12; // EOS/USDT pair

// get reserves
const auto [ reserveIn, reserveOut ] = defibox::getReserves( pair_id, quantity.symbol );

// calculate out price
const asset out = defibox::getAmountOut( quantity, reservesIn, reservesOut );
// => "2.6500 USDT"
```

## Table of Content

- [STATIC `getAmountOut`](#action-getAmountOut)
- [STATIC `getAmountIn`](#action-getAmountIn)
- [STATIC `quote`](#action-quote)
