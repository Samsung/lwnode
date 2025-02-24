/*
 * Copyright (c) 2025-present Samsung Electronics Co., Ltd
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
 */

#pragma once

#include <iomanip>
#include <iomanip>  // std::setw
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

class MemTrace {
 public:
  static void Add(void* address, const std::string& id = ".") {
    std::lock_guard<std::mutex> lock(mutex_);
    active_[id].insert(address);
    ++stats_[id].added;
  }

  static void Remove(void* address, const std::string& id = ".") {
    std::lock_guard<std::mutex> lock(mutex_);
    active_[id].erase(address);
    ++stats_[id].removed;
    if (active_[id].empty()) {
      active_.erase(id);
    }
  }

  static size_t GetActiveCount(const std::string& id = ".") {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_.count(id) ? active_[id].size() : 0;
  }

  static size_t GetTotalActiveCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t total = 0;
    for (const auto& pair : active_) {
      total += pair.second.size();
    }
    return total;
  }

  static void PrintStats(const std::string& id = "") {
    const int width = 20;
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "[  STATS   ]" << std::endl;
    if (id.empty()) {
      if (!stats_.empty()) {
        for (const auto& pair : stats_) {
          // clang-format off
        const auto& stat = pair.second;
        size_t active_count = active_.count(pair.first) ? active_[pair.first].size() : 0;
        std::cout << std::setw(width) << std::left << pair.first << " |+"
                  << stat.added << "|-"
                  << stat.removed << "| "
                  << active_count << "|" << std::endl;
          // clang-format on
        }
      } else {
        std::cout << "No statistics available." << std::endl;
      }
    } else {
      if (stats_.count(id)) {
        // clang-format off
      const auto& stat = stats_[id];
      size_t active_count = active_.count(id) ? active_[id].size() : 0;
      std::cout << std::setw(width) << std::left << id << " |+"
                << stat.added << "|-"
                << stat.removed << "| "
                << active_count << "|" << std::endl;
        // clang-format on
      } else {
        std::cout << "No statistics available for ID: " << id << std::endl;
      }
    }
  }

 private:
  struct Stats {
    size_t added = 0;
    size_t removed = 0;
  };

#if __cplusplus >= 201703L
  static inline std::unordered_map<std::string, std::unordered_set<void*>>
      active_;
  static inline std::unordered_map<std::string, Stats> stats_;
  static inline std::mutex mutex_;
#else
  static std::unordered_map<std::string, std::unordered_set<void*>> active_;
  static std::unordered_map<std::string, Stats> stats_;
  static std::mutex mutex_;
#endif
};

#if !defined(NDEBUG) || defined(USE_MEM_TRACE)
#define TRACE_ADD(id, address) MemTrace::Add(address, #id)
#define TRACE_REMOVE(id, address) MemTrace::Remove(address, #id)
#else
#define TRACE_ADD(id, address)
#define TRACE_REMOVE(id, address)
#endif
