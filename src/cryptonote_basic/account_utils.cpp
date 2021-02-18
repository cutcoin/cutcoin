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

#include "account_utils.h"

namespace cryptonote {

void get_coin_burn_address(account_public_address &address, crypto::secret_key &view_secret_key)
{
  crypto::hash seed{coin_burn_seed};
  sc_reduce32(reinterpret_cast<unsigned char*>(seed.data));
  rct::key seed_key = rct::hash2rct(seed);
  view_secret_key = rct::rct2sk(seed_key);
  crypto::secret_key_to_public_key(view_secret_key, address.m_view_public_key);

  crypto::hash h{};
  crypto::cn_fast_hash(seed.data, sizeof(seed.data), h);
  sc_reduce32(reinterpret_cast<unsigned char*>(h.data));
  seed_key = rct::hash2rct(h);
  address.m_spend_public_key = rct::rct2pk(seed_key);
}

account_base get_coin_burn_account()
{
  account_base           cb_acc;
  account_public_address address{};
  crypto::secret_key     view_secret_key;
  get_coin_burn_address(address, view_secret_key);
  cb_acc.create_from_viewkey(address, view_secret_key);
  return cb_acc;
}

}  // namespace cryptonote