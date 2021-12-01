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

#ifndef CUTCOIN_SPECIAL_ACCOUNTS_H
#define CUTCOIN_SPECIAL_ACCOUNTS_H

#include "cryptonote_basic/account.h"
#include "cryptonote_basic/cryptonote_basic.h"

namespace cryptonote {

void get_coin_burn_address(account_public_address &address, crypto::secret_key &view_secret_key);
  // Generates the coin burn address (deterministically) and store it to the public address 'address' and
  // secret view key 'view_secret_key'. The public address can be used as a payments receiver and
  // the 'view_secret_key' allows to check that the payment was sent exactly to this address.
  // The spend secret key is not known, thus the funds sent to the coin burn address cannot be used again
  // and are burnt out.

account_base get_coin_burn_account();
  // Return 'account_base' account corresponding to the coin burn address. The 'm_spend_secret_key'
  // is filled with fake data.

struct LpAccount
{
public:
  LpAccount(const LpAccount&)= delete;

  LpAccount& operator=(const LpAccount&)= delete;

  static void set_default_network_type(network_type type);
    // Set default network type to'type'.

  static bool check_destination_output(const tx_out             &out,
                                       const crypto::public_key &pub_key,
                                       const rct::rctSig        &rct_signatures,
                                       const rct::keyV          &omega_v,
                                       size_t                    output_index,
                                       Amount                   &amount);
    // Return 'true' if the specified 'out' has liquidity pool destination address.

  static bool check_destination_tx(const transaction &tx, Amount amount, TokenId token_id);
    // Return 'true' if the specified 'tx' has outputs with with the specified 'token_id' and summary amount
    // greater or equal than the specified 'amount'.

  static const account_base &get();


private:
  LpAccount ();

  ~LpAccount()= default;

  static const account_base &get(network_type type);
    // Return 'account_base' account corresponding to the specified network 'type'. This address is just one of
    // ordinary Cutcoin addresses.

  static network_type    d_network_type;

  static const size_t    d_num_networks{3};

  account_base           d_accounts[d_num_networks];
};

const crypto::hash coin_burn_seed{{ 1,   2,   3,   5,   7,  11,  13,  17,
                                   19,  23,  29,  31,  37,  41,  43,  47,
                                   53,  59,  61,  67,  71,  73,  79,  83,
                                   89,  97, 101, 103, 107, 109, 113, 127}};
  // Builtin constant seed for Coin burn address.

}  // namespace cryptonote

#endif //CUTCOIN_SPECIAL_ACCOUNTS_H
