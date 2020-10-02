#include <eosio/eosio.hpp>

#include "../defibox.hpp"

using namespace eosio;

class [[eosio::contract]] basic : public contract {

public:
    using contract::contract;

    [[eosio::action]]
    void getreserves( const uint64_t pair_id, const symbol tokenA )
    {
        const auto [ reserveIn, reserveOut] = defibox::swap::getReserves( pair_id, tokenA );
        print( reserveIn );
        print( reserveOut );
    }

    [[eosio::action]]
    void getamountout( const asset amountIn, const asset reserveIn, const asset reserveOut, const asset expect ) {
        const asset amountOut = defibox::swap::getAmountOut( amountIn, reserveIn, reserveOut );
        print( amountOut );
        check( amountOut == expect, "getAmountOut does not match expect");
    }

    [[eosio::action]]
    void getamountin( const asset amountOut, const asset reserveIn, const asset reserveOut, const asset expect ) {
        const asset amountIn = defibox::swap::getAmountIn( amountOut, reserveIn, reserveOut );
        print( amountIn );
        check( amountIn == expect, "getAmountIn does not match expect");
    }

    [[eosio::action]]
    void quote( const asset amountA, const asset reserveA, const asset reserveB, const asset expect ) {
        const asset amountB = defibox::swap::quote( amountA, reserveA, reserveB );
        print( amountB );
        check( amountB == expect, "quote does not match expect");
    }

    [[eosio::action]]
    void getrate( const asset amountA, const asset reserveA, const asset reserveB, const asset expect ) {
        const asset amountB = defibox::swap::quote( amountA, reserveA, reserveB );
        print( amountB );
        check( amountB == expect, "quote does not match expect");
    }
};