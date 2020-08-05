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

#ifndef CUTCOIN_MULTISIG_TX_SET_H
#define CUTCOIN_MULTISIG_TX_SET_H

#include <crypto/crypto.h>
#include "pending_tx.h"

#include <boost/serialization/version.hpp>

#include <unordered_set>
#include <vector>

namespace tools {

struct multisig_tx_set
{
  pending_tx_v m_ptx;
  std::unordered_set<crypto::public_key> m_signers;

  BEGIN_SERIALIZE_OBJECT()
  FIELD(m_ptx)
  FIELD(m_signers)
  END_SERIALIZE()
};

}  // namespace tools

BOOST_CLASS_VERSION(tools::multisig_tx_set, 1)

namespace boost {
namespace serialization {

template <typename Archive>
inline
void serialize(Archive &a, tools::multisig_tx_set &x, const boost::serialization::version_type /*ver*/)
{
  a & x.m_ptx;
  a & x.m_signers;
}

}  // namespace serialization
}  // namespace boost

#endif //CUTCOIN_MULTISIG_TX_SET_H
