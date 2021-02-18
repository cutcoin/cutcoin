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

#ifndef CUTCOIN_TX_TEMPLATE_H
#define CUTCOIN_TX_TEMPLATE_H

#include "pending_tx.h"

namespace tools {

typedef std::tuple<uint64_t, crypto::public_key, rct::key> get_outs_entry;

struct TxTemplate {
  std::vector <size_t> selected_transfers;
  std::vector <cryptonote::tx_destination_entry> dsts;
  cryptonote::transaction tx;
  pending_tx ptx;

  std::vector <std::vector<get_outs_entry>> outs;

  uint64_t      needed_money{0};
  uint_fast64_t accumulated_fee{0};
  uint64_t      accumulated_outputs{0};
  uint64_t      accumulated_change{0};
  uint64_t      needed_fee{0};
  size_t        weight{0};


  TxTemplate(){ }

//  void add(const account_public_address &addr,
//           bool                          is_subaddress,
//           token_id_type                 token_id,
//           uint64_t                      amount,
//           unsigned int                  original_output_index,
//           bool                          merge_destinations) {
//    if (merge_destinations) {
//      std::vector<cryptonote::tx_destination_entry>::iterator i;
//      i = std::find_if(dsts.begin(), dsts.end(), [&](const cryptonote::tx_destination_entry &d) {
//        return !memcmp(&d.addr, &addr, sizeof(addr));
//      });
//      if (i == dsts.end()) {
//        dsts.push_back(tx_destination_entry(token_id, 0, addr, is_subaddress));
//        i = dsts.end() - 1;
//      }
//      i->amount += amount;
//    } else {
//      THROW_WALLET_EXCEPTION_IF(original_output_index > dsts.size(), error::wallet_internal_error,
//                                std::string("original_output_index too large: ") +
//                                std::to_string(original_output_index) + " > " + std::to_string(dsts.size()));
//      if (original_output_index == dsts.size())
//        dsts.push_back(tx_destination_entry(token_id, 0, addr, is_subaddress));
//      THROW_WALLET_EXCEPTION_IF(memcmp(&dsts[original_output_index].addr, &addr, sizeof(addr)),
//                                error::wallet_internal_error, "Mismatched destination address");
//      dsts[original_output_index].amount += amount;
//    }
//  }
};

}  // namespace tools

#endif //CUTCOIN_TX_TEMPLATE_H
