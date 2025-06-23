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

namespace LWNode {

class GlobalConfiguration {
 public:
  static GlobalConfiguration& GetInstance();

  GlobalConfiguration(const GlobalConfiguration&) = delete;
  GlobalConfiguration& operator=(const GlobalConfiguration&) = delete;

  GlobalConfiguration(GlobalConfiguration&&) = delete;
  GlobalConfiguration& operator=(GlobalConfiguration&&) = delete;

  void set_gc_interval(int interval) { gc_interval_ = interval; }
  int gc_interval() { return gc_interval_; }

  void set_gc_free_space_divisor(int divisor) {
    gc_free_space_divisor_ = divisor;
  }
  int gc_free_space_divisor() { return gc_free_space_divisor_; }

 private:
  GlobalConfiguration() = default;
  ~GlobalConfiguration() = default;

  int gc_interval_ = -1;  // TODO: change optional type
  int gc_free_space_divisor_ = -1;
};

}  // namespace LWNode
