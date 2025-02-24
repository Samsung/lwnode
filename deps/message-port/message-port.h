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

#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

class Port;

class EXPORT_API MessageEvent {
 public:
  static std::shared_ptr<MessageEvent> New(const std::string& data);

  std::shared_ptr<MessageEvent> Clone() const;
  const std::string& data() const;
  const std::string& origin() const;
  const std::vector<std::weak_ptr<Port>> ports() const;
  const std::weak_ptr<Port>& target() const;

  ~MessageEvent();

 private:
  MessageEvent(const std::string& data);

  struct Internal;
  std::unique_ptr<Internal> internal_;
  friend class Port;
};

class EXPORT_API Port {
 public:
  using OnMessageCallback = std::function<void(const MessageEvent*)>;
  enum Result {
    NoError = 0,
    NoSink,
    NoOnMessage,
    InvalidMessageEvent,
    InvalidPortLoop,
  };

  Result PostMessage(std::shared_ptr<MessageEvent> event);
  void OnMessage(const OnMessageCallback& callback);
  void Unref();

  ~Port();

 private:
  Port();

  struct Internal;
  std::unique_ptr<Internal> internal_;
  friend struct Channel;
};
