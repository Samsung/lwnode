#define MYTEST_CONFIG_USE_MAIN
#include "mytest.h"

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

std::string getTimestamp() {
  using namespace std::chrono;

  auto now = system_clock::now();
  auto timeT = system_clock::to_time_t(now);
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  std::ostringstream oss;
  oss << std::put_time(std::gmtime(&timeT), "%M:%S") << '.' << std::setw(3)
      << std::setfill('0') << ms.count();

  return oss.str();
}

TEST0(Embedtest, MessagePort2_Post_Many_JS_First) {
  auto runtime = std::make_shared<lwnode::Runtime>();

  std::promise<void> promise;
  std::future<void> init_future = promise.get_future();
  const char* script = "test/embedding/test-02-message-port-many.js";
  std::string path = (std::filesystem::current_path() / script).string();

  const bool post_first = true;
  char* args[] = {const_cast<char*>(""),
                  const_cast<char*>(path.c_str()),
                  const_cast<char*>(std::to_string(post_first).c_str())};

  std::thread worker = std::thread(
      [&](std::promise<void>&& promise) mutable {
        runtime->Start(COUNT_OF(args), args, std::move(promise));
      },
      std::move(promise));

  init_future.wait();

  int count1 = 0;

  auto port2 = runtime->GetPort();

  port2->OnMessage([&](const MessageEvent* event) {
    count1++;
    if (event->data() == "ping") {
      auto extra = std::to_string(count1);
      std::cout << getTimestamp() << " NS pong " + extra << std::endl;
      port2->PostMessage(MessageEvent::New("pong " + extra));
    } else {
      std::cout << getTimestamp() << " NS ping" << std::endl;
      port2->PostMessage(MessageEvent::New("ping"));
    }
  });

  if (post_first == 0) {
    std::cout << getTimestamp() << " NS ping" << std::endl;
    port2->PostMessage(MessageEvent::New("ping"));
  }

  worker.join();

  EXPECT_EQ(count1, 10);
}

TEST0(Embedtest, Restart) {
  int count = 0;
  for (int i = 0; i < 3; i++) {
    auto runtime = std::make_shared<lwnode::Runtime>();

    std::promise<void> promise;
    std::future<void> init_future = promise.get_future();
    const char* script = "test/embedding/test-21-runtime-hello.js";
    std::string path = (std::filesystem::current_path() / script).string();

    char* args[] = {const_cast<char*>(""), const_cast<char*>(path.c_str())};

    std::thread worker = std::thread(
        [&](std::promise<void>&& promise) mutable {
          std::cout << ++count << " Start " << std::endl;
          runtime->Start(COUNT_OF(args), args, std::move(promise));
          std::cout << count << " /Start " << std::endl;
        },
        std::move(promise));

    init_future.wait();
    worker.join();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  EXPECT_EQ(count, 3);
}
