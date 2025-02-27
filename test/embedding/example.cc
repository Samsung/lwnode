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
  const char* script = "test/embedding/test-01-message-port-basic.js";
  std::string path = (std::filesystem::current_path() / script).string();
  char* args[] = {const_cast<char*>(""), const_cast<char*>(path.c_str())};

  std::thread worker = std::thread(
      [&](std::promise<void>&& promise) mutable {
        // FIXME: Fix Runtime::Init() call to ensure environment initialization
        // before running the loop, Runtime::Run(). This workaround passes a
        // promise directly to know when that is.
        runtime->Init(COUNT_OF(args), args, std::move(promise));
        runtime->Run();
        runtime->Free();
      },
      std::move(promise));

  init_future.wait();

  int count1 = 0;
  auto port2 = runtime->GetPort();
  port2->OnMessage([&](const MessageEvent* event) {
    std::cout << event->data() << std::endl;
    count1++;
  });
  port2->PostMessage(MessageEvent::New("ping"));

  worker.join();
  return 0;
}
