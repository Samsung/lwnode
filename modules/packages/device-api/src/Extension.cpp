// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Extension.h"

#include <dlfcn.h>

#include "TizenDeviceAPIBase.h"
#include "TizenInputDeviceManager.h"
#include "ExtensionAdapter.h"
#include "EscargotPublic.h"
#include "TizenDeviceAPILoaderForEscargot.h"

namespace wrt {
namespace xwalk {

Extension::Extension(const std::string& path, RuntimeVariableProvider* provider)
    : initialized_(false),
      library_path_(path),
      xw_extension_(0),
      use_trampoline_(true),
      created_instance_callback_(NULL),
      destroyed_instance_callback_(NULL),
      shutdown_callback_(NULL),
      handle_msg_callback_(NULL),
      handle_sync_msg_callback_(NULL),
      rv_provider_(provider) {}

Extension::Extension(const std::string& path, const std::string& name,
                     const std::vector<std::string>& entry_points,
                     RuntimeVariableProvider* provider)
    : initialized_(false),
      handle_(NULL),
      library_path_(path),
      xw_extension_(0),
      name_(name),
      entry_points_(entry_points),
      use_trampoline_(true),
      created_instance_callback_(NULL),
      destroyed_instance_callback_(NULL),
      shutdown_callback_(NULL),
      handle_msg_callback_(NULL),
      handle_sync_msg_callback_(NULL),
      rv_provider_(provider) {}

Extension::~Extension() {
  if (!initialized_) return;

  if (handle_) dlclose(handle_);

  if (shutdown_callback_) shutdown_callback_(xw_extension_);
  ExtensionAdapter::GetInstance()->UnregisterExtension(this);
}

bool Extension::Initialize() {
  if (initialized_) return true;

  DEVICEAPI_LOG_INFO("========== << Initialize >> ENTER ==========");
  DEVICEAPI_SLOG_INFO("Extension Module library : [%s]", library_path_.c_str());

  DEVICEAPI_ASSERT(handle_ == NULL);
  handle_ = dlopen(library_path_.c_str(), RTLD_LAZY);
  if (!handle_) {
    const char* error = (const char*)dlerror();
    DEVICEAPI_LOG_ERROR("Error loading extension '%s'. Reason: %s",
                        library_path_.c_str(),
                        (error != NULL ? error : "unknown"));
    return false;
  }

  XW_Initialize_Func initialize =
      reinterpret_cast<XW_Initialize_Func>(dlsym(handle_, "XW_Initialize"));
  if (!initialize) {
    DEVICEAPI_LOG_ERROR("Error loading extension");
    DEVICEAPI_SLOG_ERROR("[%s] couldn't get XW_Initialize function",
                         library_path_.c_str());
    dlclose(handle_);
    handle_ = NULL;
    return false;
  }

  ExtensionAdapter* adapter = ExtensionAdapter::GetInstance();
  xw_extension_ = adapter->GetNextXWExtension();
  adapter->RegisterExtension(this);

  int ret = initialize(xw_extension_, ExtensionAdapter::GetInterface);
  if (ret != XW_OK) {
    DEVICEAPI_LOG_ERROR("Error loading extension");
    DEVICEAPI_SLOG_ERROR("[%s] XW_Initialize function returned error value.",
                         library_path_.c_str());
    dlclose(handle_);
    handle_ = NULL;
    return false;
  }

  if (name_ == "tizen.tvinputdevice") {
    DeviceAPI::TizenInputDeviceManager::getInstance()->start();
  }

  initialized_ = true;
  DEVICEAPI_LOG_INFO("========== << Initialize >> END ==========");
  return true;
}

ExtensionInstance* Extension::CreateInstance() {
  Initialize();
  ExtensionAdapter* adapter = ExtensionAdapter::GetInstance();
  XW_Instance xw_instance = adapter->GetNextXWInstance();
  return new ExtensionInstance(this, xw_instance);
}

void Extension::GetRuntimeVariable(const char* key, char* value,
                                   size_t value_len) {
  if (rv_provider_) {
    rv_provider_->GetRuntimeVariable(key, value, value_len);
  }
}

int Extension::CheckAPIAccessControl(const char* /*api_name*/) {
  // TODO
  return XW_OK;
}

int Extension::RegisterPermissions(const char* /*perm_table*/) {
  // TODO
  return XW_OK;
}

ExtensionInstance::ExtensionInstance(Extension* extension,
                                     XW_Instance xw_instance)
    : extension_(extension),
      xw_instance_(xw_instance),
      instance_data_(NULL),
      post_message_listener_(NULL),
      post_data_listener_(NULL) {
  DEVICEAPI_LOG_INFO("Enter");
  ExtensionAdapter::GetInstance()->RegisterInstance(this);
  XW_CreatedInstanceCallback callback = extension_->created_instance_callback_;
  if (callback) callback(xw_instance_);
}

ExtensionInstance::~ExtensionInstance() {
  DEVICEAPI_LOG_INFO("Enter");
  XW_DestroyedInstanceCallback callback =
      extension_->destroyed_instance_callback_;
  if (callback) callback(xw_instance_);
  ExtensionAdapter::GetInstance()->UnregisterInstance(this);
}

void ExtensionInstance::HandleMessage(const std::string& msg) {
  XW_HandleMessageCallback callback = extension_->handle_msg_callback_;
  if (callback) callback(xw_instance_, msg.c_str());
}

void ExtensionInstance::HandleSyncMessage(const std::string& msg) {
  XW_HandleSyncMessageCallback callback = extension_->handle_sync_msg_callback_;
  if (callback) {
    sync_reply_msg_.clear();
    callback(xw_instance_, msg.c_str());
  }
}

void ExtensionInstance::PostMessage(const std::string& msg) {
  if (post_message_listener_) {
    post_message_listener_->PostMessageToJS(msg);
  }
}

void ExtensionInstance::SyncReply(const std::string& reply) {
  sync_reply_msg_ = reply;
}

void ExtensionInstance::HandleData(const std::string& msg, uint8_t* buffer,
                                   size_t len) {
  XW_HandleDataCallback callback = extension_->handle_data_callback_;
  if (callback) callback(xw_instance_, msg.c_str(), buffer, len);
}

void ExtensionInstance::HandleSyncData(const std::string& msg, uint8_t* buffer,
                                       size_t len) {
  XW_HandleDataCallback callback = extension_->handle_sync_data_callback_;
  if (callback) {
    sync_reply_msg_.clear();
    sync_reply_buffer_len_ = 0;
    sync_reply_buffer_ = NULL;
    // sync_reply_buffer_ will be freed by XWalkExtensionModule
    callback(xw_instance_, msg.c_str(), buffer, len);
  }
}

void ExtensionInstance::PostData(const std::string& msg, uint8_t* buffer,
                                 size_t len) {
  if (post_data_listener_) {
    post_data_listener_->PostDataToJS(msg, buffer, len);
  }
}

void ExtensionInstance::SyncDataReply(const std::string& reply, uint8_t* buffer,
                                      size_t len) {
  sync_reply_msg_ = reply;
  sync_reply_buffer_ = buffer;
  sync_reply_buffer_len_ = len;
}

}  // namespace xwalk
}  // namespace wrt

namespace DeviceAPI {

ESPostListener::ESPostListener(Escargot::ContextRef* context,
                               Escargot::ObjectRef* listener)
    : context_(context), listener_(listener) {
  DEVICEAPI_LOG_INFO("Enter");
  GC_add_roots(&listener_, &listener_ + sizeof(Escargot::ObjectRef*));
}

ESPostListener::~ESPostListener() {
  DEVICEAPI_LOG_INFO("Enter");
  finalize();
}

void ESPostListener::finalize() {
  DEVICEAPI_LOG_INFO("Enter");
  GC_remove_roots(&listener_, &listener_ + sizeof(Escargot::ObjectRef*));
  listener_ = nullptr;
  context_ = nullptr;
}

ESPostMessageListener::IdlerRegister_t
    ESPostMessageListener::AddIdlerToMainThread = nullptr;

void ESPostMessageListener::PostMessageToJS(const std::string& msg) {
  DEVICEAPI_LOG_INFO(
      "ESPostMessageListener::PostMessageToJS (msg %s listener %p context "
      "%p)",
      msg.c_str(), listener_, context_);

  ExtensionManagerInstance* extensionManagerInstance =
      ExtensionManagerInstance::get(context_);
  if (!extensionManagerInstance) {
    return;
  }

  struct Params {
    Escargot::ContextRef* context;
    Escargot::ObjectRef* listener;
    std::string msg;
  };

  Params* params = new Params();
  params->context = context_;
  params->listener = listener_;
  params->msg = msg;
  DEVICEAPI_LOG_INFO("Post message");

  if (AddIdlerToMainThread) {
    AddIdlerToMainThread(
        [](void* data) {
          DEVICEAPI_LOG_INFO("Add idle\n");
          Params* params = (Params*)data;
          Escargot::ContextRef* context = params->context;

          auto result = Escargot::Evaluator::execute(
              context,
              [](Escargot::ExecutionStateRef* state,
                 Params* params) -> Escargot::ValueRef* {
                Escargot::ObjectRef* listener = params->listener;
                std::string msg = params->msg;
                Escargot::ValueRef* arguments[] = {Escargot::ValueRef::create(
                    Escargot::StringRef::createFromASCII(msg.c_str(),
                                                         msg.size()))};
                return listener->call(state, Escargot::ValueRef::createNull(),
                                      1, arguments);
              },
              params);
          if (result.error.hasValue()) {
            DEVICEAPI_LOG_ERROR("Uncaught %s\n",
                                result.resultOrErrorToString(context)
                                    ->toStdUTF8String()
                                    .c_str());
          }

          delete params;
          return 0;
        },
        params);
  } else {
    DEVICEAPI_LOG_WARN("idler is ignored.\n");
  }
}

void ESPostDataListener::PostDataToJS(const std::string& msg, uint8_t* buffer,
                                      size_t len) {
  DEVICEAPI_LOG_INFO("ESPostDataListener::PostDataToJS (%s, %zu)", msg.c_str(),
                     len);

  ExtensionManagerInstance* extensionManagerInstance =
      ExtensionManagerInstance::get(context_);
  if (!extensionManagerInstance) {
    return;
  }

  auto result = Escargot::Evaluator::execute(
      context_, [](Escargot::ExecutionStateRef* state) -> Escargot::ValueRef* {
#if 0
            Escargot::ValueRef* arguments[] = {Escargot::ValueRef::create(Escargot::StringRef::createFromASCII(msg.c_str(), msg.size()))};
            return listener_->call(state, Escargot::ValueRef::createNull(), 1, arguments);
#else
            DEVICEAPI_LOG_ERROR("NOT IMPLEMENTED");
            DEVICEAPI_ASSERT_SHOULD_NOT_BE_HERE();
            return Escargot::ValueRef::createUndefined();
#endif
      });
  if (result.error.hasValue()) {
    DEVICEAPI_LOG_ERROR(
        "Uncaught %s\n",
        result.resultOrErrorToString(context_)->toStdUTF8String().c_str());
  }
}

}  // namespace DeviceAPI
