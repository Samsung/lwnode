/*
 * Copyright (c) 2025-present Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <async-uv.h>
#include <channel.h>
#include <message-port.h>
#include <optional>
#include "es-helper.h"
#include "nd-debug.h"
#include "nd-logger.h"
#include "nd-mod-base.h"
#include "nd-vm-message-channel.h"

using namespace Escargot;

#define COUNT_OF(array) (sizeof(array) / sizeof((array)[0]))

#if !defined(LWNODE)
#include "nd-vm.h"

using MessageChannelType = std::shared_ptr<MessageChannel>;

static MessageChannelType GetMessageChannel(ContextRef* context) {
  return VM::Get(context)->message_channel().lock();
}
#else
#include "api/context.h"
#include "api/isolate.h"
#include "lwnode.h"
#include "v8.h"

using MessageChannelType = MessageChannel*;

static MessageChannelType GetMessageChannel(ContextRef* context) {
  EscargotShim::ContextWrap* lwContext =
      EscargotShim::ContextWrap::fromEscargot(context);
  void* channel =
      lwContext->GetAlignedPointerFromEmbedderData(LWNode::kMessageChannel);
  CHECK_NOT_NULL(channel);
  return reinterpret_cast<MessageChannel*>(channel);
}
#endif

static ObjectRef* InstantiateMessageEvent(ExecutionStateRef* state,
                                          const MessageEvent* event) {
  ContextRef* context = state->context();
  GlobalObjectRef* global = context->globalObject();

  // Get MessageEvent
  MessageChannelType message_channel = GetMessageChannel(state->context());
  FunctionObjectRef* klass = message_channel->MessageEventClass();
  if (klass == nullptr) {
#if !defined(LWNODE)
    ValueRef* value = global->get(state, OneByteString("MessageEvent"));
#else
    auto process = global->get(state, OneByteString("process"))->asObject();
    auto lwnode = process->get(state, OneByteString("lwnode"))->asObject();
    ValueRef* value = lwnode->get(state, OneByteString("MessageEvent"));
#endif
    CHECK(value->isFunctionObject());
    klass = value->asFunctionObject();
    message_channel->SetMessageEventClass(klass);
  }

  // Create a new MessageEvent
  auto option = ObjectRef::create(state);
  // TODO: Use atomic string
  option->set(state, OneByteString("data"), OneByteString(event->data()));
  option->set(state, OneByteString("origin"), OneByteString(event->origin()));

  ValueRef* argv[] = {OneByteString("message"), option};
  return klass->construct(state, COUNT_OF(argv), argv)->asObject();
}

// MessagePortWrap
// ---------------------------------------------------------------------------
class MessagePortWrap : public BaseObject {
 public:
  const char* id() override { return "MessagePortWrap"; }

  MessagePortWrap(ObjectRef* object) : BaseObject(object) {
    onmessage_ = ValueRef::createUndefined();
  }
  ~MessagePortWrap() { TRACE(MSGPORT_JS, "DELETE", this); }

  static ValueRef* New(ExecutionStateRef* state,
                       ValueRef* this_value,
                       size_t argc,
                       ValueRef** argv,
                       OptionalRef<ObjectRef> new_target) {
    if (new_target.hasValue()) {
      return CreateWrapper<MessagePortWrap>(this_value);
    }
    return ValueRef::createUndefined();
  }

  static ValueRef* OnMessageGetter(ExecutionStateRef* state,
                                   ValueRef* this_value,
                                   size_t argc,
                                   ValueRef** argv,
                                   OptionalRef<ObjectRef> new_target) {
    auto self = GetExtraData<MessagePortWrap>(this_value);
    return self->onmessage_;
  }

  static ValueRef* OnMessageSetter(ExecutionStateRef* state,
                                   ValueRef* this_value,
                                   size_t argc,
                                   ValueRef** argv,
                                   OptionalRef<ObjectRef> new_target) {
    auto self = GetExtraData<MessagePortWrap>(this_value);

    CHECK(argv[0]->isFunctionObject());
    bool maybe_first_time = self->onmessage_->isUndefinedOrNull();
    TRACE(MSGPORT_JS, "onmessage is registered");
    self->onmessage_ = argv[0]->asFunctionObject();

    if (maybe_first_time) {
      if (auto message_channel = GetMessageChannel(state->context())) {
        message_channel->Start();
      }
    }
    return ValueRef::createUndefined();
  }

  static ValueRef* PostMessage(ExecutionStateRef* state,
                               ValueRef* this_value,
                               size_t argc,
                               ValueRef** argv,
                               OptionalRef<ObjectRef> new_target) {
    TRACE(MSGPORT_JS, "PostMessage");
    if (argc == 0 || !argv[0]->isString()) {
      return ValueRef::createUndefined();
    }

    auto self = GetExtraData<MessagePortWrap>(this_value);
    std::shared_ptr<Port> port = self->port_.lock();
    if (port == nullptr) {
      return ValueRef::createUndefined();
    }
    port->PostMessage(
        MessageEvent::New(argv[0]->asString()->toStdUTF8String()));

    return ValueRef::createUndefined();
  }

  static ObjectRef* Instantiate(ExecutionStateRef* state,
                                std::shared_ptr<Port> port) {
    // TODO: cache template if needed
    FunctionTemplateRef* ftpl = NewFunctionTemplate(0, New);
    ObjectTemplateRef* ptpl = ftpl->prototypeTemplate();
    SetProperty(ptpl, "onmessage", OnMessageGetter, OnMessageSetter);
    SetMethod(ptpl, "postMessage", 0, PostMessage);

    ObjectRef* klass = ftpl->instantiate(state->context());
    ObjectRef* instance = klass->construct(state, 0, nullptr)->asObject();

    // Init Port
    GetExtraData<MessagePortWrap>(instance)->InitPort(state->context(),
                                                      std::move(port));
    return instance->asObject();
  }

  void InitPort(ContextRef* context, std::shared_ptr<Port> port) {
    TRACE(MSGPORT_JS, "port->OnMessage is registered");
    port_ = port;

    // Start listening messages
    port->OnMessage([this, context](const MessageEvent* event) {
      TRACEF(MSGPORT_JS, "OnMessage: %s\n", event->data());

      if (!onmessage_->isFunctionObject()) {
        return;
      }

      ExecResult result =
          Eval::execute(context, [this, event](ExecutionStateRef* state) {
            ObjectRef* event_object = InstantiateMessageEvent(state, event);
            ValueRef* argv[] = {event_object};
            Maybe<ValueRef*> maybe;

#if !defined(LWNODE)
            TryCatchScope scope(state->context());
            maybe = CallFunction(state->context(),
                                 ValueRef::createUndefined(),
                                 onmessage_->asFunctionObject(),
                                 COUNT_OF(argv),
                                 argv);
#else
            auto lwIsolate = EscargotShim::IsolateWrap::GetCurrent();
            v8::HandleScope handle_scope(lwIsolate->toV8());

            TryCatchScope scope(state->context(), false);
            maybe = CallFunction(state->context(),
                                 ValueRef::createUndefined(),
                                 onmessage_->asFunctionObject(),
                                 COUNT_OF(argv),
                                 argv);

            if (scope.HasCaught()) {
              lwIsolate->ScheduleThrow(scope.exception());
            }
#endif
            return maybe.FromMaybe(ValueRef::createUndefined());
          });
    });
  }

 private:
  // TODO: Use WeakPtr for safe
  ValueRef* onmessage_;
  std::weak_ptr<Port> port_;
};

static ObjectRef* Init(ContextRef* context, ObjectRef* target) {
  ExecResult result =
      Eval::execute(context, [&target](ExecutionStateRef* state) {
        // MainMessagePort (port2)
        std::shared_ptr<Port> port2 =
            GetMessageChannel(state->context())->port2();
        SetProperty(state,
                    target,
                    "MainMessagePort",
                    MessagePortWrap::Instantiate(state, std::move(port2)));

        return target;
      });
  return result.checkedValue()->asObject();
}

ObjectRef* ModuleMessagePortInit(ContextRef* context, ObjectRef* target) {
  return Init(context, target);
}
