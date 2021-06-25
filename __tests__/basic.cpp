#include <eosio/eosio.hpp>

#include <sx.defibox/defibox.hpp>

using namespace eosio;

class [[eosio::contract]] basic : public contract {

public:
    using contract::contract;

    [[eosio::action]]
    void getreserves( const uint64_t pair_id, const symbol sort )
    {
        const auto [ reserveIn, reserveOut] = defibox::get_reserves( pair_id, sort );
        print( reserveIn );
        print( reserveOut );
    }

    [[eosio::action]]
    void getfee() {
        const uint8_t fee = defibox::get_fee();
        print( fee );
    }

    [[eosio::action]]
    void getpairid(symbol_code symcode) {
        const uint64_t pair_id = defibox::get_pairid_from_lptoken( symcode );
        print( pair_id );
    }

    [[eosio::action]]
    void getlptoken(uint64_t pair_id) {
        const auto lptoken = defibox::get_lptoken_from_pairid( pair_id );
        print( lptoken );
    }
};