// Copyright (c) 2020-2022, CUT coin
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

#include "exchange_util.h"

#include "common/i18n.h"
#include "rpc/core_rpc_server_commands_defs.h"
#include "wallet2.h"

#include <boost/format.hpp>

namespace tools {

namespace {
  const auto BLOCKS_BETWEEN_ROUNDS = 5;
}

Exchanger::Exchanger(wallet2 &wallet)
: d_wallet(wallet) {
  d_exchange_context.exchange_state = ExchangeState::completed;
}

void Exchanger::exchange(const uint32_t                       subaddress_account,
                         const cryptonote::CompositeTransfer &composite_transfer,
                         StatusCallback                       finished_cb,
                         StatusCallback                       exchange_round_cb,
                         size_t                               max_rounds,
                         double                               slippage,
                         size_t                               custom_fake_outs_count) {

  ExchangeContext ec;

  ec.exchange_state         = ExchangeState::in_progress;
  ec.pending_txs.clear();
  ec.current_round          = 0;
  ec.next_round_height      = 0;
  ec.max_rounds             = max_rounds;
  ec.initial_ratio          = 0.0;
  ec.cur_ratio              = 0.0;
  ec.max_slippage           = slippage;
  ec.cur_slippage           = slippage;
  ec.finished_cb            = finished_cb;
  ec.exchange_round_cb      = exchange_round_cb;
  ec.subaddress_account     = subaddress_account;
  ec.custom_fake_outs_count = custom_fake_outs_count;
  ec.lp_name                = cryptonote::tokens_to_lpname(composite_transfer.d_token1, composite_transfer.d_token2);
  ec.amount                 = composite_transfer.d_amount;
  ec.pool_interest          = composite_transfer.d_pool_interest;
  ec.side                   = composite_transfer.d_side;

  {
    std::lock_guard<std::mutex> lock(d_exchange_state_mutex);
    d_exchange_context = ec;
  }

  exchange_round();
}

bool Exchanger::in_progress() const {
  std::lock_guard<std::mutex> lock(d_exchange_state_mutex);
  return d_exchange_context.exchange_state == ExchangeState::in_progress;
}

void Exchanger::exchange_round() {
  using namespace cryptonote;

  pending_tx_v ptx_vector{};

  std::lock_guard<std::mutex> lock(d_exchange_state_mutex);

  ExchangeContext &ec = d_exchange_context;

  if (ec.exchange_state == ExchangeState::completed) {
    return;
  }

  if (ec.current_round >= ec.max_rounds) {
    process_event(tr("Exceeded maximal number of tries"), crypto::null_hash, true, false,
                  ec.cur_ratio, ec.cur_slippage, ec.current_round);
    ec.exchange_state = ExchangeState::completed;
    return;
  }

  LiquidityPool lp_summary{};
  {
    COMMAND_RPC_GET_LIQUIDITY_POOL::request req{};
    req.name = ec.lp_name;
    COMMAND_RPC_GET_LIQUIDITY_POOL::response res{};
    bool r = d_wallet.invoke_http_bin("/get_liquidity_pool.bin", req, res);

    if (!r) {
      process_event(tr("Could not get liquidity pool"), crypto::null_hash, true, false,
                    ec.cur_ratio, ec.cur_slippage, ec.current_round);
      ec.exchange_state = ExchangeState::completed;
      return;
    }

    if (res.status == CORE_RPC_STATUS_BUSY) {
      process_event(tr("RPC error"), crypto::null_hash, true, false,
                    ec.cur_ratio, ec.cur_slippage, ec.current_round);
      ec.exchange_state = ExchangeState::completed;
      return;
    }

    if (res.status != CORE_RPC_STATUS_OK) {
      process_event(tr("RPC error"), crypto::null_hash, true, false,
                    ec.cur_ratio, ec.cur_slippage, ec.current_round);
      ec.exchange_state = ExchangeState::completed;
      return;
    }

    if (res.liquidity_pool.token1 == cryptonote::CUTCOIN_ID) {
      process_event(tr("Could not find the specified liquidity pool"), crypto::null_hash, true, false,
                    ec.cur_ratio, ec.cur_slippage, ec.current_round);
      ec.exchange_state = ExchangeState::completed;
      return;
    }

    const auto &lp = res.liquidity_pool;
    lp_summary.d_token1 = lp.token1;
    lp_summary.d_token2 = lp.token2;
    lp_summary.d_ratio = {lp.amount1, lp.amount2};
    lp_summary.d_lptoken = lp.lp_token;
    lp_summary.d_lp_amount = lp.lp_amount;
  }

  Amount derived_amount = 0;
  if (ec.side == ExchangeSide::buy) {
    derived_amount = derive_buy_amount_from_lp_pair(lp_summary.d_ratio, ec.amount, ec.pool_interest);
  } else if (ec.side == ExchangeSide::sell) {
    derived_amount = derive_sell_amount_from_lp_pair(lp_summary.d_ratio, ec.amount, ec.pool_interest);
  } else {
    process_event(tr("Cross exchanges are not allowed in a slippage mode"), crypto::null_hash, true, false,
                  ec.cur_ratio, ec.cur_slippage, ec.current_round);
    ec.exchange_state = ExchangeState::completed;
    return;
  }

  if (ec.current_round == 0) {  // remember initial exchange ratio at first iteration
    ec.initial_ratio = (double) ec.amount / derived_amount;
  }

  ec.cur_ratio = (double) ec.amount / derived_amount;
  ec.cur_slippage = ec.side == ExchangeSide::buy ?
                            (ec.initial_ratio - ec.cur_ratio) / ec.initial_ratio :
                            (ec.cur_ratio - ec.initial_ratio) / ec.initial_ratio;

  if (ec.cur_slippage > ec.max_slippage) {
    process_event(tr("Exchange rate change has exceeded the slippage range"), crypto::null_hash, true, false,
                  ec.cur_ratio, ec.cur_slippage, ec.current_round);
    ec.exchange_state = ExchangeState::completed;
    return;
  }

  CompositeTransfer ct{};
  ct.d_side = ec.side;
  ct.d_token1 = lp_summary.d_token1;
  ct.d_token2 = lp_summary.d_token2;
  ct.d_amount = ec.amount;
  ct.d_pool_interest = config::DEX_FEE_PER_MILLE;

  ExchangeTransfer summary{};
  d_wallet.exchange_transfer(
    ec.subaddress_account, ct, ptx_vector, summary, ec.custom_fake_outs_count);

  if (ptx_vector.size() != 1) {
    process_event(tr("Could not create transaction for the exchange"), crypto::null_hash, true, false,
                  ec.cur_ratio, ec.cur_slippage, ec.current_round);
    ec.exchange_state = ExchangeState::completed;
    return;
  }

  auto tx_hash = get_transaction_hash(ptx_vector.back().tx);

  std::ostringstream info;
  info << tr("Run new round of slippage exchange") << std::endl;
  if (ec.side == ExchangeSide::buy) {
      info << boost::format(tr("You are going to buy %s with exchange rate %f."))
                % token_id_to_name(lp_summary.d_token1)
                % ec.cur_ratio << std::endl;
      info << boost::format(tr("You are sending to the pool %s %s"))
                % print_money(derived_amount) % token_id_to_name(lp_summary.d_token2) << std::endl;
      info << boost::format(tr("and receiving to the wallet %s %s"))
                % print_money(ct.d_amount) % token_id_to_name(lp_summary.d_token1) << std::endl;
      info << boost::format(tr("price impact %.2f%%"))
                % price_impact(lp_summary.d_ratio, {ct.d_amount, derived_amount}) << std::endl;
      info << boost::format(tr("pool interest %s %s."))
                % print_money(derive_buy_amount_from_lp_pair(lp_summary.d_ratio, ct.d_amount, ct.d_pool_interest) -
                              derive_buy_amount_from_lp_pair(lp_summary.d_ratio, ct.d_amount, 0))
                % token_id_to_name(lp_summary.d_token2) << std::endl;
  }
  else {
    info << boost::format(tr("You are going to sell %s with exchange rate %f."))
            % token_id_to_name(lp_summary.d_token1)
            % ec.cur_ratio << std::endl;
    info << boost::format(tr("You are receiving to the wallet %s %s"))
              % print_money(derived_amount) % token_id_to_name(lp_summary.d_token2) << std::endl;
    info << boost::format(tr("and sending to the pool %s %s"))
              % print_money(ct.d_amount) % token_id_to_name(lp_summary.d_token1) << std::endl;
    info << boost::format(tr("price impact %.2f%%"))
              % price_impact(lp_summary.d_ratio, {ct.d_amount, derived_amount}) << std::endl;
    info << boost::format(tr("pool interest %s %s."))
              % print_money(derive_sell_amount_from_lp_pair(lp_summary.d_ratio, ct.d_amount, 0) -
                            derive_sell_amount_from_lp_pair(lp_summary.d_ratio, ct.d_amount, ct.d_pool_interest))
              % token_id_to_name(lp_summary.d_token2) << std::endl;
  }
  info << boost::format(tr("You will pay total fee %s CUT for this operation."))
            % print_money(ptx_vector[0].fee);
  process_event(info.str(), tx_hash, false, false,
                ec.cur_ratio, ec.cur_slippage, ec.current_round);
  ++ec.current_round;

  try {
    auto &ptx = ptx_vector.back();
    d_wallet.commit_tx(ptx);
  }
  catch(const std::exception& err) {
    std::ostringstream oss;
    oss << "Failed to commit tx " << tx_hash << " :" << err.what();
    process_event(oss.str(), tx_hash, false, false,
                  ec.cur_ratio, ec.cur_slippage, ec.current_round);
    return;
  }

  ec.pending_txs[tx_hash] = ExchangeInfo{ec.cur_ratio, ec.cur_slippage, ec.current_round};
}

void Exchanger::process_tx(const crypto::hash &txid) {
  std::lock_guard<std::mutex> lock(d_exchange_state_mutex);

  ExchangeContext &ec = d_exchange_context;

  if (ec.pending_txs.count(txid) == 0) {
    return;
  }

  auto& ec_info = ec.pending_txs[txid];

  if (ec.exchange_state == ExchangeState::in_progress) {
    process_event(tr("Slippage exchange completed"), txid, true, true,
                     ec_info.ratio, ec_info.slippage, ec_info.n_rounds);
  }
  else {
    process_event(tr("Internal error: wrong exchange status"), txid, true, false,
                  ec_info.ratio, ec_info.slippage, ec_info.n_rounds);
  }
  ec.pending_txs.clear();
  ec.exchange_state = ExchangeState::completed;
}

void Exchanger::process_block(uint64_t height) {
  bool new_round = false;
  {
    std::lock_guard<std::mutex> lock(d_exchange_state_mutex);
    ExchangeContext &ec = d_exchange_context;

    new_round = ec.pending_txs.empty() || ec.next_round_height == height;
    if (new_round) {
      ec.next_round_height = height + BLOCKS_BETWEEN_ROUNDS;
    }
  }

  if (new_round) {
    exchange_round();
  }
}

void Exchanger::stop_exchange() {
  std::lock_guard<std::mutex> lock(d_exchange_state_mutex);

  ExchangeContext &ec = d_exchange_context;

  ec.pending_txs.clear();
  ec.exchange_state = ExchangeState::completed;

  process_event(tr("Slippage exchange has been stopped"), crypto::null_hash, true, false,
                ec.cur_ratio, ec.cur_slippage, ec.current_round);
}

void Exchanger::process_event(const std::string  &message,
                              const crypto::hash &tx_hash,
                              bool                completed,
                              bool                success,
                              double              ratio,
                              double              slippage,
                              size_t              n_rounds)
{
  completed ?
    d_exchange_context.finished_cb({message, tx_hash, completed, success, ratio, slippage, n_rounds}) :
    d_exchange_context.exchange_round_cb({message, tx_hash, completed, success, ratio, slippage, n_rounds});
}

}  // namespace tools
