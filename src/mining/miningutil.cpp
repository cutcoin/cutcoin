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

#include "miningutil.h"
#include "atan.h"

#include <common/uint128.hpp>
#include <crypto/crypto.h>
#include <crypto/hash.h>
#include <cryptonote_basic/difficulty.h>
#include <cryptonote_config.h>

#include <limits>
#include <vector>

namespace mining {

void find_pos_hash(const crypto::key_image &key_image, const crypto::hash &hash, crypto::hash &result)
{
  const size_t size = crypto::HASH_SIZE << 1;
  char bytes[size];

  size_t index = 0;
  for (const auto b: key_image.data) {
    bytes[index] = b;
    ++index;
  }
  for (const auto b: hash.data) {
    bytes[index] = b;
    ++index;
  }

  crypto::cn_fast_hash(bytes, size, result);
}

bool check_pos_hash(const crypto::hash &pos_hash,
                    uint64_t            amount,
                    uint64_t difficulty,
                    uint64_t expected_time_delta)
{
  uint64_t time_delta;
  get_new_block_time_delta(pos_hash, amount, difficulty, time_delta);

  uint64_t time_delta_in_s = std::max<uint64_t>(time_delta / 1000, 1);
  return expected_time_delta == time_delta_in_s;
}

uint64_t denominate_amount(uint64_t amount)
{
  return amount / COIN;
}

void get_new_block_time_delta(const crypto::hash &pos_hash,
                              uint64_t            amount,
                              uint64_t            difficulty,
                              uint64_t           &time_delta)
{
  uint32_t lower_bound = 0;
  uint32_t upper_bound = std::numeric_limits<uint32_t>::max();

  num::u128_t target = target_from_hash(pos_hash);
  uint64_t base_target = base_target_from_difficulty(difficulty);

  uint64_t da = denominate_amount(amount);

  uint32_t block_mining_time = mining::binary_search(
      lower_bound,
      upper_bound,
      target,
      std::bind(mining::target_function, std::placeholders::_1, da, base_target));

  if(block_mining_time == 0) {
    block_mining_time = 1;
  }

  time_delta =  static_cast<uint64_t>(block_mining_time);
}

void get_difficulty(const std::vector<cryptonote::difficulty_type> &difficulties,
                    const std::vector<uint64_t> &t_deltas,
                    cryptonote::difficulty_type &difficulty)
{
  assert(difficulties.size() == t_deltas.size());

  num::u128_t diff_counter = 0;
  num::u128_t time_counter = 0;

  size_t num_elements = difficulties.size();

  for (size_t i = 0; i < num_elements; ++i) {
    diff_counter = diff_counter + difficulties[i] * (i + 1);
    time_counter = time_counter + t_deltas[i] * (i + 1);
  }

  if (time_counter == 0) {
    time_counter = 1;
  }

  num::u128_t new_difficulty = (t_block_generation_target * diff_counter / time_counter);

  if (new_difficulty.val[1]) {
    throw std::runtime_error{"Overflow detected in the difficulty evaluation algorithm"};
  }

  difficulty = new_difficulty.val[0];
}

uint64_t target_from_hash(const crypto::hash &hash)
{
  return *(uint64_t *)(hash.data + sizeof(hash.data) - sizeof(uint64_t));
}

uint64_t base_target_from_difficulty(uint64_t difficulty)
{
  difficulty=std::max<uint64_t>(difficulty, 1);
  return std::max<uint64_t>(294352916919967ULL/difficulty, 1);
}

uint32_t binary_search(const uint32_t &t1,
                       const uint32_t &t2,
                       const num::u128_t &target,
                       std::function<num::u128_t(uint32_t)> f)
{
  uint32_t mid, l_bound = t1, u_bound = t2;

  if (l_bound > u_bound) {
    throw std::runtime_error{"Lower bound bigger than upper bound"};
  }

  while (u_bound - l_bound > 1) {
    mid = l_bound + ((u_bound - l_bound) >> 1);
    f(mid) > target ? u_bound = mid: l_bound = mid;
  }

  return f(l_bound) <= target ? l_bound: u_bound;
}

num::u128_t target_function(uint32_t t, uint64_t amount, uint64_t base_target){
  // we may have overflow here, so must catch possible exception outside
  num::u128_t r1 = num::u128_t(amount) * base_target * t;
  int64_t shifted_t = t - t_block_generation_target;
  int64_t r2 = mining::sign(shifted_t) * mining::atan2c(std::abs(shifted_t), t_scaling_factor << 3);
  return r1 * ((r_scaling_factor >> 1) + r2) >> r_scaling_shift;
}

} // namespace mining
