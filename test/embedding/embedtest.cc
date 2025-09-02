#include "mytest.h"

#include <chrono>
#include <filesystem>
#include <fstream>
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

class OnScopeLeave {
 public:
  // clang-format off
    using Function = std::function<void()>;
    OnScopeLeave(const OnScopeLeave& other) = delete;
    OnScopeLeave& operator=(const OnScopeLeave& other) = delete;
    explicit OnScopeLeave(Function&& function) : function_(std::move(function)) {}
    OnScopeLeave(OnScopeLeave&& other) : function_(std::move(other.function_)) { other.function_ = nullptr; }
    ~OnScopeLeave() { if (function_) function_(); }
    static WARN_UNUSED_RESULT OnScopeLeave create(Function&& function) { return OnScopeLeave(std::move(function)); }
  // clang-format on
 private:
  Function function_;
};

static std::filesystem::path getOldPath(
    const std::filesystem::path& original_path) {
  std::filesystem::path old_path;
  if (original_path.has_extension()) {
    old_path =
        original_path.parent_path() / (original_path.stem().string() + "_old" +
                                       original_path.extension().string());
  } else {
    old_path = original_path.parent_path() /
               (original_path.filename().string() + "_old");
  }
  return old_path;
}

static void RenameToOld(const std::string& file_path) {
  std::filesystem::path original_path(file_path);
  std::filesystem::path old_path = getOldPath(original_path);

  try {
    if (std::filesystem::exists(original_path)) {
      std::filesystem::rename(original_path, old_path);
      std::cout << "Renamed '" << original_path << "' to '" << old_path << "'"
                << std::endl;
    }
  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << "Error renaming to old: " << e.what() << std::endl;
  }
}

static void RenameToOriginal(const std::string& file_path) {
  std::filesystem::path original_path(file_path);
  std::filesystem::path old_path = getOldPath(original_path);

  try {
    if (std::filesystem::exists(old_path)) {
      std::filesystem::rename(old_path, original_path);
      std::cout << "Reverted '" << old_path << "' to '" << original_path << "'"
                << std::endl;
    }
  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << "Error reverting to original: " << e.what() << std::endl;
  }
}

TEST0(Embedtest, Rename_Test) {
  std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
  std::filesystem::path original_temp_file_path =
      temp_dir / "embedtest_rename_test_file.tmp";
  std::filesystem::path old_temp_file_path =
      getOldPath(original_temp_file_path);

  auto _ = OnScopeLeave::create([&]() {
    std::error_code ec;
    std::filesystem::remove(original_temp_file_path, ec);
    std::filesystem::remove(old_temp_file_path, ec);
  });

  // 2. Setup
  std::error_code ec;
  std::filesystem::remove(original_temp_file_path, ec);
  std::filesystem::remove(old_temp_file_path, ec);
  {
    std::ofstream outfile(original_temp_file_path);
  }
  ASSERT_EQ(std::filesystem::exists(original_temp_file_path), true);

  // 3. Test RenameToOld
  RenameToOld(original_temp_file_path.string());
  ASSERT_EQ(!std::filesystem::exists(original_temp_file_path), true);
  ASSERT_EQ(std::filesystem::exists(old_temp_file_path), true);

  // 4. Test RenameToOriginal
  RenameToOriginal(original_temp_file_path.string());
  ASSERT_EQ(std::filesystem::exists(original_temp_file_path), true);
  ASSERT_EQ(!std::filesystem::exists(old_temp_file_path), true);
}

extern std::string getExternalBuiltinsPath();

TEST0(Embedtest, StartWithNoResource) {
  const char* script = "test/embedding/test-22-runtime-heartbeat.js";
  std::string path = (std::filesystem::current_path() / script).string();

  char* args[] = {const_cast<char*>(""),
                  const_cast<char*>("--unhandled-rejections=strict"),
                  const_cast<char*>("--expose-gc"),
                  const_cast<char*>(path.c_str())};

  std::string builtin_path = getExternalBuiltinsPath();

  // 0. Setup
  auto _ = OnScopeLeave::create([&]() { RenameToOriginal(builtin_path); });
  RenameToOriginal(builtin_path);

  // 1. Simulate no lwnode.dat
  RenameToOld(builtin_path);

  {
    auto runtime = std::make_shared<lwnode::Runtime>();

    std::promise<void> promise;
    std::future<void> init_future = promise.get_future();

    int result = -1;

    std::thread worker = std::thread(
        [&](std::promise<void>&& promise) mutable {
          result = runtime->Start(COUNT_OF(args), args, std::move(promise));
        },
        std::move(promise));

    init_future.wait_for(std::chrono::seconds(2));

    ASSERT(result == 100);

    worker.join();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "" << std::endl;
  }

  // 2. Simulate reverting lwnode.dat.
  RenameToOriginal(builtin_path);

  {
    auto runtime = std::make_shared<lwnode::Runtime>();

    std::promise<void> promise;
    std::future<void> init_future = promise.get_future();

    int result = 0;

    std::thread worker = std::thread(
        [&](std::promise<void>&& promise) mutable {
          result = runtime->Start(COUNT_OF(args), args, std::move(promise));
        },
        std::move(promise));

    init_future.wait();

    std::this_thread::sleep_for(std::chrono::seconds(3));
    runtime->Stop();

    worker.join();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT(result == 0);
  }
}

extern void ForceAllowMultipleInstance();

int main(int argc, char* argv[]) {
  ForceAllowMultipleInstance();
  return RUN_ALL_TESTS(argc, argv);
}
