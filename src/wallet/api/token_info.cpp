// Copyright (c) 2018-2022, CUT coin
// Copyright (c) 2017-2018, The Monero Project
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

#include "token_info.h"

#include "common_defines.h"
#include "rpc/core_rpc_server_commands_defs.h"
#include "crypto/hash.h"
#include "wallet.h"
#include "wallet/wallet2.h"

#include <vector>

namespace Monero {
  
TokenInfo::~TokenInfo() {}
  
TokenInfoImpl::TokenInfoImpl(WalletImpl *wallet)
    : m_wallet(wallet) {}

void TokenInfoImpl::refresh()
{
  using namespace cryptonote;

  LOG_PRINT_L2("Refreshing all tokens info");

  std::vector<cryptonote::TokenSummary> tokens;
  m_wallet->m_wallet->get_tokens(tokens, "", false);

  clearRows();
  for (uint32_t i = 0; i < tokens.size(); ++i) {
    std::string token_type;
    if (tokens[i].d_type.is_public()) {
      token_type = "public supply";
    }
    else if (tokens[i].d_type.is_mintable()) {
      token_type = "mintable";
    }
    else if (tokens[i].d_type.is_lptoken()) {
      token_type = "LP token";
    }
    else {
      token_type = "private supply";
    }

    m_rows.push_back(new TokenInfoRow(
      i,
      std::to_string(tokens[i].d_token_id),
      cryptonote::token_id_to_name(tokens[i].d_token_id),
      std::to_string(tokens[i].d_token_supply),
      token_type
    ));
  }
}

void TokenInfoImpl::clearRows() {
   for (auto r : m_rows) {
     delete r;
   }
   m_rows.clear();
}

std::vector<TokenInfoRow*> TokenInfoImpl::getAll() const
{
  return m_rows;
}

TokenInfoImpl::~TokenInfoImpl()
{
  clearRows();
}

} // namespace Monero
