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

#include "dex.h"

#include "token.h"

#include <string>
#include <vector>

namespace cryptonote {

const std::string delimiter{"/"};

std::string tokens_to_lpname(const TokenId &id1, const TokenId &id2)
{
  std::string s1 = token_id_to_name(id1);
  std::string s2 = token_id_to_name(id2);

  if (is_cutcoin(id1)) {
    return s2 + delimiter + s1;
  }

  if (is_cutcoin(id2)) {
    return s1 + delimiter + s2;
  }

  return s1 < s2 ? s1 + delimiter + s2: s2 + delimiter + s1;
}

std::string tokens_to_lpname(const std::string &token1, const std::string &token2)
{
  if (token1 == CUTCOIN_NAME) {
    return token2 + delimiter + token1;
  }

  if (token2 == CUTCOIN_NAME) {
    return token1 + delimiter + token2;
  }

  return token1 < token2 ? token1 + delimiter + token2: token2 + delimiter + token1;
}

bool lpname_to_tokens(const std::string &name, TokenId &token1, TokenId &token2)
{
  size_t pos = 0;
  std::string token, tmp_name{name};
  std::vector<std::string> tokens;
  while ((pos = tmp_name.find(delimiter)) != std::string::npos) {
    token = tmp_name.substr(0, pos);
    tokens.push_back(token);
    tmp_name.erase(0, pos + delimiter.length());
  }
  tokens.push_back(tmp_name);

  if (tokens.size() != 2) {
    return false;
  }

  if ((!is_cutcoin(tokens[0]) && !validate_token_name(tokens[0])) ||
    (!is_cutcoin(tokens[1]) && !validate_token_name(tokens[1]))) {
    return false;
  }

  token1 = token_name_to_id(tokens[0]);
  token2 = token_name_to_id(tokens[1]);
  return true;
}

bool validate_lpname(const std::string &pool_name)
{
  std::size_t delimiter_pos = pool_name.find(delimiter);
  if (delimiter_pos  == std::string::npos) {
    return false;
  }

  std::string t1 = pool_name.substr(0, delimiter_pos);
  std::string t2 = pool_name.substr(delimiter_pos + 1);

  return !(t1 == t2);
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

  TokenId t1 = token1;
  TokenId t2 = token2;

  for (size_t i = 0; i < pools.size(); ++i) {
    // use original pool to copy the data
    ExchangeTransfer et;
    et.d_token1    = pools[i].d_token1;
    et.d_token2    = pools[i].d_token2;
    et.d_old_ratio = pools[i].d_ratio;

    // use pool copy to invert if needed
    if (t1 == pools[i].d_token1) {
      const LiquidityPool &p = pools[i];
      if (side == ExchangeSide::buy) {
        amount2 = derive_buy_amount_from_lp_pair(p.d_ratio, amount1, pool_interest);
        et.d_new_ratio = {p.d_ratio.d_amount1 - amount1, p.d_ratio.d_amount1 + amount2};
      } else {
        amount2 = derive_sell_amount_from_lp_pair(p.d_ratio, amount1, pool_interest);
        et.d_new_ratio = {p.d_ratio.d_amount1 + amount1, p.d_ratio.d_amount1 - amount2};
      }
      t1      = p.d_token1;
      amount1 = amount2;
    }
    if (t1 == pools[i].d_token2) {
      const LiquidityPool p = inverse(pools[i]);
      if (side == ExchangeSide::buy) {
        amount2 = derive_buy_amount_from_lp_pair(p.d_ratio, amount1, pool_interest);
        et.d_new_ratio = {p.d_ratio.d_amount1 - amount2, p.d_ratio.d_amount1 + amount1};
      }
      else {
        amount2 = derive_buy_amount_from_lp_pair(p.d_ratio, amount1, pool_interest);
        et.d_new_ratio = {p.d_ratio.d_amount1 + amount2, p.d_ratio.d_amount1 - amount1};
      }
      t1      = p.d_token1;
      amount1 = amount2;
    }
    exchange_transfers.emplace_back(et);
  }

  Amount old_amount1, old_amount2;
  Amount new_amount1, new_amount2;

  if (token1 == exchange_transfers[0].d_token1) {
    old_amount1 = exchange_transfers[0].d_old_ratio.d_amount1;
    new_amount1 = exchange_transfers[0].d_new_ratio.d_amount1;
  }
  else {
    old_amount1 = exchange_transfers[0].d_old_ratio.d_amount2;
    new_amount1 = exchange_transfers[0].d_new_ratio.d_amount2;
  }

  if (token2 == exchange_transfers[exchange_transfers.size() - 1].d_token1) {
    old_amount2 = exchange_transfers[exchange_transfers.size() - 1].d_old_ratio.d_amount1;
    new_amount2 = exchange_transfers[exchange_transfers.size() - 1].d_new_ratio.d_amount1;
  }
  else {
    old_amount2 = exchange_transfers[exchange_transfers.size() - 1].d_old_ratio.d_amount2;
    new_amount2 = exchange_transfers[exchange_transfers.size() - 1].d_new_ratio.d_amount2;
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
