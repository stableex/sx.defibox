#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create accounts
cleos create account eosio basic EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio myaccount EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

# build
eosio-cpp __tests__/basic.cpp -I ..

# deploy
cleos set contract basic . basic.wasm basic.abi

# getReserve
cleos -v push action basic getreserves '[12, "4,EOS"]' -p basic
# //=>

# getfee
cleos -v push action basic getfee '[]' -p basic
# //=> "30"

# getReserve
cleos -v push action basic getpairid '["BOXBGQ"]' -p basic
# //=>