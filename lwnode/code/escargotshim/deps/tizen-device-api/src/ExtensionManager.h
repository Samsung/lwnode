// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_SERVICE_NODE_EXTENSION_MANAGER_H_
#define WRT_SERVICE_NODE_EXTENSION_MANAGER_H_

#include <string>
#include <set>
#include <map>

#include "XW_Extension.h"
#include "XW_Extension_SyncMessage.h"
#include "Extension.h"

namespace wrt {
class RuntimeVariableProvider;
namespace xwalk {

    typedef std::map<std::string, Extension*> ExtensionMap;

    class ExtensionManager {
    public:
        static ExtensionManager* GetInstance();

        void RegisterExtensionsInDirectory(RuntimeVariableProvider* provider);
#if 0
  void RegisterExtensionsByMetadata(RuntimeVariableProvider* provider);
  void RegisterExtensionsByMetadata(RuntimeVariableProvider* provider,
                                    const std::string& metafile_path);
#endif

        ExtensionMap& extensions()
        {
            return extensions_;
        }

        bool RegisterExtension(Extension* extension);

    private:
        ExtensionManager();
        virtual ~ExtensionManager();

        ExtensionMap extensions_;

        std::set<std::string> extension_symbols_;
    };

} // namespace xwalk
} // namespace wrt

#endif // WRT_SERVICE_NODE_EXTENSION_MANAGER_H_
