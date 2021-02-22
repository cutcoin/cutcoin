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


#ifndef CUTCOIN_TOKEN_H
#define CUTCOIN_TOKEN_H

#include <cryptonote_config.h>

#include <cstdint>
#include <limits>
#include <string>

namespace cryptonote {

using TokenId = std::uint64_t;
  // Provide token identifier type.
  // The default value '0' corresponds to Cutcoin itself.

const TokenId CUTCOIN_ID = 0;
  // Cutcoin identifier.

using TokenUnit = std::uint64_t;
  // Token amount type.

const TokenUnit MIN_TOKEN_SUPPLY = 1;
  // The minimal token supply.

const TokenUnit MAX_TOKEN_SUPPLY = std::numeric_limits<uint64_t>::max() / COIN;
  // The maximal token supply.

const std::size_t TOKEN_GENESIS_OUTPUTS = 11;
  // Number of the outputs for the token genesis tx.
  // The value should be equal to DEFAULT_MIX + 1.

const std::string PROHIBITED_TOKEN_NAMES[] = {};
const std::string PROHIBITED_TOKEN_NAMES_PREFIXES[] = {"CUT"};
const char TOKEN_ALLOWED_CHARACTERS[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"};

enum TokenType: std::uint64_t {
  // Token type.

  public_supply = 1,
    // Represents a token with publicly visible supply

  hidden_supply = 2
    // Represents a token with hidden supply.
};

struct TokenSummary {
  // Represent summary token characteristics.

  TokenId       d_token_id;      // unique token id

  TokenType     d_type;          // token type

  TokenUnit     d_token_supply;  // total token supply

  std::uint64_t d_unit;          // token unit. Currently not used, all tokens have Cutcoin unit 1.0e10.
};

TokenId token_name_to_id(const std::string &token_name);
  // Copying first n bytes of string, n matches token_id_type size

std::string token_id_to_name(TokenId token_id);

bool validate_token_name(const std::string &token_name);

constexpr
bool is_cutcoin(const TokenId &token_id)
  // Return true if the specified 'token_is' is 'CUTCOIN_ID'.
{
  return token_id == CUTCOIN_ID;
}

constexpr
bool is_valid_token_supply(TokenUnit supply)
  // Return 'true' if the specified 'supply' lays in the allowed range.
{
  return MIN_TOKEN_SUPPLY <= supply && supply <= MAX_TOKEN_SUPPLY;
}

constexpr
bool is_token_with_public_supply(const TokenType &token_type)
  // Return 'true' if the specified 'token_summary' represents a token with publicly visible supply.
{
  return token_type == TokenType::public_supply;
}

constexpr
bool is_token_with_hidden_supply(const TokenType &token_type)
  // Return 'true' if the specified 'token_summary' represents a token with hidden supply.
{
  return token_type == TokenType::hidden_supply;
}

}  // namespace cryptonote

#endif //CUTCOIN_TOKEN_H