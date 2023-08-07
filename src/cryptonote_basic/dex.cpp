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

#include "dex.h"

#include "token.h"

#include <string>
#include <vector>

namespace cryptonote {

const std::string lp_delimiter{"/"};

std::string tokens_to_lpname(const TokenId &id1, const TokenId &id2)
{
  std::string s1 = token_id_to_name(id1);
  std::string s2 = token_id_to_name(id2);

  if (is_cutcoin(id1)) {
    return s2 + lp_delimiter + s1;
  }

  if (is_cutcoin(id2)) {
    return s1 + lp_delimiter + s2;
  }

  return s1 < s2 ? s1 + lp_delimiter + s2 : s2 + lp_delimiter + s1;
}

std::string tokens_to_lpname(const std::string &token1, const std::string &token2)
{
  if (token1 == CUTCOIN_NAME) {
    return token2 + lp_delimiter + token1;
  }

  if (token2 == CUTCOIN_NAME) {
    return token1 + lp_delimiter + token2;
  }

  return token1 < token2 ? token1 + lp_delimiter + token2 : token2 + lp_delimiter + token1;
}

bool lpname_to_tokens(const std::string &name, TokenId &token1, TokenId &token2)
{
  if (!validate_lpname(name)) {
    return false;
  }

  std::size_t delimiter_pos = name.find(lp_delimiter);
  token1 = token_name_to_id(name.substr(0, delimiter_pos));
  token2 = token_name_to_id(name.substr(delimiter_pos + 1));

  return true;
}

bool tokens_direct_order(const std::string &token1, const std::string &token2)
{
  if (token1 == CUTCOIN_NAME) {
    return false;
  }
  if (token2 == CUTCOIN_NAME) {
    return true;
  }
  return token1 < token2;
}

bool validate_lpname(const std::string &pool_name)
{
  std::size_t delimiter_pos = pool_name.find(lp_delimiter);
  if (delimiter_pos  == std::string::npos) {
    return false;
  }

  std::string t1 = pool_name.substr(0, delimiter_pos);
  std::string t2 = pool_name.substr(delimiter_pos + 1);

  if (!tokens_direct_order(t1, t2)) {
    return false;
  }

  return !(t1 == t2);
}

bool validate_slippage(double slippage)
{
    return slippage >= MIN_SLIPPAGE && slippage <= MAX_SLIPPAGE;
}

bool pools_to_composite_exchange_transfer(std::vector<ExchangeTransfer>    &exchange_transfers,
                                          ExchangeTransfer                 &summary,
                                          const TokenId                    &token1,
                                          const TokenId                    &token2,
                                          const Amount                     &amount,
                                          const Amount                     &pool_interest,
                                          const std::vector<LiquidityPool> &pools,
                                          const ExchangeSide               &side)
{
  if (pools.empty()) {
    return false;
  }

  if (token1 == token2) {
    return false;
  }

  if (pools.size() != 1) {
    return false;
  }

  if (pools[0].d_token1 != token1 && pools[0].d_token2 != token1) {
    return false;
  }

  if (pools[0].d_token1 != token2 && pools[0].d_token2 != token2) {
    return false;
  }

  Amount amount1 = amount;
  Amount amount2 = 0;

  const LiquidityPool &p = pools[0];
  if (side == ExchangeSide::buy) {
    amount2 = derive_buy_amount_from_lp_pair(p.d_ratio, amount1, pool_interest);
    summary = {
      p.d_token1,
      p.d_token2,
      p.d_ratio,
      {p.d_ratio.d_amount1 - amount1, p.d_ratio.d_amount2 + amount2}
    };
  } else {
    amount2 = derive_sell_amount_from_lp_pair(p.d_ratio, amount1, pool_interest);
    summary = {
      p.d_token1,
      p.d_token2,
      p.d_ratio,
      {p.d_ratio.d_amount1 + amount1, p.d_ratio.d_amount2 - amount2}
    };
  }
  exchange_transfers.push_back(summary);
  return true;
}

bool cross_pools_to_composite_exchange_transfer(std::vector<ExchangeTransfer>    &exchange_transfers,
                                                ExchangeTransfer                 &summary,
                                                const TokenId                    &token1,
                                                const TokenId                    &token2,
                                                const Amount                     &amount,
                                                const Amount                     &pool_interest,
                                                const std::vector<LiquidityPool> &pools)
{
  if (pools.empty()) {
    return false;
  }

  if (token1 == token2) {
    return false;
  }

  if (pools[0].d_token1 != token1 && pools[0].d_token2 != token1) {
    return false;
  }

  if (pools[pools.size() - 1].d_token1 != token2 && pools[pools.size() - 1].d_token2 != token2) {
    return false;
  }

  Amount amount1 = amount;
  Amount amount2 = 0;

  if (pools.size() == 1) {
    const LiquidityPool &p = pools[0];

    ExchangeSide side = token1 == p.d_token1 ? ExchangeSide::buy: ExchangeSide::sell;

    ExchangeTransfer et_info{};
    if (side == ExchangeSide::buy) {
      amount2 = derive_buy_amount_from_lp_pair(p.d_ratio, amount1, pool_interest);
      et_info = {
        p.d_token1,
        p.d_token2,
        p.d_ratio,
        {p.d_ratio.d_amount1 - amount1, p.d_ratio.d_amount2 + amount2}
      };

      summary = {
        token1,
        token2,
        et_info.d_old_ratio,
        et_info.d_new_ratio
      };
    }
    else {
      amount2 = derive_inv_buy_amount_from_lp_pair(p.d_ratio, amount1, pool_interest);
      et_info = {
        p.d_token1,
        p.d_token2,
        p.d_ratio,
        {p.d_ratio.d_amount1 + amount2, p.d_ratio.d_amount2 - amount1}
      };

      summary = {
        token1,
        token2,
        {et_info.d_old_ratio.d_amount2, et_info.d_old_ratio.d_amount1},
        {et_info.d_new_ratio.d_amount2, et_info.d_new_ratio.d_amount1}
      };
    }

    exchange_transfers.push_back(et_info);

    return true;
  }

  std::vector<LiquidityPool> pools_copy;
  TokenId leading_token = token1;
  for (const auto p: pools) {
      if (leading_token == p.d_token1) {
        pools_copy.push_back(p);
        leading_token = p.d_token2;
      }
      else if (leading_token == p.d_token2) {
        pools_copy.emplace_back(inverse(p));
        leading_token = p.d_token1;
      }
      else {
        return false;
      }
  }

  for (size_t i = 0; i < pools_copy.size(); ++i) {
    const LiquidityPool &p = pools[i];
    const LiquidityPool &pc = pools_copy[i];
    ExchangeTransfer et{};

    et.d_token1 = p.d_token1;
    et.d_token2 = p.d_token2;
    et.d_old_ratio = p.d_ratio;

    amount2 = derive_buy_amount_from_lp_pair(pc.d_ratio, amount1, pool_interest);
    if (pc.d_ratio.d_amount1 < amount1 || pc.d_ratio.d_amount2 + amount2 < pc.d_ratio.d_amount2) return false;
    if (pc == pools[i]) {
      et.d_new_ratio = {pc.d_ratio.d_amount1 - amount1, pc.d_ratio.d_amount2 + amount2};
    } else {
      et.d_new_ratio = {pc.d_ratio.d_amount2 + amount2, pc.d_ratio.d_amount1 - amount1};
    }

    amount1 = amount2;

    exchange_transfers.emplace_back(et);
  }

  Amount old_amount1, old_amount2;
  Amount new_amount1, new_amount2;

  const ExchangeTransfer &etf = exchange_transfers[0];
  const ExchangeTransfer &etl = exchange_transfers[exchange_transfers.size() - 1];

  if (token1 == etf.d_token1) {
    old_amount1 = etf.d_old_ratio.d_amount1;
    new_amount1 = etf.d_new_ratio.d_amount1;
  }
  else {
    old_amount1 = etf.d_old_ratio.d_amount2;
    new_amount1 = etf.d_new_ratio.d_amount2;
  }

  if (token2 == etl.d_token1) {
    old_amount2 = etl.d_old_ratio.d_amount1;
    new_amount2 = etl.d_new_ratio.d_amount1;
  }
  else {
    old_amount2 = etl.d_old_ratio.d_amount2;
    new_amount2 = etl.d_new_ratio.d_amount2;
  }

  summary = {
    token1,
    token2,
    {old_amount1, old_amount2},
    {new_amount1, new_amount2}
  };

  return true;
}

}  // namespace cryptonote
