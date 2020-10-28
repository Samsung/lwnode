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

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_DATA_H_
#define XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_DATA_H_

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_H_
#error "You should include XW_Extension.h before this file"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// XW_INTERNAL_DATA_INTERFACE: allow extensions to exchange binary chunk data
// between extension and loader.
//

#define XW_INTERNAL_DATA_INTERFACE_1 "XW_InternalDataInterface_1"
#define XW_INTERNAL_DATA_INTERFACE XW_INTERNAL_DATA_INTERFACE_1

// Synchronous / Asynchronous data exchanging interface
typedef void (*XW_HandleDataCallback)(XW_Instance instance, const char* message,
                                      uint8_t* buffer, size_t len);

struct XW_Internal_DataInterface_1 {
    void (*RegisterSync)(XW_Extension extension,
                         XW_HandleDataCallback handle_data);

    void (*RegisterAsync)(XW_Extension extension,
                          XW_HandleDataCallback handle_data);

    void (*SetSyncReply)(XW_Instance instance, const char* reply,
                         uint8_t* buffer, size_t len);

    void (*PostData)(XW_Instance instance, const char* message, uint8_t* buffer,
                     size_t len);
};

typedef struct XW_Internal_DataInterface_1 XW_Internal_DataInterface;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_DATA_H_
