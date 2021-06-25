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
- [STATIC `get_pairid_from_lptoken`](#static-get_pairid_from_lptoken)
- [STATIC `get_withdraw_out`](#static-get_withdraw_out)

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

const auto [reserve0, reserve1] = defibox::get_reserves( pair_id, sort );
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

## STATIC `get_pairid_from_lptoken`

Get Defibox pair id from LP token

### params

- `{symbol_code} symcode` - Defibox LP token

### returns

- `{uint64_t}` - pair id

### example

```c++
const uint64_t pair_id = defibox::get_pairid_from_lptoken( {"BOXGL"} );
// => 194

```
## STATIC `get_withdraw_out`

Get reserve tokens amounts after liquidity withdraw

### params

- `{extended_asset} lp_token` - LP tokens

### returns

- `{pair<extended_asset, extended_asset>}` - pair of reserve assets to receive after withdaw

### example

```c++
const extended_asset lp_tokens = extended_asset{ 12345678, { {"BOXGL", 0}, "lptoken.defi"_n } };
const auto [amount1, amount2] = defibox::get_withdraw_out( lp_tokens );
// amount1 => "4583.1234 EOS"
// amount2 => "1803.353300 BOX"
```