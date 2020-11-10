#pragma once

#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <math.h>

namespace defibox {

    using eosio::asset;
    using eosio::symbol;
    using eosio::name;
    using eosio::singleton;
    using eosio::multi_index;
    using eosio::time_point_sec;
    using eosio::current_time_point;

    // reference
    const name id = "defibox"_n;
    const name code = "swap.defi"_n;
    const std::string description = "Defibox Converter";

    /**
     * Custom Token struct
     */
    struct token {
        name contract;
        symbol symbol;

        std::string to_string() const {
            return contract.to_string() + "-" + symbol.code().to_string();
        };
    };

    /**
     * Defibox pairs
     */
    struct [[eosio::table]] pairs_row {
        uint64_t            id;
        token               token0;
        token               token1;
        asset               reserve0;
        asset               reserve1;
        uint64_t            liquidity_token;
        double              price0_last;
        double              price1_last;
        double              price0_cumulative_last;
        double              price1_cumulative_last;
        time_point_sec      block_time_last;

        uint64_t primary_key() const { return id; }
    };
    typedef eosio::multi_index< "pairs"_n, pairs_row > pairs;

    /**
     * Defibox config
     */
    struct [[eosio::table]] config_row {
        uint8_t             status = 0;
        uint64_t            pair_id = 663;      //last added pair_id
        uint8_t             trade_fee = 20;
        uint8_t             protocol_fee = 10;
        name                fee_account = "fees.defi"_n;
    };
    typedef eosio::singleton< "config"_n, config_row > config;

    /**
     * Defibox stakes
     */
    struct [[eosio::table("stakes")]] stakes_row {
        name            owner;
        uint64_t        staked;
        uint64_t        refunding;
        time_point_sec  release_time;

        uint64_t primary_key() const { return owner.value; }
    };
    typedef eosio::multi_index< "stakes"_n, stakes_row > stakes;

    /**
     * Defibox stat
     */
    struct [[eosio::table("stat")]] stat_row {
        uint64_t            locked;
        uint64_t            staked;
        uint64_t            refunding;
    };
    typedef eosio::singleton< "stat"_n, stat_row > stat;

    /**
     * Defibox mine pools
     */
    struct [[eosio::table("pools")]] pools_row {
        uint64_t        pair_id;
        double_t        weight;
        asset           balance;
        asset           issued;
        time_point_sec  last_issue_time;
        time_point_sec  start_time;
        time_point_sec  end_time;

        uint64_t primary_key() const { return pair_id; }
    };
    typedef eosio::multi_index< "pools"_n, pools_row > pools;

    /**
     * ## STATIC `get_fee`
     *
     * Get Defibox total fee
     *
     * ### returns
     *
     * - `{uint8_t}` - total fee (trade + protocol)
     *
     * ### example
     *
     * ```c++
     * const uint8_t fee = defibox::get_fee();
     * // => 30
     * ```
     */
    static uint8_t get_fee()
    {
        //return 30;
        defibox::config _config( "swap.defi"_n, "swap.defi"_n.value );
        defibox::config_row config = _config.get_or_default();
        return config.trade_fee + config.protocol_fee;
    }

    /**
     * ## STATIC `get_reserves`
     *
     * Get reserves for a pair
     *
     * ### params
     *
     * - `{uint64_t} pair_id` - pair id
     * - `{symbol} sort` - sort by symbol (reserve0 will be first item in pair)
     *
     * ### returns
     *
     * - `{pair<asset, asset>}` - pair of reserve assets
     *
     * ### example
     *
     * ```c++
     * const uint64_t pair_id = 12;
     * const symbol sort = symbol{"EOS", 4};
     *
     * const auto [reserve0, reserve1] = defibox::get_reserves( pair_id, sort );
     * // reserve0 => "4585193.1234 EOS"
     * // reserve1 => "12568203.3533 USDT"
     * ```
     */
    static std::pair<asset, asset> get_reserves( const uint64_t pair_id, const symbol sort )
    {
        // table
        defibox::pairs _pairs( "swap.defi"_n, "swap.defi"_n.value );
        auto pairs = _pairs.get( pair_id, "DefiboxLibrary: INVALID_PAIR_ID" );

        eosio::check( pairs.reserve0.symbol == sort || pairs.reserve1.symbol == sort, "DefiboxLibrary: sort symbol doesn't match");

        return sort == pairs.reserve0.symbol ?
            std::pair<asset, asset>{ pairs.reserve0, pairs.reserve1 } :
            std::pair<asset, asset>{ pairs.reserve1, pairs.reserve0 };
    }

    /**
     * ## STATIC `get_rewards`
     *
     * Get rewards for trading
     *
     * ### params
     *
     * - `{uint64_t} pair_id` - pair id
     * - `{asset} in` - input quantity
     * - `{asset} out` - output quantity
     *
     * ### returns
     *
     * - {asset} = rewards in BOX
     *
     * ### example
     *
     * ```c++
     * const uint64_t pair_id = 12;
     * const asset in = asset{10000, {"EOS", 4}};
     * const asset out = asset{12345, {"USDT", 4}};
     *
     * const auto rewards = defibox::get_rewards( pair_id, in, out );
     * // rewards => "0.123456 BOX"
     * ```
     */

    static asset get_rewards( const uint64_t pair_id, asset in, asset out )
    {
        asset rewards {0, symbol{"BOX",6}};
        if(in.symbol != symbol{"EOS",4}) std::swap(in, out);
        if(in.symbol != symbol{"EOS",4})
            return rewards;     //return 0 if non-EOS pair

        defibox::pools _pools( "mine2.defi"_n, "mine2.defi"_n.value );
        auto poolit = _pools.find( pair_id );
        if (poolit==_pools.end()) return rewards;

        if( eosio::current_time_point().sec_since_epoch() > poolit->end_time.sec_since_epoch()) return rewards;  //not issued anymore

        float newsecs = eosio::current_time_point().sec_since_epoch() - poolit->last_issue_time.sec_since_epoch();  //seconds since last update
        auto total = poolit->balance.amount + poolit->weight * 0.002 * 0.7 * newsecs * 1000000; //adjust vs last update time

        rewards.amount = total - total * pow(0.9999, in.amount / 10000);

        return rewards;
    }

}