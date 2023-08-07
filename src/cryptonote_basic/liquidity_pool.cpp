// Copyright (c) 2021-2022, CUT coin
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

#include "liquidity_pool.h"

#include "common/uint128.hpp"

namespace cryptonote {

Amount get_lp_token_to_funds(const AmountRatio &ratio) noexcept {
  const num::u128_t a{ratio.d_amount1};
  const num::u128_t b{ratio.d_amount2};
  return sqrt(a * b).val[0];
}

Amount get_lp_token_increment(const Amount             &lp_token_amount,
                              const AmountRatio &old_ratio,
                              const AmountRatio &new_ratio)
{
  const num::u128_t a1{old_ratio.d_amount1};
  const num::u128_t b1{old_ratio.d_amount2};
  const num::u128_t a2{new_ratio.d_amount1};
  const num::u128_t b2{new_ratio.d_amount2};
  const num::u128_t lp{lp_token_amount};

  const num::u128_t r1 = sqrt(a1 * b1);
  const num::u128_t r2 = sqrt(a2 * b2);
  const num::u128_t r3 = r2 * lp / r1 - lp;
  if (r3.val[1] != 0) {
    throw std::runtime_error{"Liquidity pool ratio overflow"};
  }
  return r3.val[0];
}

Amount get_lp_token_decrement(const Amount &lp_token_amount,
                              const AmountRatio &old_ratio,
                              const AmountRatio &new_ratio)
{
  const num::u128_t a1{old_ratio.d_amount1};
  const num::u128_t b1{old_ratio.d_amount2};
  const num::u128_t a2{new_ratio.d_amount1};
  const num::u128_t b2{new_ratio.d_amount2};
  const num::u128_t lp{new_ratio.d_amount2};

  const num::u128_t r1 = (a2 + b2) * lp / (a1 + b1);
  if (r1.val[1] != 0) {
    throw std::runtime_error{"Liquidity pool ratio overflow"};
  }
  return r1.val[0];
}

AmountRatio get_funds_to_lp_token(const Amount             &old_lpt_amount,
                                  const Amount             &new_lpt_amount,
                                  const AmountRatio &ratio) noexcept {
  const num::u128_t lp1{old_lpt_amount};
  const num::u128_t lp2{new_lpt_amount};
  const num::u128_t r0_1{ratio.d_amount1};
  const num::u128_t r0_2{ratio.d_amount2};

  num::u128_t r1_1 = r0_1 * lp2 / lp1;
  num::u128_t r1_2 = r0_2 * lp2 / lp1;

  return {r1_1.val[0], r1_2.val[0]};
}

Amount token_amount_from_lp_pair(const AmountRatio &ratio, const Amount &amount)
{
  const num::u128_t a{ratio.d_amount1};
  const num::u128_t b{ratio.d_amount2};
  const num::u128_t d{amount};

  num::u128_t token_fraction;
  num::u128_t r;
  num::div128by128(d * a, b, token_fraction, r);

  // round token fraction
  if ((r << 1) >= b) {
    ++token_fraction;
  }

  if (token_fraction.val[1] != 0) {
    throw std::runtime_error{"Liquidity pool ratio overflow"};
  }

  return token_fraction.val[0];
}

Amount underlying_amount_from_lp_pair(const AmountRatio &ratio, const Amount &amount)
{
  const num::u128_t a{ratio.d_amount1};
  const num::u128_t b{ratio.d_amount2};
  const num::u128_t d{amount};

  const num::u128_t token_fraction = d * b / a;
  if (token_fraction.val[1] != 0) {
    throw std::runtime_error{"Liquidity pool ratio overflow"};
  }

  return token_fraction.val[0];
}

Amount derive_buy_amount_from_lp_pair(const AmountRatio &ratio,
                                      const Amount      &amount,
                                      const Amount      &pool_interest)
{
  const num::u128_t a1{ratio.d_amount1};
  const num::u128_t b1{ratio.d_amount2};
  const num::u128_t a{amount};
  const num::u128_t i{pool_interest};

  if (a1 == a) {
    throw std::runtime_error("Buying all liquidity is not possible");
  }

  num::u128_t token_fraction = a * b1 / (a1 - a);
  if (token_fraction.val[1] != 0) {
    throw std::runtime_error{"Liquidity pool ratio overflow"};
  }

  token_fraction = token_fraction + token_fraction * i / 1000;
  if (token_fraction.val[1] != 0) {
    throw std::runtime_error{"Liquidity pool interest overflow"};
  }

  return token_fraction.val[0];
}

Amount derive_sell_amount_from_lp_pair(const AmountRatio &ratio,
                                       const Amount      &amount,
                                       const Amount      &pool_interest)
{
  const num::u128_t a1{ratio.d_amount1};
  const num::u128_t b1{ratio.d_amount2};
  const num::u128_t a{amount};
  const num::u128_t i{pool_interest};


  num::u128_t token_fraction = a * b1 / (a1 + a);
  if (token_fraction.val[1] != 0) {
    throw std::runtime_error{"Liquidity pool ratio overflow"};
  }

  token_fraction = token_fraction - token_fraction * i / 1000;
  if (token_fraction.val[1] != 0) {
    throw std::runtime_error{"Liquidity pool interest overflow"};
  }

  return token_fraction.val[0];
}

Amount derive_inv_buy_amount_from_lp_pair(const AmountRatio &ratio,
                                          const Amount      &amount,
                                          const Amount      &pool_interest)
{
  const num::u128_t a1{ratio.d_amount1};
  const num::u128_t b1{ratio.d_amount2};
  const num::u128_t i{pool_interest};
  const num::u128_t b{amount};

  if (b1 - b1 * i / 1000 == b) {
    throw std::runtime_error("Selling all liquidity is not possible");
  }

  num::u128_t token_fraction = b * a1 / (b1 - b1 * i / 1000 - b);
  if (token_fraction.val[1] != 0) {
    throw std::runtime_error{"Liquidity pool ratio overflow"};
  }

  return token_fraction.val[0];
}

double price_impact(const AmountRatio& lp_ratio, const AmountRatio& op_ratio)
{
  double op_rate = static_cast<double>(op_ratio.d_amount1) / op_ratio.d_amount2;
  double lp_rate = static_cast<double>(lp_ratio.d_amount1) / lp_ratio.d_amount2;
  return (op_rate / lp_rate - 1) * 100;
}

double buy_price_impact(const AmountRatio &ratio, const Amount &amount, const Amount &pool_interest) noexcept
{
  Amount a = derive_buy_amount_from_lp_pair(ratio, amount, pool_interest);
  Amount t1 = ratio.d_amount1 + amount;
  Amount t2 = ratio.d_amount2 - a;
  double r1 = double(ratio.d_amount1) / ratio.d_amount2;
  double r2 = double(t1) / t2;
  return r2 / r1 - r1;
}

double sell_price_impact(const AmountRatio &ratio, const Amount &amount, const Amount &pool_interest) noexcept
{
  Amount a = derive_sell_amount_from_lp_pair(ratio, amount, pool_interest);
  Amount t1 = ratio.d_amount1 - amount;
  Amount t2 = ratio.d_amount2 + a;
  double r1 = double(ratio.d_amount1) / ratio.d_amount2;
  double r2 = double(t1) / t2;
  return r2 / r1 - r1;
}

bool check_initial_pool_liquidity(Amount amount1, Amount amount2) noexcept
{
  return amount1 > 0 && amount2 > 0;
}

}  // namespace cryptonote
