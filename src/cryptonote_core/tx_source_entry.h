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

#ifndef CUTCOIN_TX_SOURCE_ENTRY_H
#define CUTCOIN_TX_SOURCE_ENTRY_H

#include <cryptonote_basic/amount.h>
#include <cryptonote_basic/cryptonote_basic.h>
#include <cryptonote_basic/dex.h>
#include <ringct/rctOps.h>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>

#include <utility>

namespace cryptonote {

struct tx_source_entry {
  using output_entry = std::pair <uint64_t, rct::ctkey>;

  std::vector <output_entry> outputs;                            // index + key + optional ringct commitment
  size_t                     real_output;                        // index in outputs vector of real output_entry
  crypto::public_key         real_out_tx_key;                    // incoming real tx public key
  std::vector <crypto::public_key> real_out_additional_tx_keys;  // incoming real tx additional public keys
  size_t                     real_output_in_tx_index;            // index in transaction outputs vector
  Amount                     amount;                             // money
  SourceType                 o_type{SourceType::wallet};          // specify output origin: lp account or common
  bool                       rct;                                // true if the output is rct
  rct::key                   mask;                               // ringct amount mask
  rct::multisig_kLRki        multisig_kLRki;                     // multisig info
  TokenId                    token_id;                           // tx source token ID

  void push_output(uint64_t idx, const crypto::public_key &k, Amount a) {
    outputs.emplace_back(std::make_pair(idx, rct::ctkey({rct::pk2rct(k), rct::zeroCommit(a)})));
  }

  void push_output(uint64_t idx, const crypto::public_key &k, Amount a, TokenId tid) {
    outputs.emplace_back(
      std::make_pair(idx, rct::ctkey({rct::pk2rct(k), rct::gp_zeroCommit(a, rct::tokenIdToPoint(tid))})));
  }

  BEGIN_SERIALIZE_OBJECT()

  FIELD(outputs)
  FIELD(real_output)
  FIELD(real_out_tx_key)
  FIELD(real_out_additional_tx_keys)
  FIELD(real_output_in_tx_index)
  FIELD(amount)
  FIELD(rct)
  FIELD(mask)
  FIELD(multisig_kLRki)

  if (real_output >= outputs.

  size()

  )
  return false;

  END_SERIALIZE()
};

// Types
using TxSources = std::vector<tx_source_entry>;

}  // namespace cryptonote

BOOST_CLASS_VERSION(cryptonote::tx_source_entry, 1)

namespace boost {

namespace serialization {

template <class Archive>
inline void serialize(Archive &a, cryptonote::tx_source_entry &x, const boost::serialization::version_type ver)
{
  a & x.outputs;
  a & x.real_output;
  a & x.real_out_tx_key;
  a & x.real_output_in_tx_index;
  a & x.amount;
  a & x.rct;
  a & x.mask;
  if (ver < 1)
    return;
  a & x.multisig_kLRki;
  a & x.real_out_additional_tx_keys;
}

}  // namespace serialization

}  // namespace boost

#endif //CUTCOIN_TX_SOURCE_ENTRY_H
