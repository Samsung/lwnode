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

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

typedef std::unordered_map<std::string, std::string> SmapContents;

constexpr const char* kStartingAddr = "StartingAddr";
constexpr const char* kEndingAddr = "EndingAddr";
constexpr const char* kPermissions = "Permissions";
constexpr const char* kOffset = "Offset";
constexpr const char* kDev = "Dev";
constexpr const char* kInode = "Inode";
constexpr const char* kPathname = "Pathname";
constexpr const char* kRegion = "Region";

std::vector<SmapContents> parseSmaps(std::string pid);
size_t calculateTotalPssSwap(std::vector<SmapContents>& smaps);
size_t calculateTotalRss(std::vector<SmapContents>& smaps);
bool dumpMemorySnapshot(std::string outputPath,
                        std::vector<SmapContents>& smaps);

enum SnapshotStringOption {
  kShowDefault = 0,
  kShowFullInfo = 1,
  kUseShortPath = 1 << 1,
  kShowRegion = 1 << 2,
};

std::string getMemorySnapshotString(std::vector<SmapContents>& smaps,
                                    SnapshotStringOption option = kShowDefault);

bool existsFile(const std::string& path);
std::string getCurrentTimeString(std::string format = "%y%m%d-%H%M%S");
