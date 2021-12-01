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

#ifndef CUTCOIN_LIQUIDITY_POOL_H
#define CUTCOIN_LIQUIDITY_POOL_H

#include <cryptonote_basic/token.h>

# include <string>

namespace cryptonote {

struct LiquidityPoolRatio{
  Amount d_amount1;

  Amount d_amount2;
};

struct LiquidityPoolSummary {
  // Represent summary liquidity pool characteristics.

  TokenId            d_lptoken;

  TokenId            d_token1;

  TokenId            d_token2;

  Amount             d_lp_amount;

  LiquidityPoolRatio d_rate;
};

Amount get_lp_token_to_funds(const LiquidityPoolRatio &ratio);

Amount get_lp_token_increment(const Amount             &lp_token_amount,
                              const LiquidityPoolRatio &old_ratio,
                              const LiquidityPoolRatio &new_ratio);

Amount get_lp_token_decrement(const Amount             &lp_token_amount,
                              const LiquidityPoolRatio &old_ratio,
                              const LiquidityPoolRatio &new_ratio);

LiquidityPoolRatio get_funds_to_lp_token(const Amount             &old_lpt_amount,
                                         const Amount             &new_lpt_amount,
                                         const LiquidityPoolRatio &ratio);
  // Return amounts for lp pair corresponding to the specified 'lp_token_amount' and the pair 'ratio'.

Amount token_amount_from_lp_pair(const LiquidityPoolRatio &ratio, const Amount &amount);

Amount underlying_amount_from_lp_pair(const LiquidityPoolRatio &ratio, const Amount &amount);

Amount derive_buy_amount_from_lp_pair(const LiquidityPoolRatio &ratio,
                                      const Amount             &amount,
                                      const Amount             &pool_interest);
  // Return the required amount of underlying tokens corresponding to the specified 'amount' in the
  // exchange 'buy' operation.

Amount derive_sell_amount_from_lp_pair(const LiquidityPoolRatio &ratio,
                                       const Amount             &amount,
                                       const Amount             &pool_interest);
  // Return the required amount of underlying tokens corresponding to the specified 'amount' in the
  // exchange 'sell' operation.

Amount derive_buy_amount_wo_interest_from_lp_pair(const LiquidityPoolRatio &ratio,
                                                  const Amount             &amount);

Amount derive_sell_amount_wo_interest_from_lp_pair(const LiquidityPoolRatio &ratio,
                                                   const Amount             &amount);

double buy_price_impact(const LiquidityPoolRatio &ratio, const Amount &amount, const Amount &pool_interest);
  // Return the 'buy' operation price impact for a liquidity pool with the specified 'ration'
  // and for the specified 'amount'.

double sell_price_impact(const LiquidityPoolRatio &ratio, const Amount &amount, const Amount &pool_interest);
  // Return the 'sell' operation price impact for a liquidity pool with the specified 'ration'
  // and for the specified 'amount'.

bool check_initial_pool_liquidity(Amount amount1, Amount amount2);
  // Validate the initial pool liquidity.

}  // namespace cryptonote

#endif //CUTCOIN_LIQUIDITY_POOL_H
