// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#if defined(TIZEN_DEVICE_API)
#include "escargotbase.h"
#include "TizenDeviceAPILoaderForEscargot.h"
#include "ExtensionAdapter.h"

namespace wrt {
namespace xwalk {

    ExtensionAdapter::ExtensionAdapter()
        : next_xw_extension_(1)
        , next_xw_instance_(1)
    {
    }

    ExtensionAdapter::~ExtensionAdapter()
    {
    }

    ExtensionAdapter* ExtensionAdapter::GetInstance()
    {
        static ExtensionAdapter self;
        return &self;
    }

    XW_Extension ExtensionAdapter::GetNextXWExtension()
    {
        return next_xw_extension_++;
    }

    XW_Instance ExtensionAdapter::GetNextXWInstance()
    {
        return next_xw_instance_++;
    }

    void ExtensionAdapter::RegisterExtension(Extension* extension)
    {
        XW_Extension xw_extension = extension->xw_extension();
        if (!(xw_extension > 0 && xw_extension < next_xw_extension_)) {
            DEVICEAPI_LOG_WARN("xw_extension (%d) is invalid.", xw_extension);
            return;
        }
        if (extension_map_.find(xw_extension) == extension_map_.end())
            extension_map_[xw_extension] = extension;
    }

    void ExtensionAdapter::UnregisterExtension(Extension* extension)
    {
        XW_Extension xw_extension = extension->xw_extension();
        if (!(xw_extension > 0 && xw_extension < next_xw_extension_)) {
            DEVICEAPI_LOG_WARN("xw_extension (%d) is invalid.", xw_extension);
            return;
        }
        if (extension_map_.find(xw_extension) != extension_map_.end())
            extension_map_.erase(xw_extension);
    }

    void ExtensionAdapter::RegisterInstance(ExtensionInstance* instance)
    {
        XW_Instance xw_instance = instance->xw_instance();
        if (!(xw_instance > 0 && xw_instance < next_xw_instance_)) {
            DEVICEAPI_LOG_WARN("xw_instance (%d) is invalid.", xw_instance);
            return;
        }
        if (instance_map_.find(xw_instance) == instance_map_.end())
            instance_map_[xw_instance] = instance;
    }

    void ExtensionAdapter::UnregisterInstance(ExtensionInstance* instance)
    {
        XW_Instance xw_instance = instance->xw_instance();
        if (!(xw_instance > 0 && xw_instance < next_xw_instance_)) {
            DEVICEAPI_LOG_WARN("xw_instance (%d) is invalid.", xw_instance);
            return;
        }
        if (instance_map_.find(xw_instance) != instance_map_.end())
            instance_map_.erase(xw_instance);
    }

    const void* ExtensionAdapter::GetInterface(const char* name)
    {
        if (!strcmp(name, XW_CORE_INTERFACE_1)) {
            static const XW_CoreInterface_1 coreInterface1 = {
                CoreSetExtensionName,          CoreSetJavaScriptAPI,
                CoreRegisterInstanceCallbacks, CoreRegisterShutdownCallback,
                CoreSetInstanceData,           CoreGetInstanceData
            };
            return &coreInterface1;
        }

        if (!strcmp(name, XW_MESSAGING_INTERFACE_1)) {
            static const XW_MessagingInterface_1 messagingInterface1 = {
                MessagingRegister, MessagingPostMessage
            };
            return &messagingInterface1;
        }

        if (!strcmp(name, XW_INTERNAL_SYNC_MESSAGING_INTERFACE_1)) {
            static const XW_Internal_SyncMessagingInterface_1
                syncMessagingInterface1 = { SyncMessagingRegister,
                                            SyncMessagingSetSyncReply };
            return &syncMessagingInterface1;
        }

        if (!strcmp(name, XW_INTERNAL_ENTRY_POINTS_INTERFACE_1)) {
            static const XW_Internal_EntryPointsInterface_1
                entryPointsInterface1 = { EntryPointsSetExtraJSEntryPoints };
            return &entryPointsInterface1;
        }

        if (!strcmp(name, XW_INTERNAL_RUNTIME_INTERFACE_1)) {
            static const XW_Internal_RuntimeInterface_1 runtimeInterface1 = {
                RuntimeGetStringVariable
            };
            return &runtimeInterface1;
        }

        if (!strcmp(name, XW_INTERNAL_PERMISSIONS_INTERFACE_1)) {
            static const XW_Internal_PermissionsInterface_1
                permissionsInterface1 = { PermissionsCheckAPIAccessControl,
                                          PermissionsRegisterPermissions };
            return &permissionsInterface1;
        }

        if (!strcmp(name, XW_INTERNAL_DATA_INTERFACE_1)) {
            static const XW_Internal_DataInterface_1 dataInterface1 = {
                DataRegisterSync, DataRegisterAsync, DataSetSyncReply,
                DataPostData
            };
            return &dataInterface1;
        }

        DEVICEAPI_LOG_WARN("Interface '%s' is not supported.", name);
        return NULL;
    }

    Extension* ExtensionAdapter::GetExtension(XW_Extension xw_extension)
    {
        ExtensionAdapter* adapter = ExtensionAdapter::GetInstance();
        ExtensionMap::iterator it = adapter->extension_map_.find(xw_extension);
        if (it == adapter->extension_map_.end())
            return NULL;
        return it->second;
    }

    ExtensionInstance* ExtensionAdapter::GetExtensionInstance(
        XW_Instance xw_instance)
    {
        ExtensionAdapter* adapter = ExtensionAdapter::GetInstance();
        InstanceMap::iterator it = adapter->instance_map_.find(xw_instance);
        if (it == adapter->instance_map_.end())
            return NULL;
        return it->second;
    }

#define CHECK(x, xw)                                                   \
    if (!x) {                                                          \
        DEVICEAPI_LOG_WARN("Ignoring call. Invalid %s = %d", #xw, xw); \
        return;                                                        \
    }

#define RETURN_IF_INITIALIZED(x) \
    if (x->initialized_)         \
        return;

    void ExtensionAdapter::CoreSetExtensionName(XW_Extension xw_extension,
                                                const char* name)
    {
        Extension* extension = GetExtension(xw_extension);
        CHECK(extension, xw_extension);
        RETURN_IF_INITIALIZED(extension);
        extension->name_ = name;
    }

    void ExtensionAdapter::CoreSetJavaScriptAPI(XW_Extension xw_extension,
                                                const char* javascript_api)
    {
        Extension* extension = GetExtension(xw_extension);
        CHECK(extension, xw_extension);
        RETURN_IF_INITIALIZED(extension);
        extension->javascript_api_ = javascript_api;
    }

    void ExtensionAdapter::CoreRegisterInstanceCallbacks(
        XW_Extension xw_extension, XW_CreatedInstanceCallback created,
        XW_DestroyedInstanceCallback destroyed)
    {
        Extension* extension = GetExtension(xw_extension);
        CHECK(extension, xw_extension);
        RETURN_IF_INITIALIZED(extension);
        extension->created_instance_callback_ = created;
        extension->destroyed_instance_callback_ = destroyed;
    }

    void ExtensionAdapter::CoreRegisterShutdownCallback(
        XW_Extension xw_extension, XW_ShutdownCallback shutdown)
    {
        Extension* extension = GetExtension(xw_extension);
        CHECK(extension, xw_extension);
        RETURN_IF_INITIALIZED(extension);
        extension->shutdown_callback_ = shutdown;
    }

    void ExtensionAdapter::CoreSetInstanceData(XW_Instance xw_instance,
                                               void* data)
    {
        ExtensionInstance* instance = GetExtensionInstance(xw_instance);
        CHECK(instance, xw_instance);
        instance->instance_data_ = data;
    }

    void* ExtensionAdapter::CoreGetInstanceData(XW_Instance xw_instance)
    {
        ExtensionInstance* instance = GetExtensionInstance(xw_instance);
        if (instance)
            return instance->instance_data_;
        else
            return NULL;
    }

    void ExtensionAdapter::MessagingRegister(
        XW_Extension xw_extension, XW_HandleMessageCallback handle_message)
    {
        Extension* extension = GetExtension(xw_extension);
        CHECK(extension, xw_extension);
        RETURN_IF_INITIALIZED(extension);
        extension->handle_msg_callback_ = handle_message;
    }

    void ExtensionAdapter::MessagingPostMessage(XW_Instance xw_instance,
                                                const char* message)
    {
        ExtensionInstance* instance = GetExtensionInstance(xw_instance);
        CHECK(instance, xw_instance);
        instance->PostMessage(message);
    }

    void ExtensionAdapter::SyncMessagingRegister(
        XW_Extension xw_extension,
        XW_HandleSyncMessageCallback handle_sync_message)
    {
        Extension* extension = GetExtension(xw_extension);
        CHECK(extension, xw_extension);
        RETURN_IF_INITIALIZED(extension);
        extension->handle_sync_msg_callback_ = handle_sync_message;
    }

    void ExtensionAdapter::SyncMessagingSetSyncReply(XW_Instance xw_instance,
                                                     const char* reply)
    {
        ExtensionInstance* instance = GetExtensionInstance(xw_instance);
        CHECK(instance, xw_instance);
        instance->SyncReply(reply);
    }

    void ExtensionAdapter::EntryPointsSetExtraJSEntryPoints(
        XW_Extension xw_extension, const char** entry_points)
    {
        Extension* extension = GetExtension(xw_extension);
        CHECK(extension, xw_extension);
        RETURN_IF_INITIALIZED(extension);

        for (int i = 0; entry_points[i]; ++i) {
            extension->entry_points_.push_back(std::string(entry_points[i]));
        }
    }

    void ExtensionAdapter::RuntimeGetStringVariable(XW_Extension xw_extension,
                                                    const char* key,
                                                    char* value,
                                                    unsigned int value_len)
    {
        Extension* extension = GetExtension(xw_extension);
        CHECK(extension, xw_extension);
        extension->GetRuntimeVariable(key, value, value_len);
    }

    int ExtensionAdapter::PermissionsCheckAPIAccessControl(
        XW_Extension xw_extension, const char* api_name)
    {
        Extension* extension = GetExtension(xw_extension);
        if (extension)
            return extension->CheckAPIAccessControl(api_name);
        else
            return XW_ERROR;
    }

    int ExtensionAdapter::PermissionsRegisterPermissions(
        XW_Extension xw_extension, const char* perm_table)
    {
        Extension* extension = GetExtension(xw_extension);
        if (extension)
            return extension->RegisterPermissions(perm_table);
        else
            return XW_ERROR;
    }

    void ExtensionAdapter::DataRegisterSync(
        XW_Extension xw_extension, XW_HandleDataCallback handle_sync_data)
    {
        Extension* extension = GetExtension(xw_extension);
        CHECK(extension, xw_extension);
        RETURN_IF_INITIALIZED(extension);
        extension->handle_sync_data_callback_ = handle_sync_data;
    }

    void ExtensionAdapter::DataRegisterAsync(XW_Extension xw_extension,
                                             XW_HandleDataCallback handle_data)
    {
        Extension* extension = GetExtension(xw_extension);
        CHECK(extension, xw_extension);
        RETURN_IF_INITIALIZED(extension);
        extension->handle_data_callback_ = handle_data;
    }

    void ExtensionAdapter::DataSetSyncReply(XW_Instance xw_instance,
                                            const char* reply, uint8_t* buffer,
                                            size_t len)
    {
        ExtensionInstance* instance = GetExtensionInstance(xw_instance);
        CHECK(instance, xw_instance);
        instance->SyncDataReply(reply, buffer, len);
    }

    void ExtensionAdapter::DataPostData(XW_Instance xw_instance,
                                        const char* message, uint8_t* buffer,
                                        size_t len)
    {
        ExtensionInstance* instance = GetExtensionInstance(xw_instance);
        CHECK(instance, xw_instance);
        instance->PostData(message, buffer, len);
    }

} // namespace xwalk
} // namespace wrt
#endif
