// Copyright (c) 2018-2022, CUT coin
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

#ifndef CUTCOIN_MININGUTIL_H
#define CUTCOIN_MININGUTIL_H

#include <common/uint128.hpp>
#include <crypto/crypto.h>
#include <crypto/hash.h>
#include <crypto/hash-ops.h>
#include <cryptonote_basic/difficulty.h>
#include <cryptonote_config.h>

#include <chrono>

namespace mining {

const uint64_t t_scaling_factor          = 1000;    // time scaling. Current scale: [millisecond]
const uint64_t t_block_generation_target = DIFFICULTY_TARGET_V2 * t_scaling_factor;
const uint32_t block_building_time       = 100;     // time that required for 1 block building [milliseconds]
const size_t moving_average_window       = 90;

struct StakeDetails
{
  crypto::hash                          d_pos_hash;
  uint64_t                              d_amount;
  uint64_t                              d_global_index;
  std::chrono::system_clock::time_point d_new_block_timestamp;
  cryptonote::difficulty_type           d_difficulty;
};

void find_pos_hash(const crypto::key_image &key_image, const crypto::hash &hash, crypto::hash &result);

bool check_pos_hash(const crypto::hash &pos_hash, uint64_t amount, uint64_t difficulty, uint64_t expected_time_delta);

uint64_t denominate_amount(uint64_t amount);

void get_new_block_time_delta(const crypto::hash &pos_hash,
                              uint64_t            amount,
                              uint64_t            difficulty,
                              uint64_t           &time_delta);

void get_difficulty(const std::vector<cryptonote::difficulty_type> &difficulties,
                    const std::vector<uint64_t> &t_deltas,
                    cryptonote::difficulty_type &difficulty);

uint64_t target_from_hash(const crypto::hash& hash);

uint64_t base_target_from_difficulty(uint64_t difficulty);

uint32_t binary_search(const uint32_t &t1,
                       const uint32_t &t2,
                       const num::u128_t &target,
                       std::function<num::u128_t(uint32_t)> f);

num::u128_t target_function(uint32_t t, uint64_t amount, uint64_t base_target);

} // namespace mining

#endif //CUTCOIN_MININGUTIL_H
