// Copyright (c) 2018-2020, CUT coin
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

#ifndef _UINT128HPP_
#define _UINT128HPP_

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cctype>
#include <cstdlib>
#include <cassert>
#include <stdexcept>
#include <string>

namespace num {

struct u128_t {
  uint64_t val[2];

  u128_t() {
  }

  u128_t(uint64_t _val) {
    set_uint(_val);
  }

  u128_t(uint64_t _lo, uint64_t _hi) {
    val[0]=_lo;
    val[1]=_hi;
  }

  u128_t(const u128_t &f) {
    val[0]=f.val[0];
    val[1]=f.val[1];
  }

  explicit u128_t(const std::string &s) {
    set_string(s);
  }

  explicit u128_t(const char *s) {
    set_cstring(s);
  }

  void set_uint(uint64_t u) {
    val[0]=u;
    val[1]=0;
  }

  void set_string(const std::string &s) {
    set_cstring(s.c_str());
  }

  void set_cstring(const char *s);

  std::string get_string() const;

  u128_t &operator=(const u128_t &a) {
    val[0]=a.val[0];
    val[1]=a.val[1];
    return *this;
  }

  explicit operator std::string () const {
    return std::move(get_string());
  }

  explicit operator bool () const {
    return (val[0]|val[1])!=0;
  }

  u128_t& operator++();
  u128_t& operator--();
};

u128_t operator +(const u128_t &a, const u128_t &b);
u128_t operator -(const u128_t &a, const u128_t &b);
u128_t operator *(const u128_t &a, const u128_t &b);
u128_t operator *(const u128_t &a, uint64_t b);
u128_t operator /(const u128_t &a, const u128_t &b);
u128_t operator %(const u128_t &a, const u128_t &b);

u128_t operator <<(const u128_t &a, uint32_t cnt);
u128_t operator >>(const u128_t &a, uint32_t cnt);

bool operator <(const u128_t &a, const u128_t &b);

bool operator ==(const u128_t &a, const u128_t &b);

bool operator <=(const u128_t &a, const u128_t &b);

bool operator !=(const u128_t &a, const u128_t &b);

bool operator >(const u128_t &a, const u128_t &b);

bool operator >=(const u128_t &a, const u128_t &b);

// return a*b/c, result must fit in 128bit, c must evenly divide a*b
u128_t muldiv(const u128_t &a, const u128_t &b, uint64_t c, bool must_div=false);

void div128by64(const u128_t &num, uint64_t den, u128_t &quotient, uint64_t &remainder);

void div128by128(const u128_t &num, const u128_t &den, u128_t &quotient, u128_t &remainder);

template <uint32_t div>
void div128byconst(const u128_t &num, u128_t &quotient, uint32_t &remainder) {
  uint64_t qh, ql, d;
  qh=num.val[1]/div;
  d=((num.val[1]%div)<<32)|(num.val[0]>>32);
  ql=(d/div)<<32;
  d=((d%div)<<32)|(num.val[0]&UINT32_MAX);
  ql|=d/div;
  remainder=d%div;
  quotient.val[0]=ql;
  quotient.val[1]=qh;
}
}

#endif
