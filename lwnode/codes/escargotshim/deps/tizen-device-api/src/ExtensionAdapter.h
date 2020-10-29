// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_SERVICE_NODE_EXTENSION_ADAPTER_H_
#define WRT_SERVICE_NODE_EXTENSION_ADAPTER_H_

#include <map>

#include "XW_Extension.h"
#include "XW_Extension_SyncMessage.h"
#include "XW_Extension_EntryPoints.h"
#include "XW_Extension_Runtime.h"
#include "XW_Extension_Permissions.h"
#include "XW_Extension_Data.h"

#include "Extension.h"

namespace wrt {
namespace xwalk {

    class ExtensionAdapter {
    public:
        static ExtensionAdapter* GetInstance();

        XW_Extension GetNextXWExtension();
        XW_Instance GetNextXWInstance();

        void RegisterExtension(Extension* extension);
        void UnregisterExtension(Extension* extension);

        void RegisterInstance(ExtensionInstance* instance);
        void UnregisterInstance(ExtensionInstance* instance);

        // Returns the correct struct according to interface asked. This is
        // passed to external extensions in XW_Initialize() call.
        static const void* GetInterface(const char* name);

        static Extension* GetExtension(XW_Extension xw_extension);
        static ExtensionInstance* GetExtensionInstance(XW_Instance xw_instance);

    private:
        ExtensionAdapter();
        virtual ~ExtensionAdapter();

        static void CoreSetExtensionName(XW_Extension xw_extension,
                                         const char* name);
        static void CoreSetJavaScriptAPI(XW_Extension xw_extension,
                                         const char* javascript_api);
        static void CoreRegisterInstanceCallbacks(
            XW_Extension xw_extension, XW_CreatedInstanceCallback created,
            XW_DestroyedInstanceCallback destroyed);
        static void CoreRegisterShutdownCallback(XW_Extension xw_extension,
                                                 XW_ShutdownCallback shutdown);
        static void CoreSetInstanceData(XW_Instance xw_instance, void* data);
        static void* CoreGetInstanceData(XW_Instance xw_instance);
        static void MessagingRegister(XW_Extension xw_extension,
                                      XW_HandleMessageCallback handle_message);
        static void MessagingPostMessage(XW_Instance xw_instance,
                                         const char* message);
        static void SyncMessagingRegister(
            XW_Extension xw_extension,
            XW_HandleSyncMessageCallback handle_sync_message);
        static void SyncMessagingSetSyncReply(XW_Instance xw_instance,
                                              const char* reply);
        static void EntryPointsSetExtraJSEntryPoints(XW_Extension xw_extension,
                                                     const char** entry_points);
        static void RuntimeGetStringVariable(XW_Extension xw_extension,
                                             const char* key, char* value,
                                             unsigned int value_len);
        static int PermissionsCheckAPIAccessControl(XW_Extension xw_extension,
                                                    const char* api_name);
        static int PermissionsRegisterPermissions(XW_Extension xw_extension,
                                                  const char* perm_table);
        static void DataRegisterSync(XW_Extension xw_extension,
                                     XW_HandleDataCallback handle_sync_data);
        static void DataRegisterAsync(XW_Extension xw_extension,
                                      XW_HandleDataCallback handle_data);
        static void DataSetSyncReply(XW_Instance instance, const char* reply,
                                     uint8_t* buffer, size_t len);
        static void DataPostData(XW_Instance instance, const char* message,
                                 uint8_t* buffer, size_t len);

        typedef std::map<XW_Extension, Extension*> ExtensionMap;
        ExtensionMap extension_map_;

        typedef std::map<XW_Instance, ExtensionInstance*> InstanceMap;
        InstanceMap instance_map_;

        XW_Extension next_xw_extension_;
        XW_Instance next_xw_instance_;
    };

} // namespace xwalk
} // namespace wrt

#endif // WRT_SERVICE_NODE_EXTENSION_ADAPTER_H_
