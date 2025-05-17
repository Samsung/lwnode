#include <cassert>
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

void log(const std::string& message) {
  std::cout << "\033[33m" << message << "\033[0m" << std::endl;
}

std::string CallMethod(Port* port, const std::string& function) {
  try {
    auto result = port->PostMessage(MessageEventSync::New(function), 3000);
    assert(result);
    std::string message = result.value_or("error");
    return message;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    assert(false);
  }
  return "";
}

int main(int argc, char* argv[]) {
  auto runtime = std::make_shared<lwnode::Runtime>();

  std::promise<void> promise;
  std::future<void> init_future = promise.get_future();
  const char* script = "test/embedding/test-03-message-port-sync.js";
  std::string path = (std::filesystem::current_path() / script).string();
  char* args[] = {const_cast<char*>(""), const_cast<char*>(path.c_str())};

  std::thread worker = std::thread(
      [&](std::promise<void>&& promise) mutable {
        runtime->Start(COUNT_OF(args), args, std::move(promise));
      },
      std::move(promise));

  init_future.wait();

  auto port2 = runtime->GetPort();
  port2->OnMessage([&](const MessageEvent* event) {
    std::cout << event->data() << std::endl;
  });

  // async test on other thread
  std::thread worker2 = std::thread([&]() mutable {
    for (int i = 0; i < 3; i++) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout << "post message(async)" << std::endl;
      auto result = port2->PostMessage(MessageEvent::New("async"));
      assert(result);
    }
  });

  // sync post message test
  log("sync post message test - 1");
  {
    for (int i = 0; i < 5; i++) {
      auto result = CallMethod(port2.get(), "sync");
      std::cout << "result(" << i << "): " << result << std::endl;
    }
  }

  // simple case test
  log("sync post message test - 2");
  {
    auto result = port2->PostMessage(MessageEventSync::New("sync"));
    assert(result);
    if (result)
      std::cout << "result: " << *result << std::endl;
    else
      std::cout << "result: error(" << result.error() << ")" << std::endl;
  }

  // simple case test
  log("sync post message test - 3");
  {
    auto result = port2->PostMessage(MessageEventSync::New("sync"));
    assert(result);
    std::cout << "result(5): " << result.value_or("error") << std::endl;
  }

  // use same message event instance test
  log("use same message event instance test");
  {
    auto event = MessageEventSync::New("sync");

    auto result = port2->PostMessage(event);
    assert(result);
    std::cout << "result: " << result.value_or("error") << std::endl;

    result = port2->PostMessage(event);
    assert(!result);
    assert(result.error() == Port::Error::InvalidMessageEvent);
    std::cout << "result: " << result.value_or("invalid message error")
              << std::endl;
  }

  // timeout test
  log("timeout test");
  {
    auto result =
        port2->PostMessage(MessageEventSync::New("sync-timeout"), 2000);

    assert(!result);
    assert(result.error() == Port::Error::Timeout);

    std::cout << "result: " << result.value_or("timeout error") << std::endl;
  }

  // timeout test (delayed response)
  log("timeout test (delayed response)");
  {
    auto result =
        port2->PostMessage(MessageEventSync::New("delay-timeout"), 1000);

    assert(!result);
    assert(result.error() == Port::Error::Timeout);

    std::cout << "result: " << result.value_or("timeout error") << std::endl;
  }

  worker2.join();

  // exit test
  {
    port2->PostMessage(MessageEvent::New("exit"));
  }

  worker.join();
  return 0;
}
