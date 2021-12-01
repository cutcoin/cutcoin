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

#ifndef CUTCOIN_TX_H
#define CUTCOIN_TX_H

#include "cryptonote_basic/amount.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_basic/token.h"
#include "outs_entry.h"
#include "tx_creation_context.h"
#include "wallet_errors.h"

#include <vector>

namespace tools {

struct Tx {
  TxCreationContext                        tcc;
  cryptonote::transaction                  tx;
  pending_tx                               ptx;
  size_t                                   weight;
  cryptonote::Amount                       needed_fee;
  std::vector<std::vector<get_outs_entry>> outs;

  Tx()
  : weight(0)
  , needed_fee(0)
  {}

  void add(const cryptonote::account_public_address &addr,
           bool                                      is_subaddress,
           cryptonote::TokenId                       token_id,
           cryptonote::Amount                        amount,
           bool                                      send_change_to_myself,
           bool                                      required_splitting) {
    std::vector<cryptonote::tx_destination_entry>::iterator i;
    i = std::find_if(tcc.transfers[token_id].d_dsts.begin(),
                     tcc.transfers[token_id].d_dsts.end(),
                     [&](const cryptonote::tx_destination_entry &d) { return !memcmp(&d.addr, &addr, sizeof(addr)); });
    if (i == tcc.transfers[token_id].d_dsts.end()) {
      tcc.transfers[token_id].d_dsts.emplace_back(token_id, 0, addr, 0);
      tcc.transfers[token_id].d_dsts.back().set_subaddress(is_subaddress);
      tcc.transfers[token_id].d_dsts.back().set_send_change_to_myself(send_change_to_myself);
      tcc.transfers[token_id].d_dsts.back().set_required_splitting(required_splitting);
      i = tcc.transfers[token_id].d_dsts.end() - 1;
    }
    i->amount += amount;
  }

  void lp_add(const cryptonote::account_public_address &addr,
              bool                                      is_subaddress,
              cryptonote::TokenId                       token_id,
              cryptonote::Amount                        amount,
              bool                                      required_splitting) {
    std::vector<cryptonote::tx_destination_entry>::iterator i;
    i = std::find_if(tcc.lp_transfers[token_id].d_dsts.begin(),
                     tcc.lp_transfers[token_id].d_dsts.end(),
                     [&](const cryptonote::tx_destination_entry &d) { return !memcmp(&d.addr, &addr, sizeof(addr)); });
    if (i == tcc.lp_transfers[token_id].d_dsts.end()) {
      tcc.lp_transfers[token_id].d_dsts.emplace_back(token_id, 0, addr, 0);
      tcc.lp_transfers[token_id].d_dsts.back().set_subaddress(is_subaddress);
      tcc.lp_transfers[token_id].d_dsts.back().set_required_splitting(required_splitting);
      i = tcc.lp_transfers[token_id].d_dsts.end() - 1;
    }
    i->amount += amount;
  }
};

}  // namespace tools

#endif //CUTCOIN_TX_H
