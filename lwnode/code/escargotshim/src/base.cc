#include "base.h"

#include "api.h"

using namespace v8;

namespace EscargotShim {

EsScope::EsScope(v8::Isolate* isolate, const v8::Value* self) {
  isolate_ = isolate != nullptr ? IsolateWrap::fromV8(isolate)
                                : IsolateWrap::GetCurrent();
  initSelf(self);
}

EsScope::EsScope(const Local<Context>& context, const v8::Value* self) {
  isolate_ = context.IsEmpty() ? IsolateWrap::GetCurrent()
                               : VAL(*context)->context()->GetIsolate();
  initSelf(self);
}

EsScopeTemplate::EsScopeTemplate(const v8::Template* self) : EsScope() {
  initSelf(self);
}

EsScopeTemplate::EsScopeTemplate(const v8::Local<v8::Context>& context,
                                 const v8::Template* self)
    : EsScope(context, nullptr) {
  initSelf(self);
}

}  // namespace EscargotShim
