// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TizenDeviceAPIBase.h"
#include "ExtensionManager.h"

#include <glob.h>
// #include <dpl/log/secure_log.h>
#include <memory>
#include <iostream>
#include <fstream>

// #include "runtime_variable_provider.h"
// #include "picojson.h"
#include "TizenDeviceAPILoaderForEscargot.h"

namespace wrt {
namespace xwalk {

    namespace {
        // TODO: need to cleanup
        const char kExtensionDir[] = "/usr/lib/wrt-plugins-widget";
        const char kExtensionPrefix[] = "lib";
        const char kExtensionSuffix[] = ".so";
        const char kExtensionMetadataSuffix[] = ".json";
    }

    ExtensionManager::ExtensionManager()
    {
    }

    ExtensionManager::~ExtensionManager()
    {
    }

    ExtensionManager* ExtensionManager::GetInstance()
    {
        static ExtensionManager self;
        return &self;
    }

#if 0
void ExtensionManager::RegisterExtensionsByMetadata(
    RuntimeVariableProvider* provider, const std::string& metafile_path) {
  DEVICEAPI_SLOG_INFO("path : [%s]", metafile_path.c_str());
  std::ifstream metafile(metafile_path.c_str());
  if (!metafile.is_open()) {
    DEVICEAPI_LOG_ERROR("Can't open plugin metadata file");
    return;
  }

  picojson::value metadata;
  metafile >> metadata;
  if (metadata.is<picojson::array>()) {
    auto& plugins = metadata.get<picojson::array>();
    for (auto plugin = plugins.begin(); plugin != plugins.end(); ++plugin) {
      if (!plugin->is<picojson::object>())
        continue;

      std::string name = plugin->get("name").to_str();
      std::string lib = plugin->get("lib").to_str();
      if (lib.at(0) != '/') {
        lib = std::string(kExtensionDir) + "/" + lib;
      }
      std::vector<std::string> entries;
      auto& entry_points_value = plugin->get("entry_points");
      if (entry_points_value.is<picojson::array>()) {
        auto& entry_points = entry_points_value.get<picojson::array>();
        for (auto entry = entry_points.begin(); entry != entry_points.end();
             ++entry) {
          entries.push_back(entry->to_str());
        }
      } else {
        DEVICEAPI_LOG_ERROR("there is no entry points");
      }
      Extension* extension = new Extension(lib, name, entries, provider);
      RegisterExtension(extension);
    }
  } else {
    DEVICEAPI_LOG_ERROR("Not plugin metadata");
    DEVICEAPI_SLOG_ERROR("%s is not plugin metadata", metafile_path.c_str());
  }
  metafile.close();
}

void ExtensionManager::RegisterExtensionsByMetadata(
    RuntimeVariableProvider* provider) {
  std::string extension_path(kExtensionDir);
  extension_path.append("/");
  extension_path.append("*");
  extension_path.append(kExtensionMetadataSuffix);

  DEVICEAPI_LOG_INFO("Register Extension directory");
  DEVICEAPI_SLOG_INFO("path : [%s]", extension_path.c_str());

  glob_t glob_result;
  glob(extension_path.c_str(), GLOB_TILDE, NULL, &glob_result);
  for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
    RegisterExtensionsByMetadata(provider, glob_result.gl_pathv[i]);
  }
  if (glob_result.gl_pathc == 0) {
    RegisterExtensionsInDirectory(provider);
  }
}

    void ExtensionManager::RegisterExtensionsInDirectory(
        RuntimeVariableProvider* provider)
    {
        std::string extension_path(kExtensionDir);
        extension_path.append("/");
        extension_path.append(kExtensionPrefix);
        extension_path.append("*");
        extension_path.append(kExtensionSuffix);

        DEVICEAPI_LOG_INFO("Register Extension directory");
        DEVICEAPI_SLOG_INFO("path : [%s]", extension_path.c_str());

        glob_t glob_result;
        glob(extension_path.c_str(), GLOB_TILDE, NULL, &glob_result);
        for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
            Extension* extension =
                new Extension(glob_result.gl_pathv[i], provider);
            if (extension->Initialize()) {
                RegisterExtension(extension);
            }
        }
    }
#endif

    bool ExtensionManager::RegisterExtension(Extension* extension)
    {
        DEVICEAPI_LOG_INFO(
            "========== << RegisterExtension >> ENTER ==========");
        if (!extension)
            return false;

        std::string name = extension->name();

        DEVICEAPI_LOG_INFO("Register Extension name : [%s]", name.c_str());
        if (extension_symbols_.find(name) != extension_symbols_.end()) {
            DEVICEAPI_LOG_WARN(
                "Ignoring extension with name already registred. '%s'",
                name.c_str());
            return false;
        }

        std::vector<std::string>& entry_points = extension->entry_points();
        std::vector<std::string>::iterator iter;
        for (iter = entry_points.begin(); iter != entry_points.end(); ++iter) {
            if (extension_symbols_.find(*iter) != extension_symbols_.end()) {
                DEVICEAPI_LOG_WARN(
                    "Ignoring extension with entry_point already registred. "
                    "'%s'",
                    (*iter).c_str());
                return false;
            }
        }

        for (iter = entry_points.begin(); iter != entry_points.end(); ++iter) {
            extension_symbols_.insert(*iter);
        }

        extension_symbols_.insert(name);
        extensions_[name] = extension;

        DEVICEAPI_LOG_INFO("========== << RegisterExtension >> END ==========");
        return true;
    }

    void ExtensionManager::AddRuntimeVariable(const std::string& key, const std::string& value) {
      runtime_variableMap_.insert(std::make_pair(key, value));
    }

    void ExtensionManager::GetRuntimeVariable(const char* key, char* value, size_t value_len) {
      auto it = runtime_variableMap_.find(key);
      if (it != runtime_variableMap_.end()) {
        strncpy(value, it->second.c_str(), value_len);
      }
    }

} // namespace xwalk
} // namespace wrt
