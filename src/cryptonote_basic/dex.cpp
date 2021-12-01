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

#include "dex.h"

#include "token.h"

#include <string>
#include <vector>

namespace cryptonote {

const std::string delimiter{"/"};

std::string tokens_to_lpname(const TokenId &id1, const TokenId &id2)
{
  std::string s1 = token_id_to_name(id1);
  std::string s2 = token_id_to_name(id2);

  if (is_cutcoin(id1)) {
    return s2 + delimiter + s1;
  }

  if (is_cutcoin(id2)) {
    return s1 + delimiter + s2;
  }

  return s1 < s2 ? s1 + delimiter + s2: s2 + delimiter + s1;
}

std::string tokens_to_lpname(const std::string &token1, const std::string &token2)
{
  if (token1 == CUTCOIN_NAME) {
    return token2 + delimiter + token1;
  }

  if (token2 == CUTCOIN_NAME) {
    return token1 + delimiter + token2;
  }

  return token1 < token2 ? token1 + delimiter + token2: token2 + delimiter + token1;
}

bool lpname_to_tokens(const std::string &name, TokenId &token1, TokenId &token2)
{
  size_t pos = 0;
  std::string token, tmp_name{name};
  std::vector<std::string> tokens;
  while ((pos = tmp_name.find(delimiter)) != std::string::npos) {
    token = tmp_name.substr(0, pos);
    tokens.push_back(token);
    tmp_name.erase(0, pos + delimiter.length());
  }
  tokens.push_back(tmp_name);

  if (tokens.size() != 2) {
    return false;
  }

  if ((!is_cutcoin(tokens[0]) && !validate_token_name(tokens[0])) ||
    (!is_cutcoin(tokens[1]) && !validate_token_name(tokens[1]))) {
    return false;
  }

  token1 = token_name_to_id(tokens[0]);
  token2 = token_name_to_id(tokens[1]);
  return true;
}

bool validate_lpname(const std::string &pool_name)
{
  std::size_t delimeter_pos = pool_name.find(delimiter);
  if (delimeter_pos  == std::string::npos) {
    return false;
  }

  std::string t1 = pool_name.substr(0, delimeter_pos);
  std::string t2 = pool_name.substr(delimeter_pos + 1);

  return !(t1 == t2);
}


}  // namespace cryptonote
