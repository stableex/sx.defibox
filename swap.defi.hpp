#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>

#include <string>

using namespace eosio;
using namespace std;

class [[eosio::contract("swap.defi")]] swapDefi : public contract {
public:
    using contract::contract;

    struct [[eosio::table("liquidity")]] liquidity_row {
        name owner;
        uint64_t token;

        uint64_t primary_key() const { return owner.value; }
    };
    typedef eosio::multi_index< "liquidity"_n, liquidity_row > liquidity;

    struct [[eosio::table("pairs")]] pairs_row {
        uint64_t            id;
        extended_symbol     token0;
        extended_symbol     token1;
        asset               reserve0;
        asset               reserve1;
        uint64_t            liquidity_token;

        uint64_t primary_key() const { return id; }
    };
    typedef eosio::multi_index< "pairs"_n, pairs_row > pairs;
};
