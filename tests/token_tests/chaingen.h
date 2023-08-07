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

#ifndef CUTCOIN_CHAINGEN_H
#define CUTCOIN_CHAINGEN_H

#include "crypto/crypto.h"
#include "cryptonote_basic/account.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_basic/difficulty.h"

class test_generator
{
public:
  struct block_info
  {
    block_info()
      : prev_id()
      , already_generated_coins(0)
      , block_weight(0)
    {
    }

    block_info(crypto::hash a_prev_id, uint64_t an_already_generated_coins, size_t a_block_weight)
      : prev_id(a_prev_id)
      , already_generated_coins(an_already_generated_coins)
      , block_weight(a_block_weight)
    {
    }

    crypto::hash prev_id;
    uint64_t already_generated_coins;
    size_t block_weight;
  };

  enum block_fields
  {
    bf_none      = 0,
    bf_major_ver = 1 << 0,
    bf_minor_ver = 1 << 1,
    bf_timestamp = 1 << 2,
    bf_prev_id   = 1 << 3,
    bf_miner_tx  = 1 << 4,
    bf_tx_hashes = 1 << 5,
    bf_diffic    = 1 << 6,
    bf_max_outs  = 1 << 7,
    bf_hf_version= 1 << 8
  };

  void get_block_chain(std::vector<block_info>& blockchain, const crypto::hash& head, size_t n) const;
  void get_last_n_block_weights(std::vector<size_t>& block_weights, const crypto::hash& head, size_t n) const;
  uint64_t get_already_generated_coins(const crypto::hash& blk_id) const;
  uint64_t get_already_generated_coins(const cryptonote::block& blk) const;

  void add_block(const cryptonote::block& blk, size_t tsx_size, std::vector<size_t>& block_weights, uint64_t already_generated_coins,
                 uint8_t hf_version = 1);
  bool construct_block(cryptonote::block& blk, uint64_t height, const crypto::hash& prev_id,
                       const cryptonote::account_base& miner_acc, uint64_t timestamp, uint64_t already_generated_coins,
                       std::vector<size_t>& block_weights, const std::list<cryptonote::transaction>& tx_list);
  bool construct_block(cryptonote::block& blk, const cryptonote::account_base& miner_acc, uint64_t timestamp);
  bool construct_block(cryptonote::block& blk, const cryptonote::block& blk_prev, const cryptonote::account_base& miner_acc,
                       const std::list<cryptonote::transaction>& tx_list = std::list<cryptonote::transaction>());

  bool construct_block_manually(cryptonote::block& blk, const cryptonote::block& prev_block,
                                const cryptonote::account_base& miner_acc, int actual_params = bf_none, uint8_t major_ver = 0,
                                uint8_t minor_ver = 0, uint64_t timestamp = 0, const crypto::hash& prev_id = crypto::hash(),
                                const cryptonote::difficulty_type& diffic = 1, const cryptonote::transaction& miner_tx = cryptonote::transaction(),
                                const std::vector<crypto::hash>& tx_hashes = std::vector<crypto::hash>(), size_t txs_sizes = 0, size_t max_outs = 999,
                                uint8_t hf_version = 1);
  bool construct_block_manually_tx(cryptonote::block& blk, const cryptonote::block& prev_block,
                                   const cryptonote::account_base& miner_acc, const std::vector<crypto::hash>& tx_hashes, size_t txs_size);

private:
  std::unordered_map<crypto::hash, block_info> m_blocks_info;
};

inline cryptonote::difficulty_type get_test_difficulty() {return 1;}

void fill_nonce(cryptonote::block& blk, const cryptonote::difficulty_type& diffic, uint64_t height);

#endif //CUTCOIN_CHAINGEN_H
