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
#include "serialization/serialization.h"

# include <string>

namespace cryptonote {

struct AmountRatio {
  // Contain pair of values of 'Amount' type.

  Amount d_amount1;  // amount of the first token in a liquidity pool

  Amount d_amount2;  // amount of the second token (underlying) in a liquidity pool

  BEGIN_SERIALIZE()
    FIELD(d_amount1)
    FIELD(d_amount2)
  END_SERIALIZE()
};

struct LiquidityPool {
  // Represent summary liquidity pool characteristics.

  TokenId     d_lptoken;    // the id of LP token that is connected with this pool

  TokenId     d_token1;     // first token id ('token 1')

  TokenId     d_token2;     // second token id ('token 2')

  Amount      d_lp_amount;  // current amount of LP tokens in the pool

  AmountRatio d_ratio;      // current amount of the first and second tokens in the pool
};

Amount get_lp_token_to_funds(const AmountRatio &ratio) noexcept;
  // Return amount of LP tokens corresponding to the specified 'ratio' that contains amounts
  // of 'token 1' and 'token 2' in a liquidity pool.

Amount get_lp_token_increment(const Amount      &lp_token_amount,
                              const AmountRatio &old_ratio,
                              const AmountRatio &new_ratio);

Amount get_lp_token_decrement(const Amount      &lp_token_amount,
                              const AmountRatio &old_ratio,
                              const AmountRatio &new_ratio);

AmountRatio get_funds_to_lp_token(const Amount      &old_lpt_amount,
                                  const Amount      &new_lpt_amount,
                                  const AmountRatio &ratio) noexcept;
  // Return the changed ratio between amounts of 'token 1' and 'token 2' corresponding to the
  // change of LP tokens in the pool. New ratio is derived using the specified
  // 'old_lpt_amount', 'new_lpt_amount' and the 'ratio' that the tokens pair had before the change.

Amount token_amount_from_lp_pair(const AmountRatio &ratio, const Amount &amount);
  // Return the amount of 'token 1' in a liquidity pool with the specified 'ratio'
  // that corresponds to the specified 'amount' of the underlying token ('token 2').

Amount underlying_amount_from_lp_pair(const AmountRatio &ratio, const Amount &amount);
  // Return the amount of 'token 2' in a liquidity pool with the specified 'ratio'
  // that corresponds to the specified 'amount' of the underlying token ('token 1').

Amount derive_buy_amount_from_lp_pair(const AmountRatio &ratio,
                                      const Amount      &amount,
                                      const Amount      &pool_interest);
  // Return the required amount of underlying token ('token 2') corresponding to the specified 'amount'
  // and the 'pool_interest' in the 'buy' operation.

Amount derive_sell_amount_from_lp_pair(const AmountRatio &ratio,
                                       const Amount             &amount,
                                       const Amount             &pool_interest);
  // Return the required amount of underlying token ('token 2') corresponding to the specified 'amount'
  // and the 'pool_interest' in the 'sell' operation.

double buy_price_impact(const AmountRatio &ratio, const Amount &amount, const Amount &pool_interest) noexcept;
  // Return the 'buy' operation price impact for a liquidity pool with the specified 'ration'
  // and for the specified 'amount'.

double sell_price_impact(const AmountRatio &ratio, const Amount &amount, const Amount &pool_interest) noexcept;
  // Return the 'sell' operation price impact for a liquidity pool with the specified 'ration'
  // and for the specified 'amount'.

bool check_initial_pool_liquidity(Amount amount1, Amount amount2) noexcept;
  // Validate the initial pool liquidity.

constexpr bool operator ==(const AmountRatio &a, const AmountRatio &b) {
  return a.d_amount1 == b.d_amount1 && a.d_amount2 == b.d_amount2;
}

constexpr bool operator !=(const AmountRatio &a, const AmountRatio &b) {
  return !(a == b);
}

constexpr bool operator ==(const LiquidityPool &a, const LiquidityPool &b) {
  return a.d_token1 == b.d_token1 && a.d_token2 == b.d_token2 && a.d_lptoken == b.d_lptoken
  && a.d_lp_amount == b.d_lp_amount && a.d_ratio == b.d_ratio;
}

}  // namespace cryptonote

#endif //CUTCOIN_LIQUIDITY_POOL_H
