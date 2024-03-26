// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __TizenDeviceAPILoaderForEscargot__
#define __TizenDeviceAPILoaderForEscargot__


#include <mutex>
#include <map>

#include <GCUtil.h>

#include "TizenDeviceAPIBase.h"
#include "EscargotPublic.h"

namespace wrt {
namespace xwalk {
class Extension;
class ExtensionInstance;
}  // namespace xwalk
}  // namespace wrt

namespace DeviceAPI {

class ESPostListener;

#define FOR_EACH_EARLY_TIZEN_STRINGS(F) \
  F(tizen)                              \
  F(xwalk)                              \
  F(webapis)

#define FOR_EACH_LAZY_TIZEN_STRINGS(F) \
  F(utils)                             \
  F(common)                            \
  F(extension)                         \
  F(postMessage)                       \
  F(sendSyncMessage)                   \
  F(sendSyncMessageWithBinaryReply)    \
  F(sendSyncMessageWithStringReply)    \
  F(sendSyncData)                      \
  F(sendRuntimeMessage)                \
  F(sendRuntimeSyncMessage)            \
  F(sendRuntimeAsyncMessage)           \
  F(setMessageListener)                \
  F(receiveChunkData)                  \
  F(reply)                             \
  F(chunk_id)                          \
  F(string)                            \
  F(octet)

#define SUPPORTED_TIZEN_PROPERTY(F) \
  F(application)                    \
  F(bluetooth)                      \
  F(filesystem)                     \
  F(mediacontroller)                \
  F(messageport)                    \
  F(systeminfo)                     \
  F(sensorservice)                  \
  F(tvaudiocontrol)                 \
  F(preference)                     \
  F(power)                          \
  F(time)

#define SUPPORTED_TIZEN_ENTRYPOINTS(F) \
  F(ApplicationControl)                \
  F(ApplicationControlData)

class TizenStrings {
 public:
  TizenStrings(Escargot::ContextRef* context);
  void initializeEarlyStrings();
  void initializeLazyStrings();

#define DECLARE_TIZEN_STRING(name) Escargot::AtomicStringRef* name;
  FOR_EACH_EARLY_TIZEN_STRINGS(DECLARE_TIZEN_STRING);
  FOR_EACH_LAZY_TIZEN_STRINGS(DECLARE_TIZEN_STRING);
  SUPPORTED_TIZEN_PROPERTY(DECLARE_TIZEN_STRING);
  SUPPORTED_TIZEN_ENTRYPOINTS(DECLARE_TIZEN_STRING);
#undef DECLARE_TIZEN_STRING

 private:
  Escargot::ContextRef* m_context;
  bool m_initialized;
};

/*
 * Extension: (tizen, utils, common, messageport, sensorservice...) * 1
 * ExtensionManager: (manager) * 1
 * ExtensionInstance: (tizen, utils, common, messageport, sensorservice...) *
 * number of ESVMInstances
 * ExtensionManagerInstance: (manager) * number of ESVMInstances
 */

class ExtensionManagerInstance : public gc {
 public:
  ExtensionManagerInstance(Escargot::ContextRef* context);
  ~ExtensionManagerInstance();

  void* operator new(size_t size);
  void* operator new[](size_t size) = delete;

  static ExtensionManagerInstance* get(Escargot::ContextRef* context);
  TizenStrings* strings() { return m_strings; }
  wrt::xwalk::ExtensionInstance* getExtensionInstanceFromCallingContext(
      Escargot::ContextRef*, Escargot::ValueRef* thisValue);
  Escargot::ObjectRef* initializeExtensionInstance(const char*);
#if defined(STARFISH_TIZEN_WEARABLE_WIDGET)
  WebWidgetAPIInstance* webWidgetAPIInstance() {
    return m_webWidgetAPIInstance;
  }
#endif

  static wrt::xwalk::Extension* getExtension(const char* apiName);

 private:
  struct ChunkData {
    ChunkData() {}
    ChunkData(uint8_t* buffer, size_t length)
        : m_buffer(buffer), m_length(length) {}
    uint8_t* m_buffer;
    size_t m_length;
  };

  typedef std::map<size_t, ChunkData> ChunkDataMap;
  typedef std::map<Escargot::ObjectRef*, wrt::xwalk::ExtensionInstance*>
      ExtensionInstanceMap;
  typedef std::vector<ESPostListener*> ESPostListenerVector;

  Escargot::ObjectRef* createExtensionObject(
      Escargot::ExecutionStateRef* state);
  size_t addChunk(uint8_t* buffer, size_t length);
  ChunkData getChunk(size_t chunkID);

  Escargot::ContextRef* m_context;
  ExtensionInstanceMap m_extensionInstances;
  ESPostListenerVector m_postListeners;
  ChunkDataMap m_chunkDataMap;
  size_t m_chunkID;
  TizenStrings* m_strings;

#if defined(STARFISH_TIZEN_WEARABLE_WIDGET)
  WebWidgetAPIInstance* m_webWidgetAPIInstance;
#endif
#define DECLARE_TIZEN_OBJECT(name) \
  Escargot::ValueRef* VALUE_NAME_STRCAT(m_##name);
  FOR_EACH_EARLY_TIZEN_STRINGS(DECLARE_TIZEN_OBJECT);
  FOR_EACH_LAZY_TIZEN_STRINGS(DECLARE_TIZEN_OBJECT);
  SUPPORTED_TIZEN_PROPERTY(DECLARE_TIZEN_OBJECT);
  SUPPORTED_TIZEN_ENTRYPOINTS(DECLARE_TIZEN_OBJECT);
#undef DECLARE_TIZEN_OBJECT

  // static members
  typedef std::map<Escargot::ContextRef*, 
    Escargot::PersistentRefHolder<ExtensionManagerInstance>>
      ExtensionManagerInstanceMap;
  static ExtensionManagerInstanceMap s_extensionManagerInstances;
  static std::mutex s_mutex;
};

inline ExtensionManagerInstance* ExtensionManagerInstanceGet(
    Escargot::ContextRef* context) {
  return ExtensionManagerInstance::get(context);
}

ExtensionManagerInstance* initialize(Escargot::ContextRef* context);
void close(Escargot::ContextRef* context);
}  // namespace DeviceAPI

class NativeDataAccessorPropertyDataForEntryPoint
    : public Escargot::ObjectRef::NativeDataAccessorPropertyData {
 public:
  NativeDataAccessorPropertyDataForEntryPoint(
      bool isWritable, bool isEnumerable, bool isConfigurable,
      Escargot::ObjectRef::NativeDataAccessorPropertyGetter getter,
      Escargot::ObjectRef::NativeDataAccessorPropertySetter setter)
      : NativeDataAccessorPropertyData(isWritable, isEnumerable, isConfigurable,
                                       getter, setter) {
    m_data = Escargot::ValueRef::createUndefined();
  }

  void* operator new(size_t size) { return GC_MALLOC(size); }

  Escargot::ValueRef* m_data;
};

#endif  // __TizenDeviceAPILoaderForEscargot__
