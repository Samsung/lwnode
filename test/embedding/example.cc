#include <chrono>
#include <filesystem>
#include <future>
#include <iostream>
#include <thread>

#include <lwnode-public.h>
#include <message-port.h>

template <typename T, size_t N>
constexpr size_t COUNT_OF(T (&)[N]) noexcept {
  return N;
}

int main(int argc, char* argv[]) {
  lwnode::Runtime::Configuration configuration;
  if (!configuration.Set("gc_interval", 10000)) {
    std::cerr << "Failed to set gc_interval" << std::endl;
  }
  auto runtime = std::make_shared<lwnode::Runtime>(std::move(configuration));

  const char* script = "test/embedding/test-01-message-port-basic.js";
  std::string path = (std::filesystem::current_path() / script).string();
  char* args[] = {const_cast<char*>(""), const_cast<char*>(path.c_str())};

  std::thread worker = std::thread([&]() mutable {
    // FIXME: Fix Runtime::Init() call to ensure environment initialization
    // before running the loop, Runtime::Run(). This workaround passes a
    // promise directly to know when that is.
    int result = 0;
    do {
      std::cout << "start runtime" << std::endl;
      result = runtime->Start(COUNT_OF(args), args);
      std::cout << "result: " << result << std::endl;

      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    } while (result == 100);
  });

  while (true) {
    std::future_status status = runtime->WaitForReady(50);
    if (status == std::future_status::timeout) {
      std::cout << "runtime is not ready: timeout" << std::endl;
    } else if (status == std::future_status::deferred) {
      std::cout << "runtime is not ready: deferred" << std::endl;
    } else if (status == std::future_status::ready) {
      std::cout << "runtime is ready" << std::endl;
      break;
    }
  }

  int count1 = 0;
  auto port2 = runtime->GetPort();
  std::cout << "done get port" << std::endl;

  port2->OnMessage([&](const MessageEvent* event) {
    std::cout << event->data() << std::endl;
    count1++;
  });
  port2->PostMessage(MessageEvent::New("ping"));

  worker.join();
  return 0;
}
