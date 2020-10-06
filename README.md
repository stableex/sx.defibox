# **`Defibox`**

> Peripheral EOSIO smart contracts for interacting with Defibox

## Dependencies

- [`sx.uniswap`](https://github.com/stableex/sx.uniswap)

## Quickstart

```c++
#include <sx.defibox/defibox.hpp>
#include <sx.uniswap/uniswap.hpp>

// user input
const asset quantity = asset{10000, symbol{"EOS", 4}};
const uint64_t pair_id = 12; // EOS/USDT pair

// get defibox info
const auto [ reserve_in, reserve_out ] = defibox::get_reserves( pair_id, quantity.symbol );
const uint8_t fee = defibox::get_fee();

// calculate out price
const asset out = uniswap::get_amount_out( quantity, reserves_in, reserves_out, fee );
// => "2.6500 USDT"
```

## Table of Content

- [STATIC `get_reserves`](#static-get_reserves)
- [STATIC `get_fee`](#static-get_fee)

## STATIC `get_reserves`

Get reserves for a pair

### params

- `{uint64_t} pair_id` - pair id
- `{symbol} sort` - sort by symbol (reserve0 will be first item in pair)

### returns

- `{pair<asset, asset>}` - pair of reserve assets

### example

```c++
const uint64_t pair_id = 12;
const symbol sort = symbol{"EOS", 4};

const auto [reserve0, reserve1] = defibox::get_reserves( pair_id );
// reserve0 => "4585193.1234 EOS"
// reserve1 => "12568203.3533 USDT"
```

## STATIC `get_fee`

Get Defibox total fee

### returns

- `{uint8_t}` - total fee (trade + protocol)

### example

```c++
const uint8_t fee = defibox::get_fee();
// => 30
```