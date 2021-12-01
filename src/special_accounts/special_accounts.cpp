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

#include "special_accounts.h"

#include "cryptonote_basic/amount.h"
#include "cryptonote_basic/cryptonote_basic_impl.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include <ringct/rctSigs.h>
#include "ringct/rctTypes.h"
#include "string_tools.h"

#include <array>
#include <map>

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

network_type LpAccount::d_network_type;

void LpAccount::set_default_network_type(network_type type)
{
  d_network_type = type;
}

LpAccount::LpAccount()
{
  const std::map<network_type, std::array<std::string, d_num_networks> > addresses_hex = {
    {network_type::MAINNET, {
                              "cutb3SwM3TJDkCyEkHwSFkFZT5HmXMLA1PpAASjA2RzX2MvdWnjrGXe3wAk6nUdQ5eifLSTCy1djCWs6pK9TWyjEAKoVB8rNbm",
                              "c13a0f9885c9719be71f96edc38f8efa94c8e5f7ff79ed5d2944789908e26f0f",
                              "4ccf62b1b338d46b9d978bb543c157852ac0a1414bb5eb99859cc463f68e5209"}},
    {network_type::TESTNET, {
                              "TCU1jKtx4DdcuyCRjHjav5R79M9Z5CXQiMFVd6mb4SiQFi8FBU8fiNVZe8i5Au9ebSM3WXcWwrGtwLPnMxVaLBwV6yt5CdNJNL",
                              "d8e4add695321e3937fba7238bb8f44bde9d1fb315cd228b08e90d271fb4a40e",
                              "2cc077ba748065d79d8b853c7b204ae76c891c2bebff9aed4a604a26e1454d03"}},
    {network_type::STAGENET, {
                              "ctsFTPUiAFTTstpMxaexxtNqR9ob5BdQkiHwn13FZVQ71WB9HmJr5jfKiEwMbAwVVgJrF73PeDjz2dGBW5pAC4jc1oX1m1ynf4",
                              "9c9749e53ba6735c56146e0ac557a68d9561a5c66e36776392a2039fa02fb201",
                              "d75314583049cfaaeb149c559eaafbf2508f5870d9005fce962dc1620f58cc03"}}
  };

  assert(addresses_hex.size() == d_num_networks);

  for (const auto &a_info: addresses_hex) {
    address_parse_info info{};
    if (!cryptonote::get_account_address_from_str_or_url(info, a_info.first, a_info.second[0])) {
      assert(false);
    }

    const cryptonote::account_public_address address = info.address;

    crypto::secret_key viewkey;
    epee::string_tools::hex_to_pod(a_info.second[1], viewkey);

    crypto::secret_key spendkey;
    epee::string_tools::hex_to_pod(a_info.second[2], spendkey);

    d_accounts[a_info.first].create_from_keys(address, spendkey, viewkey);
  }
}

const account_base &LpAccount::get()
{
  return get(d_network_type);
}

const account_base &LpAccount::get(network_type type)
{
  static LpAccount lpa;

  assert (type <= network_type::STAGENET);
  return lpa.d_accounts[type];
}

bool LpAccount::check_destination_output(const tx_out             &out,
                                         const crypto::public_key &pub_key,
                                         const rct::rctSig        &rct_signatures,
                                         const rct::keyV          &omega_v,
                                         size_t                    output_index,
                                         Amount                   &amount)
{
  crypto::key_derivation derivation;
  bool res = generate_key_derivation(pub_key, get(d_network_type).get_keys().m_view_secret_key, derivation);
  assert(res);

  crypto::secret_key scalar1;
  crypto::derivation_to_scalar(derivation, output_index, scalar1);

  return rct::decode_to_check(rct_signatures, rct::sk2rct(scalar1), omega_v, output_index, amount, hw::get_device("default"));
}

bool LpAccount::check_destination_tx(const transaction &tx, Amount amount, TokenId token_id) {
  Amount   sum{0};
  rct::key mask{};
  uint64_t output_index{0};

  crypto::public_key pub_key = get_tx_pub_key_from_extra(tx);

  crypto::key_derivation derivation;
  bool res = generate_key_derivation(pub_key, get(d_network_type).get_keys().m_view_secret_key, derivation);
  assert(res);

  rct::keyV omega_v;
  omega_v.reserve(tx.vout.size());
  for (const auto &o: tx.vout) {
    omega_v.push_back(rct::tokenIdToPoint(o.token_id));
  }

  for (const auto &o: tx.vout) {
    if (o.token_id == token_id && o.target.type() == typeid(txout_to_key)) {
      crypto::secret_key scalar1;
      crypto::derivation_to_scalar(derivation, output_index, scalar1);

      uint64_t rct_amount = rct::decodeRctSimple(tx.rct_signatures,
                                                 rct::sk2rct(scalar1),
                                                 omega_v,
                                                 output_index,
                                                 mask,
                                                 hw::get_device("default"));
      sum += rct_amount;
    }
    ++output_index;
  }
  return sum >= amount;
}

}  // namespace cryptonote