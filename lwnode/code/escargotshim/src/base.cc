#include "base.h"

#include "api.h"

using namespace v8;

namespace EscargotShim {

EsScopeTemplate::EsScopeTemplate(const v8::Template* self) {
  isolate_ = IsolateWrap::GetCurrent();
  if (self) {
    self_ = CVAL(self)->tpl();
  }
}

EsScopeTemplate::EsScopeTemplate(const v8::Local<v8::Context>& context,
                                 const v8::Template* self) {
  isolate_ = context.IsEmpty() ? IsolateWrap::GetCurrent()
                               : VAL(*context)->context()->GetIsolate();
  context_ = isolate_->GetCurrentContext();
  if (self) {
    self_ = CVAL(self)->tpl();
  }
}

}  // namespace EscargotShim
