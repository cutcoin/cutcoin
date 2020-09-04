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

#ifndef CUTCOIN_TX_CREATION_CONTEXT_H
#define CUTCOIN_TX_CREATION_CONTEXT_H

#include "cryptonote_core/cryptonote_tx_utils.h"

#include <map>

namespace tools {

struct TxTokenData {
  std::vector<cryptonote::tx_destination_entry> d_dsts;
  std::vector<cryptonote::tx_destination_entry> d_dsts_with_change;
  std::vector<size_t> d_selected_transfers;
  cryptonote::tx_destination_entry d_change_dst;
};


struct TxCreationContext {
  using transfer_data = std::unordered_map<cryptonote::TokenId, TxTokenData>;

  explicit TxCreationContext(bool is_rct = true)
    : is_rct{is_rct} {
  }

  bool empty() {
    return tokens.empty();
  }

  // Data
  bool is_rct;
  std::map<cryptonote::TokenId, TxTokenData> tokens;
};

}  // namespace tools



#endif //CUTCOIN_TX_CREATION_CONTEXT_H
