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

#ifndef CUTCOIN_PENDING_TX_H
#define CUTCOIN_PENDING_TX_H

#include <crypto/crypto.h>
#include <cryptonote_basic/token.h>
#include <cryptonote_core/cryptonote_tx_utils.h>
#include "tx_construction_data.h"

#include <boost/serialization/version.hpp>

#include <vector>
#include <unordered_map>

namespace tools {

struct multisig_sig
{
  rct::rctSig                            sigs;
  crypto::public_key                     ignore;
  std::unordered_set<rct::key>           used_L;
  std::unordered_set<crypto::public_key> signing_keys;
  rct::multisig_out                      msout;
};

struct pending_tx
{
  // The convention for destinations is:
  // dests does not include change
  // splitted_dsts (in construction_data) does.

  cryptonote::transaction tx;
  uint64_t dust, fee;
  bool dust_added_to_fee;
  std::unordered_map<cryptonote::TokenId, cryptonote::tx_destination_entry> change_dts;
  std::vector<size_t> selected_transfers;
  std::string key_images;
  crypto::secret_key tx_key;
  std::vector<crypto::secret_key> additional_tx_keys;
  std::vector<cryptonote::tx_destination_entry> dests;
  std::vector<multisig_sig> multisig_sigs;

  tx_construction_data construction_data;

  BEGIN_SERIALIZE_OBJECT()
    FIELD(tx)
    FIELD(dust)
    FIELD(fee)
    FIELD(dust_added_to_fee)
    FIELD(change_dts)
    FIELD(selected_transfers)
    FIELD(key_images)
    FIELD(tx_key)
    FIELD(additional_tx_keys)
    FIELD(dests)
    FIELD(construction_data)
    FIELD(multisig_sigs)
  END_SERIALIZE()
};

using pending_tx_v = std::vector<pending_tx>;
  // Vector of pending transactions.

}  // namespace tools

BOOST_CLASS_VERSION(tools::multisig_sig, 0)
BOOST_CLASS_VERSION(tools::pending_tx, 4)

namespace boost {
namespace serialization {

template <typename Archive>
inline
void serialize(Archive &a, tools::multisig_sig &x, const boost::serialization::version_type /*ver*/)
{
  a & x.sigs;
  a & x.ignore;
  a & x.used_L;
  a & x.signing_keys;
  a & x.msout;
}

template <typename Archive>
inline
void serialize(Archive &a, tools::pending_tx &x, const boost::serialization::version_type ver)
{
  a & x.tx;
  a & x.dust;
  a & x.fee;
  a & x.dust_added_to_fee;
  if (ver < 4) {
    a & x.change_dts[cryptonote::CUTCOIN_ID];
  } else {
    a & x.change_dts;
  }
  if (ver < 2) {
    // load list to vector
    std::list<size_t> selected_transfers;
    a & selected_transfers;
    x.selected_transfers.clear();
    x.selected_transfers.reserve(selected_transfers.size());
    for (size_t t: selected_transfers)
      x.selected_transfers.push_back(t);
  }
  a & x.key_images;
  a & x.tx_key;
  a & x.dests;
  a & x.construction_data;
  if (ver < 1) {
    return;
  }
  a & x.additional_tx_keys;
  if (ver < 2) {
    return;
  }
  a & x.selected_transfers;
  if (ver < 3) {
    return;
  }
  a & x.multisig_sigs;
}

}  // namespace serialization
}  // namespace boost

#endif //CUTCOIN_PENDING_TX_H
