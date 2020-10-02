#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>

#include <math.h>
#include <string>

using namespace eosio;
using namespace std;

namespace defibox {

    struct token {
        name contract;
        symbol symbol;

        string to_string() const {
            return contract.to_string() + "-" + symbol.code().to_string();
        };
    };

    class [[eosio::contract("stake.defi")]] stake : public contract {
    public:
        using contract::contract;

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
    };

    class [[eosio::contract("swap.defi")]] swap : public contract {
    public:
        using contract::contract;

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
            uint64_t            pair_id = 663;
            uint8_t             trade_fee = 20;
            uint8_t             protocol_fee = 10;
            name                fee_account = "fees.defi"_n;
        };
        typedef eosio::singleton< "config"_n, config_row > config;

        /**
         * fetches and sorts the reserves for a pair
         *
         * @returns set<asset> (reserveA, reserveB)
         */
        static pair<asset, asset> getReserves( const uint64_t pair_id, const symbol_code tokenA, const symbol_code tokenB )
        {
            // table
            defibox::swap::pairs _pairs( "swap.defi"_n, "swap.defi"_n.value );
            auto itr = _pairs.find( pair_id );

            // checks
            check( tokenA != tokenB, "DefiboxLibrary: IDENTICAL_SYMBOL_CODES");
            check( itr != _pairs.end(), "DefiboxLibrary: INVALID_PAIR_ID");

            // order reserves based on tokenA & tokenB
            const asset reserveA = tokenA == itr->reserve0.symbol.code() ? itr->reserve0 : itr->reserve1;
            const asset reserveB = tokenA == itr->reserve0.symbol.code() ? itr->reserve1 : itr->reserve0;

            return pair<asset, asset>{ reserveA, reserveB };
        }

        /**
         * ## STATIC `getAmountOut`
         *
         * Given an input amount of an asset and pair reserves, returns the maximum output amount of the other asset
         *
         * ### params
         *
         * - `{asset} amountIn` - amount input
         * - `{asset} reserveIn` - reserve input
         * - `{asset} reserveOut` - reserve output
         *
         * ### example
         *
         * ```c++
         * // Inputs
         * const asset amountIn = asset{10000, symbol{"EOS", 4}};
         * const asset reserveIn = asset{45851931234, symbol{"EOS", 4}};
         * const asset reserveOut = asset{125682033533, symbol{"USDT", 4}};
         *
         * // Calculation
         * const asset amountOut = defibox::swap::getAmountOut( amountIn, reserveIn, reserveOut );
         * // => "2.7328 USDT"
         * ```
         */
        static asset getAmountOut( const asset amountIn, const asset reserveIn, const asset reserveOut )
        {
            // checks
            check(amountIn.amount > 0, "DefiboxLibrary: INSUFFICIENT_INPUT_AMOUNT");
            check(reserveIn.amount > 0 && reserveOut.amount > 0, "DefiboxLibrary: INSUFFICIENT_LIQUIDITY");

            // calculations
            const double amountInWithFee = asset_to_double(amountIn) * (10000 - get_fee() );
            const double numerator = amountInWithFee * asset_to_double(reserveOut);
            const double denominator = asset_to_double(reserveIn) * 10000 + amountInWithFee;
            const double amountOut = numerator / denominator;

            return double_to_asset( amountOut, reserveOut.symbol );
        }

        /**
         * ## STATIC `getAmountIn`
         *
         * Given an output amount of an asset and pair reserves, returns a required input amount of the other asset.
         *
         * ### params
         *
         * - `{asset} amountIn` - amount input
         * - `{asset} reserveIn` - reserve input
         * - `{asset} reserveOut` - reserve output
         *
         * ### example
         *
         * ```c++
         * // Inputs
         * const asset amountOut = asset{27328, symbol{"USDT", 4}};
         * const asset reserveIn = asset{45851931234, symbol{"EOS", 4}};
         * const asset reserveOut = asset{125682033533, symbol{"USDT", 4}};
         *
         * // Calculation
         * const asset amountIn = defibox::swap::getAmountIn( amountOut, reserveIn, reserveOut );
         * // => "1.0000 EOS"
         * ```
         */
        static asset getAmountIn( const asset amountOut, const asset reserveIn, const asset reserveOut )
        {
            // checks
            eosio::check(amountOut.amount > 0, "DefiboxLibrary: INSUFFICIENT_OUTPUT_AMOUNT");
            eosio::check(reserveIn.amount > 0 && reserveOut.amount > 0, "DefiboxLibrary: INSUFFICIENT_LIQUIDITY");

            const double numerator = asset_to_double(reserveIn) * asset_to_double(amountOut) * 10000;
            const double denominator = (asset_to_double(reserveOut) - asset_to_double(amountOut)) * (10000 - get_fee());
            const double amountIn = (numerator / denominator);

            return double_to_asset( amountIn, reserveIn.symbol );
        }

        /**
         * ## STATIC `quote`
         *
         * Given some amount of an asset and pair reserves, returns an equivalent amount of the other asset
         *
         * ### params
         *
         * - `{asset} amountA` - amount A
         * - `{asset} reserveA` - reserve A
         * - `{asset} reserveB` - reserve B
         *
         * ### example
         *
         * ```c++
         * // Inputs
         * const asset amountA = asset{10000, symbol{"EOS", 4}};
         * const asset reserveA = asset{45851931234, symbol{"EOS", 4}};
         * const asset reserveB = asset{125682033533, symbol{"USDT", 4}};
         *
         * // Calculation
         * const asset amountB = defibox::swap::quote( amountA, reserveA, reserveB );
         * // => "2.7410 USDT"
         * ```
         */
        static asset quote( const asset amountA, const asset reserveA, const asset reserveB )
        {
            check(amountA.amount > 0, "DefiboxV2Library: INSUFFICIENT_AMOUNT");
            check(reserveA.amount > 0 && reserveB.amount > 0, "DefiboxV2Library: INSUFFICIENT_LIQUIDITY");
            const double amountB = asset_to_double( amountA ) * asset_to_double( reserveB ) / asset_to_double( reserveA );
            return double_to_asset( amountB, reserveB.symbol );
        }

        /**
         * Get Defibox fee
         */
        static uint8_t get_fee()
        {
            defibox::swap::config _config( "swap.defi"_n, "swap.defi"_n.value );
            defibox::swap::config_row config = _config.get_or_default();
            return config.trade_fee + config.protocol_fee;
        }

        static double asset_to_double( const asset quantity )
        {
            if ( quantity.amount == 0 ) return 0.0;
            return quantity.amount / pow(10, quantity.symbol.precision());
        }

        static asset double_to_asset( const double amount, const symbol sym )
        {
            return asset{ static_cast<int64_t>( round(amount * pow(10, sym.precision()))), sym };
        }
    };
}