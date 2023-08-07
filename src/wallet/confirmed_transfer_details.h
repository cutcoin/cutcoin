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

#ifndef CUTCOIN_CONFIRMED_TRANSFER_DETAILS_H
#define CUTCOIN_CONFIRMED_TRANSFER_DETAILS_H

#include "payment_details.h"

#include <boost/serialization/version.hpp>

#include <unordered_map>
#include <vector>

namespace tools {

struct confirmed_transfer_details
{
  cryptonote::TokenAmounts m_amounts_in;
  cryptonote::TokenAmounts m_amounts_out;
  cryptonote::TokenAmounts m_change;
  uint64_t m_block_height;
  std::vector<cryptonote::tx_destination_entry> m_dests;
  crypto::hash m_payment_id;
  uint64_t m_timestamp;
  uint64_t m_unlock_time;
  uint32_t m_subaddr_account;   // subaddress account of your wallet to be used in this transfer
  std::set<uint32_t> m_subaddr_indices;  // set of address indices used as inputs in this transfer
  std::vector<std::pair<crypto::key_image, std::vector<uint64_t>>> m_rings; // relative
  uint64_t m_fee;


  confirmed_transfer_details()
    : m_amounts_in{}
    , m_amounts_out{}
    , m_change{{0, (uint64_t)-1}}
    , m_block_height{0}
    , m_payment_id{crypto::null_hash}
    , m_timestamp{0}
    , m_unlock_time{0}
    , m_subaddr_account{(uint32_t)-1}
    , m_fee{0}
  {}

  confirmed_transfer_details(const unconfirmed_transfer_details &utd, uint64_t height)
    : m_amounts_in(utd.m_amounts_in)
    , m_amounts_out(utd.m_amounts_out)
    , m_change(utd.m_change)
    , m_block_height(height)
    , m_dests(utd.m_dests)
    , m_payment_id(utd.m_payment_id)
    , m_timestamp(utd.m_timestamp)
    , m_unlock_time(utd.m_tx.unlock_time)
    , m_subaddr_account(utd.m_subaddr_account)
    , m_subaddr_indices(utd.m_subaddr_indices)
    , m_rings(utd.m_rings)
    , m_fee(utd.m_fee)
  {}
};

using payment_container = std::unordered_multimap<crypto::hash, payment_details>;

}  // namespace tools

BOOST_CLASS_VERSION(tools::confirmed_transfer_details, 7)

namespace boost {
namespace serialization {

template <typename Archive>
inline
void serialize(Archive &a, tools::confirmed_transfer_details &x, const boost::serialization::version_type ver)
{
  if (ver < 7) {
    a & x.m_amounts_in[cryptonote::CUTCOIN_ID];
    a & x.m_amounts_out[cryptonote::CUTCOIN_ID];
    a & x.m_change[cryptonote::CUTCOIN_ID];
  }
  else {
    a & x.m_amounts_in;
    a & x.m_amounts_out;
    a & x.m_change;
  }
  a & x.m_block_height;
  if (ver < 1)
    return;
  a & x.m_dests;
  a & x.m_payment_id;
  if (ver < 2)
    return;
  a & x.m_timestamp;
  if (ver < 3) {
    // v<3 may not have change accumulated in m_amount_out, which is a pain,
    // as it's readily understood to be sum of outputs. Whether it got added
    // or not depends on whether it came from a unconfirmed_transfer_details
    // (not included) or not (included). We can't reliably tell here, so we
    // check whether either yields a "negative" fee, or use the other if so.
    // We convert it to include change from v3
    if (!typename Archive::is_saving() && x.m_change[cryptonote::CUTCOIN_ID] != (uint64_t)-1) {
      if (x.m_amounts_in[cryptonote::CUTCOIN_ID] > (x.m_amounts_out[cryptonote::CUTCOIN_ID] + x.m_change[cryptonote::CUTCOIN_ID])) {
        x.m_amounts_out[cryptonote::CUTCOIN_ID] += x.m_change[cryptonote::CUTCOIN_ID];
      }
    }
  }
  if (ver < 4)
  {
    if (!typename Archive::is_saving())
      x.m_unlock_time = 0;
    return;
  }
  a & x.m_unlock_time;
  if (ver < 5)
  {
    x.m_subaddr_account = 0;
    return;
  }
  a & x.m_subaddr_account;
  a & x.m_subaddr_indices;
  a & x.m_rings;

  if (ver < 7) {
    x.m_fee = x.m_amounts_in[cryptonote::CUTCOIN_ID] - x.m_amounts_out[cryptonote::CUTCOIN_ID];
    return;
  }
  a & x.m_fee;
}

}  // namespace serialization
}  // namespace boost

#endif //CUTCOIN_CONFIRMED_TRANSFER_DETAILS_H
