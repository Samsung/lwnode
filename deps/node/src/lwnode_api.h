#ifndef LWNODE_NAPI_H_
#define LWNODE_NAPI_H_

#ifdef LWNODE

namespace v8 {
  class Context;
}

typedef v8::Context* napi_context;

EXTERN_C_START

NAPI_EXTERN napi_status napi_get_context(napi_env env, napi_context& context);

EXTERN_C_END

#endif
#endif
