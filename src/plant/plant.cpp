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

#include "plant.h"

#include "logging.h"

#include <common/lightthreadpool.h>
#include <common/sharedlock.h>
#include <crypto/hash.h>
#include <cryptonote_basic/blobdatatype.h>
#include <cryptonote_basic/cryptonote_basic.h>
#include <cryptonote_basic/tx_extra.h>
#include <cryptonote_core/cryptonote_tx_utils.h>
#include <cryptonote_config.h>
#include <mining/miningutil.h>
#include <rpc/core_rpc_server_commands_defs.h>
#include <storages/http_abstract_invoke.h>
#include <string_tools.h>
#include <wallet/transfer_container.h>
#include <wallet/transfer_details.h>
#include <wallet/wallet2.h>

#include <algorithm>
#include <functional>
#include <limits>
#include <string>
#include <utility>

namespace plant {


// Block of the Standard constants and utils

static const std::chrono::seconds rpc_timeout = std::chrono::minutes(3) + std::chrono::seconds(30);

Plant::Plant(std::shared_ptr<tools::wallet2> wallet,
             Client client,
             boost::mutex &idle_mutex,
             boost::condition_variable &idle_cond,
             std::shared_ptr<plant::PlantCallbacks> plant_callbacks)
: d_wallet(wallet)
, d_http_client(client)
, d_threadpool(d_num_threads)
, d_scheduler(d_threadpool)
, d_mining_task(nullptr)
, d_is_mining(false)
, d_blockchain_height(0)
, d_block_hash{}
, d_idle_mutex(idle_mutex)
, d_idle_cond(idle_cond)
, d_pos_metrics{}
, d_reward_wallet_address{}
, d_plant_callbacks{plant_callbacks}
, d_print_counter{0}
{
  d_scheduler.start();
}

Plant::~Plant()
{
  d_scheduler.shutdown();
}

bool Plant::is_mining()
{
  return d_is_mining;
}

bool Plant::start_mining(const std::string &reward_wallet_address)
{
  bool started = false;
  if (!d_is_mining.compare_exchange_strong(started, true)) {
    message_writer() << tr("Mining is already started");
    return false;
  }

  if (!d_http_client) {
    MINE_ERROR("Invalid network client provided, stop mining");
    stop_mining();
    return false;
  }

  if(!d_http_client->is_connected()) {
    MINE_DEBUG("Connecting to the daemon");
    d_http_client->connect(std::chrono::milliseconds((uint32_t)3000));
  }

  d_reward_wallet_address = reward_wallet_address;
  d_blockchain_height     = 0;
  d_block_hash            = {};

  d_scheduler.schedule_every(std::bind(&Plant::handle_blockchain_update, this), Clock::now(), d_data_update_period);
  d_scheduler.schedule_every(std::bind(&Plant::handle_reward_update, this), Clock::now(), d_reward_update_period);

  message_writer() << tr("POS staking is started");

  return true;
}

void Plant::stop_mining()
{
  d_is_mining = false;
  d_scheduler.clear();

  message_writer() << tr("POS staking is stopped");
}

void Plant::handle_blockchain_update()
{
  {
    std::lock_guard<std::mutex> lock(d_pos_state_mutex);
    boost::unique_lock<boost::mutex> lock2(d_idle_mutex);

    uint64_t cur_height;
    std::string cur_hash;
    if (!get_last_block_info(cur_height, cur_hash)) {
      MINE_ERROR("Could not retrieve blockchain info from a daemon");
      return;
    }

    if (d_blockchain_height == cur_height && d_block_hash == cur_hash) {
      return;
    }

    MINE_DEBUG("Current height differs from the saved one, do mining");

    d_blockchain_height = cur_height;
    d_block_hash        = cur_hash;

    MiningInfo mining_info{};
    if (!get_mining_info(mining_info)) {
      MINE_ERROR("Could not get data required for mining from a daemon");
      return;
    }

    d_pos_metrics.d_timestamp = mining_info.d_timestamp;
    d_pos_metrics.d_height = mining_info.d_height;
    d_pos_metrics.d_difficulty = mining_info.d_difficulty;

    tools::transfer_container transfers;
    get_transfers(transfers);
    if (transfers.empty()) {
      MINE_WARNING("No unspent outputs in the wallet");
      return;
    }

    tools::transfer_details pos_output;
    mining::StakeDetails    stake_details;
    if (!get_mining_output(mining_info, transfers, pos_output, stake_details)) {
      std::stringstream error_message;
      error_message << "No suitable unspent outputs for mining. One must have amount more or equal to "
                    << 1 << "cutcoin and maturity at least " << config::OUTPUT_STAKE_MATURITY << " blocks."
                    << std::endl;
      MINE_WARNING(error_message.str().c_str());
      return;
    }

    std::chrono::duration<int, std::milli> block_building_time(mining::block_building_time);
    if (d_mining_task) {
      d_scheduler.remove(d_mining_task);
    }

    d_mining_task = d_scheduler.schedule_at(
        std::bind(&Plant::handle_block_mining, this, d_block_hash),
        stake_details.d_new_block_timestamp - block_building_time);

    evaluate_pos_metrics();

    d_idle_cond.notify_one();
  }

  if (d_pos_metrics.d_last_block_age > 5 * DIFFICULTY_TARGET_V2) {
    message_writer() << "Last block time is too far in the past, you are likely disconnected from the network, "
                        "the information below may be obsolete, please check your daemon." << std::endl;
  }

  print_pos_metrics();

  if (d_plant_callbacks) {
    d_plant_callbacks->on_pos_metrics_updated();
  }
}

void Plant::handle_block_mining(const std::string &block_hash)
{
  if (!d_http_client->is_connected()) {
    MINE_ERROR("No connection to the daemon");
    return;
  }

  {
    std::lock_guard<std::mutex> lock(d_pos_state_mutex);
    boost::unique_lock<boost::mutex> lock2(d_idle_mutex);

    // acquire daemon again to have fresh information
    uint64_t cur_height;
    std::string cur_hash;
    if (!get_last_block_info(cur_height, cur_hash)) {
      MINE_ERROR("Could not retrieve blockchain info from a daemon");
      return;
    }

    if (d_blockchain_height != cur_height || d_block_hash != cur_hash) {
      return;
    }

    MiningInfo mining_info{};
    if (!get_mining_info(mining_info)) {
      MINE_ERROR("Could not retrieve data required for mining from a daemon");
      return;
    }

    tools::transfer_container transfers;
    get_transfers(transfers);
    if (transfers.empty()) {
      MINE_WARNING("No unspent outputs in the wallet");
      return;
    }

    tools::transfer_details pos_output;
    mining::StakeDetails stake_details;
    if (!get_mining_output(mining_info, transfers, pos_output, stake_details)) {
      std::stringstream error_message;
      error_message << "Found no suitable unspent outputs for mining. One must have amount more or equal to " << 1
                    << " and maturity at least " << config::OUTPUT_STAKE_MATURITY << " blocks."
                    << std::endl;
      MINE_WARNING(error_message.str().c_str());
      return;
    }

    cryptonote::block block_template;
    AUTO_VAL_INIT(block_template);

    std::vector<uint8_t> extra;
    cryptonote::tx_extra_pos_stamp pos_stamp{};
    if (!add_pos_stamp_to_tx_extra(extra, pos_stamp)) {
      MINE_WARNING("Failed to add PoS stamp to 'tx extra'");
      return;
    }

    crypto::hash prev_hash;
    crypto::hash prev_crypto_hash;
    crypto::hash merkle_root;
    std::vector<std::vector<tools::wallet2::get_outs_entry>> outs;

    bool res = get_pos_block_template(block_template,
                                      extra,
                                      prev_hash,
                                      prev_crypto_hash,
                                      merkle_root,
                                      outs,
                                      stake_details,
                                      pos_output);
    if (!res) {
      MINE_ERROR("Could not retrieve block template from a daemon");
      return;
    }

    tools::pending_tx stake_tx;
    res = create_pos_tx(stake_tx, extra, outs, pos_output, stake_details, prev_crypto_hash, merkle_root);
    if (!res) {
      MINE_ERROR("Could not create POS transaction");
      return;
    }

    if (!get_last_block_info(cur_height, cur_hash)) {
      MINE_ERROR("Could not retrieve blockchain info from a daemon");
      return;
    }

    if (cur_hash != block_hash) {
      MINE_WARNING("Somebody just mined a new block. Next time you'll be more lucky!");
      return;
    }

    publish_pos_block(block_template, stake_tx.tx);
  }
}

void Plant::handle_reward_update()
{
  uint64_t blocks_in_24_h = 86400 / DIFFICULTY_TARGET_V2;
  uint64_t blocks_in_48_h = blocks_in_24_h << 1;

  {
    std::lock_guard<std::mutex> lock(d_pos_state_mutex);
    boost::unique_lock<boost::mutex> lock2(d_idle_mutex);

    tools::transfer_container pos_transfers;
    d_wallet->get_pos_transfers(pos_transfers, std::max<uint64_t>(d_blockchain_height - blocks_in_24_h, 0));

    uint64_t amount_24_h = 0;
    for (const auto &t: pos_transfers) {
      if (!t.m_spent) {
        amount_24_h += t.amount();
      }
    }
    d_pos_metrics.d_forged_in_last_24 = amount_24_h / COIN;

    pos_transfers.clear();
    d_wallet->get_pos_transfers(pos_transfers, std::max<uint64_t>(d_blockchain_height - blocks_in_48_h, 0));

    uint64_t amount_48_h = 0;
    for (const auto &t: pos_transfers) {
      if (!t.m_spent) {
        amount_48_h += t.amount();
      }
    }

    d_pos_metrics.d_forged_in_last_48 = amount_48_h / COIN;
  }
}

bool Plant::get_last_block_info(uint64_t &height, std::string &hash) const
{
  cryptonote::COMMAND_RPC_GET_LAST_BLOCK_HEADER::request request = AUTO_VAL_INIT(request);
  cryptonote::COMMAND_RPC_GET_LAST_BLOCK_HEADER::response response = AUTO_VAL_INIT(response);
  if(!invoke_rpc_request("get_last_block_header", request, response)) {
    MINE_ERROR("RPC request error: 'get_last_block_header'");
    return false;
  }

  height = response.block_header.height;
  hash   = response.block_header.hash;
  return true;
}

bool Plant::get_mining_info(MiningInfo &info) const
{
  cryptonote::COMMAND_GET_BLOCK_MINING_INFO::request request = AUTO_VAL_INIT(request);
  cryptonote::COMMAND_GET_BLOCK_MINING_INFO::response response = AUTO_VAL_INIT(response);
  if(!invoke_rpc_request("get_mining_info", request, response)) {
    MINE_ERROR("RPC request error: 'get_mining_info'");
    return false;
  }

  info.d_timestamp  = response.block_mining_info.timestamp;
  info.d_height     = response.block_mining_info.height;
  info.d_difficulty = response.block_mining_info.difficulty;

  epee::string_tools::hex_to_pod(response.block_mining_info.pos_hash, info.d_posHash);

  std::stringstream debug_message;
  debug_message
      << "timestamp:  " <<  info.d_timestamp << std::endl
      << "height:     " <<  info.d_height << std::endl
      << "difficulty: " <<  info.d_difficulty << std::endl;
  MINE_DEBUG(debug_message.str().c_str());

  return true;
}

void Plant::get_transfers(tools::transfer_container &transfers)
{
  d_wallet->get_transfers(transfers);

#if defined(DEBUG_MINING)
  std::stringstream trace_message;
  trace_message
      << "Total number of transfers in the wallet: " << transfers.size() << std::endl
      << "key image | global output index | block height | amount" << std::endl;
  for (const auto &t: transfers) {
    trace_message << t.m_key_image           << " | "
                  << t.m_global_output_index << " | "
                  << t.m_block_height        << " | "
                  << t.amount() / COIN       << std::endl;
  }
  MINE_DEBUG(trace_message.str().c_str());
#endif
}

bool Plant::get_mining_output(const MiningInfo                &mining_info,
                              const tools::transfer_container &transfers,
                              tools::transfer_details         &pos_output,
                              mining::StakeDetails            &stake_details)
{
  tools::transfer_container filtered_transfers;
  std::copy_if(
      transfers.begin(),
      transfers.end(),
      std::back_inserter(filtered_transfers),
      std::bind(&Plant::fit_stake_requirements, this, std::placeholders::_1));

  if (filtered_transfers.empty()) {
    MINE_ERROR("No outputs appropriate for mining");
    return false;
  }

  crypto::hash previous_block_hash = mining_info.d_posHash;
  std::vector<mining::StakeDetails> candidates;
  for (const auto &t: filtered_transfers) {
    mining::StakeDetails candidate;
    mining::find_pos_hash(t.m_key_image, previous_block_hash, candidate.d_pos_hash);
    candidate.d_amount = t.m_amount;
    candidate.d_global_index = t.m_global_output_index;
    candidates.emplace_back(candidate);
  }

  auto it = std::min_element(
      candidates.begin(),
      candidates.end(),
      [](const mining::StakeDetails& d1, const mining::StakeDetails& d2)-> bool {
        return num::u128_t(d2.d_amount) * mining::target_from_hash(d1.d_pos_hash) <
            num::u128_t(d1.d_amount) * mining::target_from_hash(d2.d_pos_hash);
      });

  if (it == candidates.end()) {
    MINE_ERROR("Could not find best output for mining");
    return false;
  }

  stake_details = *it;
  for (const auto &t: filtered_transfers) {
    if (t.m_global_output_index == stake_details.d_global_index) {
      pos_output = t;
    }
  }

  try {
    cryptonote::difficulty_type cur_difficulty = mining_info.d_difficulty;
    TimePoint last_block_timestamp = Clock::from_time_t(mining_info.d_timestamp);

    uint64_t new_block_delta_t;  // milliseconds
    mining::get_new_block_time_delta(stake_details.d_pos_hash,
                                     stake_details.d_amount,
                                     cur_difficulty,
                                     new_block_delta_t);

    uint64_t addition = std::max<uint64_t>(new_block_delta_t / 1000,  1);
    stake_details.d_new_block_timestamp = last_block_timestamp + std::chrono::seconds(addition);

    std::time_t t1 = std::chrono::system_clock::system_clock::to_time_t(last_block_timestamp);
    std::time_t t2 = std::chrono::system_clock::system_clock::to_time_t(stake_details.d_new_block_timestamp);
    std::stringstream debug_message;
    debug_message
        << "____________________________________________________________________" << std::endl
        << "stake details: " << std::endl
        << "____________________________________________________________________" << std::endl
        << "prev block timestamp: " << t1 << std::endl
        << "new block timestamp:  " << t2 << std::endl
        << "delta t:              " << ((double)new_block_delta_t) / 1000.0 << "s" << std::endl
        << "amount:               " << stake_details.d_amount / COIN        << "cut" << std::endl
        << "POS hash:             " << stake_details.d_pos_hash      << std::endl
        << "difficulty:           " << mining_info.d_difficulty      << std::endl
        << "____________________________________________________________________" << std::endl;
    MINE_DEBUG(debug_message.str().c_str());

  } catch (const std::runtime_error& e) {
    MINE_ERROR("Overflow error. Could not find next block generation time");
    return false;
  }

  return true;
}

bool Plant::get_pos_block_template(cryptonote::block                                        &block_template,
                                   std::vector<uint8_t>                                     &extra,
                                   crypto::hash                                             &prev_hash,
                                   crypto::hash                                             &prev_crypto_hash,
                                   crypto::hash                                             &merkle_root,
                                   std::vector<std::vector<tools::wallet2::get_outs_entry>> &outs,
                                   const mining::StakeDetails                               &stake_details,
                                   const tools::transfer_details                            &pos_output) const
{
  cryptonote::COMMAND_RPC_GETPOSBLOCKTEMPLATE::request request = AUTO_VAL_INIT(request);
  cryptonote::COMMAND_RPC_GETPOSBLOCKTEMPLATE::response response = AUTO_VAL_INIT(response);

  size_t fake_outs_count = d_wallet->default_mixin();
  if (fake_outs_count == 0)
    fake_outs_count = 10;

  std::string reward_wallet_address = d_reward_wallet_address;  // copy to avoid race cond
  if (d_reward_wallet_address.empty()) {
    request.wallet_address = cryptonote::get_account_address_as_str(d_wallet->nettype(),
                                                                    false,
                                                                    d_wallet->get_address());
  } else {
    request.wallet_address = d_reward_wallet_address;
  }

  try {
    request.pos_tx_size = d_wallet->estimate_pos_tx_size(outs, fake_outs_count, pos_output, extra);
  } catch (const std::runtime_error& e) {
    MINE_ERROR("Could not estimate POS transaction size");
    return false;
  }

  if (1 != outs.size() || outs[0].empty()) {
    MINE_ERROR("Could not get outs for POS transaction");
    return false;
  }

  std::stringstream debug_message;
  debug_message
      << "request block template for wallet: " <<  request.wallet_address
      << " pos tx size: " << request.pos_tx_size
      << std::endl;
  MINE_DEBUG(debug_message.str().c_str());

  if(!invoke_rpc_request("get_pos_block_template", request, response)) {
    MINE_ERROR("RPC request error: 'get_pos_block_template'");
    return false;
  }

  if (!epee::string_tools::hex_to_pod(response.prev_hash, prev_hash)) {
    MINE_ERROR("Could not deserialize previous block PoS hash");
    return false;
  }

  if (!epee::string_tools::hex_to_pod(response.prev_crypto_hash, prev_crypto_hash)) {
    MINE_ERROR("Could not deserialize previous block Crypto hash");
    return false;
  }

  if (!epee::string_tools::hex_to_pod(response.merkleroot, merkle_root)) {
    MINE_ERROR("Could not deserialize Mercle root");
    return false;
  }

  cryptonote::blobdata blob_data;
  if(!epee::string_tools::parse_hexstr_to_binbuff(response.blocktemplate_blob, blob_data)) {
    MINE_ERROR("Could not deserialize block template");
    return false;
  }

  if(!parse_and_validate_block_from_blob(blob_data, block_template)) {
    MINE_ERROR("Could not parse block template");
    return false;
  }

  block_template.hash = stake_details.d_pos_hash;
  block_template.timestamp = std::chrono::system_clock::system_clock::to_time_t(stake_details.d_new_block_timestamp);

  return true;
}

bool Plant::create_pos_tx(tools::pending_tx                                        &stake_tx,
                          std::vector<uint8_t>                                     &extra,
                          std::vector<std::vector<tools::wallet2::get_outs_entry>> &outs,
                          const tools::transfer_details                            &pos_output,
                          const mining::StakeDetails                               &stake_details,
                          const crypto::hash                                       &prev_crypto_hash,
                          const crypto::hash                                       &merkle_root)
{
  size_t fake_outs_count = d_wallet->default_mixin();
  if (fake_outs_count == 0)
    fake_outs_count = 10;

  std::vector<cryptonote::tx_extra_field> extra_fields;
  if (!parse_tx_extra(extra, extra_fields)) {
    MINE_ERROR("Failed to parse tx extra field");
    return false;
  }

  cryptonote::tx_extra_pos_stamp pos_stamp{};
  if (!cryptonote::find_tx_extra_field_by_type(extra_fields, pos_stamp)) {
    MINE_ERROR("Failed to find PoS stamp field in tx extra");
    return false;
  }

  pos_stamp.amount      = stake_details.d_amount;
  pos_stamp.crypto_hash = prev_crypto_hash;
  pos_stamp.merkle_root = merkle_root;

  cryptonote::remove_field_from_tx_extra(extra, typeid(cryptonote::tx_extra_pos_stamp));

  if (!add_pos_stamp_to_tx_extra(extra, pos_stamp)) {
    MINE_ERROR("Failed to add PoS stamp field in tx extra");
    return false;
  }

  try {
    d_wallet->create_stake_transaction(stake_tx, outs, fake_outs_count, pos_output, extra);
  } catch (const std::runtime_error& e) {
    MINE_ERROR("Could not create PoS transaction");
    return false;
  }

  return true;
}

bool Plant::publish_pos_block(const cryptonote::block &block, const cryptonote::transaction &tx)
{
  cryptonote::COMMAND_RPC_SUBMITPOSBLOCK::request request = AUTO_VAL_INIT(request);
  cryptonote::COMMAND_RPC_SUBMITPOSBLOCK::response response = AUTO_VAL_INIT(response);

  cryptonote::blobdata block_blob = t_serializable_object_to_blob(block);
  request.block_blob = epee::string_tools::buff_to_hex_nodelimer(block_blob);

  cryptonote::blobdata tx_blob = t_serializable_object_to_blob(tx);
  request.pos_tx_blob = epee::string_tools::buff_to_hex_nodelimer(tx_blob);

  if(!invoke_rpc_request("submit_pos_block", request, response)) {
    MINE_ERROR("RPC request error: 'submit_pos_block'");
    return false;
  }

  return true;
}

bool Plant::fit_stake_requirements(const tools::transfer_details &t)
{
  return t.m_token_id == cryptonote::CUTCOIN_ID &&
         t.amount() >= COIN &&
         !t.m_spent &&
         d_wallet->is_transfer_unlocked(t) &&
         t.m_block_height + config::OUTPUT_STAKE_MATURITY <= d_blockchain_height;
}

bool Plant::is_maturing(const tools::transfer_details &t)
{
  return t.m_token_id == cryptonote::CUTCOIN_ID &&
         t.amount() >= COIN &&
         !t.m_spent &&
         d_wallet->is_transfer_unlocked(t) &&
         t.m_block_height + config::OUTPUT_STAKE_MATURITY > d_blockchain_height;
}

template<typename t_request, typename t_response>
bool Plant::invoke_rpc_request(std::string     method_name,
                               const t_request &request,
                               t_response      &response) const
{
  bool res     = false;
  size_t tries = 2;
  while (!res && !!tries) {
    d_wallet->lockTransport();
    res = epee::net_utils::invoke_http_json_rpc(
        "/json_rpc",
        method_name,
        request,
        response,
        *d_http_client,
        rpc_timeout);
    d_wallet->unlockTransport();

    --tries;
  }

  if(!res) {
    MINE_ERROR((std::string("RPC request error: ") + method_name).c_str());
    return false;
  }

  if(response.status == CORE_RPC_STATUS_BUSY) {
    MINE_ERROR((std::string("Core RPC status: ") + response.status).c_str());
    return false;
  }

  if (response.status != CORE_RPC_STATUS_OK) {
    MINE_ERROR("RPC error");
    return false;
  }
  return true;
}

void Plant::print_pos_metrics_header()
{
  message_writer()
    << "\n"
    << "_________________________________________________________________________________________________________________________" << "\n"
    << "Blockchain" << "|" << " Staking  " << "|" << " Maturing " << "|" << " Staking  " << "|" << "Time since" << "|" << "          " << "|" << "POS reward" << "|" << "POS reward" << "|" << "Best block" << "|" << "  Weekly  " << "|" << "Chance to  " << "\n"
    << "height    " << "|" << " amount   " << "|" << " outputs  " << "|" << " outputs  " << "|" << "last block" << "|" << "Difficulty" << "|" << "in 24h    " << "|" << "in 48h    " << "|" << "time      " << "|" << "  reward  " << "|" << "forge next " << "\n"
    << "          " << "|" << " (CUT)    " << "|" << " (CUT)    " << "|" << "          " << "|" << "(seconds) " << "|" << "          " << "|" << "(CUT)     " << "|" << "(CUT)     " << "|" << "(h:min:s) " << "|" << "  (CUT)   " << "|" << "block (%)  " << "\n"
    << "_________________________________________________________________________________________________________________________";
}

void Plant::print_pos_metrics()
{
  if (d_print_counter % 30 == 0) {
    print_pos_metrics_header();
  }
  ++d_print_counter;

  std::chrono::seconds expected_seconds = d_pos_metrics.d_expected_time_till_block > std::numeric_limits<long>::max()
    ? std::chrono::seconds(std::numeric_limits<long>::max())
    : std::chrono::seconds((long)d_pos_metrics.d_expected_time_till_block);

  auto h = std::chrono::duration_cast<std::chrono::hours>(expected_seconds);
  expected_seconds -= h;
  auto m = std::chrono::duration_cast<std::chrono::minutes>(expected_seconds);
  expected_seconds -= m;
  auto s = std::chrono::duration_cast<std::chrono::seconds>(expected_seconds);

  std::string expected_time_till_block = h.count() > 999
    ? std::to_string(h.count()) + "h"
    : std::to_string(h.count()) + ":" + std::to_string(m.count()) + ":" + std::to_string(s.count());


  double chance_to_mine_next_block_per_cent = d_pos_metrics.d_chance_to_mine_next_block <= 1.0
    ? 100.0 * d_pos_metrics.d_chance_to_mine_next_block : 100.0;

  message_writer()
    <<        std::setw(10) << d_pos_metrics.d_height
    << " " << std::setw(10) << d_pos_metrics.d_on_stake
    << " " << std::setw(10) << d_pos_metrics.d_maturing
    << " " << std::setw(10) << d_pos_metrics.d_pos_outputs_count
    << " " << std::setw(10) << d_pos_metrics.d_last_block_age
    << " " << std::setw(10) << d_pos_metrics.d_difficulty
    << " " << std::setw(10) << d_pos_metrics.d_forged_in_last_24
    << " " << std::setw(10) << d_pos_metrics.d_forged_in_last_48
    << " " << std::setw(10) << expected_time_till_block
    << " " << std::setw(10) << d_pos_metrics.d_expected_reward_per_week
    << " " << std::fixed << std::setw(11) << std::setprecision(9) << chance_to_mine_next_block_per_cent;
}

void Plant::evaluate_pos_metrics()
{
  uint64_t cur_time = static_cast<uint64_t>(std::chrono::system_clock::system_clock::to_time_t(std::chrono::system_clock::now()));
  d_pos_metrics.d_last_block_age = cur_time > d_pos_metrics.d_timestamp ? cur_time - d_pos_metrics.d_timestamp : 0;

  d_pos_metrics.d_on_stake                  = 0;
  d_pos_metrics.d_maturing                  = 0;
  d_pos_metrics.d_pos_outputs_count         = 0;
  d_pos_metrics.d_expected_time_till_block  = 0;
  d_pos_metrics.d_expected_reward_per_week  = 0;
  d_pos_metrics.d_chance_to_mine_next_block = 0;

  tools::transfer_container transfers;
  get_transfers(transfers);

  if (transfers.empty()) {
    return;
  }

  for (const tools::transfer_details &t: transfers) {
    if (fit_stake_requirements(t)) {
      d_pos_metrics.d_on_stake += t.amount();
      ++d_pos_metrics.d_pos_outputs_count;
    } else if (is_maturing(t)) {
      d_pos_metrics.d_maturing += t.amount();
    }
  }

  d_pos_metrics.d_on_stake /= COIN;
  d_pos_metrics.d_maturing /= COIN;

  d_pos_metrics.d_chance_to_mine_next_block = ((double)d_pos_metrics.d_on_stake) / d_pos_metrics.d_difficulty;
  d_pos_metrics.d_expected_time_till_block = (((double)DIFFICULTY_TARGET_V2) / d_pos_metrics.d_chance_to_mine_next_block) - d_pos_metrics.d_last_block_age;

  double k = 0.9999980926512037;
  double first_reward = ((double)(MONEY_SUPPLY - GENESIS_TX_REWARD)) / (1 << (EMISSION_SPEED_FACTOR_PER_MINUTE - 1));
  uint64_t blocks_per_week = 604800 / DIFFICULTY_TARGET_V2;
  double cur_supply  = (GENESIS_TX_REWARD + k * (pow(k, d_pos_metrics.d_height - 1) - 1.0) / (k - 1.0) * first_reward) / COIN;
  double week_supply = (GENESIS_TX_REWARD + k * (pow(k, d_pos_metrics.d_height + blocks_per_week - 1) - 1.0) / (k - 1.0) * first_reward) / COIN;
  double cur_balance = d_pos_metrics.d_on_stake + d_pos_metrics.d_maturing;
  double erpw = cur_balance * (week_supply / cur_supply - 1.0);
  d_pos_metrics.d_expected_reward_per_week = erpw;
}

PosMetrics Plant::pos_metrics() const
{
  return  d_pos_metrics;
}

}  // namespace plant
