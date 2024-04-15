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

#include <glib.h>
#include <libevdev/libevdev.h>
#include <libudev.h>
#include <unistd.h>
#include <algorithm>
#include <sstream>
#include "uv.h"
#include "TizenDeviceAPIBase.h"
#include "TizenInputDeviceKeyMap.h"
#include "lwnode/lwnode.h"
#include "TizenInputDeviceManager.h"

namespace DeviceAPI {

static const std::set<std::string> remoteControlDevices = {
    {"wt61p807 rc device"},
    {"SMART_VIEW"},
    {"Smart Control 2014"},
    {"Smart Control 2015"},
    {"Smart Control 2016"},
    {"Smart Control 2017"},
    {"wt61p807 panel key device"}};

DeviceReceiver::DeviceReceiver(const std::string& path, const std::string& name)
    : state_(State::None),
      path_(path),
      name_(name),
      fd_(-1),
      evdev_(nullptr),
      uvPollHandler_(nullptr) {}

DeviceReceiver::~DeviceReceiver() {
  // When device-api terminates, the DeviceReceiver automatically stops and
  // releases resources.
  stop();

  if (uvPollHandler_) {
    delete uvPollHandler_;
  }

  if (evdev_) {
    libevdev_free(evdev_);
  }
}

void DeviceReceiver::start() {
  if (state_ != State::None) {
    return;
  }

  fd_ = open(path_.c_str(), O_RDONLY | O_NONBLOCK);
  if (fd_ < 0) {
    DEVICEAPI_LOG_ERROR(
        "could not open %s: %s", path_.c_str(), strerror(errno));
    return;
  }

  if (libevdev_new_from_fd(fd_, &evdev_) < 0) {
    DEVICEAPI_LOG_ERROR("libevdev_new_from_fd failed");
    close(fd_);
    return;
  }

  uvPollHandler_ = new uv_poll_t;
  int error = uv_poll_init(uv_default_loop(), uvPollHandler_, fd_);
  if (error < 0) {
    DEVICEAPI_LOG_ERROR(
        "uv_poll_init failed(%s): %s", uv_err_name(error), uv_strerror(error));
    close(fd_);
    return;
  }
  uvPollHandler_->data = this;

  // This is to prevent the problem that the main loop of the node is not shut
  // down forever. When the device-api terminates, the handle are released.
  uv_unref(reinterpret_cast<uv_handle_t*>(uvPollHandler_));

  error = uv_poll_start(
      uvPollHandler_,
      UV_READABLE,
      [](uv_poll_t* handle, int status, int events) {
        DeviceReceiver* self = static_cast<DeviceReceiver*>(handle->data);
        input_event event;
        if (libevdev_next_event(self->evdev_,
                                LIBEVDEV_READ_FLAG_NORMAL,
                                &event) != LIBEVDEV_READ_STATUS_SUCCESS) {
          return;
        }

        if (event.type != EV_KEY || event.value > 1) {
          return;
        }

        TizenInputDeviceManager::getInstance()->emitKeyEventToJavaScript(
            event.value, event.code);
      });

  if (error < 0) {
    DEVICEAPI_LOG_ERROR(
        "uv_poll_start failed(%s): %s", uv_err_name(error), uv_strerror(error));
    close(fd_);
    return;
  }

  state_ = State::Start;
}

void DeviceReceiver::stop() {
  if (state_ != State::Start) {
    return;
  }

  DEVICEAPI_LOG_INFO("stop input device: %s", path_.c_str());

  close(fd_);
  uv_poll_stop(uvPollHandler_);
  uv_close((uv_handle_t*)uvPollHandler_, nullptr);

  state_ = State::Stop;
}

static std::string findNameAttr(struct udev_device* device) {
  struct udev_device* deviceParent = device;

  while (deviceParent != nullptr) {
    struct udev_list_entry* sysattr;
    udev_list_entry_foreach(sysattr,
                            udev_device_get_sysattr_list_entry(deviceParent)) {
      std::string name = udev_list_entry_get_name(sysattr);
      if (name == "name") {
        return udev_device_get_sysattr_value(deviceParent, name.c_str());
      }
    }

    deviceParent = udev_device_get_parent(deviceParent);
  }

  return std::string();
}

TizenInputDeviceManager* TizenInputDeviceManager::getInstance() {
  static TizenInputDeviceManager s_instance;
  return &s_instance;
}

TizenInputDeviceManager::TizenInputDeviceManager()
    : eventType_({"keyup", "keydown"}), isRunning_(false) {
#define ADD_TIZEN_KEY_TV(name, key, webapiName, webapiKey)                     \
  keyInfoMap_.push_back({name, key, webapiName, webapiKey});

  FOR_EACH_TIZEN_KEY_MAP_TV(ADD_TIZEN_KEY_TV)
#undef ADD_TIZEN_KEY_TV
}

void TizenInputDeviceManager::start() {
  if (isRunning_) {
    return;
  }

  scanDeivceReceivers();
  startDeivceReceivers();

  isRunning_ = true;
}

void TizenInputDeviceManager::scanDeivceReceivers() {
  udev* udevContext;
  udev_enumerate* enumerate = udev_enumerate_new(udevContext);
  if (enumerate == nullptr) {
    DEVICEAPI_LOG_ERROR("udev_enumerate_new failed");
    return;
  }

  // scan all input devices
  udev_enumerate_add_match_subsystem(enumerate, "input");
  udev_enumerate_scan_devices(enumerate);
  udev_list_entry* devs = udev_enumerate_get_list_entry(enumerate);

  for (udev_list_entry* item = devs; item;
       item = udev_list_entry_get_next(item)) {
    const char* path = udev_list_entry_get_name(item);
    udev_device* device = udev_device_new_from_syspath(udevContext, path);
    const char* devnode = udev_device_get_devnode(device);
    if (device != nullptr && devnode != nullptr) {
      std::string deviceName = findNameAttr(device);
      if (!deviceName.empty()) {
        if (remoteControlDevices.find(deviceName) !=
            remoteControlDevices.end()) {
          auto receiver = std::make_unique<DeviceReceiver>(devnode, deviceName);

          addDeviceReceiver(std::move(receiver));
        }
      }

      udev_device_unref(device);
    }
  }
}

bool TizenInputDeviceManager::findTizenKey(const std::string& keyName,
                                           KeyInfo& key) {
  auto iter = std::find_if(
      keyInfoMap_.begin(), keyInfoMap_.end(), [keyName](const KeyInfo& key) {
        return key.name == keyName;
      });

  if (iter == keyInfoMap_.end()) {
    DEVICEAPI_LOG_INFO("cannot find key information: %s", keyName.c_str());
    return false;
  }

  key = *iter;

  return true;
}

void TizenInputDeviceManager::addDeviceReceiver(
    std::unique_ptr<DeviceReceiver>&& receiver) {
  std::string path = receiver->path();
  auto iter =
      std::find_if(deviceReceivers_.begin(),
                   deviceReceivers_.end(),
                   [path](const std::unique_ptr<DeviceReceiver>& receiver_) {
                     return receiver_->path() == path;
                   });

  if (iter == deviceReceivers_.end()) {
    DEVICEAPI_LOG_INFO("add input device: %s", path.c_str());
    deviceReceivers_.emplace_back(std::move(receiver));
  }
}

void TizenInputDeviceManager::startDeivceReceivers() {
  for (const auto& receiver : deviceReceivers_) {
    receiver->start();
  }
}

bool TizenInputDeviceManager::registerKey(Escargot::ContextRef* contextRef,
                                          const std::string& keyName) {
  if (isEmptyDeviceReceiver()) {
    return false;
  }

  KeyInfo key;
  if (!findTizenKey(keyName, key)) {
    return false;
  }

  if (registeredKeyInfos_.find(contextRef) == registeredKeyInfos_.end()) {
    registeredKeyInfos_[contextRef] = std::set<KeyInfo>();
  }

  DEVICEAPI_LOG_INFO("register key: %s", keyName.c_str());
  registeredKeyInfos_[contextRef].insert(std::move(key));

  return true;
}

bool TizenInputDeviceManager::unregisterKey(Escargot::ContextRef* contextRef,
                                            const std::string& keyName) {
  if (isEmptyDeviceReceiver()) {
    return false;
  }

  KeyInfo key;
  if (!findTizenKey(keyName, key)) {
    return false;
  }

  if (registeredKeyInfos_.find(contextRef) == registeredKeyInfos_.end()) {
    return true;
  }

  DEVICEAPI_LOG_INFO("unregister key: %s", keyName.c_str());
  registeredKeyInfos_[contextRef].erase(std::move(key));

  return true;
}

bool TizenInputDeviceManager::registerKeyBatch(Escargot::ContextRef* contextRef,
                                               const std::string& keyName) {
  std::stringstream stream(keyName);
  std::string key;

  while (std::getline(stream, key, ',')) {
    if (!registerKey(contextRef, key)) {
      return false;
    }
  }

  return true;
}

bool TizenInputDeviceManager::unregisterKeyBatch(
    Escargot::ContextRef* contextRef, const std::string& keyName) {
  std::stringstream stream(keyName);
  std::string key;

  while (std::getline(stream, key, ',')) {
    if (!unregisterKey(contextRef, key)) {
      return false;
    }
  }

  return true;
}

void TizenInputDeviceManager::emitKeyEventToJavaScript(const int value,
                                                       const short code) {
  for (const auto& info : registeredKeyInfos_) {
    auto iter = std::find_if(
        info.second.begin(), info.second.end(), [code](const KeyInfo& data) {
          return data.code == code;
        });

    if (iter == info.second.end()) {
      continue;
    }

    if (!LWNode::Utils::IsRunningIsolate(info.first)) {
      return;
    }

    std::ostringstream oss;
    oss << "process.nextTick(() => { ";
    oss << "process.emit('" << eventType_[value]
        << "', {keyCode: " << iter->webapiCode << "}); ";
    oss << "});";

    LWNode::Utils::CompileRun(info.first, oss.str().c_str());
  }
}

bool TizenInputDeviceManager::isEmptyDeviceReceiver() {
  return deviceReceivers_.empty();
}

}  // namespace DeviceAPI
