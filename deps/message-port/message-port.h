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
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "expected.h"

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

  virtual bool IsSync() const { return false; }

  virtual ~MessageEvent();

 protected:
  MessageEvent(const std::string& data);

  struct Internal;
  std::unique_ptr<Internal> internal_;
  friend class Port;
};

class EXPORT_API MessageEventSync final : public MessageEvent {
 public:
  static std::shared_ptr<MessageEventSync> New(const std::string& data);
  ~MessageEventSync();

  struct HandleData {
    HandleData();
    ~HandleData();
    std::promise<std::string> promise;
  };

  bool IsSync() const override { return true; }

  std::shared_ptr<HandleData> handle_data() const { return handle_data_; }

 private:
  MessageEventSync(const std::string& data);

  std::shared_ptr<HandleData> handle_data_;
  friend class Port;
};

class EXPORT_API Port {
 public:
  using OnMessageCallback = std::function<void(const MessageEvent*)>;
  enum Error {
    NoError = 0,
    MessageEventQueued,
    NoSink,
    NoOnMessage,
    InvalidMessageEvent,
    InvalidPortLoop,
    Timeout,
  };
  using Result = Expected<std::string, Error>;

  Result PostMessage(std::shared_ptr<MessageEvent> event);
  Result PostMessage(std::shared_ptr<MessageEventSync> event,
                     int timeout_ms = -1);

  void OnMessage(const OnMessageCallback& callback);
  void Unref();

  ~Port();

 private:
  Port();

  Result PostMessageAsync(std::shared_ptr<MessageEvent> event);

  struct Internal;
  std::unique_ptr<Internal> internal_;
  friend struct Channel;
};
