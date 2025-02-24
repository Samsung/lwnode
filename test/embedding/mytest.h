/*
 * MyTest - Lean, hassle-free testing utility, my way.
 * Copyright 2024-present Daeyeon Jeong (daeyeon.dev@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0.
 * See http://www.apache.org/licenses/LICENSE-2.0 for details.
 */

#pragma once

#include <unistd.h>  // dup, dup2, fileno, close
#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

/*
  Usage:

  int global;

  TEST(TestSuite, SyncTest) {
    ASSERT_EQ(1, global);
  }

  TEST(TestSuite, SyncTestTimeout, 1000) {
    TEST_EXPECT_FAILURE();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_EQ(1, global);
  }

  TEST0(TestSuite, SyncTestOnCurrentThread) {   // No timeout support in TEST0.
    // Runs on the thread executing the test; other tests run on separate ones.
    ASSERT_EQ(1, global);
  }

  TEST_ASYNC(TestSuite, ASyncTest) {
    std::async(std::launch::async, [&done]() {
      ASSERT_EQ(1, global);
      done();      // Call `done()` passed as a parameter when async completes.
    }).get();
  }

  TEST_ASYNC(TestSuite, ASyncTestTimeout, 1000) {
    TEST_EXPECT_FAILURE();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_EQ(1, global);
    done();
  }

  TEST_ASYNC(TestSuite, ASyncTestSkip) {
    TEST_SKIP();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_EQ(1, global);
    done();
  }

  TEST_BEFORE_EACH(TestSuite) {
    std::cout << "Before each TestSuite test" << std::endl;
  }

  TEST_AFTER_EACH(TestSuite) {
    std::cout << "After each TestSuite test" << std::endl;
  }

  TEST_BEFORE(TestSuite) {
    std::cout << "Runs once before all TestSuite tests" << std::endl;
    global = 1;
  }

  TEST_AFTER(TestSuite) {
    std::cout << "Runs once after all TestSuite tests" << std::endl;
  }

  int main(int argc, char* argv[]) {
    return RUN_ALL_TESTS(argc, argv);
  }

  Command Line Usage Examples:
   ./your_test -s                // Silent mode, suppress stdout and stderr
   ./your_test -p "TestSuite"    // Run all tests in 'TestSuite' in the name
   ./your_test -p "-TestSuite"   // Exclude all tests in 'TestSuite' in the name

  Macros:
   TEST_SKIP              : Skip a test during execution.
   TEST_EXPECT_FAILURE    : Mark a test expected to fail (for negative tests).
   TEST_EXCLUDE           : Exclude a test from execution.
   MYTEST_CONFIG_USE_MAIN : Define to use this utility's main function.
*/

class MyTest {
 public:
  static MyTest& Instance() {
    static MyTest instance;
    return instance;
  }

  class TestSkipException : public std::runtime_error {
   public:
    explicit TestSkipException(const std::string& msg = "Expected skipped.")
        : std::runtime_error("   Skipped : " + msg) {}
  };

  class TestTimeoutException : public std::runtime_error {
   public:
    explicit TestTimeoutException(const std::string& msg)
        : std::runtime_error(" Timed out : " + msg) {}
  };

  class TestAssertException : public std::runtime_error {
   public:
    explicit TestAssertException(const std::string& msg)
        : std::runtime_error(msg) {}
  };

  void PrintTestExpect(const std::string& msg) {
    bool prev_silent = silent_;
    if (prev_silent) SilenceOutput(false);
    std::cout << std::endl
              << colors(expect_failure_ ? RESET : RED) << msg << colors(RESET)
              << std::endl;
    if (prev_silent) SilenceOutput(true);
  }

  void SilenceOutput(bool silent) {
    static int stdout_backup = -1;
    static int stderr_backup = -1;
    if (silent) {
      fflush(stdout);
      fflush(stderr);
      stdout_backup = dup(fileno(stdout));
      stderr_backup = dup(fileno(stderr));
      __attribute__((unused)) FILE* fp1 = freopen("/dev/null", "w", stdout);
      __attribute__((unused)) FILE* fp2 = freopen("/dev/null", "w", stderr);
    } else {
      if (stdout_backup != -1) {
        fflush(stdout);
        dup2(stdout_backup, fileno(stdout));
        close(stdout_backup);
        stdout_backup = -1;
      }
      if (stderr_backup != -1) {
        fflush(stderr);
        dup2(stderr_backup, fileno(stderr));
        close(stderr_backup);
        stderr_backup = -1;
      }
      std::cout.flush();
    }
    silent_ = silent;
  }

  // clang-format off
  void RegisterTest(const std::string& group_name, std::function<void()> test) { tests_.emplace_back(group_name, test); }
  void RegisterTestBeforeEach(const std::string& group_name, std::function<void()> test) { test_before_each_[group_name] = test; }
  void RegisterTestAfterEach(const std::string& group_name, std::function<void()> test) { test_after_each_[group_name] = test; }
  void RegisterTestBefore(const std::string& group_name, std::function<void()> test) { test_before_[group_name] = test; }
  void RegisterTestAfter(const std::string& group_name, std::function<void()> test) { test_after_[group_name] = test; }
  void MarkConditionPassed(bool value) { condition_passed_ = value; }
  void MarkExpectFailure(bool value) { expect_failure_ = value; }
  void AddExcludePattern(const std::string& pattern) { exclude_patterns_.emplace_back(pattern); }
  // clang-format on

  bool force() { return force_; }
  bool silent() { return silent_; }
  int timeout() { return timeout_; }
  bool expect_failure() { return expect_failure_; }
  enum colors { RESET, GREEN, RED, YELLOW, NUM_COLORS };
  const char* colors(size_t idx) { return colors_[idx]; }

  int RunAllTests(int argc, char* argv[]) {
    // Disable buffering for stdout and stderr
    setbuf(stdout, nullptr);
    setbuf(stderr, nullptr);

    // Parse CLI arguments
    bool use_color = true;
    bool silent = false;
    std::vector<std::regex> include_patterns;

    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      // clang-format off
      if (arg == "-p" && i + 1 < argc) {
        std::string pattern = argv[++i];
        try {
          (pattern[0] == '-') ? exclude_patterns_.emplace_back(pattern.substr(1)) : include_patterns.emplace_back(pattern);
        } catch (const std::exception& e) {
          std::cerr << e.what() << std::endl;
          return 1;
        }
      } else if (arg == "-t" && i + 1 < argc) { timeout_ = std::stoi(argv[++i]);
      } else if (arg == "-c") { use_color = false;
      } else if (arg == "-s") { silent = true;
      } else if (arg == "-f") { force_ = true;
      } else if (arg == "-h" || arg == "--help") { return PrintUsage(argv[0]); }
      // clang-format on
    }

    auto should_run = [this, &include_patterns](const std::string& name) {
      for (const auto& pattern : exclude_patterns_) {
        if (std::regex_search(name, pattern)) return false;
      }
      if (include_patterns.empty()) return true;
      for (const auto& pattern : include_patterns) {
        if (std::regex_search(name, pattern)) return true;
      }
      return false;
    };

    // Run tests

    int num_success = 0, num_failure = 0, num_skipped = 0;
    int num_ran_tests = 0, num_filtered_tests = 0;

    colors_ = {use_color ? "\033[0m" : "",
               use_color ? "\033[32m" : "",
               use_color ? "\033[31m" : "",
               use_color ? "\033[33m" : ""};
    const char** colors = colors_.data();

    // Categorize and filter tests by name in alphabetical and numerical order.
    std::map<std::string, std::vector<TestPair>> categorized_tests;
    for (const auto& test_pair : tests_) {
      const std::string& name = test_pair.first;
      if (!should_run(name)) continue;

      num_filtered_tests++;
      auto group_name = name.substr(0, name.find(':'));
      categorized_tests[group_name].push_back(test_pair);
    }

    // clang-format off
                         printf("%s[==========]%s Running %d test case(s).\n", colors[GREEN], colors[RESET], num_filtered_tests);
    auto PrintStart = [&colors](const std::string& name) {
                         printf("%s[ RUN      ]%s %s\n", colors[GREEN], colors[RESET], name.c_str());
    };
    auto PrintEnd = [&colors](const bool failure, const bool skipped, const std::string& name) {
      if (failure)       printf("%s[  FAILED  ]%s %s\n", colors[RED], colors[RESET], name.c_str());
      else if (skipped)  printf("%s[  SKIPPED ]%s %s\n", colors[YELLOW], colors[RESET], name.c_str());
      else               printf("%s[       OK ]%s %s\n", colors[GREEN], colors[RESET], name.c_str());
    };
    auto PrintResult = [&]() {
                         printf("%s[==========]%s %d test case(s) ran.\n", colors[GREEN], colors[RESET], num_ran_tests);
                         printf("%s[  PASSED  ]%s %d test(s)\n", colors[GREEN], colors[RESET], num_ran_tests - num_failure - num_skipped);
    (num_skipped > 0) && printf("%s[  SKIPPED ]%s %d test(s)\n", colors[YELLOW], colors[RESET], num_skipped);
    (num_failure > 0) && printf("%s[  FAILED  ]%s %d test(s)\n", colors[RED], colors[RESET], num_failure);
    };
    // clang-format on

    auto RunTest = [this, &colors, &silent](const std::string& name,
                                            const TestFunction& test,
                                            const std::string group_name =
                                                "") -> std::tuple<bool, bool> {
      bool failure = false, skipped = false;
      condition_passed_ = true;
      expect_failure_ = false;

      try {
        auto _ = OnScopeLeave::create([&, this]() {
          if (!group_name.empty() && test_after_each_.count(group_name)) {
            test_after_each_[group_name]();
          }
          if (!condition_passed_) {
            failure = true;
          }
          SilenceOutput(silent && false);
        });
        SilenceOutput(silent && true);

        if (!group_name.empty() && test_before_each_.count(group_name)) {
          test_before_each_[group_name]();
        }

        std::visit([](auto&& test) { test(); }, test);

      } catch (const TestSkipException& e) {
        skipped = true;
        printf("\n%s\n", e.what());
      } catch (const TestAssertException& e) {
        failure = true;
        auto color = colors[expect_failure_ ? RESET : RED];
        printf("\n%s%s%s\n", color, e.what(), colors[RESET]);
      } catch (const TestTimeoutException& e) {
        failure = true;
        printf("\n%s\n", e.what());
      } catch (const std::exception& e) {
        printf("\nException : %s\n", e.what());
        failure = true;
      } catch (...) {
        printf("\nException : Unknown\n");
        failure = true;
      }

      if (expect_failure_) {
        failure = !failure;
        if (failure) {
          printf("    Failed : Expected fail but passed.\n");
        } else {
          printf("    Passed : Expected fail and failed.\n");
        }
      }

      return std::make_tuple(failure, skipped);
    };

    for (const auto& category : categorized_tests) {
      const std::string& group_name = category.first;
      const std::vector<TestPair>& group_tests = category.second;
      bool group_failure = false;
      bool group_skipped = false;

      PrintStart(group_name);

      if (test_before_.count(group_name)) {
        auto [failure, skipped] = RunTest(group_name, test_before_[group_name]);
        if (skipped) {
          group_skipped = true;
          continue;
        }
        group_failure = group_failure || failure;
      }

      for (const auto& group_test : group_tests) {
        const std::string& name = group_test.first;
        const TestFunction& test = group_test.second;

        PrintStart(name);

        auto [failure, skipped] = RunTest(name, test, group_name);
        (failure ? ++num_failure : (skipped ? ++num_skipped : ++num_success));
        ++num_ran_tests;

        if (failure && !expect_failure_) printf("\n");
        PrintEnd(failure, skipped, name);
      }

      if (test_after_.count(group_name)) {
        auto [failure, skipped] = RunTest(group_name, test_after_[group_name]);
        group_failure = group_failure || failure;
      }

      PrintEnd(group_failure || num_failure > 0, group_skipped, group_name);
    }

    PrintResult();
    return num_failure > 0 ? 1 : 0;
  }

 private:
  static constexpr int kDefaultTimeoutMS = 60000;
  static constexpr const char* kCalVersion = "25.02.16";

#if !defined(WARN_UNUSED_RESULT) && defined(__GNUC__)
#define WARN_UNUSED_RESULT __attribute__((__warn_unused_result__))
#else
#define WARN_UNUSED_RESULT
#endif

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

  int PrintUsage(const char* name) {
    // clang-format off
    std::cout
      << "Usage: " << name << " [options]\n"
      << "Options:\n"
      << "  -p \"PATTERN\"  : Include tests matching PATTERN\n"
      << "  -p \"-PATTERN\" : Exclude tests matching PATTERN\n"
      << "  -t TIMEOUT    : Set the timeout value in milliseconds (default: " << timeout_ << ")\n"
      << "  -c            : Disable color output\n"
      << "  -f            : Force mode, run all tests, including skipped ones\n"
      << "  -s            : Silent mode (suppress stdout and stderr output)\n"
      << "  -h, --help    : Show this help message\n\n"
      << "Tests executed by the integrated testing utility, MyTest (v" << kCalVersion << ")\n";
    // clang-format on
    return 0;
  }

  int timeout_ = kDefaultTimeoutMS;
  bool force_ = false;
  bool silent_ = false;
  bool condition_passed_ = true;
  bool expect_failure_ = false;
  std::vector<const char*> colors_;
  std::vector<std::regex> exclude_patterns_;

  using TestFunction =
      std::variant<std::function<void()>, std::function<std::future<void>()>>;
  using TestPair = std::pair<std::string, TestFunction>;
  std::vector<TestPair> tests_;
  std::unordered_map<std::string, std::function<void()>> test_before_each_;
  std::unordered_map<std::string, std::function<void()>> test_after_each_;
  std::unordered_map<std::string, std::function<void()>> test_before_;
  std::unordered_map<std::string, std::function<void()>> test_after_;
  MyTest() {}
  MyTest(const MyTest&) = delete;
  MyTest& operator=(const MyTest&) = delete;
};

#define TEST0(group, name)                                                     \
  void group##name##_impl();                                                   \
  struct group##name##_Register {                                              \
    group##name##_Register() {                                                 \
      MyTest::Instance().RegisterTest(#group ":" #name, group##name##_impl);   \
    }                                                                          \
  } group##name##_register;                                                    \
  void group##name##_impl()

#define TEST_(is_sync, group, name, ...)                                       \
  void group##name##_impl(std::function<void()> done);                         \
  void group##name(int timeout_ms = MyTest::Instance().timeout()) {            \
    auto promise = std::make_shared<std::promise<void>>();                     \
    auto future = promise->get_future();                                       \
    auto done = [promise, &future]() {                                         \
      if (future.valid() && future.wait_for(std::chrono::seconds(0)) ==        \
                                std::future_status::timeout)                   \
        promise->set_value();                                                  \
    };                                                                         \
    std::thread([done, promise]() {                                            \
      try {                                                                    \
        group##name##_impl(done);                                              \
        if (is_sync) done();                                                   \
      } catch (...) {                                                          \
        promise->set_exception(std::current_exception());                      \
      }                                                                        \
    }).detach();                                                               \
    if (future.wait_for(std::chrono::milliseconds(timeout_ms)) ==              \
        std::future_status::timeout) {                                         \
      throw MyTest::TestTimeoutException(#group ":" #name);                    \
    }                                                                          \
    future.get();                                                              \
  }                                                                            \
  struct group##name##_Register {                                              \
    group##name##_Register() {                                                 \
      MyTest::Instance().RegisterTest(                                         \
          #group ":" #name, []() { return group##name(__VA_ARGS__); });        \
    }                                                                          \
  } group##name##_register;

#define TEST(group, name, ...)                                                 \
  TEST_(true, group, name, __VA_ARGS__)                                        \
  void group##name##_impl(std::function<void()>)

#define TEST_ASYNC(group, name, ...)                                           \
  TEST_(false, group, name, __VA_ARGS__)                                       \
  void group##name##_impl(std::function<void()> done)

#define TEST_BEFORE_EACH(group)                                                \
  void group##_BeforeEach();                                                   \
  struct group##_BeforeEach_Register {                                         \
    group##_BeforeEach_Register() {                                            \
      MyTest::Instance().RegisterTestBeforeEach(#group, group##_BeforeEach);   \
    }                                                                          \
  } group##_BeforeEach_register;                                               \
  void group##_BeforeEach()

#define TEST_AFTER_EACH(group)                                                 \
  void group##_AfterEach();                                                    \
  struct group##_AfterEach_Register {                                          \
    group##_AfterEach_Register() {                                             \
      MyTest::Instance().RegisterTestAfterEach(#group, group##_AfterEach);     \
    }                                                                          \
  } group##_AfterEach_register;                                                \
  void group##_AfterEach()

#define TEST_BEFORE(group)                                                     \
  void group##_Before();                                                       \
  struct group##_Before_Register {                                             \
    group##_Before_Register() {                                                \
      MyTest::Instance().RegisterTestBefore(#group, group##_Before);           \
    }                                                                          \
  } group##_Before_register;                                                   \
  void group##_Before()

#define TEST_AFTER(group)                                                      \
  void group##_After();                                                        \
  struct group##_After_Register {                                              \
    group##_After_Register() {                                                 \
      MyTest::Instance().RegisterTestAfter(#group, group##_After);             \
    }                                                                          \
  } group##_After_register;                                                    \
  void group##_After()

#define TEST_SKIP(msg)                                                         \
  do {                                                                         \
    if (!MyTest::Instance().force()) {                                         \
      throw MyTest::TestSkipException(msg);                                    \
    }                                                                          \
  } while (0)

#define TEST_EXPECT_FAILURE(msg)                                               \
  do {                                                                         \
    MyTest::Instance().MarkExpectFailure(true);                                \
  } while (0)

#define _VA(_1, _2, NAME, ...) NAME
#define _TEST_EXCLUDE_ARG1(group) _TEST_EXCLUDE_ARG2(group, EMPTYSTR)
#define _TEST_EXCLUDE_ARG2(group_or_pattern, name)                             \
  struct group_or_pattern##_##name##_Add_Exclude_Pattern {                     \
    group_or_pattern##_##name##_Add_Exclude_Pattern() {                        \
      std::string pattern = #group_or_pattern;                                 \
      if (std::string(#name) != "EMPTYSTR") pattern += ":" #name;              \
      MyTest::Instance().AddExcludePattern(pattern);                           \
    }                                                                          \
  } group_or_pattern##_##name##_add_exclude_pattern;
#define TEST_EXCLUDE(...)                                                      \
  _VA(__VA_ARGS__, _TEST_EXCLUDE_ARG2, _TEST_EXCLUDE_ARG1)(__VA_ARGS__)

#define RUN_ALL_TESTS(argc, argv) MyTest::Instance().RunAllTests(argc, argv)

#ifdef MYTEST_CONFIG_USE_MAIN
int main(int argc, char* argv[]) {
  return RUN_ALL_TESTS(argc, argv);
}
#endif

#ifndef FILE_NAME_
#define FILE_NAME_                                                             \
  std::string((strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__))
#endif

#define LOC_ "(" + FILE_NAME_ + ":" + std::to_string(__LINE__) + ")"

#define FORMAT_ERROR_MESSAGE_(x, cond_str, y, msg)                             \
  std::stringstream __ss;                                                      \
  __ss << msg << std::string(" ") + LOC_ << "\n";                              \
  __ss << "  Expected : (" << #x << " " cond_str " " << #y << ")\n";           \
  __ss << "    Actual : (" << x << " " cond_str " " << y << ")";

#define CHECK_CONDITION_(cond, x, y, cond_str, msg, should_throw)              \
  do {                                                                         \
    if (cond) {                                                                \
      FORMAT_ERROR_MESSAGE_(x, y, cond_str, msg);                              \
      if (should_throw) {                                                      \
        throw MyTest::TestAssertException(__ss.str());                         \
      } else {                                                                 \
        MyTest::Instance().PrintTestExpect(__ss.str());                        \
        MyTest::Instance().MarkConditionPassed(false);                         \
      }                                                                        \
    }                                                                          \
  } while (0)

#define EXPECT_(cond, x, c, y, m) CHECK_CONDITION_(!(cond), x, #c, y, m, false)
#define ASSERT_(cond, x, c, y, m) CHECK_CONDITION_(!(cond), x, #c, y, m, true)
#define EXPECT_EQ(x, y) EXPECT_((x) == (y), x, ==, y, "EXPECT_EQ failed")
#define EXPECT_NE(x, y) EXPECT_((x) != (y), x, !=, y, "EXPECT_NE failed")
#define ASSERT_EQ(x, y) ASSERT_((x) == (y), x, ==, y, "ASSERT_EQ failed")
#define ASSERT_NE(x, y) ASSERT_((x) != (y), x, !=, y, "ASSERT_NE failed")

#define EXPECT(cond) EXPECT_EQ(static_cast<bool>(cond), true)
#define ASSERT(cond) ASSERT_EQ(static_cast<bool>(cond), true)
