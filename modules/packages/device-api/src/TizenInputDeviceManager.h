/*
 * Copyright (c) 2024-present Samsung Electronics Co., Ltd
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

#ifndef __TizenInputDeviceManager__
#define __TizenInputDeviceManager__

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

struct libevdev;
struct uv_poll_s;
typedef struct uv_poll_s uv_poll_t;

namespace Escargot {
class ContextRef;
}

namespace DeviceAPI {

struct KeyInfo {
  std::string name;
  uint16_t code;
  std::string webapiName;
  uint16_t webapiCode;

  bool operator<(const KeyInfo& key) const { return code < key.code; }
};

struct RegisteredKeyData {
  RegisteredKeyData(Escargot::ContextRef* context_, KeyInfo key_)
      : context(context_), key(key_) {}
  Escargot::ContextRef* context;
  KeyInfo key;
};

class DeviceReceiver {
 public:
  enum class State { None, Start, Stop };

  DeviceReceiver(const std::string& path, const std::string& name);
  ~DeviceReceiver();

  void start();

  const std::string& path() { return path_; }

 private:
  State state_;
  const std::string path_;
  const std::string name_;
  int fd_;
  libevdev* evdev_;
  uv_poll_t* uvPollHandler_;

  void stop();
};

class TizenInputDeviceManager {
 public:
  static TizenInputDeviceManager* getInstance();

  void start();

  bool registerKey(Escargot::ContextRef* contextRef,
                   const std::string& keyName);

  bool unregisterKey(Escargot::ContextRef* contextRef,
                     const std::string& keyName);

  bool registerKeyBatch(Escargot::ContextRef* contextRef,
                        const std::string& keyName);

  bool unregisterKeyBatch(Escargot::ContextRef* contextRef,
                          const std::string& keyName);

  void emitKeyEventToJavaScript(const int value, const short code);

 private:
  TizenInputDeviceManager();

  void addDeviceReceiver(std::unique_ptr<DeviceReceiver>&& receiver);
  void scanDeivceReceivers();
  void startDeivceReceivers();

  bool findTizenKey(const std::string& keyName, KeyInfo& key);

  bool isEmptyDeviceReceiver();

  std::vector<std::unique_ptr<DeviceReceiver>> deviceReceivers_;
  std::vector<KeyInfo> keyInfoMap_;
  std::map<Escargot::ContextRef*, std::set<KeyInfo>> registeredKeyInfos_;
  const std::string eventType_[2];
  bool isRunning_;
};

}  // namespace DeviceAPI

#endif
