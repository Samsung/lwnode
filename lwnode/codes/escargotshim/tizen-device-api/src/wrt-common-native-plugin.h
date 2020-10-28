/*
 * Copyright (c) 2017-present Samsung Electronics Co., Ltd
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

#ifndef WRT_COMMON_NATIVE_PLUGIN_H_
#define WRT_COMMON_NATIVE_PLUGIN_H_

#include <string>

namespace wrt {
namespace common {

    class NativePlugin {
    public:
        virtual void OnLoad() = 0;
        virtual std::string OnCallSync(std::string& data) = 0;
        virtual std::string OnCall(std::string& data, int callback_handle) = 0;
    };

    typedef NativePlugin* create_native_plugin_t();

} // namespace common
} // namespace wrt

#define EXPORT_NATIVE_PLUGIN(pluginClass)                        \
    extern "C" wrt::common::NativePlugin* create_native_plugin() \
    {                                                            \
        return (wrt::common::NativePlugin*)(new pluginClass());  \
    }

#endif // WRT_COMMON_NATIVE_PLUGIN_H_
