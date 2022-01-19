// Copyright (c) 2021, CUT coin
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

#ifndef CUTCOIN_DEX_H
#define CUTCOIN_DEX_H

#include "cryptonote_basic/token.h"
#include "liquidity_pool.h"
#include "serialization/serialization.h"

namespace cryptonote {

enum OutputOriginType {
  Wallet = 0,
  LpAccount = 1
};

enum ExchangeSide {
  // Exchange type.

  sell = 0,

  buy  = 1
};

struct ExchangeTransfer {
  // Represent exchange transfer.

  TokenId     d_token1;

  TokenId     d_token2;

  AmountRatio d_old_ratio;  // amounts rate at the pool before the transfer has happened

  AmountRatio d_new_ratio;  // amounts rate at the pool after the transfer has happened

  BEGIN_SERIALIZE()
    FIELD(d_token1)
    FIELD(d_token2)
    FIELD(d_old_ratio)
    FIELD(d_new_ratio)
  END_SERIALIZE()
};

struct CompositeTransfer {
  // Represent composite exchange transfer that may consist of multiple exchanges
  // at different liquidity pools.

  TokenId                       d_token1;

  TokenId                       d_token2;

  Amount                        d_amount;

  Amount                        d_pool_interest;

  ExchangeSide                  d_side;

  std::vector<ExchangeTransfer> d_transfers;
};

std::string tokens_to_lpname(const TokenId &id1, const TokenId &id2);
  // Generate human-readable liquidity pool name using the specified token ids 'id1' and 'id2'.

std::string tokens_to_lpname(const std::string &token1, const std::string &token2);
  // Generate human-readable liquidity pool name using the specified token names 'token1' and 'token2'.

bool lpname_to_tokens(const std::string &name, TokenId &token1, TokenId &token2);
  // Get token ids from the specified liquidity pool 'name' and put them into 'token1' and 'token2'.

bool validate_lpname(const std::string &pool_name);
  // Validate the specified 'pool_name'.

bool pools_to_composite_exchange_transfer(std::vector<ExchangeTransfer>    &exchange_transfers,
                                          ExchangeTransfer                 &summary,
                                          const TokenId                    &token1,
                                          const TokenId                    &token2,
                                          const Amount                     &amount,
                                          const Amount                     &pool_interest,
                                          const std::vector<LiquidityPool> &pools,
                                          const ExchangeSide               &side);

constexpr
LiquidityPool inverse(const LiquidityPool &p) {
  return LiquidityPool{p.d_lptoken, p.d_token2, p.d_token1, p.d_lp_amount, {p.d_ratio.d_amount1, p.d_ratio.d_amount2}};
}

//constexpr
//ExchangeSide exchange_side(const ExchangeTransfer &et) {
//  return et.d_old_ratio.d_amount1 < et.d_new_ratio.d_amount1 ? ExchangeSide::buy : ExchangeSide::sell;
//}

}  // namespace cryptonote

#endif //CUTCOIN_DEX_H
