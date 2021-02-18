// Copyright (c) 2020-2021, CUT coin
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

#include "string_tools.h"
#include "ringct/rctOps.h"
#include "ringct/rctSigs.h"
#include "ringct/bulletproofs.h"
#include "cryptonote_basic/blobdatatype.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include "device/device.hpp"
#include "misc_log_ex.h"

#include <iterator>
#include <random>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test machinery

const size_t NUM_REPETITIONS = 100;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//TEST(bulletproofs_multipoint, mul8)
//{
//  // Concerns:
//  // 1. Any (random) hash can be used as an argument of 'hashToPoint' to generate valid EC point.
//
//  using random_bytes_engine = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;
//  random_bytes_engine rbe;
//
//  rct::key seed;
//  for (size_t i = 0; i < NUM_REPETITIONS; ++i) {
//    std::generate(std::begin(seed.bytes), std::end(seed.bytes), std::ref(rbe));
//    rct::key omega = hashToPoint(seed);
//
//    ASSERT_EQ(rct::scalarmult8(rct::identity()), rct::identity());
//    ASSERT_EQ(rct::scalarmult8(omega), rct::scalarmultKey(omega, rct::EIGHT));
//    ASSERT_EQ(rct::scalarmultKey(rct::scalarmultKey(omega, rct::INV_EIGHT), rct::EIGHT), omega);
//  }
//}
//
//TEST(bulletproofs_multipoint, valid_zero_random_ec_point)
//{
//  using random_bytes_engine = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;
//  random_bytes_engine rbe;
//
//  rct::key seed;
//  for (size_t i = 0; i < NUM_REPETITIONS; ++i) {
//    std::generate(std::begin(seed.bytes), std::end(seed.bytes), std::ref(rbe));
//    rct::key omega = hashToPoint(seed);
//
//    rct::Bulletproof proof = bulletproof_PROVE(0, rct::skGen(), omega);
//    ASSERT_TRUE(rct::bulletproof_VERIFY(proof, omega));
//  }
//}

TEST(bulletproofs_multipoint, valid_aggregated)
{
  static const size_t N_PROOFS = 8;
  
  using random_bytes_engine = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;
  random_bytes_engine rbe;
  
  rct::key seed{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  rct::key probe = hashToPoint(seed);
  
  std::vector<rct::Bulletproof> proofs(N_PROOFS);
  for (size_t n = 0; n < N_PROOFS; ++n)
  {
    size_t outputs = 2 + n;
    std::vector<uint64_t> amounts;
    rct::keyV gamma;
    for (size_t i = 0; i < outputs; ++i)
    {
      amounts.push_back(crypto::rand<uint64_t>());
      gamma.push_back(rct::skGen());
    }
    proofs[n] = bulletproof_PROVE(amounts, gamma, rct::H, probe);
  }
  ASSERT_TRUE(rct::bulletproof_VERIFY(proofs, rct::H));
}

//TEST(bulletproofs_multipoint, valid_zero_random_ec_point_2)
//{
//  using random_bytes_engine = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;
//  random_bytes_engine rbe;
//
//  rct::key seed;
//
//  std::generate(std::begin(seed.bytes), std::end(seed.bytes), std::ref(rbe));
//  rct::key probe = hashToPoint(seed);
//
//
//  for (size_t i = 0; i < NUM_REPETITIONS; ++i) {
//    std::generate(std::begin(seed.bytes), std::end(seed.bytes), std::ref(rbe));
//    rct::key omega = hashToPoint(seed);
//
//    rct::Bulletproof proof = bulletproof_PROVE(0, rct::skGen(), omega, probe);
//    ASSERT_TRUE(rct::bulletproof_VERIFY(proof, omega));
//  }
//}

