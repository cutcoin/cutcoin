//
// Created by im on 29.04.2020.
//

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
