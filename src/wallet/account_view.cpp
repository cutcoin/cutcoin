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

#include "account_view.h"
#include "common/scoped_message_writer.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include "cryptonote_basic/token.h"
#include "misc_log_ex.h"

using namespace tools;

namespace tools {

AccountView::AccountView(const transfer_details_v           &transfers,
                         const UnconfirmedTransferContainer &unconfirmed_transfers,
                         uint32_t                            index_major,
                         uint64_t                            blockchain_height)
: d_transfers(transfers)
, d_unconfirmed_transfers(unconfirmed_transfers)
, d_index_major(index_major)
{
  refresh(blockchain_height);
}


void AccountView::refresh(uint64_t blockchain_height)
{
  clear();
  find_indices_per_token();
  find_balances_per_token();
  find_unlocked_balances_per_token(blockchain_height);
  find_subaddresses_per_token();
}


void AccountView::clear()
{
  d_token_indices.clear();
  d_unlocked_balance.clear();
  d_balance.clear();
  d_token_subaddresses.clear();
}


const AccountView::SubaddressBalances &AccountView::get_unlocked_balance_per_subaddress(
    cryptonote::TokenId token_id) const
{
  return d_unlocked_balance[token_id];
}


const AccountView::SubaddressBalances &AccountView::get_balance_per_subaddress(cryptonote::TokenId token_id) const
{
  return this->d_balance[token_id];
}


const std::set<cryptonote::TokenId> &AccountView::get_wallet_tokens() const
{
  return this->d_tokens;
}

uint64_t AccountView::get_token_subaddress_num_unspent_outputs(cryptonote::TokenId token_id,
                                                               const Subaddress subaddress_index) const
{
  const std::set<Index> &token_indices = d_token_indices.at(token_id);

  uint64_t num_unspent_outputs = 0;

  for (const Index &i: token_indices) {
    const transfer_details &t = d_transfers[i];
    if (t.m_subaddr_index.minor == subaddress_index && !t.m_spent) {
      ++num_unspent_outputs;
    }
  }

  return num_unspent_outputs;
}

bool AccountView::unlocked_amount(cryptonote::Amount        &amount,
                                  cryptonote::TokenId        token_id,
                                  const Subaddresses        &user_indices) const
{
  const SubaddressBalances &token_balances = d_unlocked_balance[token_id];
  SubaddressBalances filtered_balances;
  if (user_indices.empty()) {
    filtered_balances = token_balances;
  } else {
    for (const auto &i: user_indices) {
      if (token_balances.find(i) == token_balances.end()) {
        return false;
      } else {
        filtered_balances[i] = token_balances.at(i);
      }
    }
  }

  amount = 0;
  std::for_each(filtered_balances.begin(),
                filtered_balances.end(),
                [&](const std::unordered_map<Subaddress, cryptonote::Amount>::value_type &p) {amount += p.second;});
  return true;
}


bool AccountView::amount(cryptonote::Amount  &amount,
                         cryptonote::TokenId  token_id,
                         const Subaddresses  &user_indices) const
{
  const SubaddressBalances &token_balances = d_balance[token_id];
  SubaddressBalances filtered_balances;
  if (user_indices.empty()) {
    filtered_balances = token_balances;
  } else {
    for (const auto i: user_indices) {
      if (token_balances.find(i) == token_balances.end()) {
        return false;
      } else {
        filtered_balances[i] = token_balances.at(i);
      }
    }
  }

  amount = 0;
  std::for_each(filtered_balances.begin(),
                filtered_balances.end(),
                [&](const std::unordered_map<Subaddress, cryptonote::Amount>::value_type &p) {amount += p.second;});
  return true;
}


void AccountView::find_indices_per_token()
{
  for (Index i = 0; i < d_transfers.size(); ++i) {
    const Transfer &t = d_transfers[i];
    if (t.m_subaddr_index.major == d_index_major && !t.m_spent) {
      const cryptonote::TokenId &token_id = t.m_token_id;
      d_token_indices[token_id].insert(i);
    }
  }
}

void AccountView::find_balances_per_token()
{
  for (const auto &ti: d_token_indices) {
    const cryptonote::TokenId &token_id = ti.first;
    const std::set<Index>     &indexes  = ti.second;
    d_tokens.insert(token_id);
    SubaddressBalances &token_balance   = d_balance[token_id];
    for (const Index &i: indexes) {
      const Subaddress         &s = d_transfers[i].m_subaddr_index.minor;
      const cryptonote::Amount &a = d_transfers[i].amount();
      token_balance[s] += a;
    }
  }
}

void AccountView::find_unlocked_balances_per_token(const uint64_t &blockchain_height)
{
  for (const auto &ti: d_token_indices) {
    const cryptonote::TokenId &token_id        = ti.first;
    const std::set<Index>     &indexes         = ti.second;
    SubaddressBalances        &token_balance   = d_unlocked_balance[token_id];
    for (const Index &i: indexes) {
      const Subaddress         &s = d_transfers[i].m_subaddr_index.minor;
      const cryptonote::Amount &a = is_transfer_unlocked(d_transfers[i], blockchain_height) ? d_transfers[i].amount(): 0;
      token_balance[s] += a;
    }
  }

  for (const auto &utx: d_unconfirmed_transfers) {
    if (utx.second.m_subaddr_account == d_index_major
        && utx.second.m_state != unconfirmed_transfer_details::failed) {
      for (const auto &d: utx.second.m_change) {
        const cryptonote::TokenId &token_id      = d.first;
        SubaddressBalances        &token_balance = d_balance[token_id];
        // all changes go to 0-th subaddress (in the current subaddress account)
        token_balance[0] += d.second;
      }
    }
  }
}


void AccountView::find_subaddresses_per_token()
{
  for (const auto &ti: d_token_indices) {
    const cryptonote::TokenId &token_id = ti.first;
    Subaddresses token_subaddresses;
    for (const Index &i: ti.second) {
      token_subaddresses.insert(d_transfers[i].m_subaddr_index.minor);
    }
    d_token_subaddresses[token_id] = token_subaddresses;
  }
}


bool AccountView::is_transfer_unlocked(const Transfer &td, const uint64_t &blockchain_height) const
{
  return is_transfer_unlocked(td.m_tx.unlock_time, td.m_block_height, blockchain_height);
}


bool AccountView::is_transfer_unlocked(uint64_t        unlock_time,
                                       uint64_t        block_height,
                                       const uint64_t &blockchain_height) const
{
  if (!is_tx_spendtime_unlocked(unlock_time, blockchain_height)) {
    return false;
  }
  return block_height + CRYPTONOTE_DEFAULT_TX_SPENDABLE_AGE <= blockchain_height;
}


bool AccountView::is_tx_spendtime_unlocked(uint64_t        unlock_time,
                                           const uint64_t &blockchain_height) const
{
  if (unlock_time < CRYPTONOTE_MAX_BLOCK_NUMBER) {
    //interpret as block index
    return blockchain_height - 1 + CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS >= unlock_time;
  } else {
    //interpret as time
    auto current_time = static_cast<uint64_t>(time(nullptr));
    return current_time + CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_SECONDS_V2 >= unlock_time;
  }
}


AccountView::Subaddresses AccountView::subaddress_indices(cryptonote::TokenId token_id) const
{
  const SubaddressBalances &balance_per_subaddress = get_balance_per_subaddress(token_id);
  std::set<Subaddress> indices;
  for (const auto &p: balance_per_subaddress) {
    auto r = indices.insert(p.first);
    if(!r.second) {
      LOG_ERROR("Multiple subaddresses with the same index in AccountView");
      assert(false);
    }
  }
  return indices;
}


std::set<AccountView::Index> AccountView::token_indices(cryptonote::TokenId token_id)
{
  return d_token_indices[token_id];
}


bool AccountView::pick_preferred_rct_inputs(std::vector<std::size_t>  &preferred_inputs,
                                            uint64_t                   needed_money,
                                            cryptonote::TokenId        token_id,
                                            const Subaddresses        &user_indices,
                                            const uint64_t            &blockchain_height) const
{
  preferred_inputs.clear();
  const std::set<Index> &token_indices = d_token_indices.at(token_id);

  // Try to find an rct input of enough size.
  for (const Index &i: token_indices) {
    const transfer_details &t = d_transfers[i];
    if (t.amount() >= needed_money && t.is_rct() && is_transfer_unlocked(t, blockchain_height)) {
      if (user_indices.empty() || user_indices.find(t.m_subaddr_index.minor) != user_indices.end()) {
        preferred_inputs.emplace_back(i);
        return true;
      }
    }
  }

  // Then try to find two outputs.
  // This could be made better by picking one of the outputs to be a small one, since those
  // are less useful since often below the needed money, so if one can be used in a pair,
  // it gets rid of it for the future
  std::vector<size_t> picks;
  float               current_output_relatedness = 1.0f;
  for (size_t i = 0; i < token_indices.size(); ++i) {
    const transfer_details &t1 = d_transfers[i];
    if (!t1.m_key_image_partial && t1.is_rct() && is_transfer_unlocked(t1, blockchain_height)
        && (user_indices.empty() || user_indices.find(t1.m_subaddr_index.minor) != user_indices.end())) {
      LOG_PRINT_L2("Considering input " << i << ", " << cryptonote::print_money(t1.amount()));
      for (size_t j = i + 1; j < token_indices.size(); ++j) {
        const transfer_details &t2 = d_transfers[j];
        if (t1.amount() + t2.amount() >= needed_money
            && !t2.m_key_image_partial && t2.is_rct() && is_transfer_unlocked(t2, blockchain_height)
            && t1.m_subaddr_index == t2.m_subaddr_index) {
          // Update our picks if those outputs are less related than any we
          // already found. If the same, don't update, and oldest suitable outputs
          // will be used in preference.
          float relatedness = get_output_relatedness(t1, t2);
          LOG_PRINT_L2("  with input " << j << ", " << cryptonote::print_money(t2.amount()) << ", relatedness " << relatedness);

          if (relatedness < current_output_relatedness) {
            // Reset the current picks with those, and return them directly
            // if they're unrelated. If they are related, we'll end up returning
            // them if we find nothing better
            if (relatedness == 0.0f) {
              preferred_inputs = std::vector<size_t>{i, j};
              return true;
            } else {
              current_output_relatedness = relatedness;
              picks = std::vector<size_t>{i, j};
            }
            LOG_PRINT_L0("we could use " << i << " and " << j);
          }
        }
      }
    }
  }

  preferred_inputs = picks;
  return picks.size() > 0;
}


float AccountView::get_output_relatedness(const Transfer &t1, const Transfer &t2) const
{
  int64_t dh;

  // expensive test, and same tx will fall onto the same block height below
  if (t1.m_txid == t2.m_txid)
    return 1.0f;

  // same block height -> possibly tx burst, or same tx (since above is disabled)
  dh = t1.m_block_height > t2.m_block_height
      ? t1.m_block_height - t2.m_block_height : t2.m_block_height - t1.m_block_height;
  if (dh == 0)
    return 0.9f;

  // adjacent blocks -> possibly tx burst
  if (dh == 1)
    return 0.8f;

  // could extract the payment id, and compare them, but this is a bit expensive too

  // similar block heights
  if (dh < 10)
    return 0.2f;

  // don't think these are particularly related
  return 0.0f;
}


void AccountView::separate_dust_indices(
    std::unordered_map<uint32_t, std::vector<size_t> > &unused_transfers_indices_per_subaddress,
    std::unordered_map<uint32_t, std::vector<size_t> > &unused_dust_indices_per_subaddress,
    const Subaddresses                                 &user_indices,
    const cryptonote::TokenId                           token_id,
    const uint64_t                                      fractional_threshold,
    std::pair<uint64_t, uint64_t>                       range,
    bool                                                ignore_fractional_outputs,
    bool                                                use_rct,
    const uint64_t                                     &blockchain_height) const
{
  unused_transfers_indices_per_subaddress.clear();
  unused_dust_indices_per_subaddress.clear();

  const std::set<Index> &token_indices = d_token_indices.at(token_id);

  for (const Index &index: token_indices) {
    const Transfer &t = d_transfers[index];
    const uint64_t &amount = d_transfers[index].amount();
    const bool &is_rct     = d_transfers[index].is_rct();
    if (ignore_fractional_outputs && amount < fractional_threshold) {
      MDEBUG("Ignoring output " << index
             << " of amount " << cryptonote::print_money(amount)
             << " which is below threshold " << cryptonote::print_money(fractional_threshold));
      continue;
    }

    if (!t.m_key_image_partial
        && (use_rct ? true : !is_rct)
        && is_transfer_unlocked(t, blockchain_height)
        && (user_indices.empty() || user_indices.find(t.m_subaddr_index.minor) != user_indices.end())) {

      const uint32_t index_minor = t.m_subaddr_index.minor;
      if (range.first < amount && amount <= range.second) {
        if (is_rct || cryptonote::is_valid_decomposed_amount(amount)) {
          unused_transfers_indices_per_subaddress[index_minor].push_back(index);
        } else {
          unused_dust_indices_per_subaddress[index_minor].push_back(index);
        }
      }
    }
  }
}

}  // namespace tools
