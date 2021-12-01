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

const uint8_t f_is_subaddress      = 1;
const uint8_t f_change_to_myself   = 1 << 1;
const uint8_t f_lpw_origin         = 1 << 2;
const uint8_t f_required_splitting = 1 << 3;

struct tx_destination_entry {
  TokenId                token_id;      // token id
  uint64_t               amount;        // money
  account_public_address addr;          // destination address
  uint8_t                flags;         // general purpose flags;

  tx_destination_entry()
  : token_id(CUTCOIN_ID)
  , amount(0)
  , addr(AUTO_VAL_INIT(addr))
  , flags(0)
  {}
    // Create this object. Default constructor.

  tx_destination_entry(TokenId id,
                       uint64_t a,
                       const account_public_address &ad,
                       u_int8_t f)
  // Create this object.
  : token_id(id)
  , amount(a)
  , addr(ad)
  , flags(f)
  {}


  tx_destination_entry(TokenId id,
                       uint64_t a,
                       const account_public_address &ad,
                       bool is_subaddress,
                       bool send_change_to_myself,
                       bool lpw_origin,
                       bool required_splitting)
    // Create this object.
    : token_id(id)
    , amount(a)
    , addr(ad)
  {
    set_subaddress(is_subaddress);
    set_send_change_to_myself(send_change_to_myself);
    set_lpw_origin(lpw_origin);
    set_required_splitting(required_splitting);
  }

  BEGIN_SERIALIZE_OBJECT()
    VARINT_FIELD(amount)
    FIELD(addr)
    VARINT_FIELD(token_id)
    VARINT_FIELD(flags)
  END_SERIALIZE()

  inline void set_subaddress(bool val) {
    if (val) {
      flags |= f_is_subaddress;
    }
    else {
      flags &= (~f_is_subaddress);
    }
  }

  inline bool is_subaddress() const {
    return flags & f_is_subaddress;
  }

  inline void set_send_change_to_myself(bool val) {
    if (val) {
      flags |= f_change_to_myself;
    }
    else {
      flags &= (~f_change_to_myself);
    }
  }

  inline bool is_send_change_to_myself() const {
    return flags & f_change_to_myself;
  }

  inline void set_lpw_origin(bool val) {
    if (val) {
      flags |= f_lpw_origin;
    }
    else {
      flags &= (~f_lpw_origin);
    }
  }

  inline bool is_lpw_origin() const {
    return flags & f_lpw_origin;
  }

  inline void set_required_splitting(bool val) {
    if (val) {
      flags |= f_required_splitting;
    }
    else {
      flags &= (~f_required_splitting);
    }
  }

  inline bool is_required_splitting() const {
    return flags & f_required_splitting;
  }
};

}  // namespace cryptonote

BOOST_CLASS_VERSION(cryptonote::tx_destination_entry, 3)

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

  if (ver < 2) {
    x.token_id = cryptonote::CUTCOIN_ID;
    return;
  }
  a & x.token_id;

  if (ver < 3) {
    bool is_subaddress = x.is_subaddress();
    a & is_subaddress;
    x.set_subaddress(is_subaddress);
    return;
  }

  a & x.flags;
}

}  // namespace serialization

}  // namespace boost

#endif //CUTCOIN_TX_DESTINATION_ENTRY_H
