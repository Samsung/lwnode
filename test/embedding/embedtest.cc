#define MYTEST_CONFIG_USE_MAIN
#include "mytest.h"

#include <filesystem>
#include <future>
#include <iostream>
#include <thread>
#include <chrono>

#include <lwnode-public.h>
#include <message-port.h>


#define COUNT_OF(array) (sizeof(array) / sizeof((array)[0]))

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
  auto runtime = new lwnode::Runtime();

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
        runtime->Init(COUNT_OF(args), args, std::move(promise));
        runtime->Run();
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
