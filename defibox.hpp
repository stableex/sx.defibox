#pragma once

#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <sx.utils/utils.hpp>
#include <math.h>

namespace defibox {

    using eosio::asset;
    using eosio::symbol;
    using eosio::name;
    using eosio::singleton;
    using eosio::multi_index;
    using eosio::time_point_sec;
    using eosio::current_time_point;
    using eosio::extended_symbol;
    using eosio::extended_asset;

    // reference
    const name id = "defibox"_n;
    const name code = "swap.defi"_n;
    const name lp_code = "lptoken.defi"_n;
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
        defibox::pairs _pairs( code, code.value );
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

    static int64_t _eos_amount = 0;                //save EOS amount to avoid dealing with conversion to EOS on multipath swaps

    static asset get_rewards( const uint64_t pair_id, asset in, asset out )
    {
        asset rewards {0, symbol{"BOX",6}};
        if(in.symbol != symbol{"EOS",4}) std::swap(in, out);

        auto eos_amount = (in.symbol == symbol{"EOS",4}) ? in.amount : _eos_amount; //make sure we are not converting rewards
        if(eos_amount/10000 == 0) return rewards;      //multipath swap should start at EOS for positive rewards

        defibox::pools _pools( "mine2.defi"_n, "mine2.defi"_n.value );
        auto poolit = _pools.find( pair_id );
        if (poolit==_pools.end()) return rewards;

        if( eosio::current_time_point().sec_since_epoch() > poolit->end_time.sec_since_epoch()) return rewards;  //not issued anymore

        float newsecs = eosio::current_time_point().sec_since_epoch() - poolit->last_issue_time.sec_since_epoch();  //seconds since last update
        auto total = poolit->balance.amount + poolit->weight * 0.002 * 0.7 * newsecs * 1000000; //adjust vs last update time

        rewards.amount = total - total * pow(0.9999, eos_amount/10000);

        return rewards;
    }

    /**
     * ## STATIC `get_pairid_from_lptoken`
     *
     * Get pair id from supplied BOX*** lp symbol code
     *
     * ### params
     *
     * - `{symbol_code} symcode` - BOX*** symbol code
     *
     * ### returns
     *
     * - `{uint64_t}` - defibox pair id
     *
     * ### example
     *
     * ```c++
     * const symbol_code symcode = symbolcode{"BOXGL"};
     *
     * const auto pair_id = defibox::get_pairid_from_lptoken( symcode );
     * // pair_id => 194
     * ```
     */
    static uint64_t get_pairid_from_lptoken( eosio::symbol_code lp_symcode )
    {
        std::string str = lp_symcode.to_string();
        if(str.length() < 3) return 0;
        uint64_t res = 0;
        if(str[0]!='B' || str[1]!='O' || str[2]!='X') return 0;
        for(auto i = 3; i < str.length(); i++){
            res *= 26;
            res += str[i] - 'A' + 1;
        }
        return res;
    }

    /**
     * ## STATIC `get_lptoken_from_pairid`
     *
     * Get LP token based on Defibox pair id
     *
     * ### params
     *
     * - `{uint64_t} pair_id` - Defibox pair id
     *
     * ### returns
     *
     * - `{extended_symbol}` - defibox lp token
     *
     * ### example
     *
     * ```c++
     * const uint64_t pair_id = 194;
     *
     * const auto ext_sym = defibox::get_lptoken_from_pairid( pair_id );
     * // ext_sym => "BOXGL,0"
     * ```
     */
    static extended_symbol get_lptoken_from_pairid( uint64_t pair_id )
    {
        if(pair_id == 0) return {};
        std::string res;
        while(pair_id){
            res = (char)('A' + pair_id % 26 - 1) + res;
            pair_id /= 26;
        }
        return { symbol { eosio::symbol_code{ "BOX" + res }, 0 }, lp_code };
    }

    /**
     * ## STATIC `get_withdraw_out`
     *
     * Get reserve tokens amounts after liquidity withdraw
     *
     * ### params
     *
     * - `{extended_asset} lp_token` - LP tokens
     *
     * ### returns
     *
     * - `{pair<extended_asset, extended_asset>}` - pair of reserve assets to receive after withdaw
     *
     * ### example
     *
     * ```c++
     * const extended_asset lp_tokens = extended_asset{ 12345678, { {"BOXGL", 0}, "lptoken.defi"_n } };
     *
     * const auto [amount1, amount2] = defibox::get_withdraw_out( lp_tokens );
     * // amount1 => "4583.1234 EOS"
     * // amount2 => "1803.353300 BOX"
     * ```
     */
    static std::pair<eosio::extended_asset, eosio::extended_asset> get_withdraw_out( eosio::extended_asset lp_token )
    {
        const auto supply = sx::utils::get_supply(lp_token.get_extended_symbol());
        const auto pair_id = get_pairid_from_lptoken(lp_token.quantity.symbol.code());
        eosio::check( lp_token.contract == lp_code && pair_id && supply.is_valid(), "DefiboxLibrary: invalid LP token");

        defibox::pairs _pairs( code, code.value );
        const auto pool = _pairs.get( pair_id, "DefiboxLibrary: INVALID_PAIR_ID" );

        const auto share = static_cast<double>( lp_token.quantity.amount ) / supply.amount;
        const auto res0 = extended_asset{ static_cast<int64_t>( pool.reserve0.amount * share ), extended_symbol{ pool.token0.symbol, pool.token0.contract } };
        const auto res1 = extended_asset{ static_cast<int64_t>( pool.reserve1.amount * share ), extended_symbol{ pool.token1.symbol, pool.token1.contract } };

        return { res0, res1 };
    }

}