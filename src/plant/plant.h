// Copyright (c) 2018-2020, CUT coin
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

#ifndef CUTCOIN_PLANT_H
#define CUTCOIN_PLANT_H

#include "plantcallbacks.h"
#include "posmetrics.h"

#include <common/lightthreadpool.h>
#include <common/scheduler.h>
#include <common/sharedlock.h>
#include <crypto/hash.h>
#include <cryptonote_basic/cryptonote_basic.h>
#include <cryptonote_basic/difficulty.h>
#include <mining/miningutil.h>
#include <net/http_client.h>
#include <wallet/pending_tx.h>
#include <wallet/transfer_container.h>
#include <wallet/transfer_details.h>
#include <wallet/wallet2.h>

#include <boost/optional/optional.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>


namespace plant {

class Plant {
  // Contain basic functions for CUT coin staking.
  // This class is not thread safe and not Movable or Copyable.
  // It is designed according to RAII.

public:
  // Types
  using Client = std::shared_ptr<epee::net_utils::http::http_simple_client>;
  using Clock = std::chrono::system_clock;
  using Period = Clock::duration;
  using TimePoint = Clock::time_point;

private:
  struct MiningInfo {
    // Keep short summary info required for staking
    uint64_t                    d_height;
    crypto::hash                d_posHash;
    cryptonote::difficulty_type d_difficulty;
    uint64_t                    d_timestamp;
  };

private:
  // Data
  const size_t d_num_threads{5};                                  // number of threads we use the threadpool
  const Period d_data_update_period{std::chrono::seconds{1}};     // blockchain update period
  const Period d_reward_update_period{std::chrono::seconds{120}}; // POS reward update period

  std::shared_ptr<tools::wallet2>  d_wallet;             // wallet. Share, don't own
  Client                           d_http_client;        // RPC http client. Share, don't own
  tools::LightThreadPool           d_threadpool;         // thread pool for tasks execution
  tools::Scheduler                 d_scheduler;          // scheduler for deferred tasks
  tools::Scheduler::SharedTask     d_mining_task;        // current scheduled mining task
  std::atomic<bool>                d_is_mining;          // state flag
  uint64_t                         d_blockchain_height;  // current blockchain height
  std::string                      d_block_hash;         // current block cryptographic hash
  std::mutex                       d_pos_state_mutex;    // protect mining state (sequence of stages) consistence
  PosMetrics                       d_pos_metrics;        // contain POS metrics
  std::string                      d_reward_wallet_address; // wallet address for rewards
  size_t                           d_print_counter;      // how many times POS metrics were printed
  std::shared_ptr<plant::PlantCallbacks> d_plant_callbacks;
  boost::mutex                    &d_idle_mutex;
  boost::condition_variable       &d_idle_cond;

public:
  // Creators

  Plant(std::shared_ptr<tools::wallet2> wallet,
        Client client,
        boost::mutex &idle_mutex,
        boost::condition_variable &idle_cond,
        std::shared_ptr<plant::PlantCallbacks> plant_callbacks);
    // Construct this object.

  ~Plant();
    // Destruct this object.

public:
  // Deleted members
  Plant(const Plant& other) = delete;
  Plant& operator = (const Plant& other) = delete;

  Plant(const Plant&& other) = delete;
  Plant& operator = (const Plant&& other) = delete;

  // Public manipulators
  bool is_mining();
    // Return 'true' if the 'Plant' is mining.

  bool start_mining(const std::string &reward_wallet_address);
    // Start staking. The specified 'reward_wallet_address' is a wallet address where rewards are transferred.
    // If 'reward_wallet_address' is an empty string the rewards are transferred to the current wallet.

  void stop_mining();
    // Stop staking process.

  PosMetrics pos_metrics() const;
    // Return POS statistics.

private:
  // Private manipulators
  void handle_blockchain_update();
    // Handle event of blockchain height update.

  void handle_block_mining(const std::string &block_hash);
    // Handle event of block mining. This event is scheduled at the specific time that depends on
    // the current network difficulty, stake amount and piece of luck.

  void handle_reward_update();
    // Invoked to evaluate this account rewords within last 24 and 48 hours.

  bool get_last_block_info(uint64_t &height, std::string &hash) const;
    // Return last block height and hash.

  bool get_mining_info(MiningInfo &info) const;
    // Return actual mining info.

  void get_transfers(tools::transfer_container &transfers);
    // Get current account transfers.

  bool get_mining_output(const MiningInfo                &mining_info,
                         const tools::transfer_container &transfers,
                         tools::transfer_details         &pos_output,
                         mining::StakeDetails            &stake_details);
    // Find the best output appropriate for mining at the current height.

  bool get_pos_block_template(cryptonote::block                                        &block_template,
                              std::vector<uint8_t>                                     &extra,
                              crypto::hash                                             &prev_hash,
                              crypto::hash                                             &prev_crypto_hash,
                              crypto::hash                                             &merkle_root,
                              std::vector<std::vector<tools::wallet2::get_outs_entry>> &outs,
                              const mining::StakeDetails                               &stake_details,
                              const tools::transfer_details                            &pos_output) const;
    // Return POS block template.

  bool create_pos_tx(tools::pending_tx                                        &stake_tx,
                     std::vector<uint8_t>                                     &extra,
                     std::vector<std::vector<tools::wallet2::get_outs_entry>> &outs,
                     const tools::transfer_details                            &pos_output,
                     const mining::StakeDetails                               &stake_details,
                     const crypto::hash                                       &prev_crypto_hash,
                     const crypto::hash                                       &merkle_root);
    // Build POS staking transaction that contains a single input and a single effective output.

  bool publish_pos_block(const cryptonote::block &block, const cryptonote::transaction &tx);
    // Publish new POS block to the daemon.

  bool fit_stake_requirements(const tools::transfer_details &t);
    // Return 'true' if the specified 't' that correspond to a specific unspent output meet
    // conditions on POS stake output.

  bool is_maturing(const tools::transfer_details &t);

  template<typename t_request, typename t_response>
  bool invoke_rpc_request(std::string      method_name,
                          const t_request &request,
                          t_response      &response) const;
    // Function-helper that templating RPC requests.

  void print_pos_metrics_header();
    // Print header for POS metrics.

  void print_pos_metrics();
    // Print current staking status and metrics.

  void evaluate_pos_metrics();
};

} // namespace plant

#endif //CUTCOIN_PLANT_H
