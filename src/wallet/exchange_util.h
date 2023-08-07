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

#ifndef CUTCOIN_EXCHANGE_UTIL_H
#define CUTCOIN_EXCHANGE_UTIL_H

#include <common/lightthreadpool.h>
#include <common/scheduler.h>

#include <cryptonote_basic/cryptonote_basic.h>
#include <cryptonote_basic/dex.h>

#include <chrono>
#include <unordered_map>

namespace tools {

class wallet2;

enum class ExchangeState: int {
  in_progress = 0,
  completed   = 1
};

struct ExchangeStatus {
  std::string message;
  crypto::hash tx_hash;
  bool completed;
  bool success;
  double ratio;
  double slippage;
  size_t n_rounds;
};

struct ExchangeInfo {
  double ratio;
  double slippage;
  size_t n_rounds;
};

using StatusCallback =  std::function<void(ExchangeStatus)>;
using TxInfoMap = std::unordered_map<crypto::hash, ExchangeInfo>;

struct ExchangeContext {
  ExchangeState            exchange_state;
  TxInfoMap                pending_txs;
  double                   initial_ratio;
  double                   cur_ratio;
  double                   max_slippage;
  double                   cur_slippage;
  size_t                   current_round;
  uint64_t                 next_round_height;
  size_t                   max_rounds;
  StatusCallback           finished_cb;
  StatusCallback           exchange_round_cb;
  uint32_t                 subaddress_account;
  size_t                   custom_fake_outs_count;
  std::string              lp_name;
  cryptonote::Amount       amount;
  cryptonote::Amount       pool_interest;
  cryptonote::ExchangeSide side;
};


class Exchanger {

public:
  using Clock = std::chrono::system_clock;
  using Period = Clock::duration;

private:

  wallet2&           d_wallet;
  mutable std::mutex d_exchange_state_mutex;  // protect exchange state consistence
  ExchangeContext    d_exchange_context;


public:
  explicit Exchanger(wallet2 &wallet);

public:
  void exchange(const uint32_t                       subaddress_account,
                const cryptonote::CompositeTransfer &composite_transfer,
                StatusCallback                       finished_cb,
                StatusCallback                       exchange_round_cb,
                size_t                               max_rounds,
                double                               slippage,
                size_t                               custom_fake_outs_count);

  bool in_progress() const;

  void process_tx(const crypto::hash &txid);

  void process_block(uint64_t height);

  void stop_exchange();

private:
  void exchange_round();

  void process_event(const std::string  &message,
                     const crypto::hash &tx_hash,
                     bool                completed,
                     bool                success,
                     double              ratio,
                     double              slippage,
                     size_t              n_rounds);
};

}  // namespace tools

#endif //CUTCOIN_EXCHANGE_UTIL_H
