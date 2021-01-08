#pragma once

#include <EscargotPublic.h>
#include <v8.h>

#include "utils/gc.h"

namespace EscargotShim {

class Isolate : public gc {
 public:
  virtual ~Isolate();

  static Isolate* New();
  void Initialize(const v8::Isolate::CreateParams& params);

  // Cast functions
  static v8::Isolate* toV8(Isolate* iso) {
    return reinterpret_cast<v8::Isolate*>(iso);
  }
  static Isolate* fromV8(v8::Isolate* iso) {
    return reinterpret_cast<Isolate*>(iso);
  }
  static Isolate* fromV8(v8::internal::Isolate* iso) {
    return reinterpret_cast<Isolate*>(iso);
  }

  // V8 internals
  void set_array_buffer_allocator_shared(
      std::shared_ptr<v8::ArrayBuffer::Allocator> allocator) {
    m_array_buffer_allocator_shared = std::move(allocator);
  }

  void set_array_buffer_allocator(v8::ArrayBuffer::Allocator* allocator) {
    m_array_buffer_allocator = allocator;
  }
  v8::ArrayBuffer::Allocator* array_buffer_allocator() const {
    return m_array_buffer_allocator;
  }

 private:
  Isolate();

  v8::ArrayBuffer::Allocator* m_array_buffer_allocator = nullptr;
  std::shared_ptr<v8::ArrayBuffer::Allocator> m_array_buffer_allocator_shared;
};

}  // namespace EscargotShim
