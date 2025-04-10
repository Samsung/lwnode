#include <cassert>
#include <chrono>
#include <filesystem>
#include <future>
#include <iostream>
#include <thread>

#include <lwnode-public.h>
#include <message-port.h>

#define COUNT_OF(array) (sizeof(array) / sizeof((array)[0]))

int main(int argc, char* argv[]) {
  auto runtime = std::make_shared<lwnode::Runtime>();

  std::promise<void> promise;
  std::future<void> init_future = promise.get_future();
  const char* script = "test/embedding/test-03-message-port-sync.js";
  std::string path = (std::filesystem::current_path() / script).string();
  char* args[] = {const_cast<char*>(""), const_cast<char*>(path.c_str())};

  std::thread worker = std::thread(
      [&](std::promise<void>&& promise) mutable {
        // FIXME: Fix Runtime::Init() call to ensure environment initialization
        // before running the loop, Runtime::Run(). This workaround passes a
        // promise directly to know when that is.
        runtime->Start(COUNT_OF(args), args, std::move(promise));
      },
      std::move(promise));

  init_future.wait();

  auto port2 = runtime->GetPort();
  port2->OnMessage([&](const MessageEvent* event) {
    std::cout << event->data() << std::endl;
  });

  // sync post message test
  {
    for (int i = 0; i < 5; i++) {
      auto result = port2->PostMessage(MessageEventSync::New("sync"));
      if (result)
        std::cout << "result(" << i << "): " << *result << std::endl;
      else
        std::cout << "result(" << i << "): error(" << result.error() << ")"
                  << std::endl;
    }

    auto result = port2->PostMessage(MessageEventSync::New("sync"));
    std::cout << "result(5): " << result.value_or("error") << std::endl;
  }

  // use same message event instance test
  {
    auto event = MessageEventSync::New("sync");

    auto result = port2->PostMessage(event);
    assert(result);
    std::cout << "result: " << result.value_or("error") << std::endl;

    result = port2->PostMessage(event);
    assert(!result);
    assert(result.error() == Port::Error::InvalidMessageEvent);
    std::cout << "result: " << result.value_or("invalid message error") << std::endl;
  }

  // timeout test
  {
    auto result =
        port2->PostMessage(MessageEventSync::New("sync-timeout"), 2000);

    assert(!result);
    assert(result.error() == Port::Error::Timeout);

    std::cout << "result: " << result.value_or("timeout error") << std::endl;
  }

  // async post message test
  {
    port2->PostMessage(MessageEvent::New("exit"));
  }

  worker.join();
  return 0;
}
