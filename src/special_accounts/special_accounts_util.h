// Copyright (c) 2020-2021, CUT coin
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef CUTCOIN_SPECIAL_ACCOUNTS_UTIL_H
#define CUTCOIN_SPECIAL_ACCOUNTS_UTIL_H

#include "cryptonote_basic/amount.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_basic/token.h"

namespace cryptonote {

bool validate_amount_burnt(const transaction &tx, const TokenId &token_id, const Amount &burnt_amount);
  // Return 'true' if the specified transaction 'tx'
  // 1. has outputs with the specified 'token_id'
  // 2. their destination is coin burn address
  // 3. Their summary amount is greater or equal to the specified 'amount'.

bool check_transfer_to_liquidity_pool(const transaction &tx, const TokenId &token_id, const Amount &amount);
  // Return 'true' if the specified transaction 'tx'
  // 1. has outputs with the specified 'token_id'
  // 2. their destination is liquidity pool address
  // 3. Their summary amount is equal to the specified 'amount'.

Amount get_transfer_to_liquidity_pool(const transaction &tx, const TokenId &token_id);
  // Return sum amount of all tokens that
  // 1. have outputs with the specified 'token_id'
  // 2. their destination is liquidity pool address

}  // namespace cryptonote

#endif //CUTCOIN_SPECIAL_ACCOUNTS_UTIL_H
