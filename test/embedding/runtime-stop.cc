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
  auto runtime = std::make_shared<lwnode::Runtime>();

  const char* script = "test/embedding/test-20-runtime-stop.js";
  std::string path = (std::filesystem::current_path() / script).string();
  char* args[] = {const_cast<char*>(""), const_cast<char*>(path.c_str())};

  std::thread worker = std::thread([&]() mutable {
    // FIXME: Fix Runtime::Init() call to ensure environment initialization
    // before running the loop, Runtime::Run(). This workaround passes a
    // promise directly to know when that is.
    runtime->Start(COUNT_OF(args), args);
  });

  runtime->WaitForReady();

  std::cout << "[C]sleep app" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));
  runtime->Stop();
  std::cout << "[C]terminate app" << std::endl;

  worker.join();
  return 0;
}
