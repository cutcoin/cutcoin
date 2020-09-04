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

#ifndef CUTCOIN_ACCOUNT_VIEW_H
#define CUTCOIN_ACCOUNT_VIEW_H

#include "cryptonote_basic/amount.h"
#include "cryptonote_basic/token.h"
#include "transfer_details.h"
#include "unconfirmed_transfer_details.h"

#include <map>
#include <unordered_map>
#include <vector>

namespace tools {

class AccountView {
  // Provide view and basic functionality to the transactions container 'std::vector<Transfer>'.
  // The class is not thread safe.
  // The underlying tx container must be immutable during this object lifetime.

private:
  // Types

  using Transfer                     = transfer_details;
    // Short alias for commonly used 'transfer_details' type.

  using UnconfirmedTransfer          = unconfirmed_transfer_details;
    // Short alias for commonly used 'unconfirmed_transfer_details' type.

  using UnconfirmedTransferContainer = std::unordered_map<crypto::hash, UnconfirmedTransfer>;
    // Container for the 'UnconfirmedTransfer' elements.

  using Index                        = std::size_t;
    // Zero-based user tx index in the array of all user txs.

  using Subaddress                   = uint32_t;
    // Account subaddress corresponding to tx minor index.

  using IndicesPerToken              = std::unordered_map<cryptonote::TokenId, std::set<Index>>;
    // User tx indices grouped by 'token_id'.

  using Subaddresses                 = std::set<Subaddress>;
    // Container for the 'Subaddress' elements.

  using SubaddressesPerToken         = std::unordered_map<cryptonote::TokenId, Subaddresses>;
    // Subaddresses grouped by 'token_id'.

public:
  // Types

  using SubaddressBalances           = std::unordered_map<Subaddress, cryptonote::Amount>;
  // Subaddress balances.

  using BalancesPerToken             = std::unordered_map<cryptonote::TokenId, SubaddressBalances>;
  // Subaddresses grouped by 'token_id'.

public:
  // Creators
  AccountView(const transfer_details_v           &transfers,
              const UnconfirmedTransferContainer &unconfirmed_transfers,
              uint32_t                            index_major,
              uint64_t                            blockchain_height);
    // Construct this object.

public:
  void refresh(uint64_t blockchain_height);
    // Clean this object and rescan 'transfers' and 'unconfirmed_transfers'.

  void clear();
    // Clean this object.

  const SubaddressBalances &get_unlocked_balance_per_subaddress(cryptonote::TokenId token_id) const;
    // Return token unlocked balances per subaddress, the token is specified with 'token_id'.

  const SubaddressBalances &get_balance_per_subaddress(cryptonote::TokenId token_id) const;
    // Return token balances per subaddress, the token is specified with 'token_id'.

  const std::set<cryptonote::TokenId> &get_wallet_tokens() const;
    // Return IDs of all the tokens in the current account.

  uint64_t get_token_subaddress_num_unspent_outputs(cryptonote::TokenId token_id, const Subaddress) const;
    // Count unspent outputs for specified token and subaddress.

  bool unlocked_amount(cryptonote::Amount &amount, cryptonote::TokenId token_id, const Subaddresses &user_indices = {}) const;
    // Return true on success. Assign to 'amount' current unlocked balance for the specified 'token_id'
    // and the specified 'user_indices'. If user_indices are empty use all the indices with
    // non-zero unlocked balance.

  bool amount(cryptonote::Amount &amount, cryptonote::TokenId token_id, const Subaddresses &user_indices = {}) const;
    // Return true on success. Assign to 'amount' current balance for the specified 'token_id'
    // and the specified 'user_indices'. If user_indices are empty use all the indices with
    // non-zero balance.

  Subaddresses subaddress_indices(cryptonote::TokenId token_id) const;
    // Return all subaddress indices (minor indices) having the tokens with the specified 'token_id'.

  std::set<Index> token_indices(cryptonote::TokenId token_id);
    // Return all indices of the transfers with the specified 'token_id'.

  bool pick_preferred_rct_inputs(std::vector<std::size_t>  &preferred_inputs,
                                 uint64_t                   needed_money,
                                 cryptonote::TokenId        token_id,
                                 const Subaddresses        &user_indices,
                                 const uint64_t            &blockchain_height) const;
    // Pick preferred rct inputs for a transfer.

  float get_output_relatedness(const Transfer &t1, const Transfer &t2) const;
    // This returns a handwave estimation of how much two outputs are related
    // If they're from the same tx, then they're fully related. From close block
    // heights, they're kinda related. The actual values don't matter, just
    // their ordering, but it could become more murky if we add scores later.

  void separate_dust_indices(
      std::unordered_map<uint32_t, std::vector<size_t> > &unused_transfers_indices_per_subaddress,
      std::unordered_map<uint32_t, std::vector<size_t> > &unused_dust_indices_per_subaddress,
      const Subaddresses                                 &user_indices,
      const cryptonote::TokenId                           token_id,
      const cryptonote::Amount                            fractional_threshold,
      std::pair<uint64_t, uint64_t>                       range,
      bool                                                ignore_fractional_outputs,
      bool                                                use_rct,
      const uint64_t                                     &blockchain_height) const;
    // Separate transfer indices with dust input from regular ones. Populate corresponding
    // 'unused_transfers_indices_per_subaddress' and 'unused_dust_indices_per_subaddress'.

private:
  //Manipulators
  void find_indices_per_token();

  void find_balances_per_token();

  void find_unlocked_balances_per_token(const uint64_t &blockchain_height);

  void find_subaddresses_per_token();

  bool is_transfer_unlocked(const Transfer &td, const uint64_t &blockchain_height) const;

  bool is_transfer_unlocked(uint64_t unlock_time, uint64_t block_height, const uint64_t &blockchain_height) const;

  bool is_tx_spendtime_unlocked(uint64_t unlock_time, const uint64_t &blockchain_height) const;

private:
  const transfer_details_v           &d_transfers;              // ref to the confirmed transfers (not owned)
  const UnconfirmedTransferContainer &d_unconfirmed_transfers;  // ref to the unconfirmed transfers (not owned)
  const uint32_t                      d_index_major;            // account index (major index)
  IndicesPerToken                     d_token_indices;          // user tx indices grouped by 'token_id'; [token_id: {user_tx_index}]
  mutable BalancesPerToken            d_unlocked_balance;       // unlocked subaddress balances grouped by 'token_id'
  mutable BalancesPerToken            d_balance;                // all subaddresses balances grouped by 'token_id'
  SubaddressesPerToken                d_token_subaddresses;     // dictionary {token_id: set[subaddress_index]]}
                                                                // for locked and unlocked subaddresses balances
  std::set<cryptonote::TokenId>       d_tokens;                 // token ids present in this wallet
};

}  // namespace tools

#endif //CUTCOIN_ACCOUNT_VIEW_H
