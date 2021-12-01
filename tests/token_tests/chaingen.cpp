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

#include "chaingen.h"

#include "crypto/hash.h"
#include "cryptonote_basic/cryptonote_basic_impl.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include "cryptonote_basic/miner.h"
#include "cryptonote_core/cryptonote_tx_utils.h"
#include "misc_language.h"

void test_generator::get_block_chain(std::vector<block_info>& blockchain, const crypto::hash& head, size_t n) const
{
  crypto::hash curr = head;
  while (crypto::null_hash != curr && blockchain.size() < n)
  {
    auto it = m_blocks_info.find(curr);
    if (m_blocks_info.end() == it)
    {
      throw std::runtime_error("block hash wasn't found");
    }

    blockchain.push_back(it->second);
    curr = it->second.prev_id;
  }

  std::reverse(blockchain.begin(), blockchain.end());
}

void test_generator::get_last_n_block_weights(std::vector<size_t>& block_weights, const crypto::hash& head, size_t n) const
{
  std::vector<block_info> blockchain;
  get_block_chain(blockchain, head, n);
  BOOST_FOREACH(auto& bi, blockchain)
  {
    block_weights.push_back(bi.block_weight);
  }
}

uint64_t test_generator::get_already_generated_coins(const crypto::hash& blk_id) const
{
  auto it = m_blocks_info.find(blk_id);
  if (it == m_blocks_info.end())
    throw std::runtime_error("block hash wasn't found");

  return it->second.already_generated_coins;
}

uint64_t test_generator::get_already_generated_coins(const cryptonote::block& blk) const
{
  crypto::hash blk_hash;
  cryptonote::get_block_hash(blk, blk_hash);
  return get_already_generated_coins(blk_hash);
}

void test_generator::add_block(const cryptonote::block& blk, size_t txs_weight, std::vector<size_t>& block_weights, uint64_t already_generated_coins, uint8_t hf_version)
{
  const size_t block_weight = txs_weight + get_transaction_weight(blk.miner_tx);
  uint64_t block_reward;
  cryptonote::get_block_reward(epee::misc_utils::median(block_weights),
                               block_weight,
                               already_generated_coins,
                               block_reward,
                               hf_version);
  m_blocks_info[get_block_hash(blk)] = block_info(blk.prev_id, already_generated_coins + block_reward, block_weight);
}

bool test_generator::construct_block(cryptonote::block& blk, uint64_t height, const crypto::hash& prev_id,
                                     const cryptonote::account_base& miner_acc, uint64_t timestamp, uint64_t already_generated_coins,
                                     std::vector<size_t>& block_weights, const std::list<cryptonote::transaction>& tx_list)
{
  blk.major_version = CURRENT_BLOCK_MAJOR_VERSION;
  blk.minor_version = CURRENT_BLOCK_MINOR_VERSION;
  blk.timestamp = timestamp;
  blk.prev_id = prev_id;

  blk.tx_hashes.reserve(tx_list.size());
  BOOST_FOREACH(const cryptonote::transaction &tx, tx_list)
  {
    crypto::hash tx_hash;
    get_transaction_hash(tx, tx_hash);
    blk.tx_hashes.push_back(tx_hash);
  }

  uint64_t total_fee = 0;
  size_t txs_weight = 0;
  BOOST_FOREACH(auto& tx, tx_list)
  {
    uint64_t fee = 0;
    bool r = get_tx_fee(tx, fee);
    CHECK_AND_ASSERT_MES(r, false, "wrong transaction passed to construct_block");
    total_fee += fee;
    txs_weight += get_transaction_weight(tx);
  }

  blk.miner_tx = AUTO_VAL_INIT(blk.miner_tx);
  size_t target_block_weight = txs_weight + get_transaction_weight(blk.miner_tx);
  while (true)
  {
    if (!construct_miner_tx(height,
                            epee::misc_utils::median(block_weights),
                            already_generated_coins,
                            target_block_weight,
                            total_fee,
                            miner_acc.get_keys().m_account_address,
                            blk.miner_tx,
                            cryptonote::blobdata(),
                            10))
      return false;

    size_t actual_block_weight = txs_weight + get_transaction_weight(blk.miner_tx);
    if (target_block_weight < actual_block_weight)
    {
      target_block_weight = actual_block_weight;
    }
    else if (actual_block_weight < target_block_weight)
    {
      size_t delta = target_block_weight - actual_block_weight;
      blk.miner_tx.extra.resize(blk.miner_tx.extra.size() + delta, 0);
      actual_block_weight = txs_weight + get_transaction_weight(blk.miner_tx);
      if (actual_block_weight == target_block_weight)
      {
        break;
      }
      else
      {
        CHECK_AND_ASSERT_MES(target_block_weight < actual_block_weight, false, "Unexpected block size");
        delta = actual_block_weight - target_block_weight;
        blk.miner_tx.extra.resize(blk.miner_tx.extra.size() - delta);
        actual_block_weight = txs_weight + get_transaction_weight(blk.miner_tx);
        if (actual_block_weight == target_block_weight)
        {
          break;
        }
        else
        {
          CHECK_AND_ASSERT_MES(actual_block_weight < target_block_weight, false, "Unexpected block size");
          blk.miner_tx.extra.resize(blk.miner_tx.extra.size() + delta, 0);
          target_block_weight = txs_weight + get_transaction_weight(blk.miner_tx);
        }
      }
    }
    else
    {
      break;
    }
  }

  //blk.tree_root_hash = get_tx_tree_hash(blk);

  // Nonce search...
  blk.nonce = 0;
  while (!cryptonote::miner::find_nonce_for_given_block(blk, get_test_difficulty(), height))
    blk.timestamp++;

  add_block(blk, txs_weight, block_weights, already_generated_coins);

  return true;
}

bool test_generator::construct_block(cryptonote::block& blk, const cryptonote::account_base& miner_acc, uint64_t timestamp)
{
  std::vector<size_t> block_weights;
  std::list<cryptonote::transaction> tx_list;
  return construct_block(blk, 0, crypto::null_hash, miner_acc, timestamp, 0, block_weights, tx_list);
}

bool test_generator::construct_block(cryptonote::block& blk, const cryptonote::block& blk_prev,
                                     const cryptonote::account_base& miner_acc,
                                     const std::list<cryptonote::transaction>& tx_list/* = std::list<cryptonote::transaction>()*/)
{
  uint64_t height = boost::get<cryptonote::txin_gen>(blk_prev.miner_tx.vin.front()).height + 1;
  crypto::hash prev_id = get_block_hash(blk_prev);
  // Keep difficulty unchanged
  uint64_t timestamp = blk_prev.timestamp + DIFFICULTY_BLOCKS_ESTIMATE_TIMESPAN;
  uint64_t already_generated_coins = get_already_generated_coins(prev_id);
  std::vector<size_t> block_weights;
  get_last_n_block_weights(block_weights, prev_id, CRYPTONOTE_REWARD_BLOCKS_WINDOW);

  return construct_block(blk, height, prev_id, miner_acc, timestamp, already_generated_coins, block_weights, tx_list);
}

bool test_generator::construct_block_manually(cryptonote::block                 &blk,
                                              const cryptonote::block           &prev_block,
                                              const cryptonote::account_base    &miner_acc,
                                              int                                actual_params/* = bf_none*/,
                                              uint8_t                            major_ver/* = 0*/,
                                              uint8_t                            minor_ver/* = 0*/,
                                              uint64_t                           timestamp/* = 0*/,
                                              const crypto::hash                &prev_id/* = crypto::hash()*/,
                                              const cryptonote::difficulty_type &diffic/* = 1*/,
                                              const cryptonote::transaction     &miner_tx/* = transaction()*/,
                                              const std::vector<crypto::hash>   &tx_hashes/* = std::vector<crypto::hash>()*/,
                                              size_t                             txs_weight/* = 0*/,
                                              size_t                             max_outs/* = 0*/,
                                              uint8_t                            hf_version/* = 1*/)
{
  blk.major_version = actual_params & bf_major_ver ? major_ver : CURRENT_BLOCK_MAJOR_VERSION;
  blk.minor_version = actual_params & bf_minor_ver ? minor_ver : CURRENT_BLOCK_MINOR_VERSION;
  blk.timestamp     = actual_params & bf_timestamp ? timestamp : prev_block.timestamp + DIFFICULTY_BLOCKS_ESTIMATE_TIMESPAN; // Keep difficulty unchanged
  blk.prev_id       = actual_params & bf_prev_id   ? prev_id   : get_block_hash(prev_block);
  blk.tx_hashes     = actual_params & bf_tx_hashes ? tx_hashes : std::vector<crypto::hash>();
  max_outs          = actual_params & bf_max_outs ? max_outs : 9999;
  hf_version        = actual_params & bf_hf_version ? hf_version : 1;

  size_t height = get_block_height(prev_block) + 1;
  uint64_t already_generated_coins = get_already_generated_coins(prev_block);
  std::vector<size_t> block_weights;
  get_last_n_block_weights(block_weights, get_block_hash(prev_block), CRYPTONOTE_REWARD_BLOCKS_WINDOW);
  if (actual_params & bf_miner_tx)
  {
    blk.miner_tx = miner_tx;
  }
  else
  {
    size_t current_block_weight = txs_weight + get_transaction_weight(blk.miner_tx);
    // TODO: This will work, until size of constructed block is less then CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE
    if (!construct_miner_tx(height,
                            epee::misc_utils::median(block_weights),
                            already_generated_coins,
                            current_block_weight,
                            0,
                            miner_acc.get_keys().m_account_address,
                            blk.miner_tx,
                            cryptonote::blobdata(),
                            max_outs,
                            hf_version))
      return false;
  }

  //blk.tree_root_hash = get_tx_tree_hash(blk);

  cryptonote::difficulty_type a_diffic = actual_params & bf_diffic ? diffic : get_test_difficulty();
  fill_nonce(blk, a_diffic, height);

  add_block(blk, txs_weight, block_weights, already_generated_coins, hf_version);

  return true;
}

bool test_generator::construct_block_manually_tx(cryptonote::block& blk, const cryptonote::block& prev_block,
                                                 const cryptonote::account_base& miner_acc,
                                                 const std::vector<crypto::hash>& tx_hashes, size_t txs_weight)
{
  return construct_block_manually(blk,
                                  prev_block,
                                  miner_acc,
                                  bf_tx_hashes,
                                  0,
                                  0,
                                  0,
                                  crypto::hash(),
                                  0,
                                  cryptonote::transaction(),
                                  tx_hashes,
                                  txs_weight);
}

void fill_nonce(cryptonote::block& blk, const cryptonote::difficulty_type &diffic, uint64_t height)
{
  blk.nonce = 0;
  while (!cryptonote::miner::find_nonce_for_given_block(blk, diffic, height))
    blk.timestamp++;
}