/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
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

#if !defined(NDEBUG)

#include "cctest.h"

#include <EscargotPublic.h>
#include "api/context.h"
#include "api/handlescope.h"
#include "api/isolate.h"
#include "internal-api.h"

#include <fstream>
#include <string>
#include "api/es-helper.h"
#include "api/utils/gc-container.h"
#include "lwnode.h"

using namespace Escargot;
using namespace EscargotShim;
using namespace LWNode;

class Count {
 public:
  ~Count() {
    if (count_ != 0) {
      LWNODE_DLOG_ERROR("Count is: %d", count_);
    }
  }
  void add() { ++count_; }
  void sub() { --count_; }
  void reset() { count_ = 0; }
  int get() { return count_; }

 private:
  int count_ = 0;
};

struct ObjectBasic : public gc {};

struct HandleScope : public gc {
 public:
  void add(ObjectBasic* value) {
    LWNODE_CALL_TRACE("%p", value);
    handles_.push_back(value);
  }

  bool remove(ObjectBasic* value) {
    auto it = std::find(handles_.begin(), handles_.end(), value);
    if (it != handles_.end()) {
      handles_.erase(it);
      return true;
    }
    return false;
  }

  void clear() {
    LWNODE_CALL_TRACE();

    for (auto it = handles_.begin(); it != handles_.end(); it++) {
      LWNODE_DLOG_INFO("%p", *it);
    }

    handles_.clear();
  }

  ObjectBasic* get(size_t idx) {
    LWNODE_CHECK(idx < handles_.size());
    return handles_[idx];
  }

 private:
  GCVector<ObjectBasic*, true> handles_;
};

struct Object : public gc {
  GCVector<HandleScope*, true> handleScopes;
};

// --- Test Start ---

static GCTracer g_tracer;
#define GC_TRACE_RESET() g_tracer.reset()
#define GC_TRACE_ADD_POINTER(value) g_tracer.add(value, #value)
#define GC_TRACE_GET_ALIVE_COUNT() g_tracer.getAllocatedCount()
#define GC_TRACE_PRINT_STATE()                                                 \
  if (EscargotShim::Flags::isTraceGCEnabled()) {                               \
    g_tracer.printState();                                                     \
  }

UNINITIALIZED_TEST(internal_GCObject1) {
  GC_TRACE_RESET();

  []() {
    Object* object = new Object();
    GC_TRACE_ADD_POINTER(object);
  }();

  CcTest::CollectAllGarbage();

  CHECK_EQ(GC_TRACE_GET_ALIVE_COUNT(), 0);
}

UNINITIALIZED_TEST(internal_GCObject2) {
  GC_TRACE_RESET();

  []() {
    auto scope = new HandleScope();
    auto basic1 = new ObjectBasic();
    auto basic2 = new ObjectBasic();

    scope->add(basic1);
    scope->add(basic2);

    GC_TRACE_ADD_POINTER(basic1);
    GC_TRACE_ADD_POINTER(basic2);

    CHECK_EQ(GC_TRACE_GET_ALIVE_COUNT(), 2);
  }();

  CcTest::CollectAllGarbage();

  CHECK_EQ(GC_TRACE_GET_ALIVE_COUNT(), 0);
}

UNINITIALIZED_TEST(internal_GCObject3) {
  GC_TRACE_RESET();

  []() {
    auto object1 = new Object();
    auto object2 = new ObjectBasic();
    auto handleScope1 = new HandleScope();
    auto handleScope2 = new HandleScope();

    GC_TRACE_ADD_POINTER(handleScope2);
    GC_TRACE_ADD_POINTER(object2);
    CHECK_EQ(GC_TRACE_GET_ALIVE_COUNT(), 2);

    object1->handleScopes.push_back(handleScope1);
    object1->handleScopes.push_back(handleScope2);
    handleScope2->add(object2);

    GC_TRACE_PRINT_STATE();
  }();

  CcTest::CollectAllGarbage();

  GC_TRACE_PRINT_STATE();

  // object1 isn't hold, everything can be released
  EXPECT_EQ(g_tracer.getAllocatedCount(), 0);
}

UNINITIALIZED_TEST(DISABLED_internal_GCObject4) {
  GC_TRACE_RESET();

  Escargot::PersistentRefHolder<Object> holder;

  [&holder]() {
    auto object1 = new Object();
    auto object2 = new ObjectBasic();
    auto handleScope1 = new HandleScope();
    auto handleScope2 = new HandleScope();

    GC_TRACE_ADD_POINTER(object1);
    GC_TRACE_ADD_POINTER(object2);
    GC_TRACE_ADD_POINTER(handleScope2);

    EXPECT_EQ(GC_TRACE_GET_ALIVE_COUNT(), 3);

    holder.reset(object1);

    object1->handleScopes.push_back(handleScope1);
    object1->handleScopes.push_back(handleScope2);
    handleScope2->add(object2);

    EXPECT_EQ(object1->handleScopes.back(), handleScope2);

    object1->handleScopes.pop_back();
  }();

  CcTest::CollectGarbage();

  GC_TRACE_PRINT_STATE();

  // the count may be expected as 1, but it's not guarantee that
  // gc runs the deallocation for all.
  ASSERT_LE(GC_TRACE_GET_ALIVE_COUNT(), 2);

  holder.release();

  CcTest::CollectGarbage();

  // Although holder's released, it's not guarantee that gc runs the
  // but it's not guarantee that gc runs the deallocation for all.
  EXPECT_LE(GC_TRACE_GET_ALIVE_COUNT(), 1);
}

UNINITIALIZED_TEST(internal_GCContainer) {
  g_tracer.reset();

  Escargot::PersistentRefHolder<GCContainer<ObjectBasic*>> holder;

  [&holder]() {
    auto obj = new ObjectBasic();
    GCContainer<ObjectBasic*>* vector =
        new GCContainer<ObjectBasic*>(1, 1, obj);

    holder.reset(vector);

    g_tracer.add(vector, "vector");
    g_tracer.add(obj, "obj");

    CcTest::CollectGarbage();

    CHECK_EQ(g_tracer.getAllocatedCount(), 2);
  }();

  holder.release();

  CcTest::CollectGarbage();

  CHECK_LE(g_tracer.getAllocatedCount(), 1);
}

static ValueRef* compileRun(ContextRef* context, const char* source) {
  auto r = EvalResultHelper::compileRun(context, source);
  LWNODE_CHECK(r.isSuccessful());
  return r.result;
}

static int shadow_y_setter_call_count;
static int shadow_y_getter_call_count;

TEST(internal_Escargot_ShadowObject) {
  LocalContext env;
  auto esContext =
      IsolateWrap::fromV8(env->GetIsolate())->GetCurrentContext()->get();

  EvalResultHelper::attachBuiltinPrint(esContext, esContext->globalObject());

  auto esFunctionTemplate = FunctionTemplateRef::create(
      AtomicStringRef::emptyAtomicString(),
      0,
      false,
      true,  // isConstruction
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         OptionalRef<ObjectRef> newTarget) -> ValueRef* {
        Escargot::OptionalRef<Escargot::FunctionObjectRef> callee =
            state->resolveCallee();

        // newTarget has value when this function called as constructor.
        if (newTarget.hasValue()) {
          return thisValue;
        }

        return ValueRef::createUndefined();
      });

  auto esInstanceTemplate = esFunctionTemplate->instanceTemplate();
  auto esPrototypeTemplate = esFunctionTemplate->prototypeTemplate();

  // SetHandler (setNamedPropertyHandler) on esInstanceTemplate
  ObjectTemplateNamedPropertyHandlerData esNamedPropertyHandlerData;
  esNamedPropertyHandlerData.getter =
      [](ExecutionStateRef* state,
         ObjectRef* self,
         ValueRef* receiver,
         void* data,
         const TemplatePropertyNameRef& propertyName) -> OptionalRef<ValueRef> {
    return Escargot::OptionalRef<Escargot::ValueRef>();
  };

  esNamedPropertyHandlerData.setter =
      [](ExecutionStateRef* state,
         ObjectRef* self,
         ValueRef* receiver,
         void* data,
         const TemplatePropertyNameRef& propertyName,
         ValueRef* value) -> OptionalRef<ValueRef> {
    return Escargot::OptionalRef<Escargot::ValueRef>();
  };

  esInstanceTemplate->setNamedPropertyHandler(esNamedPropertyHandlerData);

  // Set Getter/Setter for "y" on esInstanceTemplate
#ifdef SET_Y_AS_DATA_PROPERTY
  auto getter =
      [](ExecutionStateRef* state,
         ObjectRef* self,
         ObjectRef::NativeDataAccessorPropertyData* data) -> ValueRef* {
    shadow_y_getter_call_count++;
    return ValueRef::create(42);
  };

  auto setter = [](ExecutionStateRef* state,
                   ObjectRef* self,
                   ObjectRef::NativeDataAccessorPropertyData* data,
                   ValueRef* setterInputData) -> bool {
    shadow_y_setter_call_count++;
    return true;
  };

  struct AccessorPropertyData
      : public ObjectRef::NativeDataAccessorPropertyData {
    AccessorPropertyData(bool isWritable,
                         bool isEnumerable,
                         bool isConfigurable,
                         ObjectRef::NativeDataAccessorPropertyGetter getter,
                         ObjectRef::NativeDataAccessorPropertySetter setter)
        : ObjectRef::NativeDataAccessorPropertyData(
              isWritable, isEnumerable, isConfigurable, getter, setter){};

    void* operator new(size_t size) { return GC_MALLOC(size); }
  };

  auto accessorPropData =
      new AccessorPropertyData(true, true, true, getter, setter);
  esInstanceTemplate->setNativeDataAccessorProperty(
      StringRef::createFromUTF8("y"), accessorPropData);
#else
  auto getter = FunctionTemplateRef::create(
      AtomicStringRef::emptyAtomicString(),
      0,
      false,
      false,
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         OptionalRef<ObjectRef> newTarget) -> ValueRef* {
        shadow_y_getter_call_count++;
        return ValueRef::create(42);
      });

  auto setter = FunctionTemplateRef::create(
      AtomicStringRef::emptyAtomicString(),
      0,
      false,
      false,
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         OptionalRef<ObjectRef> newTarget) -> ValueRef* {
        shadow_y_setter_call_count++;
        return ValueRef::createUndefined();
      });

  esInstanceTemplate->setAccessorProperty(
      StringRef::createFromUTF8("y"), getter, setter, true, true);
#endif

  // instantiate (v8::FunctionTemplate::GetFunction)
  ObjectRef* esFunction = esFunctionTemplate->instantiate(esContext);

  // construct (v8::ObjectTemplate::NewInstance)
  std::vector<ValueRef*> arguments;
  auto r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* state,
         ObjectRef* self,
         size_t argc,
         ValueRef** argv) -> ValueRef* {
        return self->construct(state, argc, argv);
      },
      esFunction,
      arguments.size(),
      arguments.data());

  LWNODE_CHECK(r.isSuccessful());

  auto esInstance = r.result->asObject();

  // set esInstance to global.__proto__
  ObjectRefHelper::setProperty(esContext,
                               esContext->globalObject(),
                               StringRef::createFromUTF8("__proto__"),
                               esInstance);

  // {"enumerable":true,"configurable":true}
  compileRun(esContext,
             "print(JSON.stringify(Object.getOwnPropertyDescriptor(this.__"
             "proto__, \"y\")))");

  compileRun(esContext, "y = 43");
  CHECK_EQ(1, shadow_y_setter_call_count);
  CHECK_EQ(42, compileRun(esContext, "y")->asInt32());
  CHECK_EQ(1, shadow_y_getter_call_count);
}

TEST(internal_Escargot_Extends) {
  LocalContext env;
  auto esContext =
      IsolateWrap::fromV8(env->GetIsolate())->GetCurrentContext()->get();

  EvalResultHelper::attachBuiltinPrint(esContext, esContext->globalObject());

  auto esFunctionTemplate = FunctionTemplateRef::create(
      AtomicStringRef::create(esContext, "ContextifyScript"),
      0,
      false,
      true,  // isConstruction
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         OptionalRef<ObjectRef> newTarget) -> ValueRef* {
        if (newTarget.hasValue()) {
          return thisValue;
        }

        return ValueRef::createUndefined();
      });

  auto esPrototypeTemplate = esFunctionTemplate->prototypeTemplate();

  esPrototypeTemplate->set(
      TemplatePropertyNameRef(StringRef::createFromUTF8("RunInThisContext")),
      FunctionTemplateRef::create(
          AtomicStringRef::emptyAtomicString(),
          0,
          false,
          false,  // isConstruction
          [](ExecutionStateRef* state,
             ValueRef* thisValue,
             size_t argc,
             ValueRef** argv,
             OptionalRef<ObjectRef> newTarget) -> ValueRef* {
            if (newTarget.hasValue()) {
              return thisValue;
            }

            printf("ContextifyScript::RunInThisContext [NATIVE]\n");

            return ValueRef::createUndefined();
          }),
      true,
      true,
      true);

  ObjectRefHelper::setProperty(esContext,
                               esContext->globalObject(),
                               StringRef::createFromUTF8("ContextifyScript"),
                               esFunctionTemplate->instantiate(esContext));

  const char* source =
      R"(
        function assert(condition, message) {
            if (!condition) {
              throw new Error(message || 'Assertion failed');
            }
        }

        function test() {
          print("[ContextifyScript]");
          let parent = new ContextifyScript();
          parent.RunInThisContext();

          class Script extends ContextifyScript {
              constructor() {
                  super();
              }

              RunInThisContext() {
                  print('Script::RunInThisContext [JS]');

                  super.RunInThisContext();
              }

              Test() {}
          };

          print("[Script]");
          let child = new Script();
          child.RunInThisContext();

          print("child instance of Script: " + (child instanceof Script));

          print(Object.getOwnPropertyDescriptor(child.__proto__, 'Test'));
          print(Object.getOwnPropertyDescriptor(child.__proto__, 'RunInThisContext'));
          print(Object.getOwnPropertyDescriptor(child.__proto__.__proto__, 'RunInThisContext'));

          assert((child instanceof Script), 'should be an instance of Script');

          assert(Object.getOwnPropertyDescriptor(child.__proto__, "Test"), 'Empty descriptor');
          assert(Object.getOwnPropertyDescriptor(child.__proto__, "RunInThisContext"), 'Empty descriptor');
          assert(Object.getOwnPropertyDescriptor(child.__proto__.__proto__, "RunInThisContext"), 'Empty descriptor');
        }

        test();
      )";

  auto r = EvalResultHelper::compileRun(esContext, source);

  if (!r.isSuccessful()) {
    printf("%s\n", EvalResultHelper::getErrorString(esContext, r).c_str());
  }

  CHECK_EQ(r.isSuccessful(), true);
}

TEST(internal_Escargot_toIndex32_Regression) {
  LocalContext env;
  auto esContext =
      IsolateWrap::fromV8(env->GetIsolate())->GetCurrentContext()->get();

  auto esValue = ValueRef::create(-1);
  uint32_t index = ValueRef::InvalidIndex32Value;
  auto r = Evaluator::execute(
      esContext,
      [](ExecutionStateRef* esState, ValueRef* self, uint32_t* index) {
        *index = self->toIndex32(esState);
        return ValueRef::create(*index);
      },
      esValue,
      &index);

  CHECK_EQ(ValueRef::InvalidIndex32Value, index);
}

TEST(internal_Escargot_MapUsingIntegerKey) {
  LocalContext env;
  auto esContext =
      IsolateWrap::fromV8(env->GetIsolate())->GetCurrentContext()->get();

  EvalResultHelper::attachBuiltinPrint(esContext, esContext->globalObject());

  auto r = Evaluator::execute(
      esContext, [](ExecutionStateRef* esState) -> ValueRef* {
        auto err_map = MapObjectRef::create(esState);
        err_map->set(esState,
                     ValueRef::create(1),
                     StringRef::createFromUTF8("positive"));
        err_map->set(esState,
                     ValueRef::create(-1),
                     StringRef::createFromUTF8("negative"));
        return err_map;
      });

  CHECK_EQ(r.isSuccessful(), true);

  ObjectRefHelper::setProperty(esContext,
                               esContext->globalObject(),
                               StringRef::createFromUTF8("err_map"),
                               r.result->asMapObject());
  const char* source =
      R"(
        function assert(condition, message) {
          if (!condition) throw new Error(message || 'Assertion failed');
        }

        for (const element of err_map) {
          print(element);
        }

        assert(err_map.get(1) == 'positive');
        assert(err_map.get(-1) == 'negative');
      )";

  auto r2 = EvalResultHelper::compileRun(esContext, source);

  CHECK_EQ(true, r2.isSuccessful());
}

class ObjectDataBase : public gc {
 public:
  virtual bool isConfigureData() = 0;
};

class ConfigureData : public ObjectDataBase {
 public:
  virtual bool isConfigureData() { return true; };
  ConfigureData(size_t count) { internalFieldCount_ = count; }
  size_t internalFieldCount() { return internalFieldCount_; }

 private:
  size_t internalFieldCount_{0};
};

class InternalFieldData : public ObjectDataBase {
 public:
  virtual bool isConfigureData() { return false; };

  void*& operator[](const size_t index) {
    LWNODE_CHECK(isConfigureData() == false);
    return internalFields_[index];
  }

  ValueRef* internalFields(const size_t index) {
    LWNODE_CHECK(isConfigureData() == false);
    void* p = internalFields_[index];
    if (p == nullptr) {
      return ValueRef::createUndefined();
    }
    return reinterpret_cast<ValueRef*>(p);
  }

  bool setInternalFields(size_t index, ValueRef* value) {
    LWNODE_CHECK(isConfigureData() == false);
    internalFields_[index] = value;
    return true;
  }

  static InternalFieldData* create(ConfigureData* config) {
    return new InternalFieldData(config);
  }

 private:
  InternalFieldData(ConfigureData* config)
      : internalFields_(
            std::move(GCContainer<void*>(config->internalFieldCount()))) {}

  GCContainer<void*> internalFields_;
};

enum InternalFields {
  kSlot,
  kInternalFieldCount,
};

static void mixinProtoMethod(FunctionTemplateRef* ft) {
  ObjectTemplateRef* esPrototypeTemplate = ft->prototypeTemplate();

  esPrototypeTemplate->set(
      TemplatePropertyNameRef(StringRef::createFromUTF8("isStreamBase")),
      ValueRef::create(true),
      true,
      true,
      true);

  {
    // v8::ObjectTemplate::PrototypeTemplate()->SetAccessor
    bool isActsLikeJSGetterSetter = true;

    esPrototypeTemplate->setNativeDataAccessorProperty(
        StringRef::createFromUTF8("onread"),
        new ObjectRef::NativeDataAccessorPropertyData(
            true,
            true,
            true,
            // getter
            [](ExecutionStateRef* state,
               ObjectRef* self,
               ValueRef* receiver,
               ObjectRef::NativeDataAccessorPropertyData* propertyData)
                -> ValueRef* {
              InternalFieldData* data = reinterpret_cast<InternalFieldData*>(
                  receiver->asObject()->extraData());
              LWNODE_DLOG_INFO(
                  "GET: receiver: %p data: %p self: %p", receiver, data, self);

              if (!data) {
                return ValueRef::createUndefined();
              }
              return data->internalFields(InternalFields::kSlot);
            },
            // setter
            [](ExecutionStateRef* state,
               ObjectRef* self,
               ValueRef* receiver,
               ObjectRef::NativeDataAccessorPropertyData* propertyData,
               ValueRef* setterInputData) -> bool {
              InternalFieldData* data = reinterpret_cast<InternalFieldData*>(
                  receiver->asObject()->extraData());
              LWNODE_DLOG_INFO(
                  "SET: receiver: %p data: %p self: %p", receiver, data, self);

              if (!data) {
                LWNODE_DLOG_ERROR("object data empty");
                return false;
              }
              return data->setInternalFields(
                  InternalFields::kSlot, setterInputData->asFunctionObject());
            }),
        isActsLikeJSGetterSetter);
  }
}

typedef void (*MethodSetter)(FunctionTemplateRef* ft);

static FunctionTemplateRef* createFunctionTemplate(
    ContextRef* esContext,
    std::string name,
    bool isConstructor,
    FunctionTemplateRef* parent = nullptr,
    MethodSetter methodSetter = nullptr) {
  auto esFunctionTemplate = FunctionTemplateRef::create(
      AtomicStringRef::create(esContext, name.c_str(), name.length()),
      0,
      false,
      isConstructor,
      [](ExecutionStateRef* state,
         ValueRef* thisValue,
         size_t argc,
         ValueRef** argv,
         OptionalRef<ObjectRef> newTarget) -> ValueRef* {
        LWNODE_DLOG_INFO("FUNC: thisValue: %p newTarget.hasValue: %s",
                         thisValue,
                         strBool(newTarget.hasValue()));

        if (newTarget.hasValue()) {
          auto thisObject = thisValue->asObject();
          auto configureData =
              reinterpret_cast<ConfigureData*>(thisObject->extraData());

          if (configureData) {
            // replace configure data with instance data
            LWNODE_CHECK(configureData->isConfigureData());
            thisObject->setExtraData(InternalFieldData::create(configureData));
          }

          LWNODE_DLOG_INFO("NEW: thisValue: %p data: %p",
                           thisValue,
                           thisObject->extraData());

          return thisValue;
        }

        Escargot::OptionalRef<Escargot::FunctionObjectRef> callee =
            state->resolveCallee();

        return ValueRef::createUndefined();
      });

  esFunctionTemplate->instanceTemplate()->setInstanceExtraData(
      new ConfigureData(InternalFields::kInternalFieldCount));

  if (parent) {
    esFunctionTemplate->inherit(parent);
  }

  if (methodSetter) {
    methodSetter(esFunctionTemplate);
  }

  return esFunctionTemplate;
}

TEST(Escargot_InlineCache_Regression) {
  LocalContext env;

  auto esContext =
      IsolateWrap::fromV8(env->GetIsolate())->GetCurrentContext()->get();
  EvalResultHelper::attachBuiltinPrint(esContext, esContext->globalObject());

  auto esFunctionTemplate = createFunctionTemplate(
      esContext,
      "Pipe",
      true,
      createFunctionTemplate(
          esContext,
          "StreamWrap",
          true,
          createFunctionTemplate(esContext, "HandleWrap", true),
          mixinProtoMethod));

  ObjectRefHelper::setProperty(
      esContext,
      esContext->globalObject(),
      StringRef::createFromUTF8("Pipe"),
      // instantiate (v8::FunctionTemplate::GetFunction)
      esFunctionTemplate->instantiate(esContext));

  const char* source =
      R"(
        function assert(condition, message) {
          if (!condition) throw new Error(message || 'Assertion failed');
        }

        assert(Pipe != undefined);
        print.ptr('Pipe', Pipe);

        function onRead() {
          assert(onRead.pipe_being_test == this);
          print.ptr('pipe', this,'onRead called');
        }

        print.ptr('onRead', onRead);
        print('-'.repeat(60));

        let pipe = new Pipe();
        print.ptr('pipe', pipe);

        assert(pipe.__proto__ == Pipe.prototype);
        assert(pipe.__proto__.__proto__.constructor.name == 'StreamWrap');
        assert(pipe.__proto__.__proto__.__proto__.constructor.name == 'HandleWrap');
        assert(pipe.isStreamBase == true);

        // crash should not occur on the lines commented
        for (let i=0; i<5; i++) {
          pipe.onread = onRead;
        }

        pipe.onread = onRead;
        assert(pipe.onread == onRead);

        onRead.pipe_being_test = pipe;
        pipe.onread();

        print('-'.repeat(60));

        function test() {
          // @note In order to induce escargot's inline cache, calling
          // the following setter should be called inside a function defined.
          let pipe = new Pipe();
          pipe.onread = onRead;
        }

        test();
        test();
        test();
        test();
        test(); // crash should not occur on this line
      )";

  auto r = EvalResultHelper::compileRun(esContext, source);

  CHECK_EQ(true, r.isSuccessful());
}

TEST(DISABLED_Escargot_ObjectCreate_Regression) {
  // related test: test/parallel/test-tls-external-accessor.js
  LocalContext env;

  auto esContext =
      IsolateWrap::fromV8(env->GetIsolate())->GetCurrentContext()->get();
  EvalResultHelper::attachBuiltinPrint(esContext, esContext->globalObject());

  auto esFunctionTemplate = createFunctionTemplate(
      esContext, "Pipe", true, nullptr, mixinProtoMethod);

  ObjectRefHelper::setProperty(esContext,
                               esContext->globalObject(),
                               StringRef::createFromUTF8("Pipe"),
                               esFunctionTemplate->instantiate(esContext));

  const char* source =
      R"(
        function assert(condition, message) {
          if (!condition) throw new Error(message || 'Assertion failed');
        }

        assert(Pipe != undefined);
        print.ptr('Pipe', Pipe);

        function onRead() {
          assert(onRead.pipe_being_test == this);
          print.ptr('pipe', this,'onRead called');
        }

        print('-'.repeat(60));

        // create an instance using new constructor
        let pipe = new Pipe();
        print.ptr('pipe', pipe);
        assert(pipe.__proto__ == Pipe.prototype);
        assert(pipe.__proto__.constructor.name == 'Pipe');
        assert(pipe.isStreamBase == true);

        pipe.onread = onRead;
        assert(pipe.onread == onRead);

        onRead.pipe_being_test = pipe;
        pipe.onread();

        print('-'.repeat(60));

        // create an instance using Object.create
        let pipe2 = Object.create(pipe);
        print.ptr('pipe2', pipe2);

        assert(pipe2.__proto__ == pipe);
        assert(pipe2.__proto__.constructor.name == 'Pipe');
        assert(pipe2.isStreamBase == true);

        pipe2.onread = onRead;
        assert(pipe2.onread == onRead); // <--
        print('-'.repeat(60));
      )";

  auto r = EvalResultHelper::compileRun(esContext, source);

  CHECK_EQ(true, r.isSuccessful());
}

struct FileData {
  void* byte{nullptr};
  long int size{0};

  FileData(void* data, long int data_size) {
    byte = data;
    size = data_size;
  }
  FileData() = default;
};

static FileData readFile(std::string filename) {
  FILE* file = std::fopen(filename.c_str(), "rb");
  if (!file) {
    return FileData();
  }

  std::fseek(file, 0, SEEK_END);
  long int fileSize = std::ftell(file);
  std::fseek(file, 0, SEEK_SET);

  void* buffer = Memory::gcMallocAtomicUncollectable(fileSize);
  std::fread(buffer, 1, fileSize, file);
  std::fclose(file);

  return FileData(buffer, fileSize);
}

static int g_reload_count;
static void* LoadReloadableSource(void* userData) {
  LWNODE_LOG_INFO("LoadReloadableSource");
  g_reload_count++;
  auto data = (Utils::ReloadableSourceData*)userData;
  if (data->preloadedData) {  // move memory ownership to js engine
    LWNODE_LOG_INFO("cached");
    auto buffer = data->preloadedData;
    data->preloadedData = nullptr;
    return buffer;
  }
  LWNODE_LOG_INFO("reload");
  FileData dest = readFile(data->path());
  LWNODE_CHECK_NOT_NULL(dest.byte);
  return dest.byte;
}

static void UnloadReloadableSource(void* preloadedData, void* userData) {
  LWNODE_LOG_INFO("UnloadReloadableSource");
  auto data = (Utils::ReloadableSourceData*)userData;
  if (data->preloadedData) {
    Memory::gcFree(data->preloadedData);
    data->preloadedData = nullptr;
  }
  Memory::gcFree(preloadedData);
}

class ReloadableContentScope {
 public:
  ReloadableContentScope(std::string filename, const bool isOneByteString) {
    std::string content;

    if (isOneByteString) {
      content = R"(
        function hello() { print('hello'); }
        function world() { print('world'); }
        hello();
      )";
    } else {
      content = R"(
        // ╔════════════════════════
        // ║ 16 bits content
        // ╚════════════════════════
        function hello() { print('hello'); }
        function world() { print('world'); }
        hello();
      )";
    }

    std::ofstream ofile(filename);
    if (ofile.is_open()) {
      ofile << content << std::endl;
      ofile.close();
    }
    filename_ = filename;
  }
  ~ReloadableContentScope() { std::remove(filename_.c_str()); }

 private:
  std::string filename_;
};

TEST(ReloadableString8) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  context->Global()
      ->Set(context,
            v8_str("print"),
            v8::FunctionTemplate::New(
                isolate,
                [](const v8::FunctionCallbackInfo<v8::Value>& args) -> void {
                  v8::String::Utf8Value utf8(args.GetIsolate(), args[0]);
                  printf("%s\n", *utf8);
                })
                ->GetFunction(context)
                .ToLocalChecked())
      .Check();

  std::string filename = "./tmp.test-reloadable-string.js";

  {
    g_reload_count = 0;
    const bool isOneByteString = true;
    ReloadableContentScope scope(filename, isOneByteString);

    FileData dest = readFile(filename);
    LWNODE_CHECK_NOT_NULL(dest.byte);
    v8::Script::Compile(
        context,
        Utils::NewReloadableString(
            isolate,
            Utils::ReloadableSourceData::create(
                filename, dest.byte, dest.size, isOneByteString),
            LoadReloadableSource,
            UnloadReloadableSource)
            .ToLocalChecked())
        .ToLocalChecked()
        ->Run(context);

    dest = FileData();  // note: remove address of preloaded buffer from stack
    MemoryUtil::gc();
    IsolateWrap::fromV8(isolate)->vmInstance()->enterIdleMode();
    v8::Script::Compile(
        context, v8::String::NewFromUtf8(isolate, "world();").ToLocalChecked())
        .ToLocalChecked()
        ->Run(context);
    EXPECT_GE(g_reload_count, 1);  // this depends on GC result
  }
}

TEST(DISABLED_ReloadableString16) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  context->Global()
      ->Set(context,
            v8_str("print"),
            v8::FunctionTemplate::New(
                isolate,
                [](const v8::FunctionCallbackInfo<v8::Value>& args) -> void {
                  v8::String::Utf8Value utf8(args.GetIsolate(), args[0]);
                  printf("%s\n", *utf8);
                })
                ->GetFunction(context)
                .ToLocalChecked())
      .Check();

  std::string filename = "./tmp.test-reloadable-string.js";

  {
    const bool isOneByteString = false;
    ReloadableContentScope scope(filename, isOneByteString);

    FileData dest = readFile(filename);
    LWNODE_CHECK_NOT_NULL(dest.byte);

    auto data = Utils::ReloadableSourceData::create(
        filename, dest.byte, dest.size, isOneByteString);

    Escargot::StringRef* source = Escargot::StringRef::createReloadableString(
        IsolateWrap::fromV8(isolate)->vmInstance(),
        isOneByteString,
        data->preloadedDataLength(),
        data,  // data should be gc-managed.
        LoadReloadableSource,
        UnloadReloadableSource);
    CHECK_NOT_NULL(source);

    ContextRef* context =
        ContextRef::create(IsolateWrap::fromV8(isolate)->vmInstance());
    ScriptParserRef* parser = context->scriptParser();

    ScriptParserRef::InitializeScriptResult result = parser->initializeScript(
        source,
        StringRef::createFromUTF8(filename.c_str(), filename.length()),
        false);

    if (!result.isSuccessful()) {
      LWNODE_LOG_ERROR("(%d) %s",
                       result.parseErrorCode,
                       result.parseErrorMessage->toStdUTF8String().c_str());
    }

    ASSERT_EQ(result.isSuccessful(), true);
  }

  {
    g_reload_count = 0;
    const bool isOneByteString = false;
    ReloadableContentScope scope(filename, isOneByteString);

    FileData dest = readFile(filename);
    LWNODE_CHECK_NOT_NULL(dest.byte);
    v8::Script::Compile(
        context,
        Utils::NewReloadableString(
            isolate,
            Utils::ReloadableSourceData::create(
                filename, dest.byte, dest.size, isOneByteString),
            LoadReloadableSource,
            UnloadReloadableSource)
            .ToLocalChecked())
        .ToLocalChecked()
        ->Run(context);

    dest = FileData();  // note: remove address of preloaded buffer from stack
    MemoryUtil::gc();
    IsolateWrap::fromV8(isolate)->vmInstance()->enterIdleMode();
    v8::Script::Compile(
        context, v8::String::NewFromUtf8(isolate, "world();").ToLocalChecked())
        .ToLocalChecked()
        ->Run(context);
    EXPECT_GE(g_reload_count, 1);  // this depends on GC result
  }
}

#endif
