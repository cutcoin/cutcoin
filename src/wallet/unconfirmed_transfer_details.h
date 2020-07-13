// Copyright (c) 2020, CUT coin
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

#ifndef CUTCOIN_UNCONFIRMED_TRANSFER_DETAILS_H
#define CUTCOIN_UNCONFIRMED_TRANSFER_DETAILS_H

#include <cryptonote_core/cryptonote_tx_utils.h>

#include <vector>

namespace tools
{
    struct unconfirmed_transfer_details
    {
      cryptonote::transaction_prefix m_tx;
      uint64_t m_amount_in;
      uint64_t m_amount_out;
      uint64_t m_change;
      uint64_t m_fee;
      cryptonote::TokenId m_token_id;
      time_t m_sent_time;
      std::vector<cryptonote::tx_destination_entry> m_dests;
      crypto::hash m_payment_id;
      enum { pending, pending_not_in_pool, failed } m_state;
      uint64_t m_timestamp;
      uint32_t m_subaddr_account;   // subaddress account of your wallet to be used in this transfer
      std::set<uint32_t> m_subaddr_indices;  // set of address indices used as inputs in this transfer
      std::vector<std::pair<crypto::key_image, std::vector<uint64_t>>> m_rings; // relative
    };
} // namespace tools

namespace boost {
namespace serialization {

template<class Archive>
inline
void serialize(Archive &a, tools::unconfirmed_transfer_details &x, const boost::serialization::version_type ver) {
  a & x.m_change;
  a & x.m_sent_time;
  if (ver < 5) {
    cryptonote::transaction tx;
    a & tx;
    x.m_tx = (const cryptonote::transaction_prefix &) tx;
  } else {
    a & x.m_tx;
  }
  if (ver < 1)
    return;
  a & x.m_dests;
  a & x.m_payment_id;
  if (ver < 2)
    return;
  a & x.m_state;
  if (ver < 3)
    return;
  a & x.m_timestamp;
  if (ver < 4)
    return;
  a & x.m_amount_in;
  a & x.m_amount_out;
  if (ver < 6) {
    // v<6 may not have change accumulated in m_amount_out, which is a pain,
    // as it's readily understood to be sum of outputs.
    // We convert it to include change from v6
    if (!typename Archive::is_saving() && x.m_change != (uint64_t) -1)
      x.m_amount_out += x.m_change;
  }
  if (ver < 7) {
    x.m_subaddr_account = 0;
    return;
  }
  a & x.m_subaddr_account;
  a & x.m_subaddr_indices;
  if (ver < 8) {
    x.m_token_id = cryptonote::CUTCOIN_ID;
    x.m_fee = 0;
    return;
  }
  a & x.m_rings;
  a & x.m_token_id;
  a & x.m_fee;
}

}  // namespace serialization
}  // namespace boost


#endif //CUTCOIN_UNCONFIRMED_TRANSFER_DETAILS_H
