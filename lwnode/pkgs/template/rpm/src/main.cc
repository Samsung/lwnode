#include <assert.h>

#include "node_api.h"
#include "node_escargot.h"

using namespace node;

napi_value Method(napi_env env, napi_callback_info info) {
  nescargot_printf("call Method\n");
  napi_status status;
  napi_value world;
  status = napi_create_string_utf8(env, "world", 5, &world);
  assert(status == napi_ok);

  return world;
}

#define DECLARE_NAPI_METHOD(name, func) \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

void Init(napi_env env, napi_value exports, napi_value module, void* priv) {
  napi_status status;
  napi_property_descriptor desc = DECLARE_NAPI_METHOD("hello", Method);
  status = napi_define_properties(env, exports, 1, &desc);
  assert(status == napi_ok);
}

NAPI_MODULE(hello, Init)
