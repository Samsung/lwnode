#include <dlog.h>
#include <cstdlib>
#include <future>
#include <thread>

#include "lwnode-public.h"

#define APP_LOG_TAG "lwnode"

int main(int argc, char* argv[]) {
  dlog_print(
      DLOG_INFO, APP_LOG_TAG, "[Native] message-port example app started...");

  std::promise<void> promise;
  std::future<void> init_future = promise.get_future();

  auto runtime = std::make_shared<lwnode::Runtime>();

  std::thread t([&]() {
    dlog_print(DLOG_INFO, APP_LOG_TAG, "[Native] start thread");

    lwnode::SetDlogID(APP_LOG_TAG);  // set dlog tag for lwnode

    std::string path = "/opt/usr/apps/lwnode-example-messageport/res/";

    if (!lwnode::InitScriptRootPath(path)) {
      dlog_print(DLOG_ERROR,
                 APP_LOG_TAG,
                 "[Native] fail to set script root path: %s",
                 path.c_str());

      return;
    }

    char* args[] = {
        const_cast<char*>(""), const_cast<char*>("index.js"), nullptr};

    dlog_print(DLOG_INFO, APP_LOG_TAG, "[Native] runtime.Start() called...");
    runtime->Start(2, args, std::move(promise));

    dlog_print(DLOG_INFO, APP_LOG_TAG, "[Native] end thread");
  });

  // Wait until the js script is initialized to get the port.
  init_future.wait();

  std::shared_ptr<Port> port = runtime->GetPort();

  // Register a callback function to receive messages from the js script.
  port->OnMessage([](const MessageEvent* event) {
    dlog_print(DLOG_INFO,
               APP_LOG_TAG,
               "[Native] received message from js: %s",
               event->data().c_str());
  });

  // Send a message to the js script. The main script will send back a
  // message.
  port->PostMessage(MessageEvent::New("ping"));

  t.join();

  return 0;
}
