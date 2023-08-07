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

#include "liquidity_pool_info.h"
#include "wallet.h"
#include "cryptonote_config.h"
#include "common/i18n.h"

using namespace cryptonote;

namespace Monero {

LiquidityPoolInfoImpl::LiquidityPoolInfoImpl(WalletImpl *wallet)
  : m_wallet(wallet)
{
}

LiquidityPoolInfoImpl::~LiquidityPoolInfoImpl()
{
}

void LiquidityPoolInfoImpl::refresh()
{
  LOG_PRINT_L2("Refreshing liquidity pools");

  m_rows.clear();
  m_wallet->clearStatus();

  auto wallet = m_wallet->get_wallet();
  COMMAND_RPC_GET_LIQUIDITY_POOLS::request req{};
  COMMAND_RPC_GET_LIQUIDITY_POOLS::response res{};
  bool r = wallet->invoke_http_bin("/get_liquidity_pools.bin", req, res);
  if (!r || res.status != CORE_RPC_STATUS_OK) {
    m_wallet->setStatusError(tr("Failed to get liquidity pools"));
    return;
  }

  std::map<std::string, const pool_summary*> sorted_pools;
  for (const auto& lp_summary: res.liquidity_pools) {
    sorted_pools[tokens_to_lpname(lp_summary.token1, lp_summary.token2)] = &lp_summary;
  }

  std::size_t row_id = 0;
  for (const auto &pair: sorted_pools) {
    const std::string& lp_name = pair.first;
    const pool_summary& lp_summary = *pair.second;

    m_rows.emplace_back(row_id++,
                        lp_name,
                        token_id_to_name(lp_summary.lp_token),
                        lp_summary.amount1,
                        lp_summary.amount2);
  }
}

const std::vector<LiquidityPoolInfoRow>& LiquidityPoolInfoImpl::getAll() const
{
  return m_rows;
}

} // namespace
