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

#ifndef __ESCARGOT_REGISTRY__
#define __ESCARGOT_REGISTRY__

#include "escargotbase.h"

template <typename T>
constexpr typename std::underlying_type<T>::type toUnderlyingType(T value) {
  return static_cast<typename std::underlying_type<T>::type>(value);
}

template <typename enum_t, typename value_t>
class Registry : public gc {
 public:
  void set(enum_t key, value_t value) {
    const auto& it = m_registry.find(key);
    if (it == m_registry.end()) {
      m_registry.insert(std::make_pair(key, value));
    }
  }

  value_t get(enum_t key) {
    const auto& it = m_registry.find(key);
    if (it == m_registry.end()) {
      NESCARGOT_LOG_ERROR("Unregistered key is requested\n");
      NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();
      return nullptr;
    }
    return it->second;
  }

  void iterate(
      std::function<bool(enum_t const& key, value_t const& value)> iterator) {
    auto it = m_registry.begin();
    while (it != m_registry.end()) {
      if (iterator(it->first, it->second) == false) {
        return;
      }
      it++;
    }
  }

  struct IdHash {
    std::size_t operator()(const enum_t& key) const {
      return toUnderlyingType(key);
    }
  };

 private:
  GCUnorderedMap<enum_t, value_t, IdHash> m_registry;
};

#endif
