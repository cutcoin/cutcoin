// Copyright (c) 2018-2022, CUT coin
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


#ifndef CUTCOIN_TOKEN_H
#define CUTCOIN_TOKEN_H

#include <crypto/crypto.h>
#include <cryptonote_basic/amount.h>
#include <cryptonote_config.h>

#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>

namespace cryptonote {

using TokenId = std::uint64_t;
  // Provide token identifier type.
  // The default value '0' corresponds to Cutcoin itself.

const TokenId CUTCOIN_ID = 0;
  // Cutcoin identifier.

using TokenAmount = std::pair<TokenId, Amount>;
  // Token amount with the specified id.

using TokenAmounts = std::unordered_map<TokenId, Amount>;
  // Basic container for different token amounts.

const Amount MIN_TOKEN_SUPPLY = COIN;
  // The minimal token supply.

const Amount MAX_TOKEN_SUPPLY = std::numeric_limits<uint64_t>::max() / COIN * COIN;
  // The maximal token supply.

const std::size_t TOKEN_GENESIS_OUTPUTS = 11;
  // Number of the outputs for the token genesis tx.
  // The value should be equal to DEFAULT_MIX + 1.

const std::string CUTCOIN_NAME{"CUTCOIN"};

const std::string PROHIBITED_TOKEN_NAMES[] = {CUTCOIN_NAME};
const std::string PROHIBITED_TOKEN_NAMES_PREFIXES[] = {"CUT"};
const char TOKEN_ALLOWED_CHARACTERS[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"};

const std::string LP_PREFIX = "lp";
  // All liquidity pool tokens start with this prefix.

class TokenType {
  // Contains 'Token type' and functions-helpers.

public:
  enum Value : std::uint64_t {
    // Token type.

    undefined = 0,

    public_supply = 1,
      // Represents a token with publicly visible supply.

    hidden_supply = 2,
      // Represents a token with hidden supply.

    mintable_supply = 4,
      // Represents a token with mintable (unlimited) supply.

    lptoken = 8
      // Represents liquidity pool token.
  };

private:
  Value d_value;

public:
  TokenType() = default;

  TokenType(uint64_t value)
  {
    d_value = static_cast<Value>(value);
  }

  constexpr
  bool is_undefined() const
    // Return 'true' if the specified 'token_type' represents a token with undefined type.
  {
    return !static_cast<bool>(d_value);
  }

  constexpr
  bool is_public() const
    // Return 'true' if the specified 'token_type' represents a token with publicly visible supply.
  {
    return static_cast<bool>(d_value & TokenType::public_supply);
  }

  constexpr
  bool is_hidden() const
    // Return 'true' if the specified 'token_type' represents a token with hidden supply.
  {
    return static_cast<bool>(d_value & TokenType::hidden_supply);
  }

  constexpr
  bool is_mintable() const
    // Return 'true' if the specified 'token_type' represents a token with mintable supply.
  {
    return static_cast<bool>(d_value & TokenType::mintable_supply);
  }

  constexpr
  bool is_lptoken() const
    // Return 'true' if the specified 'token_type' represents a liquidity pool token.
  {
    return static_cast<bool>(d_value & TokenType::lptoken);
  }

  constexpr
  static bool is_undefined(const TokenType &type)
  {
    return !static_cast<bool>(type.d_value);
  }

  constexpr
  static bool is_public(const TokenType &type)
  {
    return static_cast<bool>(type.d_value & public_supply);
  }

  constexpr
  static bool is_hidden(const TokenType &type)
  {
    return static_cast<bool>(type.d_value & hidden_supply);
  }

  constexpr
  static bool is_mintable(const TokenType &type)
  {
    return static_cast<bool>(type.d_value & mintable_supply);
  }

  constexpr
  static bool is_lptoken(const TokenType &type)
  {
    return static_cast<bool>(type.d_value & lptoken);
  }

  constexpr
  std::uint64_t to_uint() const
    // Convert to uint64_t.
  {
    return d_value;
  }

};

struct TokenSummary {
  // Represent summary token characteristics.

  TokenId       d_token_id;      // unique token id

  TokenType     d_type;          // token type

  Amount        d_token_supply;  // total token supply

  std::uint64_t d_unit;          // token unit. Currently not used, all tokens have Cutcoin unit 1.0e10

  crypto::public_key d_pkey{crypto::NullKey::p()};  // token public key
  crypto::secret_key d_skey{crypto::NullKey::s()};  // token secret key
  crypto::signature  d_signature;     // creator's signature
};

TokenId token_name_to_id(const std::string &token_name);
  // Copying first n bytes of string, n matches token_id_type size.

std::string token_id_to_name(TokenId token_id);
  // Convert integer 'token_id' to human-readable token name.

bool validate_token_name(const std::string &token_name);
  // Validate the specified 'token_name'.

bool validate_lptoken_name(const std::string &token_name);
  // Validate the specified 'token_name' of lp token.

bool is_lptoken(TokenId token_id);

constexpr
bool is_cutcoin(const TokenId &token_id)
  // Return true if the specified 'token_is' is 'CUTCOIN_ID'.
{
  return token_id == CUTCOIN_ID;
}

bool is_cutcoin(const std::string &token_name);
  // Return true if the specified 'token_name' is 'CUTCOIN_NAME'.

constexpr
bool is_valid_token_coin_supply(Amount supply)
  // Return 'true' if the specified coin 'supply' lays in the allowed range.
{
  return MIN_TOKEN_SUPPLY / COIN <= supply && supply <= MAX_TOKEN_SUPPLY / COIN;
}

constexpr
bool is_valid_token_supply(Amount supply)
  // Return 'true' if the specified 'supply' lays in the allowed range.
{
  return MIN_TOKEN_SUPPLY <= supply && supply <= MAX_TOKEN_SUPPLY;
}

constexpr
bool is_valid_lptoken_supply(Amount supply)
// Return 'true' if the specified 'supply' == MAX_TOKEN_SUPPLY.
{
  return supply * COIN == MAX_TOKEN_SUPPLY;
}

}  // namespace cryptonote

#endif //CUTCOIN_TOKEN_H
