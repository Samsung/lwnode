// Copyright 2011 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_INIT_V8_H_
#define V8_INIT_V8_H_

#ifdef V8_OS_WIN

// Setup for Windows shared library export.
#ifdef BUILDING_V8_SHARED
#define V8_EXPORT_PRIVATE __declspec(dllexport)
#elif USING_V8_SHARED
#define V8_EXPORT_PRIVATE __declspec(dllimport)
#else
#define V8_EXPORT_PRIVATE
#endif  // BUILDING_V8_SHARED

#else  // V8_OS_WIN

// Setup for Linux shared library export.
#if V8_HAS_ATTRIBUTE_VISIBILITY
#ifdef BUILDING_V8_SHARED
#define V8_EXPORT_PRIVATE __attribute__((visibility("default")))
#else
#define V8_EXPORT_PRIVATE
#endif
#else
#define V8_EXPORT_PRIVATE
#endif

#endif  // V8_OS_WIN

namespace v8 {

class Platform;
class StartupData;

namespace internal {

class Isolate;

// Superclass for classes only using static method functions.
// The subclass of AllStatic cannot be instantiated at all.
class AllStatic {
#ifdef DEBUG
 public:
  AllStatic() = delete;
#endif
};

class V8 : public AllStatic {
 public:
  // Global actions.

  static bool Initialize();
  static void TearDown();

  // Report process out of memory. Implementation found in api.cc.
  // This function will not return, but will terminate the execution.
  [[noreturn]] static void FatalProcessOutOfMemory(Isolate* isolate,
                                                   const char* location,
                                                   bool is_heap_oom = false);

  static void InitializePlatform(v8::Platform* platform);
  static void ShutdownPlatform();
  V8_EXPORT_PRIVATE static v8::Platform* GetCurrentPlatform();
  // Replaces the current platform with the given platform.
  // Should be used only for testing.
  V8_EXPORT_PRIVATE static void SetPlatformForTesting(v8::Platform* platform);

  static void SetSnapshotBlob(StartupData* snapshot_blob);

 private:
  static void InitializeOncePerProcessImpl();
  static void InitializeOncePerProcess();

  // v8::Platform to use.
  static v8::Platform* platform_;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_INIT_V8_H_
