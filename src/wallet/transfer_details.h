// Copyright (c) 2019-2022, CUT coin
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

#ifndef CUTCOIN_TRANSFER_DETAILS_H
#define CUTCOIN_TRANSFER_DETAILS_H

#include <cryptonote_basic/cryptonote_basic.h>
#include "cryptonote_basic/cryptonote_format_utils.h"
#include <cryptonote_basic/subaddress_index.h>
#include <cryptonote_basic/token.h>
#include <ringct/rctOps.h>
#include <ringct/rctTypes.h>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>

#include <vector>

namespace tools {

struct multisig_info
{
  struct LR
  {
    rct::key m_L;
    rct::key m_R;

    BEGIN_SERIALIZE_OBJECT()
      FIELD(m_L)
      FIELD(m_R)
    END_SERIALIZE()
  };

  crypto::public_key m_signer;
  std::vector<LR> m_LR;
  std::vector<crypto::key_image> m_partial_key_images; // one per key the participant has

  BEGIN_SERIALIZE_OBJECT()
    FIELD(m_signer)
    FIELD(m_LR)
    FIELD(m_partial_key_images)
  END_SERIALIZE()
};

struct transfer_details
{
  uint64_t m_block_height;
  cryptonote::transaction_prefix m_tx;
  crypto::hash m_txid;
  size_t m_internal_output_index;
  uint64_t m_global_output_index;
  bool m_spent;
  uint64_t m_spent_height;
  crypto::key_image m_key_image; //TODO: key_image stored twice :(
  rct::key m_mask;
  uint64_t m_amount;
  cryptonote::TokenId m_token_id;
  bool m_rct;
  bool m_key_image_known;
  size_t m_pk_index;
  cryptonote::subaddress_index m_subaddr_index;
  bool m_key_image_partial;
  std::vector<rct::key> m_multisig_k;
  std::vector<multisig_info> m_multisig_info; // one per other participant

  bool is_rct() const { return m_rct; }
  uint64_t amount() const { return m_amount; }
  const crypto::public_key &get_public_key() const
  {
    return boost::get<const cryptonote::txout_to_key>(m_tx.vout[m_internal_output_index].target).key;
  }

  BEGIN_SERIALIZE_OBJECT()
    FIELD(m_block_height)
    FIELD(m_tx)
    FIELD(m_txid)
    FIELD(m_internal_output_index)
    FIELD(m_global_output_index)
    FIELD(m_spent)
    FIELD(m_spent_height)
    FIELD(m_key_image)
    FIELD(m_mask)
    FIELD(m_amount)
    FIELD(m_token_id)
    FIELD(m_rct)
    FIELD(m_key_image_known)
    FIELD(m_pk_index)
    FIELD(m_subaddr_index)
    FIELD(m_key_image_partial)
    FIELD(m_multisig_k)
    FIELD(m_multisig_info)
  END_SERIALIZE()
};

using transfer_details_v = std::vector<transfer_details>;
  // Vector of transfer details.

} // namespace tools

BOOST_CLASS_VERSION(tools::multisig_info, 1)
BOOST_CLASS_VERSION(tools::transfer_details, 10)

namespace boost {
namespace serialization {

template <typename Archive>
inline
void serialize(Archive &a, tools::multisig_info::LR &x, const boost::serialization::version_type /*ver*/)
{
  a & x.m_L;
  a & x.m_R;
}

template <typename Archive>
inline
void serialize(Archive &a, tools::multisig_info &x, const boost::serialization::version_type /*ver*/)
{
  a & x.m_signer;
  a & x.m_LR;
  a & x.m_partial_key_images;
}

template <typename Archive>
inline
typename std::enable_if<!Archive::is_loading::value, void>::type initialize_transfer_details(
                                                                     Archive                                  &a,
                                                                     tools::transfer_details                  &x,
                                                                     const boost::serialization::version_type  /*ver*/)
{
}
template <typename Archive>
inline
typename std::enable_if<Archive::is_loading::value, void>::type initialize_transfer_details(
                                                                         Archive                                  &a,
                                                                         tools::transfer_details                  &x,
                                                                         const boost::serialization::version_type  ver)
{
  if (ver < 1) {
    x.m_mask = rct::identity();
    x.m_amount = x.m_tx.vout[x.m_internal_output_index].amount;
  }
  if (ver < 2) {
    x.m_spent_height = 0;
  }
  if (ver < 4) {
    x.m_rct = x.m_tx.vout[x.m_internal_output_index].amount == 0;
  }
  if (ver < 6) {
    x.m_key_image_known = true;
  }
  if (ver < 7) {
    x.m_pk_index = 0;
  }
  if (ver < 8) {
    x.m_subaddr_index = {};
  }
  if (ver < 9) {
    x.m_key_image_partial = false;
    x.m_multisig_k.clear();
    x.m_multisig_info.clear();
  }
  if (ver < 10) {
    x.m_token_id = cryptonote::CUTCOIN_ID;
  }
}

template <typename Archive>
inline
void serialize(Archive &a, tools::transfer_details &x, const boost::serialization::version_type ver)
{
  a & x.m_block_height;
  a & x.m_global_output_index;
  a & x.m_internal_output_index;
  if (ver < 3) {
    cryptonote::transaction tx;
    a & tx;
    x.m_tx = (const cryptonote::transaction_prefix&)tx;
    x.m_txid = cryptonote::get_transaction_hash(tx);
  }
  else {
    a & x.m_tx;
  }
  a & x.m_spent;
  a & x.m_key_image;
  if (ver < 1) {
    // ensure mask and amount are set
    initialize_transfer_details(a, x, ver);
    return;
  }
  a & x.m_mask;
  a & x.m_amount;
  if (ver < 2) {
    initialize_transfer_details(a, x, ver);
    return;
  }
  a & x.m_spent_height;
  if (ver < 3) {
    initialize_transfer_details(a, x, ver);
    return;
  }
  a & x.m_txid;
  if (ver < 4) {
    initialize_transfer_details(a, x, ver);
    return;
  }
  a & x.m_rct;
  if (ver < 5) {
    initialize_transfer_details(a, x, ver);
    return;
  }
  if (ver < 6) {
    // v5 did not properly initialize
    uint8_t u;
    a & u;
    x.m_key_image_known = true;
    return;
  }
  a & x.m_key_image_known;
  if (ver < 7) {
    initialize_transfer_details(a, x, ver);
    return;
  }
  a & x.m_pk_index;
  if (ver < 8) {
    initialize_transfer_details(a, x, ver);
    return;
  }
  a & x.m_subaddr_index;
  if (ver < 9) {
    initialize_transfer_details(a, x, ver);
    return;
  }
  a & x.m_multisig_info;
  a & x.m_multisig_k;
  a & x.m_key_image_partial;

  if (ver < 10) {
    x.m_token_id = cryptonote::CUTCOIN_ID;
    return;
  }
  a & x.m_token_id;
}

}  // namespace serialization
}  // namespace boost

#endif //CUTCOIN_TRANSFER_DETAILS_H
