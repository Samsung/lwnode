/*
 * Copyright (c) 2024-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
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
