/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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

#ifndef ESCARGOT_BASE_H
#define ESCARGOT_BASE_H

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// #include <cstdint>
// #include <queue>
// #include <vector>
// #include <set>
// #include <memory>
// #include <sstream>
// #include <functional>
// #include <algorithm>
// #include <cmath>
// #include <limits>
// #include <locale>
// #include <clocale>
// #include <cwchar>
// #include <numeric>
// #include <stdarg.h>
// #include <atomic>
// #include <condition_variable>
// #include <exception>
// #include <future>
// #include <iostream>
// #include <mutex>
// #include <thread>

#include "base/programoptions.h"

// --------------------------------------------------------------------------------
//                     BASIC MACROS AND XPLATFORM DEFINITIONS
// --------------------------------------------------------------------------------

#define COLOR_RESET "\033[0m"
#define COLOR_DIM "\033[0;2m"
#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_GREY "\033[0;37m"
#define COLOR_BLACK "\033[0;30m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_MAGENTA "\033[0;35m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_DARKGREY "\033[01;30m"
#define COLOR_BRED "\033[01;31m"
#define COLOR_BYELLOW "\033[01;33m"
#define COLOR_BBLUE "\033[01;34m"
#define COLOR_BMAGENTA "\033[01;35m"
#define COLOR_BCYAN "\033[01;36m"
#define COLOR_BGREEN "\033[01;32m"
#define COLOR_WHITE "\033[01;37m"

#define DEFINE_GETTER(Type, MemberName)                                        \
  Type MemberName() const { return m_##MemberName; }

#define DEFINE_SETTER(Type, MemberName, FaceName)                              \
  void set##FaceName(Type value) { m_##MemberName = value; }

#define DEFINE_GETTER_SETTER(Type, MemberName, FaceName)                       \
  DEFINE_GETTER(Type, MemberName)                                              \
  DEFINE_SETTER(Type, MemberName, FaceName)

#define NESCARGOT_CONSOLE_LOG(...)                                             \
  do {                                                                         \
    fprintf(stdout, __VA_ARGS__);                                              \
  } while (0);

// Logs
#define NESCARGOT_LOG_ERROR(fmt, ...)                                          \
  do {                                                                         \
    fprintf(stderr, COLOR_BRED fmt COLOR_RESET, ##__VA_ARGS__);                \
  } while (0);

#if defined(NDEBUG)

#define NESCARGOT_LOG_INFO(...)
#define NESCARGOT_LOG_WARN(fmt, ...)
#define NESCARGOT_CONDITIONAL_LOG(...)
#define NESCARGOT_LOG(...)
#define CHECK_DEBUG_LEVEL(...) (false)

#else

#define NESCARGOT_LOG_INFO(...)                                                \
  do {                                                                         \
    fprintf(stdout, __VA_ARGS__);                                              \
  } while (0);

#define NESCARGOT_LOG_WARN(fmt, ...)                                           \
  do {                                                                         \
    fprintf(stderr, COLOR_YELLOW fmt COLOR_RESET, ##__VA_ARGS__);              \
  } while (0);

#define NESCARGOT_CONDITIONAL_LOG(condition, type, fmt, ...)                   \
  do {                                                                         \
    if (condition) {                                                           \
      NESCARGOT_LOG_##type(fmt, ##__VA_ARGS__);                                \
    }                                                                          \
  } while (0)

#define GET_GLOBAL_OPTIONS(type, key)                                          \
  Starfish::GlobalOptions::instance().get<type>(key)

#define CHECK_DEBUG_LEVEL(level)                                               \
  (GET_GLOBAL_OPTIONS(int, "DEBUG_LEVEL") >= level)

#define NESCARGOT_LOG(level, logtype, fmt, ...)                                \
  NESCARGOT_CONDITIONAL_LOG(                                                   \
      CHECK_DEBUG_LEVEL(level), logtype, fmt, __VA_ARGS__);

#endif

// ASSERT
#if defined(NDEBUG)

#define NESCARGOT_ASSERT(assertion)                                            \
  do {                                                                         \
    if (!(assertion)) {                                                        \
      NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();                                   \
    }                                                                          \
  } while (0)

#else

#define NESCARGOT_ASSERT(assertion) assert(assertion)

#endif  // defined(NDEBUG)

// For Debugging
#if defined(NDEBUG) || defined(NESCARGOT_TEST)

#define NESCARGOT_UNIMPLEMENTED(msg)
#define NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE(msg)

#else

#define NESCARGOT_UNIMPLEMENTED(msg)                                           \
  do {                                                                         \
    NESCARGOT_LOG_INFO(                                                        \
        COLOR_RED                                                              \
        "NESCARGOT_UNIMPLEMENTED (TODO: %s) at %s (%s:%d)\n" COLOR_RESET,      \
        (msg[0] == '\0' ? "UNCERTAIN" : msg),                                  \
        __PRETTY_FUNCTION__,                                                   \
        __FILE__,                                                              \
        __LINE__);                                                             \
    assert(true);                                                              \
  } while (0)

#define NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE(msg)                             \
  do {                                                                         \
    NESCARGOT_LOG_INFO(COLOR_YELLOW                                            \
                       "NESCARGOT_UNIMPLEMENTED_BUT_TOLERABLE (TODO: %s) at "  \
                       "%s (%s:%d)\n" COLOR_RESET,                             \
                       (msg[0] == '\0' ? "UNCERTAIN" : msg),                   \
                       __PRETTY_FUNCTION__,                                    \
                       __FILE__,                                               \
                       __LINE__);                                              \
  } while (0)

#endif

#define NESCARGOT_FATAL_ERROR()                                                \
  do {                                                                         \
    NESCARGOT_LOG_ERROR("NESCARGOT_FATAL_ERROR at %s (%s:%d)\n",               \
                        __PRETTY_FUNCTION__,                                   \
                        __FILE__,                                              \
                        __LINE__);                                             \
    ::abort();                                                                 \
  } while (0)

#define NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE()                                  \
  do {                                                                         \
    NESCARGOT_LOG_ERROR("NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE at %s (%d)\n",    \
                        __FILE__,                                              \
                        __LINE__);                                             \
    ::abort();                                                                 \
  } while (0)

#ifndef _countof
template <typename T, size_t N>
inline size_t _countof(T (&)[N]) {
  return N;
}
#endif

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#ifndef _WIN32
#define strnicmp strncasecmp
#define __debugbreak __builtin_trap
#endif

#ifndef __APPLE__
#if defined(_MSC_VER) && _MSC_VER <= 1800  // VS2013?
#define THREAD_LOCAL __declspec(thread)
#else  // VS2015+, linux Clang etc.
#define THREAD_LOCAL thread_local
#endif  // VS2013?
#else   // __APPLE__
#define THREAD_LOCAL _Thread_local
#endif

#ifdef __cplusplus
#define JS_INVALID_REFERENCE nullptr
#else
#define JS_INVALID_REFERENCE 0
#endif

// Use the definitions from Escargot
// #define LIKELY(x) __builtin_expect((x), 1)
// #define UNLIKELY(x) __builtin_expect((x), 0)

template <typename Target, typename Source>
inline Target Downcast(Source* source) {
#if !defined(NDEBUG)
  NESCARGOT_ASSERT(source != nullptr);
  typedef typename std::remove_pointer<Target>::type TargetType;
  static_assert(std::is_base_of<Source, TargetType>::value == true,
                "Wrong type cast");
#endif

#if !defined(NDEBUG) && (defined(__GXX_RTTI) || defined(_CPPRTTI))
  auto casted = dynamic_cast<Target>(source);
  NESCARGOT_ASSERT(casted != nullptr);
  return casted;
#else
  return static_cast<Target>(source);
#endif
}

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef __APPLE__
inline int isnan(double x) {
  return std::isnan(x);
}
inline int isfinite(double x) {
  return std::isfinite(x);
}
inline bool isinf(double x) {
  return std::isinf(x);
}
#elif defined(__linux__)
#include <cmath>
using std::isfinite;
using std::isinf;
using std::isnan;
#endif

#define C_STR(valueRef) valueRef->asString()->toStdUTF8String().c_str()

// --------------------------------------------------------------------------------
//                                GC CONTAINERS
// --------------------------------------------------------------------------------

#include "GCUtil.h"
#include "base/vector.h"

// typedef of GC-aware vector
template <typename T, typename Allocator = GCUtil::gc_malloc_allocator<T>>
using GCVectorT = Starfish::Vector<T, Allocator>;

template <typename T, typename Allocator = GCUtil::gc_malloc_allocator<T>>
class GCVector : public GCVectorT<T, Allocator>, public gc {};

// typedef of GC-aware vector with atomic contents
template <typename T,
          typename Allocator = GCUtil::gc_malloc_atomic_allocator<T>>
using GCAtomicVectorT = Starfish::Vector<T, Allocator>;

template <typename T,
          typename Allocator = GCUtil::gc_malloc_atomic_allocator<T>>
class GCAtomicVector : public GCAtomicVectorT<T, Allocator>, public gc {};

// typedef of GC-aware list
template <typename T, typename Allocator = GCUtil::gc_malloc_allocator<T>>
using GCListT = std::list<T, Allocator>;

template <typename T, typename Allocator = GCUtil::gc_malloc_allocator<T>>
class GCList : public GCListT<T, Allocator>, public gc {};

// typedef of GC-aware deque
template <typename T, typename Allocator = GCUtil::gc_malloc_allocator<T>>
using GCDequeT = std::deque<T, Allocator>;

template <typename T, typename Allocator = GCUtil::gc_malloc_allocator<T>>
class GCDeque : public GCDequeT<T, Allocator>, public gc {};

// typedef of GC-aware unordered_map
template <typename Key,
          typename Value,
          typename Hasher = std::hash<Key>,
          typename Predicate = std::equal_to<Key>,
          typename Allocator =
              GCUtil::gc_malloc_allocator<std::pair<Key const, Value>>>
using GCUnorderedMapT =
    std::unordered_map<Key, Value, Hasher, Predicate, Allocator>;

template <typename Key,
          typename Value,
          typename Hasher = std::hash<Key>,
          typename Predicate = std::equal_to<Key>,
          typename Allocator =
              GCUtil::gc_malloc_allocator<std::pair<Key const, Value>>>
class GCUnorderedMap
    : public GCUnorderedMapT<Key, Value, Hasher, Predicate, Allocator>,
      public gc {};

template <typename Key,
          typename Value,
          typename Hasher = std::hash<Key>,
          typename Predicate = std::equal_to<Key>,
          typename Allocator =
              GCUtil::gc_malloc_allocator<std::pair<Key const, Value>>>
using GCUnorderedMultiMapT =
    std::unordered_multimap<Key, Value, Hasher, Predicate, Allocator>;

template <typename Key,
          typename Value,
          typename Hasher = std::hash<Key>,
          typename Predicate = std::equal_to<Key>,
          typename Allocator =
              GCUtil::gc_malloc_allocator<std::pair<Key const, Value>>>
class GCUnorderedMultiMap
    : public GCUnorderedMultiMapT<Key, Value, Hasher, Predicate, Allocator>,
      public gc {};

// typedef of GC-aware map
template <typename Key,
          typename Value,
          typename Comparator,
          typename Allocator =
              GCUtil::gc_malloc_allocator<std::pair<Key const, Value>>>
using GCMapT = std::map<Key, Value, Comparator, Allocator>;

template <typename Key,
          typename Value,
          typename Comparator,
          typename Allocator =
              GCUtil::gc_malloc_allocator<std::pair<Key const, Value>>>
class GCMap : public GCMapT<Key, Value, Comparator, Allocator>, public gc {};

// typedef of GC-aware unordered_set
template <typename T,
          typename Hasher = std::hash<T>,
          typename Predicate = std::equal_to<T>,
          typename Allocator = GCUtil::gc_malloc_allocator<T>>
using GCUnorderedSetT = std::unordered_set<T, Hasher, Predicate, Allocator>;

template <typename T,
          typename Hasher = std::hash<T>,
          typename Predicate = std::equal_to<T>,
          typename Allocator = GCUtil::gc_malloc_allocator<T>>
class GCUnorderedSet : public GCUnorderedSetT<T, Hasher, Predicate, Allocator>,
                       public gc {};

#endif
