// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TizenDeviceAPILoaderForEscargot.h"

#include <dlfcn.h>
#include "ExtensionAdapter.h"
#include "ExtensionManager.h"


using namespace Escargot;

namespace DeviceAPI {

TizenStrings::TizenStrings(ContextRef* context)
    : m_context(context)
    , m_initialized(false)
{
    initializeEarlyStrings();
}

#define INIT_TIZEN_STRING(name) \
    name = AtomicStringRef::create(m_context, "" #name);
void TizenStrings::initializeEarlyStrings()
{
    DEVICEAPI_LOG_INFO("Enter");

    FOR_EACH_EARLY_TIZEN_STRINGS(INIT_TIZEN_STRING)
    SUPPORTED_TIZEN_PROPERTY(INIT_TIZEN_STRING)
    SUPPORTED_TIZEN_ENTRYPOINTS(INIT_TIZEN_STRING)
}

void TizenStrings::initializeLazyStrings()
{
    DEVICEAPI_LOG_INFO("Enter");

    if (m_initialized)
        return;

    FOR_EACH_LAZY_TIZEN_STRINGS(INIT_TIZEN_STRING)

    m_initialized = true;
}
#undef INIT_TIZEN_STRING

void printArguments(ContextRef* context, size_t argc, ValueRef** argv)
{
    Evaluator::execute(
        context,
        [](ExecutionStateRef* state, size_t argc,
           ValueRef** argv) -> ValueRef* {
#ifdef DEBUG
            DEVICEAPI_LOG_INFO("printing %zu arguments", argc);
            for (size_t i = 0; i < argc; i++) {
                DEVICEAPI_LOG_INFO(
                    "argument %zu : %s", i,
                    argv[i]->toString(state)->toStdUTF8String().c_str());
            }
#endif
            return ValueRef::createUndefined();
        },
        argc, argv);
}

void* ExtensionManagerInstance::operator new(size_t size)
{
    static bool typeInited = false;
    static GC_descr descr;
    if (!typeInited) {
        GC_word obj_bitmap[GC_BITMAP_SIZE(ExtensionManagerInstance)] = { 0 };
        GC_set_bit(obj_bitmap,
                   GC_WORD_OFFSET(ExtensionManagerInstance, m_context));
        GC_set_bit(obj_bitmap,
                   GC_WORD_OFFSET(ExtensionManagerInstance, m_strings));
#if defined(STARFISH_TIZEN_WEARABLE_WIDGET)
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(ExtensionManagerInstance,
                                              m_webWidgetAPIInstance));
#endif
#define DECLARE_TIZEN_VALUE(name)                                   \
    GC_set_bit(obj_bitmap, GC_WORD_OFFSET(ExtensionManagerInstance, \
                                          VALUE_NAME_STRCAT(m_##name)));
        FOR_EACH_EARLY_TIZEN_STRINGS(DECLARE_TIZEN_VALUE);
        FOR_EACH_LAZY_TIZEN_STRINGS(DECLARE_TIZEN_VALUE);
        SUPPORTED_TIZEN_PROPERTY(DECLARE_TIZEN_VALUE);
        SUPPORTED_TIZEN_ENTRYPOINTS(DECLARE_TIZEN_VALUE);
#undef DECLARE_TIZEN_VALUE
        descr = GC_make_descriptor(obj_bitmap,
                                   GC_WORD_LEN(ExtensionManagerInstance));
        typeInited = true;
    }
    return GC_MALLOC_EXPLICITLY_TYPED(size, descr);
}

wrt::xwalk::Extension* ExtensionManagerInstance::getExtension(
    const char* apiName)
{
    
    auto extensionManager = wrt::xwalk::ExtensionManager::GetInstance();
    wrt::xwalk::ExtensionMap& extensions = extensionManager->extensions();

    const char* extensionsDirPath = "/usr/lib/tizen-extensions-crosswalk";
    auto it = extensions.find(apiName);
    if (it == extensions.end()) {
        DEVICEAPI_LOG_INFO("Creating a new extension: %s\n", apiName);
        char library_path[512];
        if (!strcmp(apiName, "tizen")) {
            snprintf(library_path, 512, "%s/libtizen.so", extensionsDirPath);
        } else if (!strcmp(apiName, "sensorservice")) {
            snprintf(library_path, 512, "%s/libtizen_sensor.so", extensionsDirPath);
        } else if (!strcmp(apiName, "sa")) {
            snprintf(library_path, 512, "%s/libwebapis_sa.so", extensionsDirPath);
        } else if (!strcmp(apiName, "tvaudiocontrol")) {
            snprintf(library_path, 512, "%s/libtizen_tvaudio.so", extensionsDirPath);
        } else {
            snprintf(library_path, 512, "%s/libtizen_%s.so", extensionsDirPath, apiName);
        }

        wrt::xwalk::Extension* extension =
            new wrt::xwalk::Extension(library_path, extensionManager);

        if (extension->Initialize()) {
            extensionManager->RegisterExtension(extension);
            extensions[apiName] = extension;
            return extension;
        } else {
            DEVICEAPI_LOG_INFO("Cannot initialize extension %s", apiName);
            return nullptr;
        }
    } else {
        DEVICEAPI_LOG_INFO("Load extension: %s\n", apiName);
        return it->second;
    }
}

static const char deviceAPISourcePieceStart[] = R"(
    (function(extension) {
        extension.internal = {};
        extension.internal._ = {};
        let internalAPI = [
            "sendSyncMessage",
            "sendSyncMessageWithBinaryReply",
            "sendSyncMessageWithStringReply",
        ];

        for (let name of internalAPI) {
            extension.internal._[name] = extension[name];
            extension.internal[name] = function () {
                return extension.internal._[name].apply(extension, arguments);
            };
            delete extension[name];
        }

        var exports = {};
        var window = {
            Object,
        };
    (function() {'use strict';
    )";

// sizeof(deviceAPISourcePieceStart) should point string length of deviceAPISourcePieceStart
static_assert(sizeof(deviceAPISourcePieceStart) > sizeof(size_t), "");

static const char deviceAPISourcePieceEnd[] = R"(
    })();
    return exports;})
    )";

// sizeof(deviceAPISourcePieceEnd) should point string length of deviceAPISourcePieceStart
static_assert(sizeof(deviceAPISourcePieceEnd) > sizeof(size_t), "");

static void* loadDeviceAPISource(void* data)
{
    wrt::xwalk::Extension* extension = ExtensionManagerInstance::getExtension((const char*)data);

    auto startLen = sizeof(deviceAPISourcePieceStart) - 1;
    auto endLen = sizeof(deviceAPISourcePieceEnd) - 1;

    char* dest = (char*)malloc(startLen + endLen + extension->javascript_api().size() + 1);
    char* ptr = dest;
    strncpy(ptr, deviceAPISourcePieceStart, startLen);
    ptr += startLen;

    strncpy(ptr, extension->javascript_api().data(), extension->javascript_api().size());
    ptr += extension->javascript_api().size();

    strncpy(ptr, deviceAPISourcePieceEnd, endLen);
    ptr += endLen;

    *ptr = 0;

    return dest;
}

static void unloadDeviceAPISource(void* memoryPtr, void* callbackData)
{
    free(memoryPtr);
}

// Caution: this function is called only inside existing js execution context,
// so we don't handle JS exception around ESFunctionObject::call()
ObjectRef* ExtensionManagerInstance::initializeExtensionInstance(
    const char* apiName)
{
    DEVICEAPI_LOG_INFO("Enter");

    auto execResult = Evaluator::execute(
               m_context,
               [](ExecutionStateRef* state, ExtensionManagerInstance* self,
                  const char* apiName) -> ValueRef* {
                   wrt::xwalk::Extension* extension = getExtension(apiName);
                   if (!extension) {
                       DEVICEAPI_LOG_INFO("Cannot load extension %s", apiName);
                       return ObjectRef::create(state);
                   }

                   const std::string& source = extension->javascript_api();
                   for (auto c: source) {
                       if (static_cast<unsigned char>(c) > 127) {
                           // TODO support unicode version of source
                           DEVICEAPI_ASSERT_SHOULD_NOT_BE_HERE();
                       }
                   }

                   char* newApiName = (char*)GC_MALLOC_ATOMIC(strlen(apiName) + 1);
                   memcpy(newApiName, apiName, strlen(apiName));
                   newApiName[strlen(apiName)] = 0;

                   auto startLen = sizeof(deviceAPISourcePieceStart) - 1;
                   auto endLen = sizeof(deviceAPISourcePieceEnd) - 1;

                   size_t sourceLength = startLen + endLen + extension->javascript_api().size();
                   StringRef* apiSource = Escargot::StringRef::createReloadableString(
                       state->context()->vmInstance(), true,
                       sourceLength, newApiName, loadDeviceAPISource, unloadDeviceAPISource);

                   std::string jsFileName = apiName;
                   jsFileName += ".js";
                   jsFileName = "tizen_api_internal_" + jsFileName;

                   FunctionObjectRef* initializer =
                       self->m_context->scriptParser()
                           ->initializeScript(
                               apiSource,
                               StringRef::createFromUTF8(jsFileName.c_str(),
                                                         jsFileName.length()))
                           .script.value()
                           ->execute(state)
                           ->asFunctionObject();

                   ObjectRef* extensionObject = self->createExtensionObject(state);
                   wrt::xwalk::ExtensionInstance* extensionInstance = extension->CreateInstance();
                   self->m_extensionInstances[extensionObject] = extensionInstance;

                   ValueRef* arguments[] = { ValueRef::create(extensionObject) };
                   return initializer
                       ->call(state, ValueRef::createNull(), 1, arguments)
                       ->toObject(state);
               },
               this, apiName);

    if (!execResult.isSuccessful()) {
        DEVICEAPI_LOG_ERROR("Error: %s\n",
        execResult.resultOrErrorToString(m_context)->toStdUTF8String().c_str());
    }
    return execResult.result->asObject();
}

ObjectRef* ExtensionManagerInstance::createExtensionObject(
    ExecutionStateRef* state)
{
    DEVICEAPI_LOG_INFO("Enter");

    ObjectRef* extensionObject = ObjectRef::create(state);

    FunctionObjectRef* postMessageFn = FunctionObjectRef::create(
        state,
        FunctionObjectRef::NativeFunctionInfo(
            m_strings->postMessage,
            [](ExecutionStateRef* state, ValueRef* thisValue, size_t argc,
               ValueRef** argv, bool isNewExpression) -> ValueRef* {
                DEVICEAPI_LOG_ERROR("extension.postMessage UNIMPLEMENTED");
                printArguments(state->context(), argc, argv);
                DEVICEAPI_ASSERT_SHOULD_NOT_BE_HERE();
                return ValueRef::createUndefined();
            },
            0, true, true));

    extensionObject->defineDataProperty(
        state, ValueRef::create(m_strings->postMessage->string()),
        ValueRef::create(postMessageFn), true, true, true);

    FunctionObjectRef* sendSyncMessageFn = FunctionObjectRef::create(
        state,
        FunctionObjectRef::NativeFunctionInfo(
            m_strings->sendSyncMessage,
            [](ExecutionStateRef* state, ValueRef* thisValue, size_t argc,
               ValueRef** argv, bool isNewExpression) -> ValueRef* {
                DEVICEAPI_LOG_INFO("extension.sendSyncMessage");
                printArguments(state->context(), argc, argv);

                ExtensionManagerInstance* extensionManagerInstance =
                    get(state->context());
                wrt::xwalk::ExtensionInstance* extensionInstance =
                    extensionManagerInstance
                        ->getExtensionInstanceFromCallingContext(
                            state->context(), thisValue);
                if (!extensionInstance || argc != 1) {
                    DEVICEAPI_LOG_ERROR("extensionInstance == nullptr");
                    return ValueRef::create(false);
                }

                StringRef* message = argv[0]->toString(state);
                extensionInstance->HandleSyncMessage(
                    message->toStdUTF8String());

                std::string reply = extensionInstance->sync_replay_msg();
                DEVICEAPI_LOG_INFO(
                    "extension.sendSyncMessage Done with reply %s",
                    reply.c_str());

                if (reply.empty()) {
                    return ValueRef::createNull();
                }
                return ValueRef::create(
                    StringRef::createFromUTF8(reply.c_str(), reply.size()));
            },
            0, true, true));

    extensionObject->defineDataProperty(
        state, ValueRef::create(m_strings->sendSyncMessage->string()),
        ValueRef::create(sendSyncMessageFn), true, true, true);

    FunctionObjectRef* sendSyncMessageWithBinaryReplyFn = FunctionObjectRef::create(
        state,
        FunctionObjectRef::NativeFunctionInfo(
            m_strings->sendSyncMessageWithBinaryReply,
            [](ExecutionStateRef* state, ValueRef* thisValue, size_t argc,
               ValueRef** argv, bool isNewExpression) -> ValueRef* {
                DEVICEAPI_LOG_ERROR("Not implemented: extension.sendSyncMessageWithBinaryReply");
                printArguments(state->context(), argc, argv);

                return ValueRef::createNull();
            },
            0, true, true));

    extensionObject->defineDataProperty(
        state, ValueRef::create(m_strings->sendSyncMessageWithBinaryReply->string()),
        ValueRef::create(sendSyncMessageWithBinaryReplyFn), true, true, true);

    FunctionObjectRef* sendSyncMessageWithStringReplyFn = FunctionObjectRef::create(
        state,
        FunctionObjectRef::NativeFunctionInfo(
            m_strings->sendSyncMessageWithStringReply,
            [](ExecutionStateRef* state, ValueRef* thisValue, size_t argc,
               ValueRef** argv, bool isNewExpression) -> ValueRef* {
                DEVICEAPI_LOG_ERROR("Not implemented: extension.sendSyncMessageWithStringReply");
                printArguments(state->context(), argc, argv);

                return ValueRef::createNull();
            },
            0, true, true));

    extensionObject->defineDataProperty(
        state, ValueRef::create(m_strings->sendSyncMessageWithStringReply->string()),
        ValueRef::create(sendSyncMessageWithStringReplyFn), true, true, true);

    FunctionObjectRef* sendSyncDataFn = FunctionObjectRef::create(
        state,
        FunctionObjectRef::NativeFunctionInfo(
            m_strings->sendSyncData,
            [](ExecutionStateRef* state, ValueRef* thisValue, size_t argc,
               ValueRef** argv, bool isNewExpression) -> ValueRef* {
                DEVICEAPI_LOG_INFO("extension.sendSyncData");
                printArguments(state->context(), argc, argv);

                ExtensionManagerInstance* extensionManagerInstance =
                    get(state->context());
                wrt::xwalk::ExtensionInstance* extensionInstance =
                    extensionManagerInstance
                        ->getExtensionInstanceFromCallingContext(
                            state->context(), thisValue);
                if (!extensionInstance || argc < 1) {
                    return ValueRef::create(false);
                }

                ChunkData chunkData(nullptr, 0);
                if (argc > 1) {
                    ValueRef* dataValue = argv[1];
                    if (dataValue->isObject()) {
                        ObjectRef* arrayData = dataValue->toObject(state);
                        size_t length =
                            arrayData
                                ->get(state,
                                      ValueRef::create(
                                          StringRef::createFromASCII("length")))
                                ->toLength(state);
                        uint8_t* buffer =
                            (uint8_t*)malloc(sizeof(uint8_t) * length);
                        for (size_t i = 0; i < length; i++) {
                            buffer[i] = static_cast<uint8_t>(
                                arrayData->get(state, ValueRef::create(i))
                                    ->toNumber(state));
                        }
                        chunkData = ChunkData(buffer, length);
                    } else if (dataValue->isString()) {
                        StringRef* stringData = dataValue->toString(state);
                        chunkData = ChunkData(
                            (uint8_t*)stringData->toStdUTF8String().c_str(),
                            stringData->length());
                    }
                }

                StringRef* message = argv[0]->toString(state);
                extensionInstance->HandleSyncData(message->toStdUTF8String(),
                                                  chunkData.m_buffer,
                                                  chunkData.m_length);

                uint8_t* replyBuffer = nullptr;
                size_t replyLength = 0;
                std::string reply = extensionInstance->sync_data_reply_msg(
                    &replyBuffer, &replyLength);

                DEVICEAPI_LOG_INFO(
                    "extension.sendSyncData Done with reply %s (buffer %s)",
                    reply.c_str(), replyBuffer);

                if (reply.empty()) {
                    return ValueRef::createNull();
                }

                ObjectRef* returnObject = ObjectRef::create(state);
                returnObject->defineDataProperty(
                    state,
                    ValueRef::create(
                        extensionManagerInstance->strings()->reply->string()),
                    ValueRef::create(StringRef::createFromUTF8(reply.c_str(),
                                                                reply.size())),
                    true, true, true);

                if (replyBuffer || replyLength > 0) {
                    size_t chunkID = extensionManagerInstance->addChunk(
                        replyBuffer, replyLength);
                    returnObject->defineDataProperty(
                        state,
                        ValueRef::create(extensionManagerInstance->strings()
                                             ->chunk_id->string()),
                        ValueRef::create(chunkID), true, true, true);
                }

                return ValueRef::create(returnObject);
            },
            0, true, true));

    extensionObject->defineDataProperty(
        state, ValueRef::create(m_strings->sendSyncData->string()),
        ValueRef::create(sendSyncDataFn), true, true, true);

    FunctionObjectRef* sendRuntimeMessageFn = FunctionObjectRef::create(
        state,
        FunctionObjectRef::NativeFunctionInfo(
            m_strings->sendRuntimeMessage,
            [](ExecutionStateRef* state, ValueRef* thisValue, size_t argc,
               ValueRef** argv, bool isNewExpression) -> ValueRef* {
                DEVICEAPI_LOG_ERROR(
                    "extension.sendRuntimeMessage UNIMPLEMENTED");
                printArguments(state->context(), argc, argv);
                DEVICEAPI_ASSERT_SHOULD_NOT_BE_HERE();
                return ValueRef::createUndefined();
            },
            0, true, true));

    extensionObject->defineDataProperty(
        state, ValueRef::create(m_strings->sendRuntimeMessage->string()),
        ValueRef::create(sendRuntimeMessageFn), true, true, true);

    FunctionObjectRef* sendRuntimeAsyncMessageFn = FunctionObjectRef::create(
        state,
        FunctionObjectRef::NativeFunctionInfo(
            m_strings->sendRuntimeAsyncMessage,
            [](ExecutionStateRef* state, ValueRef* thisValue, size_t argc,
               ValueRef** argv, bool isNewExpression) -> ValueRef* {
                DEVICEAPI_LOG_ERROR(
                    "extension.sendRuntimeAsyncMessage UNIMPLEMENTED");
                printArguments(state->context(), argc, argv);
                DEVICEAPI_ASSERT_SHOULD_NOT_BE_HERE();
                return ValueRef::createUndefined();
            },
            0, true, true));

    extensionObject->defineDataProperty(
        state, ValueRef::create(m_strings->sendRuntimeAsyncMessage->string()),
        ValueRef::create(sendRuntimeAsyncMessageFn), true, true, true);

    FunctionObjectRef* sendRuntimeSyncMessageFn = FunctionObjectRef::create(
        state,
        FunctionObjectRef::NativeFunctionInfo(
            m_strings->sendRuntimeSyncMessage,
            [](ExecutionStateRef* state, ValueRef* thisValue, size_t argc,
               ValueRef** argv, bool isNewExpression) -> ValueRef* {
                DEVICEAPI_LOG_ERROR(
                    "extension.sendRuntimeSyncMessage UNIMPLEMENTED");
                printArguments(state->context(), argc, argv);
                DEVICEAPI_ASSERT_SHOULD_NOT_BE_HERE();
                return ValueRef::createUndefined();
            },
            0, true, true));

    extensionObject->defineDataProperty(
        state, ValueRef::create(m_strings->sendRuntimeSyncMessage->string()),
        ValueRef::create(sendRuntimeSyncMessageFn), true, true, true);

    FunctionObjectRef* setMessageListenerFn = FunctionObjectRef::create(
        state,
        FunctionObjectRef::NativeFunctionInfo(
            m_strings->setMessageListener,
            [](ExecutionStateRef* state, ValueRef* thisValue, size_t argc,
               ValueRef** argv, bool isNewExpression) -> ValueRef* {
                DEVICEAPI_LOG_INFO("extension.setMessageListener");
                printArguments(state->context(), argc, argv);

                ExtensionManagerInstance* extensionManagerInstance =
                    get(state->context());
                wrt::xwalk::ExtensionInstance* extensionInstance =
                    extensionManagerInstance
                        ->getExtensionInstanceFromCallingContext(
                            state->context(), thisValue);

                if (!extensionInstance || argc != 1) {
                    return ValueRef::create(false);
                }

                ValueRef* listenerValue = argv[0];
                if (listenerValue->isUndefined()) {
                    DEVICEAPI_LOG_ERROR("listenerValue == undefined");
                    extensionInstance->set_post_message_listener(nullptr);
                    return ValueRef::create(true);
                }

                if (!listenerValue->isCallable()) {
                    DEVICEAPI_LOG_ERROR("Invalid message listener.");
                    return ValueRef::create(false);
                }

                ObjectRef* listener = listenerValue->asObject();
                ESPostMessageListener* postMessageListener =
                    ESPostMessageListener::create(state->context(), listener);
                extensionInstance->set_post_message_listener(
                    postMessageListener);

                extensionManagerInstance->m_postListeners.push_back(
                    postMessageListener);

                return ValueRef::create(true);
            },
            0, true, true));

    extensionObject->defineDataProperty(
        state, ValueRef::create(m_strings->setMessageListener->string()),
        ValueRef::create(setMessageListenerFn), true, true, true);

    FunctionObjectRef* receiveChunkDataFn = FunctionObjectRef::create(
        state,
        FunctionObjectRef::NativeFunctionInfo(
            m_strings->receiveChunkData,
            [](ExecutionStateRef* state, ValueRef* thisValue, size_t argc,
               ValueRef** argv, bool isNewExpression) -> ValueRef* {
                DEVICEAPI_LOG_ERROR("extension.receiveChunkData");
                printArguments(state->context(), argc, argv);

                ExtensionManagerInstance* extensionManagerInstance =
                    get(state->context());
                wrt::xwalk::ExtensionInstance* extensionInstance =
                    extensionManagerInstance
                        ->getExtensionInstanceFromCallingContext(
                            state->context(), thisValue);

                if (!extensionInstance || argc < 1) {
                    return ValueRef::create(false);
                }

                TizenStrings* strings = extensionManagerInstance->strings();

                size_t chunkID = argv[0]->toNumber(state);
                ExtensionManagerInstance::ChunkData chunkData =
                    extensionManagerInstance->getChunk(chunkID);
                if (!chunkData.m_buffer) {
                    return ValueRef::createNull();
                }

                StringRef* type = argv[1]->toString(state);
                bool isStringType = (!type->equals(strings->octet->string()));

                ValueRef* ret;
                if (isStringType) {
                    ret = ValueRef::create(StringRef::createFromUTF8(
                        (const char*)chunkData.m_buffer,
                        strlen((const char*)chunkData.m_buffer)));
                } else {
                    ArrayObjectRef* octetArray = ArrayObjectRef::create(state);
                    for (size_t i = 0; i < chunkData.m_length; i++) {
                        octetArray->set(
                            state, ValueRef::create(i),
                            ValueRef::create(chunkData.m_buffer[i]));
                    }
                    ret = ValueRef::create(octetArray);
                }
                free(chunkData.m_buffer);
                return ret;
            },
            0, true, true));

    extensionObject->defineDataProperty(
        state, ValueRef::create(m_strings->receiveChunkData->string()),
        ValueRef::create(receiveChunkDataFn), true, true, true);

    return extensionObject;
}

wrt::xwalk::ExtensionInstance*
ExtensionManagerInstance::getExtensionInstanceFromCallingContext(
    ContextRef* context, ValueRef* thisValue)
{
    DEVICEAPI_LOG_INFO("Enter");
    if (thisValue->isUndefinedOrNull()) {
        return nullptr;
    }

    ObjectRef* obj = Evaluator::execute(m_context,
                                        [](ExecutionStateRef* state,
                                           ValueRef* thisValue) -> ValueRef* {
                                            return thisValue->toObject(state);
                                        },
                                        thisValue)
                         .result->asObject();

    auto it = m_extensionInstances.find(obj);
    if (it == m_extensionInstances.end()) {
        return nullptr;
    }

    return it->second;
}

size_t ExtensionManagerInstance::addChunk(uint8_t* buffer, size_t length)
{
    DEVICEAPI_LOG_INFO("Enter");
    size_t chunkID = m_chunkID++;
    m_chunkDataMap[chunkID] = ChunkData(buffer, length);
    return chunkID;
}

ExtensionManagerInstance::ChunkData ExtensionManagerInstance::getChunk(
    size_t chunkID)
{
    DEVICEAPI_LOG_INFO("Enter");
    auto it = m_chunkDataMap.find(chunkID);
    if (it == m_chunkDataMap.end()) {
        return ChunkData(nullptr, 0);
    } else {
        ChunkData chunkData = it->second;
        m_chunkDataMap.erase(it);
        return chunkData;
    }
}

ExtensionManagerInstance::ExtensionManagerInstanceMap
    ExtensionManagerInstance::s_extensionManagerInstances;

std::mutex ExtensionManagerInstance::s_mutex;

ExtensionManagerInstance::ExtensionManagerInstance(ContextRef* context)
    : m_context(context)
    , m_chunkID(0)
{
    DEVICEAPI_LOG_INFO("new ExtensionManagerInstance %p", this);

#define DECLARE_TIZEN_OBJECT(name) VALUE_NAME_STRCAT(m_##name) = nullptr;
    FOR_EACH_EARLY_TIZEN_STRINGS(DECLARE_TIZEN_OBJECT);
    FOR_EACH_LAZY_TIZEN_STRINGS(DECLARE_TIZEN_OBJECT);
    SUPPORTED_TIZEN_PROPERTY(DECLARE_TIZEN_OBJECT);
    SUPPORTED_TIZEN_ENTRYPOINTS(DECLARE_TIZEN_OBJECT);
#undef DECLARE_TIZEN_OBJECT

    m_strings = new TizenStrings(m_context);
    Evaluator::execute(
        m_context,
        [](ExecutionStateRef* state,
           ExtensionManagerInstance* self) -> ValueRef* {
            ValueRef* tizenGetter = ValueRef::create(FunctionObjectRef::create(
                state,
                FunctionObjectRef::NativeFunctionInfo(
                    self->m_strings->tizen,
                    [](ExecutionStateRef* state, ValueRef* thisValue,
                       size_t argc, ValueRef** argv,
                       bool isNewExpression) -> ValueRef* {
                        DEVICEAPI_LOG_INFO("Enter");

                        ExtensionManagerInstance* extensionManagerInstance =
                            get(state->context());

                        if (extensionManagerInstance->m_tizenValue) {
                            return extensionManagerInstance->m_tizenValue;
                        }
                        TizenStrings* strings =
                            extensionManagerInstance->strings();
                        strings->initializeLazyStrings();

                        // initialize tizen object
                        ObjectRef* tizenObject =
                            extensionManagerInstance
                                ->initializeExtensionInstance("tizen");
                        extensionManagerInstance->m_tizenValue =
                            ValueRef::create(tizenObject);

#define DEFINE_SUPPORTED_TIZEN_API(name)                                       \
    tizenObject->defineAccessorProperty(                                       \
        state, ValueRef::create(StringRef::createFromUTF8("" #name)),         \
        ObjectRef::AccessorPropertyDescriptor(                                 \
            ValueRef::create(FunctionObjectRef::create(                        \
                state,                                                         \
                FunctionObjectRef::NativeFunctionInfo(                         \
                    AtomicStringRef::create(state->context(), "" #name),       \
                    [](ExecutionStateRef* state, ValueRef* thisValue,          \
                       size_t argc, ValueRef** argv,                           \
                       bool isNewExpression) -> ValueRef* {                    \
                        ExtensionManagerInstance* extensionManagerInstance =   \
                            get(state->context());                             \
                        if (extensionManagerInstance->VALUE_NAME_STRCAT(       \
                                m_##name)) {                                   \
                            return extensionManagerInstance                    \
                                ->VALUE_NAME_STRCAT(m_##name);                 \
                        }                                                      \
                        DEVICEAPI_LOG_INFO("Loading plugin for %s", "" #name); \
                        ObjectRef* apiObject =                                 \
                            extensionManagerInstance                           \
                                ->initializeExtensionInstance("" #name);       \
                        extensionManagerInstance->VALUE_NAME_STRCAT(           \
                            m_##name) = ValueRef::create(apiObject);           \
                        thisValue->toObject(state)->defineDataProperty(        \
                            state, ValueRef::create(                           \
                                       StringRef::createFromUTF8("" #name)),  \
                            ValueRef::create(apiObject), false, true, false);  \
                        return ValueRef::create(apiObject);                    \
                    },                                                         \
                    0, true, true))),                                          \
            nullptr, ObjectRef::PresentAttribute::EnumerablePresent));

    SUPPORTED_TIZEN_PROPERTY(DEFINE_SUPPORTED_TIZEN_API)

#undef DEFINE_SUPPORTED_TIZEN_API

#define DEFINE_SUPPORTED_TIZEN_ENTRYPOINTS(name)                               \
    ObjectRef::NativeDataAccessorPropertyData* nativeData##name =              \
        new NativeDataAccessorPropertyDataForEntryPoint(                       \
            true, true, true,                                                  \
            [](ExecutionStateRef* state, ObjectRef* self, ValueRef* receiver,  \
               ObjectRef::NativeDataAccessorPropertyData* data) -> ValueRef* { \
                ExtensionManagerInstance* extensionManagerInstance =           \
                    get(state->context());                                     \
                if (extensionManagerInstance->VALUE_NAME_STRCAT(m_##name)) {   \
                    return extensionManagerInstance->VALUE_NAME_STRCAT(        \
                        m_##name);                                             \
                }                                                              \
                DEVICEAPI_LOG_INFO("Loading plugin for %s", "" #name);         \
                extensionManagerInstance->VALUE_NAME_STRCAT(m_##name) =        \
                    Escargot::ValueRef::createUndefined();                     \
                extensionManagerInstance->m_tizenValue->toObject(state)->get(  \
                    state, ValueRef::create(                                   \
                               StringRef::createFromASCII("application")));    \
                NativeDataAccessorPropertyDataForEntryPoint* myData =          \
                    (NativeDataAccessorPropertyDataForEntryPoint*)data;        \
                extensionManagerInstance->VALUE_NAME_STRCAT(m_##name) =        \
                    myData->m_data;                                            \
                return myData->m_data;                                         \
            },                                                                 \
            [](ExecutionStateRef* state, ObjectRef* self, ValueRef* receiver,  \
               ObjectRef::NativeDataAccessorPropertyData* data,                \
               ValueRef* setterInputData) -> bool {                            \
                NativeDataAccessorPropertyDataForEntryPoint* myData =          \
                    (NativeDataAccessorPropertyDataForEntryPoint*)data;        \
                myData->m_data = setterInputData;                              \
                return true;                                                   \
            });                                                                \
    tizenObject->defineNativeDataAccessorProperty(                             \
        state, ValueRef::create(StringRef::createFromUTF8(#name)),            \
        nativeData##name);


    SUPPORTED_TIZEN_ENTRYPOINTS(DEFINE_SUPPORTED_TIZEN_ENTRYPOINTS)

#undef DEFINE_SUPPORTED_TIZEN_ENTRYPOINTS

#if defined(STARFISH_TIZEN_WEARABLE_WIDGET)
                        WebWidgetAPIInstance* ww =
                            new (GC) WebWidgetAPIInstance();
                        extensionManagerInstance->m_webWidgetAPIInstance = ww;
                        ObjectRef* widgetAPIObj =
                            ww->createWebWidgetAPIObject(state->context());
                        tizenObject->defineDataProperty(
                            state, ValueRef::create(
                                       StringRef::createFromASCII("webWidget")),
                            ValueRef::create(widgetAPIObj), false, true, false);
#endif

                        return ValueRef::create(tizenObject);
                    },
                    0, true, true)));

            self->m_context->globalObject()->defineAccessorProperty(
                state, ValueRef::create(self->m_strings->tizen->string()),
                ObjectRef::AccessorPropertyDescriptor(
                    tizenGetter, nullptr,
                    ObjectRef::PresentAttribute::EnumerablePresent));

            ValueRef* xwalkGetter = ValueRef::create(FunctionObjectRef::create(
                state,
                FunctionObjectRef::NativeFunctionInfo(
                    self->m_strings->xwalk,
                    [](ExecutionStateRef* state, ValueRef* thisValue,
                       size_t argc, ValueRef** argv,
                       bool isNewExpression) -> ValueRef* {
                        DEVICEAPI_LOG_INFO("xwalkGetter Enter");

                        ExtensionManagerInstance* extensionManagerInstance =
                            get(state->context());
                        if (extensionManagerInstance->m_xwalkValue) {
                            return extensionManagerInstance->m_xwalkValue;
                        }
                        DEVICEAPI_LOG_INFO("Loading plugin for xwalk.utils");

                        TizenStrings* strings =
                            extensionManagerInstance->strings();
                        strings->initializeLazyStrings();

                        // initialize xwalk object
                        ObjectRef* xwalkObject =
                            extensionManagerInstance
                                ->initializeExtensionInstance("utils");
                        extensionManagerInstance->m_xwalkValue =
                            ValueRef::create(xwalkObject);

                        // re-define xwalk object
                        thisValue->toObject(state)->defineDataProperty(
                            state, ValueRef::create(strings->xwalk->string()),
                            ValueRef::create(xwalkObject), false, true, false);

                        return ValueRef::create(xwalkObject);
                    },
                    0, true, true)));

            self->m_context->globalObject()->defineAccessorProperty(
                state, ValueRef::create(self->m_strings->xwalk->string()),
                ObjectRef::AccessorPropertyDescriptor(
                    xwalkGetter, nullptr,
                    ObjectRef::PresentAttribute::EnumerablePresent));

            return ValueRef::createUndefined();
        },
        this);

#if defined(STARFISH_TIZEN_WEARABLE_WIDGET)
    m_webWidgetAPIInstance = nullptr;
#endif
    std::lock_guard<std::mutex> guard(s_mutex);
    s_extensionManagerInstances[m_context] = this;
    DEVICEAPI_LOG_INFO("ExtensionManagerInstance %zu => %zu",
                       s_extensionManagerInstances.size() - 1,
                       s_extensionManagerInstances.size());
}

ExtensionManagerInstance::~ExtensionManagerInstance()
{
    std::lock_guard<std::mutex> guard(s_mutex);
    DEVICEAPI_LOG_INFO(
        "ExtensionManagerInstance delete ExtensionManagerInstance %p", this);
    for (auto& it : m_extensionInstances)
        delete it.second;
    for (auto& it : m_postListeners)
        it->finalize();
    auto it = s_extensionManagerInstances.find(m_context);
    s_extensionManagerInstances.erase(it);
    DEVICEAPI_LOG_INFO("ExtensionManagerInstance %zu => %zu",
                       s_extensionManagerInstances.size() + 1,
                       s_extensionManagerInstances.size());
}

ExtensionManagerInstance* ExtensionManagerInstance::get(ContextRef* context)
{
    std::lock_guard<std::mutex> guard(s_mutex);
    ExtensionManagerInstance* instance = nullptr;

    auto it = s_extensionManagerInstances.find(context);
    if (it != s_extensionManagerInstances.end()) {
        instance = it->second;
    }

    return instance;
}

ExtensionManagerInstance* initialize(ContextRef* context)
{
    DEVICEAPI_LOG_INFO("ExtensionManagerInstance Enter with context %p",
                       context);
    return new ExtensionManagerInstance(context);
}

void close(ContextRef* context)
{
    DEVICEAPI_LOG_INFO("ExtensionManagerInstance Enter with context %p",
                       context);
    delete ExtensionManagerInstance::get(context);
}
}
