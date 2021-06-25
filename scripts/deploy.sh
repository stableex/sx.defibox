#!/bin/bash

eosio-cpp ./__tests__/basic.cpp -I ..

cleos set contract basic . basic.wasm basic.abi -p basic