#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create accounts
cleos create account eosio basic EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio myaccount EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

# build
eosio-cpp __tests__/basic.cpp

# deploy
cleos set contract basic . basic.wasm basic.abi

# getReserve
cleos -v push action basic getreserves '[12, "4,EOS"]' -p basic
# //=>

# getAmountOut
cleos -v push action basic getamountout '["1.0000 EOS", "4585193.1234 EOS", "12568203.3533 USDT", "2.7328 USDT"]' -p basic
# //=> "2.7328 USDT"

# getAmountIn
cleos -v push action basic getamountin '["2.7328 USDT", "4585193.1234 EOS", "12568203.3533 USDT", "1.0000 EOS"]' -p basic
# //=> "1.0000 EOS"

# quote
cleos -v push action basic quote '["1.0000 EOS", "4585193.1234 EOS", "12568203.3533 USDT", "2.7410 USDT"]' -p basic
# //=> "2.7410 USDT"
