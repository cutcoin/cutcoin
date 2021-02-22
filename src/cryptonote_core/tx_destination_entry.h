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

#ifndef CUTCOIN_TX_DESTINATION_ENTRY_H
#define CUTCOIN_TX_DESTINATION_ENTRY_H

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>

namespace cryptonote {

struct tx_destination_entry {
  TokenId                token_id;      // token id
  uint64_t               amount;        // money
  account_public_address addr;          // destination address
  bool                   is_subaddress; // show if the destination is subaddress

  tx_destination_entry()
  : amount(0)
  , addr(AUTO_VAL_INIT(addr))
  , is_subaddress(false),
  token_id(CUTCOIN_ID) {}
    // Create this object. Default constructor.

  tx_destination_entry(TokenId id, uint64_t a, const account_public_address &ad, bool is_subaddress)
  : amount(a)
  , addr(ad)
  , is_subaddress(is_subaddress)
  , token_id(id) {}
    // Create this object.

  BEGIN_SERIALIZE_OBJECT()
    VARINT_FIELD(amount)
    FIELD(addr)
    FIELD(is_subaddress)
    VARINT_FIELD(token_id)
  END_SERIALIZE()
};

}  // namespace cryptonote

BOOST_CLASS_VERSION(cryptonote::tx_destination_entry, 2)

namespace boost {

namespace serialization {

template <class Archive>
inline void serialize(Archive& a, cryptonote::tx_destination_entry& x, const boost::serialization::version_type ver)
{
  a & x.amount;
  a & x.addr;
  if (ver < 1) {
    return;
  }
  a & x.is_subaddress;
  if (ver < 2) {
    x.token_id = cryptonote::CUTCOIN_ID;
    return;
  }
  a & x.token_id;
}

}  // namespace serialization

}  // namespace boost

#endif //CUTCOIN_TX_DESTINATION_ENTRY_H
