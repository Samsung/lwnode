/*
 * Copyright (c) 2017-present Samsung Electronics Co., Ltd
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
