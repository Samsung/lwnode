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

TEST0(Embedtest, MessagePort2_Post_JS_First) {
  int count1 = 0;

  auto runtime = std::make_shared<lwnode::Runtime>();

  // 1. Set OnMessage before starting the runtime
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

  // 2. Start the runtime
  std::promise<void> promise;
  std::future<void> init_future = promise.get_future();
  const char* script = "test/embedding/test-05-message-port-first.js";
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

  // 3. Wait for the entry script to run
  init_future.wait();

  worker.join();

  EXPECT_EQ(count1, 1);
}

TEST0(Embedtest, Restart) {
  TEST_SKIP("This is not yet production-ready.");

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

TEST0(Embedtest, RestartAfterStop) {
  TEST_SKIP("This is not yet production-ready.");

  int count = 0;

  for (int i = 0; i < 3; i++) {
    auto runtime = std::make_shared<lwnode::Runtime>();

    std::promise<void> promise;
    std::future<void> init_future = promise.get_future();
    const char* script = "test/embedding/test-22-runtime-heartbeat.js";
    std::string path = (std::filesystem::current_path() / script).string();

    char* args[] = {const_cast<char*>(""),
                    const_cast<char*>("--unhandled-rejections=strict"),
                    const_cast<char*>("--expose-gc"),
                    const_cast<char*>(path.c_str())};

    std::cout << ++count << "Thread " << std::endl;
    std::thread worker = std::thread(
        [&](std::promise<void>&& promise) mutable {
          std::cout << count << " Start " << std::endl;
          runtime->Start(COUNT_OF(args), args, std::move(promise));
          std::cout << count << " /Start " << std::endl;
        },
        std::move(promise));

    init_future.wait();

    std::this_thread::sleep_for(std::chrono::seconds(3));
    runtime->Stop();

    worker.join();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << count << " /Thread " << std::endl;
  }
}

TEST(Embedtest, MessagePortErrorAfterRegisterOnMessage, 5000) {
  auto runtime = std::make_shared<lwnode::Runtime>();

  std::promise<void> promise;
  std::future<void> init_future = promise.get_future();
  const char* script = "test/embedding/test-04-message-port-error.js";
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
    std::cout << event->data() << std::endl;
    count1++;
  });
  port2->PostMessage(MessageEvent::New("ping"));

  // This test should not exit due to timeout
  worker.join();
}
