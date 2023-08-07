// Copyright (c) 2022, CUT coin
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

#include "token_balance.h"

#include "token_info.h"
#include "wallet.h"
#include "crypto/hash.h"
#include "wallet/wallet2.h"
#include "common_defines.h"

#include <vector>

namespace Monero {

TokenBalanceImpl::TokenBalanceImpl(WalletImpl *wallet)
  : m_wallet(wallet) {}

void TokenBalanceImpl::refresh(uint32_t accountIndex)
{
  LOG_PRINT_L2("Refreshing current wallet tokens");

  tools::AccountView account = m_wallet->m_wallet->get_account_view(accountIndex);
  std::set<cryptonote::TokenId> tokens = account.get_wallet_tokens();
  cryptonote::TokenAmounts locked_per_token;
  cryptonote::TokenAmounts unlocked_per_token;
  std::vector<cryptonote::TokenId> token_ids;
  for (const auto& token: tokens) {
    token_ids.push_back(token);
    account.amount(locked_per_token[token], token, {});
    account.unlocked_amount(unlocked_per_token[token], token, {});
  }

  clearRows();
  for (uint32_t i = 0; i < token_ids.size(); ++i)
  {
    m_rows.push_back(new TokenBalanceRow(
      i,
      std::to_string(token_ids[i]),
      cryptonote::token_id_to_name(token_ids[i]),
      cryptonote::print_money(locked_per_token[token_ids[i]]),
      cryptonote::print_money(unlocked_per_token[token_ids[i]])
    ));
  }
}

void TokenBalanceImpl::clearRows() {
  for (auto r : m_rows) {
    delete r;
  }
  m_rows.clear();
}

std::vector<TokenBalanceRow*> TokenBalanceImpl::getAll() const
{
  return m_rows;
}

TokenBalanceImpl::~TokenBalanceImpl()
{
  clearRows();
}

}  // namespace Monero
