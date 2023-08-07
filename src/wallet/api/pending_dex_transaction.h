// Copyright (c) 2018-2022, CUT coin
// Copyright (c) 2014-2018, The Monero Project
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

#include "wallet/api/wallet2_api.h"
#include "wallet/wallet2.h"

#include <string>
#include <vector>


namespace Monero {

class WalletImpl;

class PendingDexTransactionImpl : public PendingDexTransaction
{
public:
  explicit PendingDexTransactionImpl(WalletImpl &wallet);

  int status() const override;
  std::string errorString() const override;
  bool commit(const std::string &filename, bool overwrite) override;
  uint64_t fee() const override;
  std::vector<std::string> txid() const override;
  uint64_t txCount() const override;

  void setToken1(const std::string &name) override;

  std::string getToken1() override;

  void setToken2(const std::string &name) override;

  std::string getToken2() override;

  void setOldAmount1(uint64_t amount) override;

  uint64_t getOldAmount1() override;

  void setOldAmount2(uint64_t amount) override;

  uint64_t getOldAmount2() override;

  void setNewAmount1(uint64_t amount) override;

  uint64_t getNewAmount1() override;

  void setNewAmount2(uint64_t amount) override;

  uint64_t getNewAmount2() override;

private:
  friend class WalletImpl;
  WalletImpl &m_wallet;

  int  m_status;
  std::string m_errorString;
  std::vector<tools::pending_tx> m_pending_tx;
  std::unordered_set<crypto::public_key> m_signers;
  cryptonote::TokenAmounts m_total_sent;

  std::string     d_token1;

  std::string     d_token2;

  uint64_t        d_old_amount1;

  uint64_t        d_old_amount2;

  uint64_t        d_new_amount1;

  uint64_t        d_new_amount2;
};


}  // namespace Monero

namespace Bitmonero = Monero;
