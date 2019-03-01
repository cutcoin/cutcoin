// Copyright (c) 2018-2019, CUT coin
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

#ifndef CUTCOIN_LOGGING_H
#define CUTCOIN_LOGGING_H

#include <common/i18n.h>
#include <common/scoped_message_writer.h>

namespace plant {

const char *tr(const char *str);

tools::scoped_message_writer success_msg_writer(bool color = false);

tools::scoped_message_writer message_writer(epee::console_colors color = epee::console_color_default, bool bright = false);

tools::scoped_message_writer fail_msg_writer();

class verbosity
{
public:
  static bool &verbose();

private:
  static bool d_verbose;
};

#define MINE_FATAL(x) {if (verbosity::verbose()) {fail_msg_writer() << tr(x);} MFATAL(x);}
#define MINE_ERROR(x) {if (verbosity::verbose()) {fail_msg_writer() << tr(x);} MERROR(x);}
#define MINE_WARNING(x) {if (verbosity::verbose()) {fail_msg_writer() << tr(x);} MWARNING(x);}
#define MINE_INFO(x) {if (verbosity::verbose()) {message_writer() << tr(x);} MINFO(x);}
#define MINE_DEBUG(x) {if (verbosity::verbose()) {message_writer() << tr(x);} MDEBUG(x);}
#define MINE_TRACE(x) {if (verbosity::verbose()) {message_writer() << tr(x);} MTRACE(x);}

} // namespace plant

#endif //CUTCOIN_LOGGING_H
