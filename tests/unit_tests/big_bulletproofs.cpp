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
//
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#include "gtest/gtest.h"

#include "cryptonote_basic/amount.h"
#include "string_tools.h"
#include "ringct/rctOps.h"
#include "ringct/rctSigs.h"
#include "ringct/bulletproofs.h"
#include "cryptonote_basic/blobdatatype.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include "device/device.hpp"
#include "misc_log_ex.h"

#include <iterator>
#include <limits>
#include <random>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test machinery

const size_t NUM_REPETITIONS = 3;

using random_bytes_engine = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;
random_bytes_engine rbe;

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<cryptonote::Amount> dist(std::numeric_limits<cryptonote::Amount>::min(),
                                                       std::numeric_limits<cryptonote::Amount>::max());

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(big_bulletproofs, mul8)
{
  // Concerns:
  // 1. Any (random) hash can be used as an argument of 'hashToPoint' to generate a valid EC point.

  rct::key seed;
  for (size_t i = 0; i < NUM_REPETITIONS; ++i) {
    std::generate(std::begin(seed.bytes), std::end(seed.bytes), std::ref(rbe));
    rct::key omega = hashToPoint(seed);

    ASSERT_EQ(rct::scalarmult8(rct::identity()), rct::identity());
    ASSERT_EQ(rct::scalarmult8(omega), rct::scalarmultKey(omega, rct::EIGHT));
    ASSERT_EQ(rct::scalarmultKey(rct::scalarmultKey(omega, rct::INV_EIGHT), rct::EIGHT), omega);
  }
}

TEST(big_bulletproofs, valid_zero_amount)
{
  // Concerns:
  // 1. Any (random) EC point can be used in Big Bulletproof with 0 amount.

  rct::key seed{{}};
  for (size_t i = 0; i < NUM_REPETITIONS; ++i) {
    std::generate(std::begin(seed.bytes), std::end(seed.bytes), std::ref(rbe));
    std::vector<cryptonote::Amount> amounts = { 0 };
    rct::keyV gamma = {rct::skGen()};
    rct::BigBulletproof proof = rct::bulletproof_PROVE({std::make_tuple(amounts, gamma, hashToPoint(seed))});
    ASSERT_TRUE(rct::bulletproof_VERIFY({proof}));
  }
}

TEST(big_bulletproofs, valid_max_amount)
{
  // Concerns:
  // 1. Any (random) EC point can be used in Big Bulletproof with max amount.
  rct::key seed{{}};
  for (size_t i = 0; i < NUM_REPETITIONS; ++i) {
    std::generate(std::begin(seed.bytes), std::end(seed.bytes), std::ref(rbe));
    std::vector<cryptonote::Amount> amounts = {std::numeric_limits<cryptonote::Amount>::max()};
    rct::keyV gamma = {rct::skGen()};
    rct::BigBulletproof proof = rct::bulletproof_PROVE({std::make_tuple(amounts, gamma, hashToPoint(seed))});
    ASSERT_TRUE(rct::bulletproof_VERIFY({proof}));
  }
}

TEST(big_bulletproofs, valid_random_amount)
{
  // Concerns:
  // 1. Any (random) EC point can be used in Big Bulletproof with any random amount.
  rct::key seed{{}};
  for (size_t i = 0; i < NUM_REPETITIONS; ++i) {
    std::generate(std::begin(seed.bytes), std::end(seed.bytes), std::ref(rbe));
    std::vector<cryptonote::Amount> amounts = {dist(mt)};
    rct::keyV gamma = {rct::skGen()};
    rct::BigBulletproof proof = rct::bulletproof_PROVE({std::make_tuple(amounts, gamma, hashToPoint(seed))});
    ASSERT_TRUE(rct::bulletproof_VERIFY({proof}));
  }
}

TEST(big_bulletproofs, valid_multi_random)
{
  // Concerns:
  // 1. Different numbers of the outputs can be used in Big Bulletproof.
  rct::key seed{{}};
  for (size_t i = 0; i < 8; ++i) {
    size_t outputs = 2 + i;
    std::vector<cryptonote::Amount> amounts;
    rct::keyV gamma;
    std::generate(std::begin(seed.bytes), std::end(seed.bytes), std::ref(rbe));
  
    for (size_t j = 0; j < outputs; ++j) {
      amounts.push_back(dist(mt));
      gamma.push_back(rct::skGen());
    }
    rct::BigBulletproof proof = rct::bulletproof_PROVE({std::make_tuple(amounts, gamma, hashToPoint(seed))});
    ASSERT_TRUE(rct::bulletproof_VERIFY({proof}));
  }
}

TEST(big_bulletproofs, multi_splitting)
{
//  // Concerns:
//  // 1.
//  rct::ctkeyV sc, pc;
//  rct::ctkey sctmp, pctmp;
//  std::vector<unsigned int> index;
//  std::vector<uint64_t> inamounts, outamounts;
//  rct::key seed{{}};
//  rct::keyV omega;
//
//  std::tie(sctmp, pctmp) = rct::ctskpkGen(6000);
//  sc.push_back(sctmp);
//  pc.push_back(pctmp);
//  inamounts.push_back(6000);
//  index.push_back(1);
//
//  std::tie(sctmp, pctmp) = rct::ctskpkGen(7000);
//  sc.push_back(sctmp);
//  pc.push_back(pctmp);
//  inamounts.push_back(7000);
//  index.push_back(1);
//
//  const int mixin = 3, max_outputs = 16;
//
//  for (int n_outputs = 1; n_outputs <= max_outputs; ++n_outputs) {
//    std::vector<uint64_t> outamounts;
//    rct::keyV amount_keys;
//    rct::keyV destinations;
//    rct::key Sk, Pk;
//    uint64_t available = 6000 + 7000;
//    uint64_t amount;
//    rct::ctkeyM mixRing(sc.size());
//
//    //add output
//    for (size_t i = 0; i < n_outputs; ++i) {
//      amount = rct::randXmrAmount(available);
//      outamounts.push_back(amount);
//      amount_keys.push_back(rct::hash_to_scalar(rct::zero()));
//      rct::skpkGen(Sk, Pk);
//      destinations.push_back(Pk);
//      available -= amount;
//
//      std::generate(std::begin(seed.bytes), std::end(seed.bytes), std::ref(rbe));
//      omega.push_back(hashToPoint(seed));
//    }
//
//    for (size_t i = 0; i < sc.size(); ++i) {
//      for (size_t j = 0; j <= mixin; ++j) {
//        if (j == 1) {
//          mixRing[i].push_back(pc[i]);
//        } else {
//          mixRing[i].push_back({rct::scalarmultBase(rct::skGen()), rct::scalarmultBase(rct::skGen())});
//        }
//      }
//    }
//
//    rct::ctkeyV outSk;
//    rct::rctSig s = rct::genRctSimple(
//      [] (const rct::key &a) -> rct::key {return rct::zero();},
//      sc, destinations, inamounts, outamounts, available, mixRing, amount_keys, NULL, NULL,
//      index, outSk, rct::RangeProofPaddedBulletproof, hw::get_device("default"), omega);
//    ASSERT_TRUE(rct::verRctSimple(s, omega));
//    for (size_t i = 0; i < n_outputs; ++i) {
//      rct::key mask;
//      rct::decodeRctSimple(s, amount_keys[i], i, mask, hw::get_device("default"), omega);
//      ASSERT_TRUE(mask == outSk[i].mask);
//    }
//  }
}

TEST(big_bulletproofs, valid_aggregated)
{
  // Concerns:
  // 1. Multiple tokens with different number of outputs can be used in Big Bulletproof.
  
  const size_t N_TOKENS = 2;
  rct::key seed{{}};
  std::vector<rct::bulletproof_input_raw> bulletproofs;
  
  for (size_t i = 0; i < N_TOKENS; ++i) {
    size_t outputs = 2 + i;
    std::vector<uint64_t> amounts;
    rct::keyV gamma;
    for (size_t j = 0; j < outputs; ++j) {
      amounts.emplace_back(dist(mt));
      gamma.emplace_back(rct::skGen());
    }
    
    std::generate(std::begin(seed.bytes), std::end(seed.bytes), std::ref(rbe));
    bulletproofs.emplace_back(std::make_tuple(amounts, gamma, hashToPoint(seed)));
  }
  
  rct::BigBulletproof proof = bulletproof_PROVE(bulletproofs);
  ASSERT_TRUE(rct::bulletproof_VERIFY({proof}));
}
