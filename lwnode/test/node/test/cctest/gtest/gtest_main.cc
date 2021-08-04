// Copyright 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <cstdio>
#include <uv.h>
#include "gtest.h"

#include "node_test_fixture.h"

static inline bool startsWith(const std::string& string,
                              const std::string& prefix) {
  return (string.size() >= prefix.size()) &&
         (string.compare(0, prefix.size(), prefix) == 0);
}

void lwnode_parse_args(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);

    if (startsWith(arg, std::string("-f="))) {
      std::string f =
          std::string("*") + arg.substr(strlen("-f=")) + std::string("*");
      ::testing::GTEST_FLAG(filter) = f.c_str();
#if defined(LWNODE)
    } else if (startsWith(arg, std::string("--trace-call"))) {
      EscargotShim::Flags::add(EscargotShim::FlagType::TraceCall);
      NodeZeroIsolateTestFixture::is_trace_call_enabled_ = true;

      std::string str(arg);
      std::string::size_type pos = str.find_first_of('=');
      if (std::string::npos != pos) {
        std::stringstream ss(str.substr(pos + 1));  // +1 for skipping =
        std::string token;
        while (std::getline(ss, token, ',')) {
          if (token.find('-') == 0) {
            EscargotShim::Flags::setNagativeTraceCallId(token.substr(1));
          } else {
            EscargotShim::Flags::setTraceCallId(token);
          }
        }
      }
    } else if (startsWith(arg, std::string("--trace-gc"))) {
      EscargotShim::Flags::add(EscargotShim::FlagType::TraceGC);
#endif
    } else {
      printf("unknown options: %s\n", argv[i]);
    }
  }
}

GTEST_API_ int main(int argc, char **argv) {
  lwnode_parse_args(argc, argv);
  argv = uv_setup_args(argc, argv);
  printf("Running main() from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
