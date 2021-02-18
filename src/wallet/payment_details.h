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

#ifndef CUTCOIN_PAYMENT_DETAILS_H
#define CUTCOIN_PAYMENT_DETAILS_H

#include <crypto/crypto.h>
#include <cryptonote_basic/token.h>

#include <boost/serialization/version.hpp>

namespace tools {

struct payment_details
{
  crypto::hash m_tx_hash;
  uint64_t m_amount;
  uint64_t m_token_id;
  uint64_t m_fee;
  uint64_t m_block_height;
  uint64_t m_unlock_time;
  uint64_t m_timestamp;
  bool m_coinbase;
  cryptonote::subaddress_index m_subaddr_index;
};

struct address_tx: payment_details
{
  bool m_mempool;
  bool m_incoming;
};

struct pool_payment_details
{
  payment_details m_pd;
  bool m_double_spend_seen;
};

}  // namespace tools

BOOST_CLASS_VERSION(tools::payment_details, 5)
BOOST_CLASS_VERSION(tools::pool_payment_details, 1)

namespace boost {
namespace serialization {

template <typename Archive>
inline
void serialize(Archive& a, tools::payment_details& x, const boost::serialization::version_type ver)
{
  a & x.m_tx_hash;
  a & x.m_amount;
  a & x.m_block_height;
  a & x.m_unlock_time;
  if (ver < 1)
    return;
  a & x.m_timestamp;
  if (ver < 2)
  {
    x.m_coinbase = false;
    x.m_subaddr_index = {};
    return;
  }
  a & x.m_subaddr_index;
  if (ver < 3)
  {
    x.m_coinbase = false;
    x.m_fee = 0;
    return;
  }
  a & x.m_fee;
  if (ver < 4)
  {
    x.m_coinbase = false;
    return;
  }
  a & x.m_coinbase;
  if (ver < 5) {
    x.m_token_id = cryptonote::CUTCOIN_ID;
    return;
  }
  a & x.m_token_id;
}

template <typename Archive>
inline
void serialize(Archive& a, tools::pool_payment_details& x, const boost::serialization::version_type /*ver*/)
{
  a & x.m_pd;
  a & x.m_double_spend_seen;
}

}  // namespace serialization
}  // namespace boost

#endif //CUTCOIN_PAYMENT_DETAILS_H
