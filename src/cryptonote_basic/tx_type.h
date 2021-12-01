// Copyright (c) 2018-2021, CUT coin
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

#ifndef CUTCOIN_TX_TYPE_H
#define CUTCOIN_TX_TYPE_H

#include "serialization/serialization.h"

namespace cryptonote{

class TxType {
  // Contains Transaction (tx) type and functions-helpers.

public:
  enum Value: std::uint8_t {
    // Tx type.

    potx = 0,
    // Represents an old plain tx.

    tgtx = 1,
    // Represents a token genesis tx.

    create_liquidity_pool = 3,
    // Represents liquidity pool genesis tx.

    add_liquidity_pool = 4,
    // Represents liquidity pool genesis tx.

    take_liquidity_pool = 5,
    // Represents liquidity pool genesis tx.

    dex_buy = 6,
    // Represents liquidity pool buy tx.

    dex_sell = 7
    // Represents liquidity pool sell tx.
  };

  const static std::uint8_t hidden_supply = 0x20;
  // If 'TxType' equals to 'tgtx', represents a token with hidden supply (public supply if the flag is 0x00).

  const static std::uint8_t minting_supply = 0x40;
  // If 'TxType' equals to 'tgtx', represents a token with mintable (unlimited) supply
  // (not mintaable if the flag is 0x00).

  const static std::uint8_t lp_genesis = 0x80;
  // If 'TxType' equals to 'tgtx', represents liquidity pool token genesis tx.

  const static std::uint8_t type_mask = 0x1f;

  const static std::uint8_t flags_mask = ~type_mask;

private:
  Value d_value;

public:
  TxType() = default;

  TxType(uint8_t value)
  {
    d_value = static_cast<Value>(value);
  }

  constexpr
  bool is_plain() const
  // Return 'true' if 'd_value' represents an ordinary old style tx.
  {
    return d_value == 0;
  }

  constexpr
  bool is_tgtx() const
  // Return 'true' if 'd_value' represents a tgtx.
  {
    return (d_value & type_mask) == tgtx;
  }

  constexpr
  bool is_create_lp() const
  // Return 'true' if 'd_value' represents a liquidity pool genesis tx.
  {
    return (d_value & type_mask) == create_liquidity_pool;
  }

  constexpr
  bool is_add_liquidity() const
  // Return 'true' if 'd_value' represents a tx that adds liquidity to a pool.
  {
    return (d_value & type_mask) == add_liquidity_pool;
  }

  constexpr
  bool is_take_liquidity() const
  // Return 'true' if 'd_value' represents a tx that takes liquidity from a pool.
  {
    return (d_value & type_mask) == take_liquidity_pool;
  }

  constexpr
  bool is_dex_buy() const
  // Return 'true' if 'd_value' represents a buy dex tx.
  {
    return (d_value & type_mask) == dex_buy;
  }

  constexpr
  bool is_dex_sell() const
  // Return 'true' if 'd_value' represents a sell dex tx.
  {
    return (d_value & type_mask) == dex_sell;
  }

  constexpr
  bool has_flag_hidden_supply() const
  // Return 'true' if 'd_value' has a hidden supply flag.
  {
    return static_cast<bool>(d_value & hidden_supply);
  }

  constexpr
  bool has_flag_minting() const
  // Return 'true' if 'd_value' has a 'minting' flag.
  {
    return static_cast<bool>(d_value & minting_supply);
  }

  constexpr
  bool has_flag_lp_token() const
  // Return 'true' if 'd_value' has 'lp token' flag.
  {
    return static_cast<bool>(d_value & lp_genesis);
  }

  constexpr
  bool has_flags() const
  {
    return static_cast<bool>(d_value & flags_mask);
  }

  constexpr
  std::uint8_t to_uint() const
  // Convert to uint8_t.
  {
    return d_value;
  }

  void raise_flag(std::uint8_t flag)
  // Set any flag.
  {
    d_value = static_cast<Value>(d_value | flag);
  }

  void clear_flag(std::uint8_t flag)
  // Set any flag.
  {
    d_value = static_cast<Value>(d_value & (~flag));
  }

  BEGIN_SERIALIZE()
    FIELD(reinterpret_cast<std::uint8_t&>(d_value));
  END_SERIALIZE()
};

}  // namespace cryptonote

#endif //CUTCOIN_TX_TYPE_H
