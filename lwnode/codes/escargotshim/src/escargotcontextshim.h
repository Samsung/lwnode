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

#ifndef __ESCARGOT_CONTEXTSHIM__
#define __ESCARGOT_CONTEXTSHIM__

#include "escargotbase.h"
#include "escargotshim.h"

namespace v8 {
class Context;
}  // namespace v8

namespace Escargot {
class VMInstanceRef;
}  // namespace Escargot

namespace DeviceAPI {
class ExtensionManagerInstance;
}

namespace EscargotShim {
class JsKeepAliveWrapper;

class ContextShim : public gc {
 public:
  ContextShim(EscargotShim::IsolateShim* isolateShim,
              bool exposeGC,
              JsObjectRef globalObjectTemplateInstance);

  static ContextShim* ToContextShim(v8::Context* context);
  v8::Context* asContext();

  JsContextRef contextRef() { return m_context; }
  EscargotShim::IsolateShim* isolateShim() { return m_isolateShim; }

  void* GetAlignedPointerFromEmbedderData(int index);
  void SetAlignedPointerInEmbedderData(int index, void* value);

  JsKeepAliveWrapper* GetKeepAliveWrapper();
  JsObjectRef GetProxyOfGlobal();

 private:
  bool initializeBuiltIns();
  bool executeEscargotShimJS();
  bool checkConfigGlobalObjectTemplate();

  JsKeepAliveWrapper* m_keepAliveWrapper{JS_INVALID_REFERENCE};
  JsObjectRef m_proxyOfGlobal{JS_INVALID_REFERENCE};

  JsContextRef m_context{nullptr};
  EscargotShim::IsolateShim* m_isolateShim{nullptr};

  GCVector<void*> m_embedderData;
  JsObjectRef m_globalObjectTemplateInstance{nullptr};

#if defined(HOST_TIZEN)
  DeviceAPI::ExtensionManagerInstance* m_tizenDeviceManager{nullptr};
#endif
};

}  // namespace EscargotShim

#endif
