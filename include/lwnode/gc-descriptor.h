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

#ifndef __GCDescriptor__
#define __GCDescriptor__

#define _TYPED_GC_NEW_ARG1(Class)                                              \
  void* operator new(size_t size) {                                            \
    static bool typeInited = false;                                            \
    static GC_descr descr;                                                     \
    if (!typeInited) {                                                         \
      GC_word desc[GC_BITMAP_SIZE(Class)] = {0};                               \
      Class::fillGCDescriptor(desc);                                           \
      descr = GC_make_descriptor(desc, GC_WORD_LEN(Class));                    \
      typeInited = true;                                                       \
    }                                                                          \
    return GC_MALLOC_EXPLICITLY_TYPED(size, descr);                            \
  }                                                                            \
  void* operator new[](size_t size) = delete;                                  \
                                                                               \
 protected:                                                                    \
  static inline void fillGCDescriptor(GC_word* desc) {
#define _TYPED_GC_NEW_ARG2(Class, Parent)                                      \
  _TYPED_GC_NEW_ARG1(Class)                                                    \
  Parent::fillGCDescriptor(desc);

#define _VA_MACRO(_1, _2, x, ...) x

// After this macro is used, class access specifier is changed to `protected`.
#define BEGIN_IMPLEMENT_TYPED_GC_NEW(...)                                      \
  _VA_MACRO(__VA_ARGS__, _TYPED_GC_NEW_ARG2, _TYPED_GC_NEW_ARG1)               \
  (__VA_ARGS__)

#define SET_GC_POINTER(Class, name)                                            \
  GC_set_bit(desc, GC_WORD_OFFSET(Class, name));

inline void markHashTable(GC_word* desc, size_t base) {
#if defined(COMPILER_MSVC) || defined(COMPILER_CLANG_CL)
  GC_set_bit(desc, base + 2);
  GC_set_bit(desc, base + 5);
#else
  GC_set_bit(desc, base + 1);
  GC_set_bit(desc, base + 4);
#endif
}

#define SET_GC_COLLECTION(Class, name)                                         \
  markHashTable(desc, GC_WORD_OFFSET(Class, name));

#define END_IMPLEMENT_TYPED_GC_NEW() }

#endif
