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


#include "cryptonote_basic/amount.h"
#include "cryptonote_basic/account.h"
#include "cryptonote_basic/token.h"
#include "ringct/rctOps.h"

#include "gtest/gtest.h"

using namespace cryptonote;

namespace {

struct Commitment {
  crypto::ec_scalar vk;
  crypto::ec_scalar sk;
  crypto::ec_scalar h;
};

}  // anonymous namespace

TEST(token, sign_ownership)
{
  cryptonote::Amount a = 10000;

  cryptonote::account_base miner_account;
  miner_account.generate();

  crypto::secret_key svk = miner_account.get_keys().m_view_secret_key;
  crypto::secret_key ssk = miner_account.get_keys().m_spend_secret_key;
  crypto::secret_key hash = rct::rct2sk(rct::hash_to_scalar(hash_to_scalar(rct::tokenIdToPoint(CUTCOIN_ID + 1))));

  Commitment c{};
  c.vk = svk;
  c.sk = ssk;
  c.h = hash;

  crypto::secret_key t{};
  crypto::public_key T{};
  crypto::hash_to_scalar(&c, sizeof(Commitment), t);
  crypto::secret_key_to_public_key(t, T);

  rct::key value = rct::smearBits(a);
  crypto::hash msg_hash{};
  crypto::cn_fast_hash(&value, sizeof(crypto::hash), msg_hash);

  crypto::signature s{};
  crypto::generate_signature(msg_hash, T, t, s);

  bool res = crypto::check_signature(msg_hash, T, s);

  ASSERT_TRUE(res);
}

