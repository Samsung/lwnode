/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author lwnode-contributors
 */

#include "smaps.h"
#include <unistd.h>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <vector>

// utils

bool existsFile(const std::string& path) {
  std::ifstream fs(path);
  return fs.good();
}

std::string getCurrentTimeString(std::string format) {
  std::stringstream ss;
  auto time = std::time(nullptr);
  ss << std::put_time(std::localtime(&time), format.c_str());
  return ss.str();
}

static std::vector<std::string> tokenize(const std::string& str) {
  std::vector<std::string> result;
  std::string token;
  std::stringstream ss(str);

  while (ss >> token) {
    result.push_back(token);
  }
  return result;
}

static std::vector<std::string> tokenize(const std::string& str,
                                         const char delimiter) {
  std::vector<std::string> result;
  std::string token;
  std::stringstream ss(str);

  while (std::getline(ss, token, delimiter)) {
    result.push_back(token);
  }
  return result;
}

static inline bool startsWith(const std::string& string,
                              const std::string& prefix) {
  return (string.rfind(prefix, 0) == 0);
}

static std::string trimString(std::string& str,
                              const std::string& delimiters = " \t\n\v\f\r") {
  str.erase(str.find_last_not_of(delimiters) + 1);
  str.erase(0, str.find_first_not_of(delimiters));
  return str;
}

// parseSmaps

static std::string getMemoryRegionString(std::string pathname) {
  std::string type = "ANONYMOUS";
  if (pathname[0] != '/') {
    if (startsWith(pathname, "[stack") || startsWith(pathname, "[tstack")) {
      type = "STACK";
    } else if (startsWith(pathname, "[heap") || pathname.empty()) {
      type = "HEAP/MMAP";
    } else if (startsWith(pathname, "[vdso")) {
      type = "VDSO";
    }
  } else if (existsFile(pathname)) {
    type = "LIBRARY";
  }
  return type;
}

std::vector<SmapContents> parseSmaps(std::istream& input) {
  std::vector<SmapContents> result;
  std::string line;

  SmapContents map;
  bool isHeaderFound = false;

  while (std::getline(input, line, '\n')) {
    if (line.find("-") != std::string::npos) {
      isHeaderFound = true;
      if (map.empty() == false) {
        result.push_back(map);
        map.clear();
      }
      /*
        The format is:

        address           perms offset  dev   inode      pathname

        08048000-08049000 r-xp 00000000 03:00 8312       /opt/test
        08049000-0804a000 rw-p 00001000 03:00 8312       /opt/test
        0804a000-0806b000 rw-p 00000000 00:00 0          [heap]
        a7cb1000-a7cb2000 ---p 00000000 00:00 0
      */
      auto tokens = tokenize(line);
      auto address = tokenize(tokens[0], '-');

      map[kStartingAddr] = address[0];
      map[kEndingAddr] = address[1];
      map[kPermissions] = "p" + tokens[1].substr(0, tokens[1].size() - 1);

      std::string pathname = (tokens.size() == 6) ? tokens[5] : "";
      map[kPathname] = pathname;
      map[kRegion] = getMemoryRegionString(pathname);
    } else {
      if (isHeaderFound == false) {
        continue;
      }
      /*
        The format is:

        Size:               1084 kB
        KernelPageSize:        4 kB
        MMUPageSize:           4 kB
        Rss:                 892 kB
        Pss:                 374 kB
        Shared_Clean:        892 kB
        Shared_Dirty:          0 kB
        Private_Clean:         0 kB
        Private_Dirty:         0 kB
        Referenced:          892 kB
        Anonymous:             0 kB
        LazyFree:              0 kB
        AnonHugePages:         0 kB
        ShmemPmdMapped:        0 kB
        Shared_Hugetlb:        0 kB
        Private_Hugetlb:       0 kB
        Swap:                  0 kB
        SwapPss:               0 kB
        KernelPageSize:        4 kB
        MMUPageSize:           4 kB
        Locked:                0 kB
        THPeligible:           0
      */
      auto tokens = tokenize(line, ':');
      if (tokens.size() == 2) {
        std::string count = trimString(tokens[1]);
        if (count.rfind(" kB") != std::string::npos) {
          count = count.substr(0, count.size() - 3);  // 3: " kB"
        }
        map[trimString(tokens[0])] = count;
      }
    }
  }

  return result;
}

std::vector<SmapContents> parseSmaps(std::string pid) {
  auto smapsPath = "/proc/" + pid + "/smaps";
  if (existsFile(smapsPath)) {
    std::ifstream input("/proc/" + pid + "/smaps");
    return parseSmaps(input);
  }
  return std::vector<SmapContents>();
}

size_t calculateTotalPssSwap(std::vector<SmapContents>& smaps) {
  size_t total = 0;
  for (auto& smap : smaps) {
    total += (std::stoull(smap["Pss"]) + std::stoull(smap["Swap"]));
  }
  return total;
}

size_t calculateTotalRss(std::vector<SmapContents>& smaps) {
  size_t total = 0;
  for (auto& smap : smaps) {
    total += std::stoull(smap["Rss"]);
  }
  return total;
}

bool dumpMemorySnapshot(std::string outputPath,
                        std::vector<SmapContents>& smaps) {
  std::ofstream output(outputPath);

  if (!output) {
    return false;
  }

  output << "perms,start-addr,end-addr,vm_name,vm_size,rss,pss,swap,total_pss,"
            "region"
         << std::endl;

  const int kApiSystemPointerSize = sizeof(void*);
  const int kAlignSize = kApiSystemPointerSize * 2;

  for (auto& smap : smaps) {
    auto totalPass = std::stoull(smap["Pss"]) + std::stoull((smap["Swap"]));
    output << smap[kPermissions] << ","
           << "0x" << std::setfill('0') << std::setw(kAlignSize)
           << smap[kStartingAddr] << ","
           << "0x" << std::setfill('0') << std::setw(kAlignSize)
           << smap[kEndingAddr] << "," << smap[kPathname];
    output << "," << smap["Size"] << "," << smap["Rss"] << "," << smap["Pss"]
           << "," << smap["Swap"];
    output << "," << totalPass << "," << smap[kRegion] << std::endl;
  }
  return true;
}
