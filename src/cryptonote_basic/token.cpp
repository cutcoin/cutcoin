// Copyright (c) 2019-2021, CUT coin
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

#include "token.h"

#include <algorithm>
#include <string>
#include <boost/algorithm/string.hpp>

namespace cryptonote {

TokenId token_name_to_id(const std::string &token_name)
{
  TokenId token_id = 0;

  char *token_id_ptr = reinterpret_cast<char *>(&token_id);
  std::size_t i = 0;
  for (; i < token_name.length() && i < sizeof(cryptonote::TokenId); ++i)  {
    token_id <<= 8;
    token_id += reinterpret_cast<const unsigned char *>(token_name.c_str())[i];
  }
  for (; i < sizeof(cryptonote::TokenId); ++i) {
    token_id <<= 8;
  }
  return token_id;
}

std::string token_id_to_name(TokenId token_id)
{
  if (token_id == 0) return std::string("CUTCOIN");
  char str_ptr[8];
  for (size_t i = sizeof(TokenId); i > 0; --i)
  {
    str_ptr[i - 1] = static_cast<char>(token_id);
    token_id >>= 8;
  }
  std::string token_name(str_ptr, 8);
  token_name.erase(std::find(token_name.begin(), token_name.end(), '\0'), token_name.end());
  return token_name;
}

bool validate_token_name(const std::string &token_name)
{
  if (token_name.length() == 0 ||
      token_name.length() > sizeof(TokenId) ||
      token_name.find_first_not_of(TOKEN_ALLOWED_CHARACTERS) != std::string::npos)
  {
    return false;
  }

  for (const auto &str : PROHIBITED_TOKEN_NAMES_PREFIXES)
  {
    if (boost::algorithm::starts_with(token_name, str))
      return false;
  }

  for (const auto &str : PROHIBITED_TOKEN_NAMES)
  {
    if (token_name == str)
      return false;
  }

  return true;
}

} // namespace cryptonote