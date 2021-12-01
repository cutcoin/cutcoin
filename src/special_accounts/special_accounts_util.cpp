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

#include "special_accounts_util.h"

#include "cryptonote_basic/cryptonote_format_utils.h"
#include "ringct/rctOps.h"
#include "ringct/rctSigs.h"
#include "special_accounts.h"

namespace cryptonote {

bool validate_amount_burnt(const transaction &tx, const TokenId &token_id, const Amount &burnt_amount)
{
  account_base cba = get_coin_burn_account();
  crypto::public_key pub_key = get_tx_pub_key_from_extra(tx);

  crypto::key_derivation derivation;
  if (!generate_key_derivation(pub_key, cba.get_keys().m_view_secret_key, derivation)) {
    return false;
  }

  const cryptonote::account_public_address &pub_keys = cba.get_keys().m_account_address;

  Amount             burnt_sum{0};
  uint64_t           output_index{0};
  crypto::public_key tx_pubkey;

  rct::keyV omega_v;
  omega_v.reserve(tx.vout.size());
  for (const auto &o: tx.vout) {
    omega_v.push_back(rct::tokenIdToPoint(o.token_id));
  }

  for (const auto &o: tx.vout) {
    derive_public_key(derivation, output_index, pub_keys.m_spend_public_key, tx_pubkey);
    if (o.token_id == token_id && o.target.type() == typeid(txout_to_key)) {
      const txout_to_key &out_to_key = boost::get<txout_to_key>(o.target);
      crypto::secret_key scalar1;
      crypto::derivation_to_scalar(derivation, output_index, scalar1);

      Amount amount;
      if (rct::decode_to_check(tx.rct_signatures,
                               rct::sk2rct(scalar1),
                               omega_v,
                               output_index,
                               amount,
                               hw::get_device("default"))) {
        burnt_sum += amount;
      }

      ++output_index;
    }
  }

  return (burnt_sum >= burnt_amount);
}

bool check_transfer_to_liquidity_pool(const transaction &tx, const TokenId &token_id, const Amount &amount)
{
  return (amount == get_transfer_to_liquidity_pool(tx, token_id));
}

Amount get_transfer_to_liquidity_pool(const transaction &tx, const TokenId &token_id)
{
  crypto::public_key pub_key = get_tx_pub_key_from_extra(tx);

  rct::keyV omega_v;
  omega_v.reserve(tx.vout.size());
  for (const auto &o: tx.vout) {
    omega_v.push_back(rct::tokenIdToPoint(o.token_id));
  }

  Amount sum = 0;
  for (std::size_t i = 0; i < tx.vout.size(); ++i) {
    if (tx.vout[i].token_id == token_id) {
      Amount a;
      if (LpAccount::check_destination_output(tx.vout[i], pub_key, tx.rct_signatures, omega_v, i, a)) {
        sum += a;
      }
    }
  }
  return sum;
}

}  // namespace cryptonote