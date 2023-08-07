// Copyright (c) 2022, CUT coin
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

#ifndef CUTCOIN_WALLETERROR_H
#define CUTCOIN_WALLETERROR_H

#include <iostream>
#include <string>

namespace cryptonote {

class Result {

private:
  bool        d_status;

  std::string d_message;

public:
  // Creators

  explicit Result() noexcept;
    // Create this object. Result is positive. No error code.

  Result(const std::string &message);
    // Create this object. Result is positive. Error code is provided in the specified 'message'.

public:
  // Public members

  inline std::string to_string() const;
    // Return error message as a string value.

  constexpr
  explicit operator bool() const noexcept {
    // Represent this object as a boolean value.
    return d_status;
  }

};

inline
std::string Result::to_string() const
{
  if (d_status) {
    return "ok";
  }
  return d_message;
}

std::ostream &operator<<(std::ostream &os, const Result &r);
  // Output error message to the specified 'os' output stream.

}  // namespace cryptonote

#endif //CUTCOIN_WALLETERROR_H
