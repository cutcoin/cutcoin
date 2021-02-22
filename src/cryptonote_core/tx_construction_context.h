// Copyright (c) 2021, CUT coin
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

#ifndef CUTCOIN_TX_CONSTRUCTION_CONTEXT_H
#define CUTCOIN_TX_CONSTRUCTION_CONTEXT_H

#include "cryptonote_basic/account.h"
#include "cryptonote_basic/subaddress_index.h"
#include "tx_destination_entry.h"
#include "tx_source_entry.h"

#include <unordered_map>
#include <vector>

namespace cryptonote
{

struct TxConstructionContext {
  account_keys                                              d_sender_account_keys;
  std::unordered_map<crypto::public_key, subaddress_index>  d_subaddresses;
  TxSources                                                 d_sources;
  std::vector<tx_destination_entry>                         d_destinations;
  boost::optional<account_public_address>                   d_change_addr;
  std::vector<uint8_t>                                      d_extra;
  uint64_t                                                  d_unlock_time;
  crypto::secret_key                                        d_tx_key;
  std::vector<crypto::secret_key>                           d_additional_tx_keys;
  TxVersion                                                 d_tx_version;
  rct::RangeProofType                                       d_range_proof_type;
  rct::multisig_out                                        *d_msout;
  bool                                                      d_shuffle_outs;
  bool                                                      d_tgtx;
  bool                                                      d_hidden_supply;

public:
  TxConstructionContext();
    // Create this object.
};

}  // namespace cryptonote

#endif //CUTCOIN_TX_CONSTRUCTION_CONTEXT_H
